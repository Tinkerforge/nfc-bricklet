function octave_example_scan_for_tags()
    more off;

    global nfc;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your NFC Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    nfc = javaObject("com.tinkerforge.BrickletNFC", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register reader state changed callback to function cb_reader_state_changed
    nfc.addReaderStateChangedCallback(@cb_reader_state_changed);

    % Enable reader mode
    nfc.setMode(nfc.MODE_READER);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for reader state changed callback
function cb_reader_state_changed(e)
    global nfc;

    if e.state == nfc.READER_STATE_REQUEST_TAG_ID_READY
        tag = "";
        ret = nfc.readerGetTagID();

        for i = 1:length(java_get(ret, "tagID"))
            tmp_tag = sprintf("0x%02X", java_get(ret, "tagID")(i));

            if i < length(java_get(ret, "tagID"))
              tmp_tag = cstrcat(tmp_tag, " ");
            end

            tag = cstrcat(tag, tmp_tag);
        end

        fprintf("Found tag of type %d with ID [%s]\n", java_get(ret, "tagType"), tag);
    end

    if e.state == nfc.READER_STATE_IDLE
        nfc.readerRequestTagID();
    end
end
