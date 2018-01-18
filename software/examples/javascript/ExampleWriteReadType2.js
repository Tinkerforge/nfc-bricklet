var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your NFC/RFID Bricklet

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
    nfc.setMode(Tinkerforge.BrickletNFC.MODE_READER);
  }
);

// Register state changed callback
nfc.on(Tinkerforge.BrickletNFC.CALLBACK_READER_STATE_CHANGED,
  // Callback function for state changed callback
  function (state, idle) {
    if(state == Tinkerforge.BrickletNFC.READER_STATE_IDLE) {
      nfc.readerRequestTagID();
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY) {
      nfc.readerGetTagID(
        function (tagType, tid) {
          if(tagType != Tinkerforge.BrickletNFC.TAG_TYPE_TYPE2) {
            console.log('Tag is not type-2');

            return;
          }
          console.log('Found tag of type %d with ID [0x%s 0x%s 0x%s 0x%s]',
                      tagType,
                      tid[0].toString(16),
                      tid[1].toString(16),
                      tid[2].toString(16),
                      tid[3].toString(16));

          nfc.readerRequestPage(1, 4);
        },
        function (error) {
          console.log('Error: ' + error);
        }
      );
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR) {
      console.log('Request tag ID error');
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_PAGE_READY) {
      nfc.readerReadPage(
        function (page) {
          console.log('Page read: 0x%s 0x%s 0x%s',
                      page[0].toString(16),
                      page[1].toString(16),
                      page[2].toString(16),
                      page[3].toString(16));
          nfc.readerWritePage(1, page);
        },
        function(error) {
          console.log('Error: ' + error);
        }
      );
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_WRITE_PAGE_READY) {
      console.log('Page write OK');
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_REQUEST_PAGE_ERROR) {
      console.log('Request page error');
    }
    else if(state == Tinkerforge.BrickletNFC.READER_STATE_WRITE_PAGE_ERROR) {
      console.log('Request write page error');
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
