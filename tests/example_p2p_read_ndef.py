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
    print("new state", state)

    if state == nfc.P2P_STATE_IDLE:
        print("state idle")
        nfc.p2p_write_ndef(NDEF_RECORD)
        nfc.p2p_start_discovery()

    if state == nfc.P2P_STATE_DISCOVER_READY:
        print('state discover ready')
        nfc.p2p_start_transfer(2)

    if state == nfc.P2P_STATE_TRANSFER_NDEF_READY:
        print('state transfer ready')
        ret = nfc.p2p_read_ndef()
        print(ret)
        print(map(chr, ret))

        nfc.p2p_start_discovery()

    if state == nfc.P2P_STATE_TRANSFER_NDEF_ERROR:
        nfc.p2p_start_discovery()

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    nfc = BrickletNFC(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    nfc.register_callback(nfc.CALLBACK_P2P_STATE_CHANGED,
                          lambda x, y: cb_state_changed(x, y, nfc))

    nfc.set_mode(nfc.MODE_P2P)


    raw_input('Press key to exit\n') # Use input() in Python 3
    ipcon.disconnect()
