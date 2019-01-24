package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
	"github.com/Tinkerforge/go-api-bindings/nfc_bricklet"
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
		if state == nfc_bricklet.ReaderStateRequestTagIDReady {
			tagID, tagType, _ := nfc.ReaderGetTagID()
			fmt.Printf("Found tag of type %d with ID %v", tagType, tagID)
		} else if state == nfc_bricklet.ReaderStateRequestTagIDError {
			fmt.Println("Request tag ID error")
		}
		if idle {
			nfc.ReaderRequestTagID()
		}
	})

	// Enable reader mode
	nfc.SetMode(nfc_bricklet.ModeReader)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
