var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your NFC Bricklet
var NDEF_URI = 'www.tinkerforge.com';

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
        // Enable cardemu mode
        nfc.setMode(Tinkerforge.BrickletNFC.MODE_CARDEMU);
    }
);

// Register cardemu state changed callback
nfc.on(Tinkerforge.BrickletNFC.CALLBACK_CARDEMU_STATE_CHANGED,
  // Callback function for state changed callback
  function (state, idle) {
    if(state == Tinkerforge.BrickletNFC.CARDEMU_STATE_IDLE) {
      // Only short records are supported
      var ndefRecordURI = [
                            0xD1,                // MB/ME/CF/SR=1/IL/TNF
                            0x01,                // TYPE LENGTH
                            NDEF_URI.length + 1, // Length
                            'U'.charCodeAt(0),   // Type
                            4                    // Status
                          ];

      for(var i = 0; i < NDEF_URI.length; i++) {
        ndefRecordURI.push(NDEF_URI.charCodeAt(i));
      }

      nfc.cardemuWriteNDEF(ndefRecordURI,
        function() {
          nfc.cardemuStartDiscovery();
        }
      );
    }
    else if(state == Tinkerforge.BrickletNFC.CARDEMU_STATE_DISCOVER_READY) {
      nfc.cardemuStartTransfer(Tinkerforge.BrickletNFC.CARDEMU_TRANSFER_WRITE);
    }
    else if(state == Tinkerforge.BrickletNFC.CARDEMU_STATE_DISCOVER_ERROR) {
      console.log('Discover error');
    }
    else if(state == Tinkerforge.BrickletNFC.CARDEMU_STATE_TRANSFER_NDEF_ERROR) {
      console.log('Transfer NDEF error');
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
