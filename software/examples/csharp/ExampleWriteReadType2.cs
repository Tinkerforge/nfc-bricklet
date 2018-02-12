using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your NFC Bricklet

	// Callback function for reader state changed callback
	static void ReaderStateChangedCB(BrickletNFC sender, byte state, bool idle)
	{
		if(state == BrickletNFC.READER_STATE_IDLE)
		{
			sender.ReaderRequestTagID();
		}
		else if(state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY)
		{
			byte tagType;
			byte[] tagID;

			sender.ReaderGetTagID(out tagType, out tagID);
			
			if(tagType != BrickletNFC.TAG_TYPE_TYPE2) {
				Console.WriteLine("Tag is not type-2");

				return;
			}

			Console.WriteLine("Found tag of type {0} with ID [0x{1:X} 0x{2:X} 0x{3:X} 0x{4:X}]",
			                  tagType,
			                  tagID[0],
			                  tagID[1],
			                  tagID[2],
			                  tagID[3]);
			sender.ReaderRequestPage(1, 4);
		}
		else if(state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR) {
			Console.WriteLine("Request tag ID error");
		}
		else if(state == BrickletNFC.READER_STATE_REQUEST_PAGE_READY) {
			byte[] page = sender.ReaderReadPage();

			Console.WriteLine("Page read: 0x{0:X} 0x{1:X} 0x{2:X} 0x{3:X}",
			                  page[0],
			                  page[1],
			                  page[2],
			                  page[3]);

			sender.ReaderWritePage(1, page);
		}
		else if(state == BrickletNFC.READER_STATE_WRITE_PAGE_READY) {
			Console.WriteLine("Write page ready");
		}
		else if(state == BrickletNFC.READER_STATE_REQUEST_PAGE_ERROR) {
			Console.WriteLine("Request page error");
		}
		else if(state == BrickletNFC.READER_STATE_WRITE_PAGE_ERROR) {
			Console.WriteLine("Write page error");
		}
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletNFC nfc = new BrickletNFC(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register reader state changed callback to function ReaderStateChangedCB
		nfc.ReaderStateChangedCallback += ReaderStateChangedCB;

		// Enable reader mode
		nfc.SetMode(BrickletNFC.MODE_READER);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
