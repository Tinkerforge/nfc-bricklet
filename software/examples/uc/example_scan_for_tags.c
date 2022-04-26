// This example is not self-contained.
// It requires usage of the example driver specific to your platform.
// See the HAL documentation.

#include "src/bindings/hal_common.h"
#include "src/bindings/bricklet_nfc.h"
#include "src/bindings/errors.h"

void check(int rc, const char *msg);
void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);

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

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_nfc_create(&nfc, NULL, hal), "create device object");

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

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	// Polling with 0 will process one packet at most, so we can't miss a state change.
	tf_hal_callback_tick(hal, 0);
	if (!valid) {
		return;
	}

	valid = false;

	if(nfc_state == TF_NFC_READER_STATE_REQUEST_TAG_ID_READY) {
		ret = tf_nfc_reader_get_tag_id(&nfc, &ret_tag_type, ret_tag_id, &ret_tag_id_length);

		if(ret == TF_E_OK) {
			tf_hal_printf("Found tag of type %I8d with ID [", ret_tag_type);

			for(uint8_t i = 0; i < ret_tag_id_length; i++) {
				tf_hal_printf("%I8X", ret_tag_id[i]);

				if (i < ret_tag_id_length - 1) {
					tf_hal_printf(" ");
				}
			}

			tf_hal_printf("]\n");
		}
	}

	if(nfc_idle) {
		tf_nfc_reader_request_tag_id(&nfc);
	}
}
