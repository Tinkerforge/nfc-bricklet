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
			echo "Tag is not type-2\n";

			return;
		}

    printf("Found tag of type %u with ID [0x%X 0x%X 0x%X 0x%X]\n",
		       $ret["tag_type"],
					 $ret["tag_id"][0],
					 $ret["tag_id"][1],
					 $ret["tag_id"][2],
					 $ret["tag_id"][3]);

		$nfc->readerRequestPage(1, 4);
	}
	else if ($state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_ERROR) {
		echo "Request tag ID error\n";
	}
	else if ($state == BrickletNFC::READER_STATE_REQUEST_PAGE_READY) {
		$page = $nfc->readerReadPage();
		printf("Page read: 0x%X 0x%X 0x%X 0x%X\n", $page[0], $page[1], $page[2], $page[3]);
    $nfc->readerWritePage(1, $page);
	}
	else if ($state == BrickletNFC::READER_STATE_WRITE_PAGE_READY) {
		echo "Page write OK\n";
	}
	else if ($state == BrickletNFC::READER_STATE_REQUEST_PAGE_ERROR) {
		echo "Request page error\n";
	}
	else if ($state == BrickletNFC::READER_STATE_WRITE_PAGE_ERROR) {
		echo "Request write page error\n";
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
