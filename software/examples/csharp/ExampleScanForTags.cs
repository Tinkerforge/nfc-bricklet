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
		if(state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY)
		{
			byte tagType;
			byte[] tagID;
			String tagInfo;

			sender.ReaderGetTagID(out tagType, out tagID);

			tagInfo = String.Format("Found tag of type {0} with ID [", tagType);

			for (int i = 0; i < tagID.Length; i++)
			{
				tagInfo += String.Format("0x{0:02X}", tagID[i]);

				if (i < tagID.Length - 1)
				{
					tagInfo += " ";
				}
			}

			tagInfo += "]";

			Console.WriteLine(tagInfo);
		}

		if(idle)
		{
			sender.ReaderRequestTagID();
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
