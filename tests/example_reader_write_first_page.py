#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "abc" # Change to your UID

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC
import time

tag_type = 0

# Callback function for state changed callback
def cb_state_changed(state, idle, nfc):

    if state == nfc.READER_STATE_IDLE:
        nfc.reader_request_tag_id()

    elif state == nfc.READER_STATE_REQUEST_TAG_ID_READY:
        ret = nfc.reader_get_tag_id()
        print('Found tag of type ' +
              str(ret.tag_type) +
              ' with ID [' +
              ' '.join(map(str, map(hex, ret.tag_id))) +
              "]")

        nfc.reader_write_page(0, [16, 4, 1, 0, 13, 0, 0, 0, 0, 1, 1, 0, 0, 153, 0, 189])
#        nfc.reader_write_page(1, list(range(16)))

    elif state == nfc.READER_STATE_WRITE_PAGE_READY:
        print("write page done")

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
