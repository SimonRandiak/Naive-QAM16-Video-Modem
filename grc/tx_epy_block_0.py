"""
Video packetizer (Embedded Python Block)

PDU (UDP payload z ffmpeg) na vstupe -> SUVISLY stream bajtov na vystupe.
Ked nie su ziadne video data, blok posiela pseudonahodne vyplnove bajty
(nie ramec). Prijimac ich preskoci, lebo v nich nenajde sync word.

Format ramca (s FEC, konvolucny kod K=7 R=1/2, polynomy 171/133):
    SYNC(4)
    CONV( [LEN(2)] XOR PRBS[0..1]                 | 0x00 )   -> 6 B
    CONV( [PAYLOAD | CRC16(2)] XOR PRBS[2..]      | 0x00 )   -> 2*(LEN+3) B

CRC sa pocita nad nescramblovanym payloadom. Scrambluje sa PRED kodovanim,
s rovnakymi indexmi ako bez FEC (LEN od 0, payload od 2). Tail bajt 0x00 sa
NEscrambluje, aby encoder skoncil v stave 0. Sync slovo ostava nekodovane a
nescramblovane.

Pozor: FEC znizuje uzitocnu rychlost na polovicu (pri 300 kBd QAM16 asi
595 kbit/s pre payload) -> bitrate videa v ffmpeg nastav s rezervou.
"""

import threading
import time

import numpy as np
from gnuradio import gr
import pmt
import binascii

SYNC_WORD = (0x1ACFFC1D).to_bytes(4, "big")
MAX_LENGTH = 1800           # musi sediet s FrameSynchronizer::max_length v prijimaci


def ccsds_prbs_table():
    """Ta ista LFSR ako ccsdsLfsr v C++: posun doprava, maska 0xA9, vystup bit 0, MSB first."""
    state, taps = 0xFF, 0xA9
    table = np.zeros(255, dtype=np.uint8)
    for i in range(255):
        byte = 0
        for _ in range(8):
            out = state & 1
            fb = bin(state & taps).count("1") & 1      # parita = XOR tapov
            state = (state >> 1) | (fb << 7)
            byte = (byte << 1) | out
        table[i] = byte
    return table


PRBS = ccsds_prbs_table()
assert PRBS[:8].tobytes() == bytes.fromhex("FF480EC09A0D70BC"), "zla PRBS tabulka"


def scramble(data, offset):
    """XOR s PRBS od indexu offset (perioda 255 B)."""
    buf = np.frombuffer(data, dtype=np.uint8)
    return (buf ^ PRBS[(offset + np.arange(len(buf))) % 255]).tobytes()


# ------------------------------------------------------- konvolucny encoder
# Rovnaky ako K7Encoder v C++: stav = poslednych 6 bitov (najnovsi na bite 0),
# polynomy otocene (171 -> 0x4F, 133 -> 0x6D), na kazdy bit vyjde par (G1, G2).
G1 = 0x4F
G2 = 0x6D


def _parity(x):
    return bin(x).count("1") & 1


def conv_byte_table():
    """Pre kazdy (stav, vstupny bajt) predpocita 16 kodovanych bitov a novy stav.
    Encoder potom robi jeden lookup na bajt namiesto 8 krokov v Pythone."""
    out_tab = [[0] * 256 for _ in range(64)]
    next_tab = [[0] * 256 for _ in range(64)]
    for s0 in range(64):
        for byte in range(256):
            s, coded = s0, 0
            for i in range(7, -1, -1):              # MSB first
                reg = (s << 1) | ((byte >> i) & 1)
                coded = (coded << 2) | (_parity(reg & G1) << 1) | _parity(reg & G2)
                s = reg & 0x3F
            out_tab[s0][byte] = coded
            next_tab[s0][byte] = s
    return out_tab, next_tab


CONV_OUT, CONV_NEXT = conv_byte_table()


def conv_encode(data):
    """Bajty -> kodovane bajty (2x dlhsie). Encoder zacina v stave 0.
    Na ukoncenie bloku musi byt posledny vstupny bajt 0x00."""
    out = bytearray(2 * len(data))
    s = 0
    for i, byte in enumerate(data):
        coded = CONV_OUT[s][byte]
        out[2 * i] = coded >> 8
        out[2 * i + 1] = coded & 0xFF
        s = CONV_NEXT[s][byte]
    return bytes(out)


# impulzna odozva: 1 a nuly -> 11 10 11 11 00 01 11 00 ...
assert conv_encode(b"\x80\x00")[:2] == bytes([0b11101111, 0b00011100]), "zly konvolucny encoder"


def build_frame(data):
    """Cely ramec s FEC pre jeden payload."""
    crc = binascii.crc_hqx(data, 0xFFFF)        # CRC-16/CCITT-FALSE, nad cistymi datami
    header = scramble(len(data).to_bytes(2, "big"), 0) + b"\x00"
    body = scramble(data + crc.to_bytes(2, "big"), 2) + b"\x00"
    return SYNC_WORD + conv_encode(header) + conv_encode(body)


class blk(gr.sync_block):
    """Video packetizer: msg_in (PDU) -> continuous uint8 stream (packed bytes)"""

    def __init__(self, max_backlog=65536, fill_chunk=256, stats_period=2.0):
        gr.sync_block.__init__(
            self,
            name='Video packetizer',
            in_sig=None,            # ziadny stream vstup -> work() riadi len miesto vo vystupe
            out_sig=[np.uint8]      # packed bajty pre Constellation Modulator
        )

        self.message_port_register_in(pmt.intern('msg_in'))
        self.set_msg_handler(pmt.intern('msg_in'), self.create_frame)

        self.max_backlog = int(max_backlog)     # max bajtov cakajucich vo fronte
        self.fill_chunk = int(fill_chunk)       # max vyplne na jedno volanie work()
        self.stats_period = float(stats_period)

        self._pending = bytearray()             # fronta hotovych ramcov
        self._lock = threading.Lock()

        # Predgenerovana pseudonahodna vypln, cita sa cyklicky
        rng = np.random.default_rng(0x1ACF)
        self._filler = rng.integers(0, 256, 1 << 16, dtype=np.uint8)
        self._fill_pos = 0

        # Statistiky
        self._frames = 0
        self._dropped = 0
        self._too_long = 0
        self._payload_bytes = 0
        self._data_bytes = 0
        self._fill_bytes = 0
        self._max_seen_backlog = 0
        self._last_stats = time.monotonic()

    # ------------------------------------------------------------------ vstup
    def create_frame(self, msg):
        data = np.asarray(pmt.u8vector_elements(pmt.cdr(msg)), dtype=np.uint8).tobytes()
        if not data:
            return

        # Prijimac zahodi ramce s LEN > MAX_LENGTH, nema zmysel ich vysielat
        if len(data) > MAX_LENGTH:
            with self._lock:
                self._too_long += 1
            return

        frame = build_frame(data)

        with self._lock:
            # Ochrana pred neobmedzenym rastom latencie: radsej zahodit cely ramec
            if len(self._pending) + len(frame) > self.max_backlog:
                self._dropped += 1
                return
            self._pending += frame
            self._frames += 1
            self._payload_bytes += len(data)
            self._max_seen_backlog = max(self._max_seen_backlog, len(self._pending))

    # ----------------------------------------------------------------- vystup
    def work(self, input_items, output_items):
        out = output_items[0]
        n = len(out)

        with self._lock:
            k = min(n, len(self._pending))
            if k:
                # slice bytearray vytvori kopiu, takze del nizsie je bezpecny
                out[:k] = np.frombuffer(self._pending[:k], dtype=np.uint8)
                del self._pending[:k]
                self._data_bytes += k
                self._maybe_print_stats()
                return k

        # Fronta je prazdna -> vypln (nikdy nie uprostred ramca)
        k = min(n, self.fill_chunk)
        idx = (self._fill_pos + np.arange(k)) % len(self._filler)
        out[:k] = self._filler[idx]
        self._fill_pos = (self._fill_pos + k) % len(self._filler)
        self._fill_bytes += k
        with self._lock:
            self._maybe_print_stats()
        return k

    # ------------------------------------------------------------- diagnostika
    def _maybe_print_stats(self):
        now = time.monotonic()
        dt = now - self._last_stats
        if self.stats_period <= 0 or dt < self.stats_period:
            return
        total = self._data_bytes + self._fill_bytes
        load = 100.0 * self._data_bytes / total if total else 0.0
        print(f"[packetizer] {self._frames / dt:6.1f} frames/s  "
              f"video {8 * self._payload_bytes / dt / 1e3:7.1f} kbit/s  "
              f"kodovane {8 * self._data_bytes / dt / 1e3:7.1f} kbit/s  "
              f"vytazenie {load:5.1f} %  "
              f"backlog {len(self._pending)} B (max {self._max_seen_backlog} B)  "
              f"zahodene {self._dropped}  prilis dlhe {self._too_long}", flush=True)
        self._frames = self._payload_bytes = self._data_bytes = self._fill_bytes = 0
        self._max_seen_backlog = len(self._pending)
        self._dropped = 0
        self._too_long = 0
        self._last_stats = now
