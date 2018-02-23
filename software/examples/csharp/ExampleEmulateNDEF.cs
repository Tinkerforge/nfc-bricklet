using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your NFC Bricklet
	private static string NDEF_URI = "www.tinkerforge.com";

	// Callback function for cardemu state changed callback
	static void CardemuStateChangedCB(BrickletNFC sender, byte state, bool idle)
	{
		if(state == BrickletNFC.CARDEMU_STATE_IDLE)
		{
			byte[] ndefRecordURI = new byte[NDEF_URI.Length + 5];

			ndefRecordURI[0] = 0xD1;
			ndefRecordURI[1] = 0x01;
			ndefRecordURI[2] = (byte)(NDEF_URI.Length + 1);
			ndefRecordURI[3] = (int)'U';
			ndefRecordURI[4] = 0x04;

			// Only short records are supported
			for (int i = 0; i < NDEF_URI.Length; i++)
			{
				ndefRecordURI[5 + i] = (byte)NDEF_URI[i];
			}

			sender.CardemuWriteNdef(ndefRecordURI);
			sender.CardemuStartDiscovery();
		}
		else if(state == BrickletNFC.CARDEMU_STATE_DISCOVER_READY)
		{
			sender.CardemuStartTransfer(1);
		}
		else if(state == BrickletNFC.CARDEMU_STATE_DISCOVER_ERROR) {
			Console.WriteLine("Discover error");
		}
		else if(state == BrickletNFC.CARDEMU_STATE_TRANSFER_NDEF_ERROR) {
			Console.WriteLine("Transfer NDEF error");
		}
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletNFC nfc = new BrickletNFC(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register cardemu state changed callback to function CardemuStateChangedCB
		nfc.CardemuStateChangedCallback += CardemuStateChangedCB;

		// Enable cardemu mode
		nfc.SetMode(BrickletNFC.MODE_CARDEMU);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
