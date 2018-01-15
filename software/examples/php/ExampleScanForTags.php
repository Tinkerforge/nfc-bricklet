<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletNFC.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletNFC;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet

// Callback function for state changed callback
function cb_stateChanged($state, $idle, $user_data)
{
	$nfc = $user_data;

	if($state == BrickletNFC::READER_STATE_IDLE) {
		$nfc->readerRequestTagID();
	}
	else if ($state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_READY) {
		$tag_id = [];
		$ret = $nfc->readerGetTagID();

		if ($ret["tag_type"] != BrickletNFC::TAG_TYPE_TYPE2) {
			return;
		}

    printf("Found tag of type %u with ID [0x%X 0x%X 0x%X 0x%X]\n",
		       $ret["tag_type"],
					 $ret["tag_id"][0],
					 $ret["tag_id"][1],
					 $ret["tag_id"][2],
					 $ret["tag_id"][3]);
	}
	else if ($state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_ERROR) {
		echo "Request tag ID error\n";
	}
}

$ipcon = new IPConnection(); // Create IP connection
$nfc = new BrickletNFC(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register state changed callback to function cb_stateChanged
$nfc->registerCallback(BrickletNFC::CALLBACK_READER_STATE_CHANGED, 'cb_stateChanged', $nfc);

$nfc->setMode(BrickletNFC::MODE_READER);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
