// This example is not self-contained.
// It requires usage of the example driver specific to your platform.
// See the HAL documentation.

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "src/bindings/hal_common.h"
#include "src/bindings/bricklet_nfc.h"

void check(int rc, const char *msg);
void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);

static const char *ndef_uri = "www.tinkerforge.com";

static uint8_t nfc_state;
static bool nfc_idle;
static bool valid = false;

// Callback function for cardemu state changed callback
static void cardemu_state_changed_handler(TF_NFC *device, uint8_t state, bool idle,
                                          void *user_data) {
	(void)device; (void)user_data; // avoid unused parameter warning

	nfc_state = state;
	nfc_idle = idle;
	valid = true;
}

static TF_NFC nfc;

static char ndef_record_uri[255] = {0};
static uint8_t ndef_record_size = 0;

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_nfc_create(&nfc, NULL, hal), "create device object");

	// Register cardemu state changed callback to function cardemu_state_changed_handler
	tf_nfc_register_cardemu_state_changed_callback(&nfc,
	                                               cardemu_state_changed_handler,
	                                               NULL);

	// Enable cardemu mode
	check(tf_nfc_set_mode(&nfc, TF_NFC_MODE_CARDEMU), "call set_mode");

	// Prepare ndef record buffer
	uint8_t header_size = 5;
	uint8_t uri_len = (uint8_t)(strlen(ndef_uri)) + 1; // + 1 for the null terminator
	ndef_record_size = header_size + uri_len;

	// Only short records are supported
	ndef_record_uri[0] = (char) 0xD1;
	ndef_record_uri[1] = 0x01;
	ndef_record_uri[2] = (char) uri_len;
	ndef_record_uri[3] = 'U';
	ndef_record_uri[4] = 0x04;

	snprintf(ndef_record_uri + header_size, 255 - header_size, "%s", ndef_uri);
}

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	// Polling with 0 will process one packet at most, so we can't miss a state change.
	tf_hal_callback_tick(hal, 0);

	if (!valid) {
		return;
	}

	valid = false;

	if(nfc_state == TF_NFC_CARDEMU_STATE_IDLE) {
		tf_nfc_cardemu_write_ndef(&nfc, (uint8_t *)ndef_record_uri, ndef_record_size);
		tf_nfc_cardemu_start_discovery(&nfc);
	}
	else if(nfc_state == TF_NFC_CARDEMU_STATE_DISCOVER_READY) {
		tf_nfc_cardemu_start_transfer(&nfc, TF_NFC_CARDEMU_TRANSFER_WRITE);
	}
	else if(nfc_state == TF_NFC_CARDEMU_STATE_DISCOVER_ERROR) {
		tf_hal_printf("Discover error\n");
	}
	else if(nfc_state == TF_NFC_CARDEMU_STATE_TRANSFER_NDEF_ERROR) {
		tf_hal_printf("Transfer NDEF error\n");
	}
}
