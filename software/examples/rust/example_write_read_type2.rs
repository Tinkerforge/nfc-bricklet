use std::{error::Error, io, thread};
use tinkerforge::{ipconnection::IpConnection, nfc_bricklet::*};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your NFC Bricklet

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let nfc_bricklet = NFCBricklet::new(UID, &ipcon); // Create device object

    ipcon.connect(HOST, PORT).recv()??; // Connect to brickd
                                        // Don't use device before ipcon is connected

    //Create listener for reader state changed events.
    let reader_state_changed_listener = nfc_bricklet.get_reader_state_changed_receiver();
    // Spawn thread to handle received events. This thread ends when the nfc_bricklet
    // is dropped, so there is no need for manual cleanup.
    let nfc_bricklet_copy = nfc_bricklet.clone(); //Device objects don't implement Sync, so they can't be shared between threads (by reference). So clone the device and move the copy.
    thread::spawn(move || {
        for event in reader_state_changed_listener {
            if event.state == NFC_BRICKLET_READER_STATE_IDLE {
                nfc_bricklet_copy.reader_request_tag_id();
            } else if event.state == NFC_BRICKLET_READER_STATE_REQUEST_TAG_ID_READY {
                let (tag_id, tag_type) = nfc_bricklet_copy.reader_get_tag_id().unwrap();

                if tag_type != NFC_BRICKLET_TAG_TYPE_TYPE2 {
                    println!("Tag is not type-2");
                    return;
                }

                println!("Found tag of type {} with ID {:x?}", tag_type, tag_id);
                nfc_bricklet_copy.reader_request_page(1, 4);
            } else if event.state == NFC_BRICKLET_READER_STATE_REQUEST_TAG_ID_ERROR {
                println!("Request tag ID error");
            } else if event.state == NFC_BRICKLET_READER_STATE_REQUEST_PAGE_READY {
                let page = nfc_bricklet_copy.reader_read_page().unwrap();

                println!("Page read: 0x{0:X} 0x{1:X} 0x{2:X} 0x{3:X}", page[0], page[1], page[2], page[3]);

                nfc_bricklet_copy.reader_write_page(1, &page).unwrap();
            } else if event.state == NFC_BRICKLET_READER_STATE_WRITE_PAGE_READY {
                println!("Write page ready");
            } else if event.state == NFC_BRICKLET_READER_STATE_REQUEST_PAGE_ERROR {
                println!("Request page error");
            } else if event.state == NFC_BRICKLET_READER_STATE_WRITE_PAGE_ERROR {
                println!("Write page error");
            }
        }
    });

    // Enable reader mode
    nfc_bricklet.set_mode(NFC_BRICKLET_MODE_READER);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
