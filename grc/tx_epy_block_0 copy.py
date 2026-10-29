"""
Video packetizer (Embedded Python Block)

PDU (UDP payload z ffmpeg) na vstupe -> SUVISLY stream bajtov na vystupe.
Ked nie su ziadne video data, blok posiela pseudonahodne vyplnove bajty
(nie ramec). Prijimac ich preskoci, lebo v nich nenajde sync word.
Format ramca je nezmeneny: SYNC(4) | LEN(2) | PAYLOAD | CRC16-CCITT-FALSE(2)
"""

import threading
import time

import numpy as np
from gnuradio import gr
import pmt
import binascii

SYNC_WORD = (0x1ACFFC1D).to_bytes(4, "big")


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

        crc = binascii.crc_hqx(data, 0xFFFF)    # CRC-16/CCITT-FALSE
        frame = SYNC_WORD + len(data).to_bytes(2, "big") + data + crc.to_bytes(2, "big")

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
