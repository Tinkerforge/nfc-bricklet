function octave_example_write_read_type2()
    more off;

    global nfc;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your NFC Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    nfc = javaObject("com.tinkerforge.BrickletNFC", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register state changed callback to function cb_state_changed
    nfc.addReaderStateChangedCallback(@cb_state_changed);

    nfc.setMode(nfc.MODE_READER);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for state changed callback
function cb_state_changed(e)
    global nfc;

    if e.state == nfc.READER_STATE_IDLE
        nfc.readerRequestTagID();
    elseif e.state == nfc.READER_STATE_REQUEST_TAG_ID_READY
        ret = nfc.readerGetTagID();

        if java_get(ret, "tagType") ~= nfc.TAG_TYPE_TYPE2
          disp("Tag is not type-2");

          return;
        end

        fprintf("Found tag of type %d with ID [0x%X 0x%X 0x%X 0x%X]\n", ...
                java_get(ret, "tagType"), ...
                java_get(ret, "tagID")(1), ...
                java_get(ret, "tagID")(2), ...
                java_get(ret, "tagID")(3), ...
                java_get(ret, "tagID")(4));
        nfc.readerRequestPage(1, 4);
    elseif e.state == nfc.READER_STATE_REQUEST_TAG_ID_ERROR
        disp("Request tag ID error");
    elseif e.state == nfc.READER_STATE_REQUEST_PAGE_READY
        page = nfc.readerReadPage();
        fprintf("Page read: 0x%X 0x%X 0x%X 0x%X\n", ...
                page(1), ...
                page(2), ...
                page(3), ...
                page(4));
        nfc.readerWritePage(1, page);
    elseif e.state == nfc.READER_STATE_WRITE_PAGE_READY
        disp("Write page ready");
    elseif e.state == nfc.READER_STATE_REQUEST_PAGE_ERROR
        disp("Request page error");
    elseif e.state == nfc.READER_STATE_WRITE_PAGE_ERROR
        disp("Write page error");
    end
end
