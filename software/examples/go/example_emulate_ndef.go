package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
	"github.com/Tinkerforge/go-api-bindings/nfc_bricklet"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your NFC Bricklet.
const NDEF_URI = "www.tinkerforge.com"

func main() {
	ipcon := ipconnection.New()
	defer ipcon.Close()
	nfc, _ := nfc_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
	defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	nfc.RegisterCardemuStateChangedCallback(func(state nfc_bricklet.CardemuState, idle bool) {
		switch state {
		case nfc_bricklet.CardemuStateIdle:
			ndefRecordUDI := []uint8{0xd1, 0x01, uint8(len(NDEF_URI)) + 1, uint8('U'), 0x04}
			ndefRecordUDI = append(ndefRecordUDI, []byte(NDEF_URI)...)

			err := nfc.CardemuWriteNDEF(ndefRecordUDI)
			if err != nil {
				fmt.Printf("Error while writing ndef %s", err)
			}
			nfc.CardemuStartDiscovery()
		case nfc_bricklet.CardemuStateDiscoverReady:
			nfc.CardemuStartTransfer(nfc_bricklet.CardemuTransferWrite)
		case nfc_bricklet.CardemuStateDiscoverError:
			fmt.Println("Discover error")
		case nfc_bricklet.CardemuStateTransferNDEFError:
			fmt.Println("Transfer NDEF error")
		}
	})

	// Enable cardemu mode
	nfc.SetMode(nfc_bricklet.ModeCardemu)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
