"""
Embedded Python Blocks:

Each time this file is saved, GRC will instantiate the first class it finds
to get ports and parameters of your block. The arguments to __init__  will
be the parameters. All of them are required to have default values!
"""

import numpy as np
from gnuradio import gr
import pmt
import binascii

SYNC_WORD = np.frombuffer(
    (0x1ACFFC1D).to_bytes(4, "big"),
    dtype=np.uint8
)

class blk(gr.sync_block):  # other base classes are basic_block, decim_block, interp_block
    """Embedded Python Block example - a simple multiply const"""

    def __init__(self):  # only default arguments here
        """arguments to this function show up as parameters in GRC"""
        gr.sync_block.__init__(
            self,
            name='Video packetizer',   # will show up in GRC
            in_sig=None,
            out_sig=None
        )

        self.message_port_register_out(pmt.intern('msg_out'))
        self.message_port_register_in(pmt.intern('msg_in'))
        # if an attribute with the same name as a parameter is found,
        # a callback is registered (properties work, too).
        self.set_msg_handler(pmt.intern('msg_in'), self.create_frame)

    def create_frame(self, msg):
        metadata = pmt.car(msg)
        payload = pmt.cdr(msg)

        data = np.array(
            pmt.u8vector_elements(payload),
            dtype=np.uint8
        )

        packet_len = len(data)

        packet_len = np.frombuffer(
            packet_len.to_bytes(2, byteorder="big"),
            dtype=np.uint8
        )

        crc = binascii.crc_hqx(data.tobytes(), 0xFFFF)

        crc_bytes = np.frombuffer(
            crc.to_bytes(2, byteorder="big"),
            dtype=np.uint8
        )


        packet = np.concatenate([
            SYNC_WORD,
            packet_len,
            data,
            crc_bytes
        ])

        packet_pmt = pmt.init_u8vector(
            len(packet),
            packet.tolist()
        )

        pdu = pmt.cons(
            pmt.PMT_NIL,
            packet_pmt
        )

        self.message_port_pub(
            pmt.intern("msg_out"),
            pdu
        )



