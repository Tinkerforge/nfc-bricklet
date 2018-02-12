<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletNFC.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletNFC;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet
const NDEF_URI = 'www.tinkerforge.com';

// Callback function for cardemu state changed callback
function cb_cardemuStateChanged($state, $idle, $user_data)
{
	$nfc = $user_data;

	if($state == BrickletNFC::CARDEMU_STATE_IDLE) {
		// Only short records are supported
		$NDEFRecordURI = array(
		                        0xD1,                   // MB/ME/CF/SR=1/IL/TNF
		                        0x01,                   // TYPE LENGTH
		                        strlen(NDEF_URI) + 1, // Length
		                        ord('U'),               // Type
		                        4                       // Status
		                      );

		foreach(str_split(NDEF_URI) as $c) {
			array_push($NDEFRecordURI, ord($c));
		}

		$nfc->cardemuWriteNDEF($NDEFRecordURI);
		$nfc->cardemuStartDiscovery();
	}
	else if ($state == BrickletNFC::CARDEMU_STATE_DISCOVER_READY) {
		$nfc->cardemuStartTransfer(1);
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

// Register cardemu state changed callback to function cb_cardemuStateChanged
$nfc->registerCallback(BrickletNFC::CALLBACK_CARDEMU_STATE_CHANGED,
                       'cb_cardemuStateChanged', $nfc);

// Enable cardemu mode
$nfc->setMode(BrickletNFC::MODE_CARDEMU);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
