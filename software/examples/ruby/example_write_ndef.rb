#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_nfc'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your NFC/RFID Bricklet
NDEF_URI = 'www.tinkerforge.com'

ipcon = IPConnection.new # Create IP connection
nfc = BrickletNFC.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Register state changed callback
nfc.register_callback(BrickletNFC::CALLBACK_CARDEMU_STATE_CHANGED) do |state, idle|
  if state == BrickletNFC::CARDEMU_STATE_IDLE
    payload_uri = Array.new

    NDEF_URI.split('').each do |c|
      payload_uri.push c.ord
    end

    # Only short records are supported.
    ndef_record_uri = [
                        0xD1,                   # MB/ME/CF/SR=1/IL/TNF
                        0x01,                   # TYPE LENGTH
                        payload_uri.length + 1, # Length
                        'U'.ord,                # Type
                        4                       # Status
                      ]

    payload_uri.each do |v|
      ndef_record_uri.push v
    end

    nfc.cardemu_write_ndef ndef_record_uri
    nfc.cardemu_start_discovery
  elsif state == BrickletNFC::CARDEMU_STATE_DISCOVER_READY
    nfc.cardemu_start_transfer 1
  elsif state == BrickletNFC::CARDEMU_STATE_DISCOVER_ERROR
    puts "Discover error"
  elsif state == BrickletNFC::CARDEMU_STATE_TRANSFER_NDEF_ERROR
    puts "Transfer NDEF error"
  end
end

nfc.set_mode BrickletNFC::MODE_CARDEMU

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
