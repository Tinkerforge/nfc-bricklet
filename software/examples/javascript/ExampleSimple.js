var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var nfc = new Tinkerforge.BrickletNFC(UID, ipcon); // Create device object
var cycle = 1;

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

function printTagID(index) {
    nfc.simpleGetTagID(
        index,
        function (tagType, tagID, lastSeen) {
            if (tagID.length == 0) {
                return;
            }

            var tagInfo = '';

            for (var i = 0; i < tagID.length; i++) {
                tagInfo += '0x' + ('0' + tagID[i].toString(16).toUpperCase()).substr(-2);

                if (i < tagID.length - 1) {
                    tagInfo += ' ';
                }
            }

            console.log('  Index: %d, Tag Type: %d, Tag ID: [%s], Last Seen: %f seconds ago', index, tagType, tagInfo, lastSeen / 1000.0);
        },
        function () {
            // On error enable simple mode again
            nfc.setMode(Tinkerforge.BrickletNFC.MODE_SIMPLE);
        }
    );
}

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Enable simple mode
        nfc.setMode(Tinkerforge.BrickletNFC.MODE_SIMPLE);

        // Get current tag IDs every second
        setInterval(function () {
            console.log('\nCycle: %d', cycle++)

            for(var index = 0; index < 8; ++index) {
                printTagID(index);
            }
        }, 1000);
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
