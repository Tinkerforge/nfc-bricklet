function matlab_example_scan_for_tags()
    global nfc;
    global NDEF_URI;

    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletNFC;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your NFC/RFID Bricklet
    NDEF_URI = 'www.tinkerforge.com';

    ipcon = IPConnection(); % Create IP connection
    nfc = handle(BrickletNFC(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register state changed callback to function cb_state_changed
    set(nfc, 'CardemuStateChangedCallback', @(h, e) cb_state_changed(e));

    nfc.setMode(BrickletNFC.MODE_CARDEMU);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end

% Callback function for state changed callback
function cb_state_changed(e)
    import com.tinkerforge.BrickletNFC;
    global nfc;
    global NDEF_URI;

    if e.state == BrickletNFC.CARDEMU_STATE_IDLE
        ndef_record_uri = [];

        % Only short records are supported
        ndef_record_uri(1) = hex2dec('D1');        % MB/ME/CF/SR=1/IL/TNF
        ndef_record_uri(2) = hex2dec('01');        % TYPE LENGTH
        ndef_record_uri(3) = length(NDEF_URI) + 1; % Length
        ndef_record_uri(4) = double('U');          % Type
        ndef_record_uri(5) = hex2dec('04');        % Status

        for i = 1:length(NDEF_URI)
            ndef_record_uri(5 + i) = double(NDEF_URI(i));
        end

        nfc.cardemuWriteNdef(ndef_record_uri);
        nfc.cardemuStartDiscovery();
    elseif e.state == BrickletNFC.CARDEMU_STATE_DISCOVER_READY
        nfc.cardemuStartTransfer(1);
    elseif e.state == BrickletNFC.CARDEMU_STATE_DISCOVER_ERROR
        disp('Discover error');
    elseif e.state == BrickletNFC.CARDEMU_STATE_TRANSFER_NDEF_ERROR
        disp('Transfer NDEF error');
    end
end
