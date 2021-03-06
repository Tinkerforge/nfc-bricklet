#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_nfc'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your NFC Bricklet

ipcon = IPConnection.new # Create IP connection
nfc = BrickletNFC.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Register reader state changed callback
nfc.register_callback(BrickletNFC::CALLBACK_READER_STATE_CHANGED) do |state, idle|
  if state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_READY
    ret = nfc.reader_get_tag_id
    tag_id = Array.new

    ret[1].each do |value|
      tag_id.push "0x%02X" % value
    end

    puts "Found tag of type #{ret[0]} with ID [#{tag_id.join(" ")}]"
  end

  if idle
    nfc.reader_request_tag_id
  end
end

# Enable reader mode
nfc.set_mode BrickletNFC::MODE_READER

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
