#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your NFC Bricklet

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC

# Callback function for reader state changed callback
def cb_reader_state_changed(state, idle, nfc):
    if state == nfc.READER_STATE_REQUEST_TAG_ID_READY:
        ret = nfc.reader_get_tag_id()

        print("Found tag of type " +
              str(ret.tag_type) +
              " with ID [" +
              " ".join(map(str, map('0x{:02X}'.format, ret.tag_id))) +
              "]")

    if idle:
        nfc.reader_request_tag_id()

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    nfc = BrickletNFC(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Register reader state changed callback to function cb_reader_state_changed
    nfc.register_callback(nfc.CALLBACK_READER_STATE_CHANGED,
                          lambda x, y: cb_reader_state_changed(x, y, nfc))

    # Enable reader mode
    nfc.set_mode(nfc.MODE_READER)

    input("Press key to exit\n") # Use raw_input() in Python 2
    ipcon.disconnect()
