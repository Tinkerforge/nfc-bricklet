package main

import (
	"fmt"
    "github.com/tinkerforge/go-api-bindings/ipconnection"
    "github.com/tinkerforge/go-api-bindings/nfc_bricklet"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your NFC Bricklet.

func main() {
	ipcon := ipconnection.New()
    defer ipcon.Close()
	nfc, _ := nfc_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
    defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	nfc.RegisterReaderStateChangedCallback(func(state nfc_bricklet.ReaderState, idle bool) {
		switch state {
		case nfc_bricklet.ReaderStateIdle:
			nfc.ReaderRequestTagID()
		case nfc_bricklet.ReaderStateRequestTagIDReady:
			tagID, tagType, _ := nfc.ReaderGetTagID()
			if tagType != nfc_bricklet.TagTypeType2 {
				fmt.Println("Tag is not type-2")
				return				
			}

			fmt.Printf("Found tag of type %d with ID %v", tagType, tagID)
		case nfc_bricklet.ReaderStateRequestPageReady:
			page, _ := nfc.ReaderReadPage()
			fmt.Printf("Page read: % x", page)
			nfc.ReaderWritePage(1, page)
		case nfc_bricklet.ReaderStateWritePageReady:
			fmt.Println("Write page ready");
		case nfc_bricklet.ReaderStateRequestPageError:
			fmt.Println("Request page error");
		case nfc_bricklet.ReaderStateWritePageError:
			fmt.Println("Write page error");
		}
	})

	// Enable reader mode
	nfc.SetMode(nfc_bricklet.ModeReader)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()

	ipcon.Disconnect()
}
