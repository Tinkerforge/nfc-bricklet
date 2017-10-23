/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/protocols/tfp/tfp.h"

#include "configs/config_pn7150.h"
#include "pn7150.h"
#include "pn7150_cardemu.h"
#include "pn7150_p2p.h"
#include "pn7150_reader.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_SET_MODE: return set_mode(message);
		case FID_GET_MODE: return get_mode(message, response);
		case FID_READER_REQUEST_TAG_ID: return reader_request_tag_id(message);
		case FID_READER_GET_TAG_ID_LOW_LEVEL: return reader_get_tag_id_low_level(message, response);
		case FID_READER_GET_STATE: return reader_get_state(message, response);
		case FID_READER_WRITE_NDEF_LOW_LEVEL: return reader_write_ndef_low_level(message);
		case FID_READER_REQUEST_NDEF: return reader_request_ndef(message);
		case FID_READER_READ_NDEF_LOW_LEVEL: return reader_read_ndef_low_level(message, response);
		case FID_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE: return reader_authenticate_mifare_classic_page(message);
		case FID_READER_WRITE_PAGE_LOW_LEVEL: return reader_write_page_low_level(message);
		case FID_READER_REQUEST_PAGE: return reader_request_page(message);
		case FID_READER_READ_PAGE_LOW_LEVEL: return reader_read_page_low_level(message, response);
		case FID_CARDEMU_GET_STATE: return cardemu_get_state(message, response);
		case FID_CARDEMU_START_DISCOVERY: return cardemu_start_discovery(message);
		case FID_CARDEMU_WRITE_NDEF_LOW_LEVEL: return cardemu_write_ndef_low_level(message);
		case FID_CARDEMU_START_TRANSFER: return cardemu_start_transfer(message);
		case FID_P2P_GET_STATE: return p2p_get_state(message, response);
		case FID_P2P_START_DISCOVERY: return p2p_start_discovery(message);
		case FID_P2P_WRITE_NDEF_LOW_LEVEL: return p2p_write_ndef_low_level(message);
		case FID_P2P_START_TRANSFER: return p2p_start_transfer(message);
		case FID_P2P_READ_NDEF_LOW_LEVEL: return p2p_read_ndef_low_level(message, response);
		case FID_SET_DETECTION_LED_CONFIG: return set_detection_led_config(message);
		case FID_GET_DETECTION_LED_CONFIG: return get_detection_led_config(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


extern PN7150 pn7150;

BootloaderHandleMessageResponse set_mode(const SetMode *data) {
	if(data->mode > NFC_MODE_READER) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	// Changing mode always means going back to idle state
	pn7150.mode          = data->mode;
	pn7150.new_mode      = true;

	pn7150.reader_state  = NFC_READER_STATE_INITIALIZATION;
	pn7150.cardemu_state = NFC_CARDEMU_STATE_INITIALIZATION;
	pn7150.p2p_state     = NFC_P2P_STATE_INITIALIZATION;

    // TODO: Do we need to do something here to bring the nfc task out
    //       of the current state of the state machine?

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_mode(const GetMode *data, GetMode_Response *response) {
	response->header.length = sizeof(GetMode_Response);
	response->mode          = pn7150.mode;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse reader_request_tag_id(const ReaderRequestTagID *data) {
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		pn7150.reader_state = NFC_READER_STATE_REQUEST_TAG_ID;
		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_get_tag_id_low_level(const ReaderGetTagIDLowLevel *data, ReaderGetTagIDLowLevel_Response *response) {
	response->header.length = sizeof(ReaderGetTagIDLowLevel_Response);
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_REQUEST_TAG_ID_READY)) {
		response->tag_type = pn7150.reader_tag_type;
		response->tag_id_length = pn7150.reader_tag_id_length;
		memcpy(response->tag_id_data, pn7150.reader_tag_id, response->tag_id_length);
		memset(&response->tag_id_data[response->tag_id_length], 0, 32-response->tag_id_length);

		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_get_state(const ReaderGetState *data, ReaderGetState_Response *response) {
	response->header.length = sizeof(ReaderGetState_Response);
	response->state = pn7150.reader_state;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse reader_write_ndef_low_level(const ReaderWriteNdefLowLevel *data) {
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		uint8_t length = BETWEEN(0, ((int32_t)data->ndef_length) - ((int32_t)data->ndef_chunk_offset), 60);
		memcpy(pn7150.data + data->ndef_chunk_offset, data->ndef_chunk_data, length);

		if(data->ndef_chunk_offset + length >= data->ndef_length) {
			pn7150.reader_state = NFC_READER_STATE_WRITE_NDEF;
			pn7150.reader_ndef_length = data->ndef_length;

			// Fill end of data buffer with 0, so the write task does not have to
			// check for non-full pages
			memset(pn7150.data + pn7150.reader_ndef_length, 0, PN7150_MAX_OVERHEAD);

			pn7150_reader_update_ndef();
		}

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_request_ndef(const ReaderRequestNdef *data) {
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		pn7150.reader_state = NFC_READER_STATE_REQUEST_NDEF;

		pn7150.reader_ndef_length = 0;
		pn7150_reader_update_ndef();

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_read_ndef_low_level(const ReaderReadNdefLowLevel *data, ReaderReadNdefLowLevel_Response *response) {
	response->header.length = sizeof(ReaderReadNdefLowLevel_Response);

	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state == NFC_READER_STATE_REQUEST_NDEF_READY)) {
		uint8_t length = BETWEEN(0, ((int32_t)pn7150.data_length) - ((int32_t)pn7150.data_chunk_offset), 60);

		memcpy(response->ndef_chunk_data, pn7150.data + pn7150.data_chunk_offset, length);

		response->ndef_chunk_offset = pn7150.data_chunk_offset;
		response->ndef_length = pn7150.data_length;
		pn7150.data_chunk_offset += length;

		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_authenticate_mifare_classic_page(const ReaderAuthenticateMifareClassicPage *data) {
	if(data->key_number > NFC_KEY_B) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		pn7150.reader_mifare_auth_page = data->page;
		pn7150.reader_mifare_auth_key_number = data->key_number;
		memcpy(pn7150.reader_mifare_auth_key, data->key, 6);

		pn7150.reader_state = NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE;

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_write_page_low_level(const ReaderWritePageLowLevel *data) {
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		uint8_t length = BETWEEN(0, ((int32_t)data->data_length) - ((int32_t)data->data_chunk_offset), 58);
		memcpy(pn7150.data + data->data_chunk_offset, data->data_chunk_data, length);

		if(data->data_chunk_offset + length >= data->data_length) {
			pn7150.reader_state        = NFC_READER_STATE_WRITE_PAGE;
			pn7150.reader_write_length = data->data_length;
			pn7150.reader_write_page   = data->page;

			// Fill end of data buffer with 0, so the write task does not have to
			// check for non-full pages
			memset(pn7150.data + pn7150.reader_write_length, 0, PN7150_MAX_OVERHEAD);
		}

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_request_page(const ReaderRequestPage *data) {
	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state & NFC_READER_STATE_IDLE)) {
		pn7150.reader_state          = NFC_READER_STATE_REQUEST_PAGE;
		pn7150.reader_request_page   = data->page;
		pn7150.reader_request_length = data->length;
		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse reader_read_page_low_level(const ReaderReadPageLowLevel *data, ReaderReadPageLowLevel_Response *response) {
	response->header.length = sizeof(ReaderReadPageLowLevel_Response);

	if((pn7150.mode == NFC_MODE_READER) && (pn7150.reader_state == NFC_READER_STATE_REQUEST_PAGE_READY)) {
		uint8_t length = BETWEEN(0, ((int32_t)pn7150.data_length) - ((int32_t)pn7150.data_chunk_offset), 60);

		memcpy(response->data_chunk_data, pn7150.data + pn7150.data_chunk_offset, length);

		response->data_chunk_offset = pn7150.data_chunk_offset;
		response->data_length = pn7150.data_length;
		pn7150.data_chunk_offset += length;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse cardemu_get_state(const CardemuGetState *data, CardemuGetState_Response *response) {
	response->header.length = sizeof(CardemuGetState_Response);
	response->state         = pn7150.cardemu_state;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse cardemu_start_discovery(const CardemuStartDiscovery *data) {
	if((pn7150.mode == NFC_MODE_CARDEMU) && (pn7150.cardemu_state & NFC_CARDEMU_STATE_IDLE)) {
		pn7150.cardemu_state = NFC_CARDEMU_STATE_DISCOVER;

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse cardemu_write_ndef_low_level(const CardemuWriteNdefLowLevel *data) {
	if((pn7150.mode == NFC_MODE_CARDEMU) && (pn7150.cardemu_state & NFC_CARDEMU_STATE_IDLE)) {
		uint8_t length = BETWEEN(0, ((int32_t)data->ndef_length) - ((int32_t)data->ndef_chunk_offset), 60);
		memcpy(pn7150.data + data->ndef_chunk_offset, data->ndef_chunk_data, length);

		if(data->ndef_chunk_offset + length >= data->ndef_length) {
			pn7150.cardemu_ndef_length = data->ndef_length;

			// Fill end of data buffer with 0, so the write task does not have to
			// check for non-full pages
			memset(pn7150.data + pn7150.cardemu_ndef_length, 0, PN7150_MAX_OVERHEAD);

			pn7150_cardemu_update_ndef();
		}

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse cardemu_start_transfer(const CardemuStartTransfer *data) {
	if((pn7150.mode == NFC_MODE_CARDEMU) && (pn7150.cardemu_state == NFC_CARDEMU_STATE_DISCOVER_READY)) {
		switch(data->transfer) {
			case NFC_CARDEMU_TRANSFER_WRITE: pn7150.cardemu_state = NFC_CARDEMU_STATE_TRANSFER_NDEF; break;
			case NFC_P2P_STATE_IDLE:         pn7150.cardemu_state = NFC_CARDEMU_STATE_IDLE;          break;
			default: return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}


		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse p2p_get_state(const P2PGetState *data, P2PGetState_Response *response) {
	response->header.length = sizeof(P2PGetState_Response);
	response->state         = pn7150.p2p_state;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse p2p_start_discovery(const P2PStartDiscovery *data) {
	if((pn7150.mode == NFC_MODE_P2P) && (pn7150.p2p_state & NFC_P2P_STATE_IDLE)) {
		pn7150.p2p_state = NFC_P2P_STATE_DISCOVER;

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse p2p_write_ndef_low_level(const P2PWriteNdefLowLevel *data) {
	if((pn7150.mode == NFC_MODE_P2P) && (pn7150.p2p_state & NFC_P2P_STATE_IDLE)) {
		uint8_t length = BETWEEN(0, ((int32_t)data->ndef_length) - ((int32_t)data->ndef_chunk_offset), 60);
		memcpy(pn7150.data + data->ndef_chunk_offset, data->ndef_chunk_data, length);

		if(data->ndef_chunk_offset + length >= data->ndef_length) {
			pn7150.p2p_ndef_length = data->ndef_length;

			// Fill end of data buffer with 0, so the write task does not have to
			// check for non-full pages
			memset(pn7150.data + pn7150.p2p_ndef_length, 0, PN7150_MAX_OVERHEAD);

			pn7150_p2p_update_ndef();
		}

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse p2p_start_transfer(const P2PStartTransfer *data) {
	if((pn7150.mode == NFC_MODE_P2P) && (pn7150.p2p_state == NFC_P2P_STATE_DISCOVER_READY)) {
		switch(data->transfer) {
			case NFC_P2P_TRANSFER_READ:  pn7150.p2p_ndef_length = 0; pn7150_p2p_update_ndef(); // fall-through
			case NFC_P2P_TRANSFER_WRITE: pn7150.p2p_state = NFC_P2P_STATE_TRANSFER_NDEF; break;
			case NFC_P2P_STATE_IDLE:     pn7150.p2p_state = NFC_P2P_STATE_IDLE;          break;
			default: return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
		}

		return HANDLE_MESSAGE_RESPONSE_EMPTY;
	}
	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse p2p_read_ndef_low_level(const P2PReadNdefLowLevel *data, P2PReadNdefLowLevel_Response *response) {
	response->header.length = sizeof(P2PReadNdefLowLevel_Response);

	if((pn7150.mode == NFC_MODE_P2P) && (pn7150.p2p_state == NFC_P2P_STATE_TRANSFER_NDEF_READY) && (pn7150.p2p_ndef_length == 0)) {
		uint8_t length = BETWEEN(0, ((int32_t)pn7150.data_length) - ((int32_t)pn7150.data_chunk_offset), 60);

		memcpy(response->ndef_chunk_data, pn7150.data + pn7150.data_chunk_offset, length);

		response->ndef_chunk_offset = pn7150.data_chunk_offset;
		response->ndef_length = pn7150.data_length;
		pn7150.data_chunk_offset += length;
		return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
	}

	return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
}

BootloaderHandleMessageResponse set_detection_led_config(const SetDetectionLEDConfig *data) {
	if(data->config > NFC_DETECTION_LED_CONFIG_SHOW_DETECTION) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	pn7150.detection_led_state.config = data->config;

	// Set LED according to value
	if(pn7150.detection_led_state.config == NFC_DETECTION_LED_CONFIG_OFF) {
		XMC_GPIO_SetOutputHigh(PN7150_DETECT_LED_PIN);
	} else {
		XMC_GPIO_SetOutputLow(PN7150_DETECT_LED_PIN);
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_detection_led_config(const GetDetectionLEDConfig *data, GetDetectionLEDConfig_Response *response) {
	response->header.length = sizeof(GetDetectionLEDConfig_Response);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

bool handle_reader_state_changed_callback(void) {
	static bool is_buffered = false;
	static ReaderStateChanged_Callback cb;
	static uint8_t last_state = NFC_READER_STATE_INITIALIZATION;

	if(!is_buffered) {
		if(last_state != pn7150.reader_state) {
			last_state = pn7150.reader_state;
			tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(ReaderStateChanged_Callback), FID_CALLBACK_READER_STATE_CHANGED);
			cb.state = last_state;
			cb.idle = last_state & NFC_READER_STATE_IDLE;
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(ReaderStateChanged_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_cardemu_state_changed_callback(void) {
	static bool is_buffered = false;
	static CardemuStateChanged_Callback cb;
	static uint8_t last_state = NFC_CARDEMU_STATE_INITIALIZATION;

	if(!is_buffered) {
		if(last_state != pn7150.cardemu_state) {
			last_state = pn7150.cardemu_state;
			tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(CardemuStateChanged_Callback), FID_CALLBACK_CARDEMU_STATE_CHANGED);
			cb.state = last_state;
			cb.idle = last_state & NFC_READER_STATE_IDLE;
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(CardemuStateChanged_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_p2p_state_changed_callback(void) {
	static bool is_buffered = false;
	static P2PStateChanged_Callback cb;
	static uint8_t last_state = NFC_P2P_STATE_INITIALIZATION;

	if(!is_buffered) {
		if(last_state != pn7150.p2p_state) {
			last_state = pn7150.p2p_state;
			tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(P2PStateChanged_Callback), FID_CALLBACK_P2P_STATE_CHANGED);
			cb.state = last_state;
			cb.idle = last_state & NFC_P2P_STATE_IDLE;
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(P2PStateChanged_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
