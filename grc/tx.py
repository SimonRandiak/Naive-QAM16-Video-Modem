#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: QAM16 video link Transmitter
# Author: Simon Randiak
# GNU Radio version: 3.10.12.0

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import blocks
from gnuradio import digital
from gnuradio import gr, pdu
from gnuradio import network
from gnuradio.filter import firdes
import threading
import tx_epy_block_0 as epy_block_0  # embedded python block
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation




class tx(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "QAM16 video link Transmitter", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("QAM16 video link Transmitter")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("gnuradio/flowgraphs", "tx")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)
        self.flowgraph_started = threading.Event()

        ##################################################
        # Variables
        ##################################################
        self.symbol_rate = symbol_rate = 300_000
        self.samp_rate = samp_rate = 2_400_000
        self.sps = sps = int(samp_rate/symbol_rate)
        self.roll_off = roll_off = 0.4
        self.rrc_taps = rrc_taps = firdes.root_raised_cosine(1.0, samp_rate,symbol_rate, roll_off, (11*sps))
        self.qam = qam = digital.constellation_rect([-3-3j, -3-1j, -3+3j, -3+1j, -1-3j, -1-1j, -1+3j, -1+1j, 3-3j, 3-1j, 3+3j, 3+1j, 1-3j, 1-1j, 1+3j, 1+1j], [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
        4, 4, 4, 2, 2).base()

        ##################################################
        # Blocks
        ##################################################

        self.pdu_pdu_to_tagged_stream_0 = pdu.pdu_to_tagged_stream(gr.types.byte_t, 'packet_len')
        self.network_socket_pdu_0 = network.socket_pdu('UDP_SERVER', '127.0.0.1', '1234', 10000, False)
        self.epy_block_0 = epy_block_0.blk()
        self.digital_constellation_modulator_0 = digital.generic_mod(
            constellation=qam,
            differential=False,
            samples_per_symbol=sps,
            pre_diff_code=True,
            excess_bw=roll_off,
            verbose=False,
            log=False,
            truncate=False)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_gr_complex*1, '/home/simon/DSP/video-link/dump.iq', False)
        self.blocks_file_sink_0.set_unbuffered(False)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.epy_block_0, 'msg_out'), (self.pdu_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.network_socket_pdu_0, 'pdus'), (self.epy_block_0, 'msg_in'))
        self.connect((self.digital_constellation_modulator_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.digital_constellation_modulator_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("gnuradio/flowgraphs", "tx")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_symbol_rate(self):
        return self.symbol_rate

    def set_symbol_rate(self, symbol_rate):
        self.symbol_rate = symbol_rate
        self.set_rrc_taps(firdes.root_raised_cosine(1.0, self.samp_rate, self.symbol_rate, self.roll_off, (11*self.sps)))
        self.set_sps(int(self.samp_rate/self.symbol_rate))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_rrc_taps(firdes.root_raised_cosine(1.0, self.samp_rate, self.symbol_rate, self.roll_off, (11*self.sps)))
        self.set_sps(int(self.samp_rate/self.symbol_rate))

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps
        self.set_rrc_taps(firdes.root_raised_cosine(1.0, self.samp_rate, self.symbol_rate, self.roll_off, (11*self.sps)))

    def get_roll_off(self):
        return self.roll_off

    def set_roll_off(self, roll_off):
        self.roll_off = roll_off
        self.set_rrc_taps(firdes.root_raised_cosine(1.0, self.samp_rate, self.symbol_rate, self.roll_off, (11*self.sps)))

    def get_rrc_taps(self):
        return self.rrc_taps

    def set_rrc_taps(self, rrc_taps):
        self.rrc_taps = rrc_taps

    def get_qam(self):
        return self.qam

    def set_qam(self, qam):
        self.qam = qam




def main(top_block_cls=tx, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()
    tb.flowgraph_started.set()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
