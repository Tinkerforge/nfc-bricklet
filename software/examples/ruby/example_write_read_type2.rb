#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_nfc'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your NFC/RFID Bricklet

ipcon = IPConnection.new # Create IP connection
nfc = BrickletNFC.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Register state changed callback
nfc.register_callback(BrickletNFC::CALLBACK_READER_STATE_CHANGED) do |state, idle|
  if state == BrickletNFC::READER_STATE_IDLE
    nfc.reader_request_tag_id
  elsif state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_READY
    ret = nfc.reader_get_tag_id

    if ret[0] != BrickletNFC::TAG_TYPE_TYPE2
      puts "Tag is not type-2"

      return
    end

    printf "Found tag of type #{ret[0]} with ID [0x%X 0x%X 0x%X 0x%X]\n",
           ret[0],
           ret[1][0],
           ret[1][1],
           ret[1][2],
           ret[1][3]

    nfc.reader_request_page 1, 4
  elsif state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_ERROR
    puts "Request tag ID error"
  elsif state == BrickletNFC::READER_STATE_REQUEST_PAGE_READY
    page = nfc.reader_read_page
    printf "Page read: 0x%X 0x%X 0x%X 0x%X\n", page[0], page[1], page[2], page[3]
    nfc.reader_write_page 1, page
  elsif state == BrickletNFC::READER_STATE_WRITE_PAGE_READY
    puts "Write page ready"
  elsif state == BrickletNFC::READER_STATE_REQUEST_PAGE_ERROR
    puts "Request page error"
  elsif state == BrickletNFC::READER_STATE_WRITE_PAGE_ERROR
    puts "Write page error"
  end
end

nfc.set_mode BrickletNFC::MODE_READER

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
