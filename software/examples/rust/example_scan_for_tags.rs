use std::{error::Error, io, thread};
use tinkerforge::{ip_connection::IpConnection, nfc_bricklet::*};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your NFC Bricklet

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let nfc = NfcBricklet::new(UID, &ipcon); // Create device object

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd
                                          // Don't use device before ipcon is connected

    let reader_state_changed_receiver = nfc.get_reader_state_changed_callback_receiver();

    // Spawn thread to handle received events.
    // This thread ends when the `nfc` object
    // is dropped, so there is no need for manual cleanup.
    let nfc_copy = nfc.clone(); //Device objects don't implement Sync, so they can't be shared between threads (by reference). So clone the device and move the copy.
    thread::spawn(move || {
        for state_change in reader_state_changed_receiver {
            if state_change.state == NFC_BRICKLET_READER_STATE_REQUEST_TAG_ID_READY {
                let (tag_id, tag_type) = nfc_copy.reader_get_tag_id().unwrap();
                println!("Found tag of type {} with ID {:x?}", tag_type, tag_id);
            } else if state_change.state == NFC_BRICKLET_READER_STATE_REQUEST_TAG_ID_ERROR {
                println!("Request tag ID error");
            }
            if state_change.idle {
                nfc_copy.reader_request_tag_id();
            }
        }
    });

    // Enable reader mode
    nfc.set_mode(NFC_BRICKLET_MODE_READER);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
