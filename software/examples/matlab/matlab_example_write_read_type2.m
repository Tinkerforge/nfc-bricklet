function matlab_example_write_read_type2()
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
    import com.tinkerforge.BrickletNFC;
    global nfc;

    if e.state == BrickletNFC.READER_STATE_IDLE
        nfc.readerRequestTagID();
    elseif e.state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY
        ret = nfc.readerGetTagID();

        if ret.tagType ~= BrickletNFC.TAG_TYPE_TYPE2
            disp('Tag is not type-2');

            return;
        end

        fprintf('Found tag of type %d with ID [0x%X 0x%X 0x%X 0x%X]\n', ...
                ret.tagType, ...
                ret.tagID(1), ...
                ret.tagID(2), ...
                ret.tagID(3), ...
                ret.tagID(4));
        nfc.readerRequestPage(1, 4);
    elseif e.state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR
        disp('Request tag ID error');
    elseif e.state == BrickletNFC.READER_STATE_REQUEST_PAGE_READY
        page = nfc.readerReadPage();
        fprintf('Page read: 0x%X 0x%X 0x%X 0x%X\n', ...
                page(1), ...
                page(2), ...
                page(3), ...
                page(4));
        nfc.readerWritePage(1, page);
    elseif e.state == BrickletNFC.READER_STATE_WRITE_PAGE_READY
        disp('Write page ready');
    elseif e.state == BrickletNFC.READER_STATE_REQUEST_PAGE_ERROR
        disp('Request page error');
    elseif e.state == BrickletNFC.READER_STATE_WRITE_PAGE_ERROR
        disp('Write page error');
    end
end
