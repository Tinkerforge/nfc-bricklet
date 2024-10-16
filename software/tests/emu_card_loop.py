#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "2aFp" # Change XYZ to the UID of your NFC Bricklet
NDEF_URI = "www.tinkerforge.com"

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC
import time

count = 0

# Callback function for cardemu state changed callback
def cb_cardemu_state_changed(state, idle, nfc):
    global count
    if state == nfc.CARDEMU_STATE_IDLE:
        count += 1
        # Only short records are supported
        ndef_record_uri = [
            0xD1,              # MB/ME/CF/SR=1/IL/TNF
            0x01,              # TYPE LENGTH
            len(NDEF_URI) + len(str(count)) + 1, # Length
            ord("U"),          # Type
            0x04               # Status
        ]

        for c in list(NDEF_URI):
            ndef_record_uri.append(ord(c))
        for c in list(str(count)):
            ndef_record_uri.append(ord(c))

        nfc.cardemu_write_ndef(ndef_record_uri)
        nfc.cardemu_start_discovery()
        print("Discover start")
    elif state == nfc.CARDEMU_STATE_DISCOVER_READY:
        nfc.cardemu_start_transfer(nfc.CARDEMU_TRANSFER_WRITE)
        print("Discover ready")
        time.sleep(1)
        nfc.set_mode(nfc.MODE_CARDEMU)
    elif state == nfc.CARDEMU_STATE_DISCOVER_ERROR:
        print("Discover error")
    elif state == nfc.CARDEMU_STATE_TRANSFER_NDEF_ERROR:
        print("Transfer NDEF error")

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    nfc = BrickletNFC(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Register cardemu state changed callback to function cb_cardemu_state_changed
    nfc.register_callback(nfc.CALLBACK_CARDEMU_STATE_CHANGED,
                          lambda x, y: cb_cardemu_state_changed(x, y, nfc))

    #nfc.cardemu_set_tag_id(4, [0x42, 0x23, 0x13, 0x37, 0x00, 0x00, 0x00])

    # Enable cardemu mode
    nfc.set_mode(nfc.MODE_CARDEMU)

    input("Press key to exit\n") # Use raw_input() in Python 2
    ipcon.disconnect()
