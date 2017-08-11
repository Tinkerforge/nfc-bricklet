#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "abc" # Change to your UID

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC
import time

NDEF_RECORD = [
    0xD1,   # MB/ME/CF/1/IL/TNF
    0x01,   # TYPE LENGTH
    0x08,   # PAYLOAD LENTGH
    ord('T'),    # TYPE
    0x02,   # Status
    ord('e'), ord('n'), # Language
    ord('T'), ord('e'), ord('s'), ord('t'), ord('2')
]

# Callback function for state changed callback
def cb_state_changed(state, idle, nfc):
    print("state", state)
    if state == nfc.READER_STATE_IDLE:
        nfc.reader_request_tag_id()

    elif state == nfc.READER_STATE_REQUEST_TAG_ID_READY:
        ret = nfc.reader_get_tag_id()
        print('Found tag of type ' +
              str(ret.tag_type) +
              ' with ID [' +
              ' '.join(map(str, map(hex, ret.tag_id))) +
              "]")

        nfc.reader_write_ndef(NDEF_RECORD)

    elif state == nfc.READER_STATE_WRITE_NDEF_READY:
        print("ndef write ready")

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    nfc = BrickletNFC(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Register state changed callback to function cb_state_changed
    nfc.register_callback(nfc.CALLBACK_READER_STATE_CHANGED,
                          lambda x, y: cb_state_changed(x, y, nfc))

    nfc.set_mode(nfc.MODE_READER)

    raw_input('Press key to exit\n') # Use input() in Python 3
    ipcon.disconnect()
