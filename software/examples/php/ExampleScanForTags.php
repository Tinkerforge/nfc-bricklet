<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletNFC.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletNFC;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet

// Callback function for reader state changed callback
function cb_readerStateChanged($state, $idle, $user_data)
{
    $nfc = $user_data;

    if ($state == BrickletNFC::READER_STATE_REQUEST_TAG_ID_READY) {
        $tag_id = [];
        $ret = $nfc->readerGetTagID();

        foreach ($ret["tag_id"] as $value) {
            array_push($tag_id, sprintf("0x%02X", $value));
        }

        printf("Found tag of type %u with ID [%s]\n", $ret["tag_type"], join(" ", $tag_id));
    }

    if ($idle) {
        $nfc->readerRequestTagID();
    }
}

$ipcon = new IPConnection(); // Create IP connection
$nfc = new BrickletNFC(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register reader state changed callback to function cb_readerStateChanged
$nfc->registerCallback(BrickletNFC::CALLBACK_READER_STATE_CHANGED,
                       'cb_readerStateChanged', $nfc);

// Enable reader mode
$nfc->setMode(BrickletNFC::MODE_READER);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
