#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "abc" # Change to your UID

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC
import time

tag_type = 0
old_state = None

# Callback function for state changed callback
def cb_state_changed(state, idle, nfc):

    if old_state is not None:
        print 'State transition', old_state, '-->', state
    else:
        print 'Initial State', state

    global old_state
    old_state = state

    if state == nfc.READER_STATE_IDLE:
        nfc.reader_request_tag_id()

    elif state == nfc.READER_STATE_REQUEST_TAG_ID_READY:
        ret = nfc.reader_get_tag_id()
        print('Found tag of type ' +
              str(ret.tag_type) +
              ' with ID [' +
              ' '.join(map(str, map(hex, ret.tag_id))) +
              "]")
        print '\n\n'

        if ret.tag_type == nfc.TAG_TYPE_MIFARE_CLASSIC:
            nfc.reader_authenticate_mifare_classic_page(1, 0, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
        else:
            nfc.reader_request_page(1, 16)

    elif state == nfc.READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_READY:
        nfc.reader_request_page(1, 16)

    elif state == nfc.READER_STATE_REQUEST_PAGE_READY:
        data = nfc.reader_read_page()
        print(len(data), data)
        print '\n\n'
        nfc.reader_request_tag_id()
    else:
        print '\n\n'

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
