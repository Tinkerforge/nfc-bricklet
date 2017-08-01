#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "abc" # Change to your UID

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_nfc import BrickletNFC
import time
import ndef

NDEF_RECORD = [
    0xD1,   # MB/ME/CF/1/IL/TNF
    0x01,   # TYPE LENGTH
    0x08,   # PAYLOAD LENTGH
    ord('T'),    # TYPE
    0x02,   # Status
    ord('e'), ord('n'), # Language
    ord('T'), ord('e'), ord('s'), ord('t'), ord('2')
]

def wifi_ndef_record():
    credential = ndef.wifi.Credential()
    credential.set_attribute('network-index', 1)
    credential.set_attribute('ssid', b'my network name')
    credential.set_attribute('authentication-type', 'WPA2-Personal')
    credential.set_attribute('encryption-type', 'AES')
    credential.set_attribute('network-key', b'my secret password')
    credential.set_attribute('mac-address', b'\xFF\xFF\xFF\xFF\xFF\xFF')
    wfa_ext = ndef.wifi.WifiAllianceVendorExtension()
    wfa_ext.set_attribute('network-key-shareable', 1)
    credential['vendor-extension'] = [wfa_ext.encode()]
    print(credential)

    record = ndef.wifi.WifiSimpleConfigRecord()
    record.name = 'my config token'
    record.set_attribute('credential', credential)
    record.set_attribute('rf-bands', ('2.4GHz', '5.0GHz'))
    wfa_ext = ndef.wifi.WifiAllianceVendorExtension()
    wfa_ext.set_attribute('version-2', 0x20)
    record['vendor-extension'] = [wfa_ext.encode()]
    print(record)

    ndef_record = map(ord, ''.join(ndef.message_encoder([record])))
    return ndef_record


# Callback function for state changed callback
def cb_state_changed(state, idle, nfc):
    print("new state", state)

    if state == nfc.CARDEMU_STATE_IDLE:
        nfc.cardemu_write_ndef(wifi_ndef_record())
        nfc.cardemu_start_discovery()

    if state == nfc.CARDEMU_STATE_DISCOVER_READY:
        print('start transfer')
        nfc.cardemu_start_transfer(True)

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    nfc = BrickletNFC(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    nfc.register_callback(nfc.CALLBACK_CARDEMU_STATE_CHANGED,
                          lambda x, y: cb_state_changed(x, y, nfc))

    nfc.set_mode(nfc.MODE_CARDEMU)


    raw_input('Press key to exit\n') # Use input() in Python 3
    ipcon.disconnect()
