<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletNFC.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletNFC;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet
const NDEF_URI = 'www.tinkerforge.com';

// Callback function for state changed callback
function cb_stateChanged($state, $idle, $user_data)
{
	$nfc = $user_data;

	if($state == BrickletNFC::CARDEMU_STATE_IDLE) {
		$payloadURI = [];

		foreach(str_split(NDEF_URI) as $c) {
			array_push($payloadURI, ord($c));
		}

		// Only short records are supported
		$NDEFRecordURI = array(
		                        0xD1,                   // MB/ME/CF/SR=1/IL/TNF
		                        0x01,                   // TYPE LENGTH
		                        count($payloadURI) + 1, // Length
		                        ord('U'),               // Type
		                        4                       // Status
		                      );

		foreach($payloadURI as $d) {
			array_push($NDEFRecordURI, $d);
		}

		$nfc->cardemuWriteNDEF($NDEFRecordURI);
		$nfc->cardemuStartDiscovery();
	}
	else if ($state == BrickletNFC::CARDEMU_STATE_DISCOVER_READY) {
		$nfc->cardemuStartTransfer(True);
	}
	else if ($state == BrickletNFC::CARDEMU_STATE_DISCOVER_ERROR) {
		echo 'Discover error\n';
	}
	else if ($state == BrickletNFC::CARDEMU_STATE_TRANSFER_NDEF_ERROR) {
		echo 'Transfer NDEF error\n';
	}
}

$ipcon = new IPConnection(); // Create IP connection
$nfc = new BrickletNFC(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register state changed callback to function cb_stateChanged
$nfc->registerCallback(BrickletNFC::CALLBACK_CARDEMU_STATE_CHANGED, 'cb_stateChanged', $nfc);

$nfc->setMode(BrickletNFC::MODE_CARDEMU);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
