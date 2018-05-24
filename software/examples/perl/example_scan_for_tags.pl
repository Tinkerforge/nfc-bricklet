#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletNFC;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your NFC Bricklet

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
our $nfc = Tinkerforge::BrickletNFC->new(&UID, $ipcon); # Create device object

# Callback subroutine for reader state changed callback
sub cb_reader_state_changed
{
    my ($state, $idle) = @_;

    if($state == $nfc->READER_STATE_IDLE) {
        $nfc->reader_request_tag_id();
    }
    elsif ($state == $nfc->READER_STATE_REQUEST_TAG_ID_READY) {
      my @ret = $nfc->reader_get_tag_id();
      my @tag_id_bytes;

      for my $v (@{$ret[1]}) {
        push(@tag_id_bytes, sprintf("0x%X", $v));
      }

      print "Found tag of type " . $ret[0] . " with ID [" . join(" ", @tag_id_bytes). "]\n";
    }
    elsif ($state == $nfc->READER_STATE_REQUEST_TAG_ID_ERROR) {
      print "Request tag ID error\n";
    }
}

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Register reader state changed callback to subroutine cb_reader_state_changed
$nfc->register_callback($nfc->CALLBACK_READER_STATE_CHANGED, 'cb_reader_state_changed');

# Enable reader mode
$nfc->set_mode($nfc->MODE_READER);

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
