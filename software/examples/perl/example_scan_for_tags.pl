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

    if ($state == Tinkerforge::BrickletNFC->READER_STATE_REQUEST_TAG_ID_READY) {
        my ($tag_type, $tag_id) = $nfc->reader_get_tag_id();
        my @tag_info;

        for my $v (@{$tag_id}) {
            push(@tag_info, sprintf("0x%02X", $v));
        }

        print "Found tag of type " . $tag_type . " with ID [" . join(" ", @tag_info) . "]\n";
    }

    if ($idle) {
        $nfc->reader_request_tag_id();
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
