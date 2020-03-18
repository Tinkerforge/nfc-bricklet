#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletNFC;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your NFC Bricklet
use constant NDEF_URI => "www.tinkerforge.com";

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
our $nfc = Tinkerforge::BrickletNFC->new(&UID, $ipcon); # Create device object

# Callback subroutine for cardemu state changed callback
sub cb_cardemu_state_changed
{
    my ($state, $idle) = @_;

    if ($state == $nfc->CARDEMU_STATE_IDLE) {
        my @ndef_record_uri = (
            0xD1,                  # MB/ME/CF/SR=1/IL/TNF
            0x01,                  # TYPE LENGTH
            length(&NDEF_URI) + 1, # Length
            ord("U"),              # Type
            4                      # Status
        );

        for my $v (split('', &NDEF_URI)) {
            push(@ndef_record_uri, ord($v));
        }

        $nfc->cardemu_write_ndef(\@ndef_record_uri);
        $nfc->cardemu_start_discovery();
    }
    elsif ($state == $nfc->CARDEMU_STATE_DISCOVER_READY) {
        $nfc->cardemu_start_transfer($nfc->CARDEMU_TRANSFER_WRITE);
    }
    elsif ($state == $nfc->CARDEMU_STATE_DISCOVER_ERROR) {
        print "Discover error\n";
    }
    elsif ($state == $nfc->CARDEMU_STATE_TRANSFER_NDEF_ERROR) {
        print "Transfer NDEF error\n";
    }
}

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Register cardemu state changed callback to subroutine cb_cardemu_state_changed
$nfc->register_callback($nfc->CALLBACK_CARDEMU_STATE_CHANGED, 'cb_cardemu_state_changed');

# Enable cardemu mode
$nfc->set_mode($nfc->MODE_CARDEMU);

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
