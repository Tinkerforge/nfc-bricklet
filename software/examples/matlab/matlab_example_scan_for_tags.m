function matlab_example_scan_for_tags()
    global nfc;

    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletNFC;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your NFC Bricklet

    ipcon = IPConnection(); % Create IP connection
    nfc = handle(BrickletNFC(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register reader state changed callback to function cb_reader_state_changed
    set(nfc, 'ReaderStateChangedCallback', @(h, e) cb_reader_state_changed(e));

    % Enable reader mode
    nfc.setMode(BrickletNFC.MODE_READER);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end

% Callback function for reader state changed callback
function cb_reader_state_changed(e)
    global nfc;

    import com.tinkerforge.BrickletNFC;

    if e.state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY
        tag = '';
        ret = nfc.readerGetTagID();

        for i = 1:length(ret.tagID)
            tmp_tag = sprintf('0x%02X', ret.tagID(i));

            if i < length(ret.tagID)
                tmp_tag = strcat(tmp_tag, ' ');
            end

            tag = strcat(tag, tmp_tag);
        end

        fprintf('Found tag of type %d with ID [%s]\n', ret.tagType, tag);
    end

    if e.state == nfc.READER_STATE_IDLE
        nfc.readerRequestTagID();
    end
end
