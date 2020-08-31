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

static int ret = 0;
static uint8_t ret_tag_type = 0;
static uint8_t ret_tag_id_length = 0;
static uint8_t ret_tag_id[32];

void example_loop(TF_HalContext *hal) {
	// Poll for callbacks
	// Polling with 0 will process one packet at most, so we can't miss a state change.
	tf_hal_callback_tick(hal, 0);
	if (!valid) {
		return;
	}

	valid = false;

	if(nfc_state == TF_NFC_READER_STATE_IDLE) {
		tf_nfc_reader_request_tag_id(&nfc);
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_TAG_ID_READY) {
		ret = tf_nfc_reader_get_tag_id(&nfc, &ret_tag_type, ret_tag_id, &ret_tag_id_length);

		if(ret != TF_E_OK) {
			return;
		}

		tf_hal_printf("Found tag of type %I8d with ID [", ret_tag_type);

		for(uint8_t i = 0; i < ret_tag_id_length; i++) {
			tf_hal_printf("%I8X", ret_tag_id[i]);
		}
		tf_hal_printf("]\n");
	}
	else if(nfc_state == TF_NFC_READER_STATE_REQUEST_TAG_ID_ERROR) {
		tf_hal_printf("Request tag ID error\n");
	}
}
