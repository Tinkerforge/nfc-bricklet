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
  // Callback function for state changed callback
  function (state, idle) {
    if(state == Tinkerforge.BrickletNFC.READER_STATE_IDLE) {
      nfc.readerRequestTagID();
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY) {
      nfc.readerGetTagID(
        function (tagType, tid) {
          console.log('Found tag of type %d with ID [0x%s 0x%s 0x%s 0x%s]',
                      tagType,
                      tid[0].toString(16),
                      tid[1].toString(16),
                      tid[2].toString(16),
                      tid[3].toString(16));
        },
        function (error) {
          console.log('Error: ' + error);
        }
      );
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR) {
      console.log('Request tag ID error');
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
