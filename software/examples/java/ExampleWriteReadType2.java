import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletNFC;

public class ExampleWriteReadType2 {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your NFC Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletNFC nfc = new BrickletNFC(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Add state changed listener
		nfc.addReaderStateChangedListener(new BrickletNFC.ReaderStateChangedListener() {
			public void readerStateChanged(int state, boolean idle) {
				if (state == BrickletNFC.READER_STATE_IDLE) {
					try {
						nfc.readerRequestTagID();
					}
					catch (Exception e) {
						return;
					}
				}
				else if(state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY) {
					try{
						int i = 0;
						StringBuilder tag = new StringBuilder();
						BrickletNFC.ReaderGetTagID ret = nfc.readerGetTagID();

						if (ret.tagType != BrickletNFC.TAG_TYPE_TYPE2) {
							System.out.println("Tag is not type-2");

							return;
						}

						for (int v : ret.tagID) {
							if (i < ret.tagID.length - 1) {
								tag.append(String.format("0x%X ", v));
							}
							else {
								tag.append(String.format("0x%X", v));
							}

							i++;
						}

						System.out.format("Found tag of type %d with ID [%s]\n", ret.tagType, tag);

						nfc.readerRequestPage(1, 4);
					}
					catch (Exception e) {
						return;
					}
				}
				else if (state == BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR) {
					System.out.println("Request tag ID error");
				}
				else if (state == BrickletNFC.READER_STATE_REQUEST_PAGE_READY) {
					try {
						int[] page = nfc.readerReadPage();

						System.out.format("Page read: 0x%X 0x%X 0x%X 0x%X\n", page[0], page[1], page[2], page[3]);
		        nfc.readerWritePage(1, page);
					}
					catch (Exception e) {
						return;
					}
				}
				else if (state == BrickletNFC.READER_STATE_WRITE_PAGE_READY) {
					System.out.println("Write page ready");
				}
				else if (state == BrickletNFC.READER_STATE_REQUEST_PAGE_ERROR) {
					System.out.println("Request page error");
				}
				else if (state == BrickletNFC.READER_STATE_WRITE_PAGE_ERROR) {
					System.out.println("Write page error");
				}
			}
		});

		nfc.setMode(BrickletNFC.MODE_READER);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
