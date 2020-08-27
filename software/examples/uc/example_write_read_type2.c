#include "bindings/hal_common.h"
#include "bindings/bricklet_nfc.h"
#include "bindings/errors.h"

#define UID "XYZ" // Change XYZ to the UID of your NFC Bricklet

void example_setup(TF_HalContext *hal);
void example_loop(TF_HalContext *hal);

void check(int rc, const char* msg);

static uint8_t nfc_state;
static bool nfc_idle;
static bool valid = false;

// Callback function for reader state changed callback
static void reader_state_changed_handler(TF_NFC *device, uint8_t state, bool idle,
                                  void *user_data) {
	(void)device; (void)user_data; // avoid unused parameter warning

	nfc_state = state;
	nfc_idle = idle;
	valid = true;
}

static TF_NFC nfc;

void example_setup(TF_HalContext *hal) {
	// Create device object
	check(tf_nfc_create(&nfc, UID, hal), "create device object");

	// Register reader state changed callback to function reader_state_changed_handler
	tf_nfc_register_reader_state_changed_callback(&nfc,
	                                              reader_state_changed_handler,
	                                              NULL);

	// Enable reader mode
	check(tf_nfc_set_mode(&nfc, TF_NFC_MODE_READER), "call set_mode");
}

void example_loop(TF_HalContext *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);

	if (!valid) {
		return;
	}

	valid = false;

	if(nfc_state == TF_NFC_READER_STATE_IDLE) {
		tf_nfc_reader_request_tag_id(&nfc);
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_TAG_ID_READY) {
		int ret = 0;
		uint8_t ret_tag_type = 0;
		uint8_t ret_tag_id_length = 0;
		uint8_t ret_tag_id[32] = {0};

		ret = tf_nfc_reader_get_tag_id(&nfc, &ret_tag_type, ret_tag_id, &ret_tag_id_length);

		if(ret != TF_E_OK) {
			return;
		}

		if(ret_tag_type != TF_NFC_TAG_TYPE_TYPE2) {
			tf_hal_printf("Tag is not type-2\n");
			return;
		}

		tf_hal_printf("Found tag of type %d with ID [0x%X 0x%X 0x%X 0x%X]\n",
		       ret_tag_type,
		       ret_tag_id[0],
		       ret_tag_id[1],
		       ret_tag_id[2],
		       ret_tag_id[3]);
		tf_nfc_reader_request_page(&nfc, 1, 4);
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_TAG_ID_ERROR) {
		tf_hal_printf("Request tag ID error\n");
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_PAGE_READY) {
		int ret = 0;
		uint16_t ret_data_length = 0;
		uint8_t ret_data[4] = {0};

		ret = tf_nfc_reader_read_page(&nfc, ret_data, &ret_data_length);

		if(ret != TF_E_OK) {
			return;
		}
		tf_hal_printf("Page read: 0x%X 0x%X 0x%X 0x%X\n",
		       ret_data[0],
		       ret_data[1],
		       ret_data[2],
		       ret_data[3]);
		tf_nfc_reader_write_page(&nfc, 1, ret_data, ret_data_length);
	}
	else if(nfc_state == TF_NFC_READER_STATE_WRITE_PAGE_READY) {
		tf_hal_printf("Write page ready\n");
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_PAGE_ERROR) {
		tf_hal_printf("Request page error\n");
	}
	else if(nfc_state == TF_NFC_READER_STATE_WRITE_PAGE_ERROR) {
		tf_hal_printf("Write page error\n");
	}
}
