var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var nfc = new Tinkerforge.BrickletNFC(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Enable reader mode
        nfc.setMode(Tinkerforge.BrickletNFC.MODE_READER);
    }
);

// Register reader state changed callback
nfc.on(Tinkerforge.BrickletNFC.CALLBACK_READER_STATE_CHANGED,
    // Callback function for reader state changed callback
    function (state, idle) {
        if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY) {
            nfc.readerGetTagID(
                function (tagType, tagID) {
                    var tagInfo = ''

                    for (var i = 0; i < tagID.length; i++) {
                        tagInfo += '0x' + ('0' + tagID[i].toString(16).toUpperCase()).substr(-2)

                        if (i < tagID.length - 1) {
                            tagInfo += ' '
                        }
                    }

                    console.log('Found tag of type %d with ID [%s]', tagType, tagInfo)
                },
                function (error) {
                    console.log('Could not get tag ID: ' + error)
                }
            );
        }

        if(idle) {
            nfc.readerRequestTagID();
        }
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
