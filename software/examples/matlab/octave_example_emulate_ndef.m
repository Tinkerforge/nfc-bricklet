function octave_example_emulate_ndef()
    more off;

    global nfc;
    global NDEF_URI;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your NFC Bricklet
    NDEF_URI = "www.tinkerforge.com";

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    nfc = javaObject("com.tinkerforge.BrickletNFC", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register cardemu state changed callback to function cb_cardemu_state_changed
    nfc.addCardemuStateChangedCallback(@cb_cardemu_state_changed);

    % Enable cardemu mode
    nfc.setMode(nfc.MODE_CARDEMU);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for cardemu state changed callback
function cb_cardemu_state_changed(e)
    global nfc;
    global NDEF_URI;

    if e.state == nfc.CARDEMU_STATE_IDLE
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

        nfc.cardemuWriteNDEF(ndef_record_uri);
        nfc.cardemuStartDiscovery();
    elseif e.state == nfc.CARDEMU_STATE_DISCOVER_READY
        nfc.cardemuStartTransfer(1);
    elseif e.state == nfc.CARDEMU_STATE_DISCOVER_ERROR
        disp("Discover error");
    elseif e.state == nfc.CARDEMU_STATE_TRANSFER_NDEF_ERROR
        disp("Transfer NDEF error");
    end
end
