"""
Video packetizer (Embedded Python Block)

PDU (UDP payload z ffmpeg) na vstupe -> SUVISLY stream bajtov na vystupe.
Ked nie su ziadne video data, blok posiela pseudonahodne vyplnove bajty
(nie ramec). Prijimac ich preskoci, lebo v nich nenajde sync word.

Format ramca:
    SYNC(4) | [LEN(2) | PAYLOAD | CRC16-CCITT-FALSE(2)] XOR CCSDS PRBS
CRC sa pocita nad nescramblovanym payloadom. Scrambluje sa vsetko za sync
slovom od indexu 0 (PRBS perioda 255 B). Sync slovo ostava nescramblovane.
"""

import threading
import time

import numpy as np
from gnuradio import gr
import pmt
import binascii

SYNC_WORD = (0x1ACFFC1D).to_bytes(4, "big")


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
        self._data_bytes = 0
        self._fill_bytes = 0
        self._max_seen_backlog = 0
        self._last_stats = time.monotonic()

    # ------------------------------------------------------------------ vstup
    def create_frame(self, msg):
        data = np.asarray(pmt.u8vector_elements(pmt.cdr(msg)), dtype=np.uint8).tobytes()
        if not data:
            return

        crc = binascii.crc_hqx(data, 0xFFFF)    # CRC-16/CCITT-FALSE, nad cistymi datami

        # LEN | PAYLOAD | CRC -> scramble od indexu 0 (np.resize = table[i % 255])
        body = np.frombuffer(len(data).to_bytes(2, "big") + data + crc.to_bytes(2, "big"),
                             dtype=np.uint8)
        frame = SYNC_WORD + (body ^ np.resize(PRBS, len(body))).tobytes()

        with self._lock:
            # Ochrana pred neobmedzenym rastom latencie: radsej zahodit cely ramec
            if len(self._pending) + len(frame) > self.max_backlog:
                self._dropped += 1
                return
            self._pending += frame
            self._frames += 1
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
              f"data {8 * self._data_bytes / dt / 1e3:7.1f} kbit/s  "
              f"vytazenie {load:5.1f} %  "
              f"backlog {len(self._pending)} B (max {self._max_seen_backlog} B)  "
              f"zahodene {self._dropped}", flush=True)
        self._frames = self._data_bytes = self._fill_bytes = 0
        self._max_seen_backlog = len(self._pending)
        self._dropped = 0
        self._last_stats = now
