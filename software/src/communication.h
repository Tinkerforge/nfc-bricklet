/* nfc-bricklet
 * Copyright (C) 2018, 2021, 2024 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants

#define NFC_MODE_OFF 0
#define NFC_MODE_CARDEMU 1
#define NFC_MODE_P2P 2
#define NFC_MODE_READER 3
#define NFC_MODE_SIMPLE 4

#define NFC_TAG_TYPE_MIFARE_CLASSIC 0
#define NFC_TAG_TYPE_TYPE1 1
#define NFC_TAG_TYPE_TYPE2 2
#define NFC_TAG_TYPE_TYPE3 3
#define NFC_TAG_TYPE_TYPE4 4
#define NFC_TAG_TYPE_TYPE5 5 // ISO 15693

#define NFC_READER_STATE_INITIALIZATION 0
#define NFC_READER_STATE_IDLE 128
#define NFC_READER_STATE_ERROR 192
#define NFC_READER_STATE_REQUEST_TAG_ID 2
#define NFC_READER_STATE_REQUEST_TAG_ID_READY 130
#define NFC_READER_STATE_REQUEST_TAG_ID_ERROR 194
#define NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE 3
#define NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_READY 131
#define NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_ERROR 195
#define NFC_READER_STATE_WRITE_PAGE 4
#define NFC_READER_STATE_WRITE_PAGE_READY 132
#define NFC_READER_STATE_WRITE_PAGE_ERROR 196
#define NFC_READER_STATE_REQUEST_PAGE 5
#define NFC_READER_STATE_REQUEST_PAGE_READY 133
#define NFC_READER_STATE_REQUEST_PAGE_ERROR 197
#define NFC_READER_STATE_WRITE_NDEF 6
#define NFC_READER_STATE_WRITE_NDEF_READY 134
#define NFC_READER_STATE_WRITE_NDEF_ERROR 198
#define NFC_READER_STATE_REQUEST_NDEF 7
#define NFC_READER_STATE_REQUEST_NDEF_READY 135
#define NFC_READER_STATE_REQUEST_NDEF_ERROR 199

#define NFC_KEY_A 0
#define NFC_KEY_B 1

#define NFC_READER_WRITE_TYPE4_CAPABILITY_CONTAINER 3
#define NFC_READER_WRITE_TYPE4_NDEF 4

#define NFC_READER_REQUEST_TYPE4_CAPABILITY_CONTAINER 3
#define NFC_READER_REQUEST_TYPE4_NDEF 4

#define NFC_CARDEMU_STATE_INITIALIZATION 0
#define NFC_CARDEMU_STATE_IDLE 128
#define NFC_CARDEMU_STATE_ERROR 192
#define NFC_CARDEMU_STATE_DISCOVER 2
#define NFC_CARDEMU_STATE_DISCOVER_READY 130
#define NFC_CARDEMU_STATE_DISCOVER_ERROR 194
#define NFC_CARDEMU_STATE_TRANSFER_NDEF 3
#define NFC_CARDEMU_STATE_TRANSFER_NDEF_READY 131
#define NFC_CARDEMU_STATE_TRANSFER_NDEF_ERROR 195

#define NFC_CARDEMU_TRANSFER_ABORT 0
#define NFC_CARDEMU_TRANSFER_WRITE 1

#define NFC_P2P_STATE_INITIALIZATION 0
#define NFC_P2P_STATE_IDLE 128
#define NFC_P2P_STATE_ERROR 192
#define NFC_P2P_STATE_DISCOVER 2
#define NFC_P2P_STATE_DISCOVER_READY 130
#define NFC_P2P_STATE_DISCOVER_ERROR 194
#define NFC_P2P_STATE_TRANSFER_NDEF 3
#define NFC_P2P_STATE_TRANSFER_NDEF_READY 131
#define NFC_P2P_STATE_TRANSFER_NDEF_ERROR 195

#define NFC_P2P_TRANSFER_ABORT 0
#define NFC_P2P_TRANSFER_WRITE 1
#define NFC_P2P_TRANSFER_READ 2

#define NFC_DETECTION_LED_CONFIG_OFF 0
#define NFC_DETECTION_LED_CONFIG_ON 1
#define NFC_DETECTION_LED_CONFIG_SHOW_HEARTBEAT 2
#define NFC_DETECTION_LED_CONFIG_SHOW_DETECTION 3

#define NFC_BOOTLOADER_MODE_BOOTLOADER 0
#define NFC_BOOTLOADER_MODE_FIRMWARE 1
#define NFC_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define NFC_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define NFC_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define NFC_BOOTLOADER_STATUS_OK 0
#define NFC_BOOTLOADER_STATUS_INVALID_MODE 1
#define NFC_BOOTLOADER_STATUS_NO_CHANGE 2
#define NFC_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define NFC_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define NFC_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define NFC_STATUS_LED_CONFIG_OFF 0
#define NFC_STATUS_LED_CONFIG_ON 1
#define NFC_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define NFC_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_SET_MODE 1
#define FID_GET_MODE 2
#define FID_READER_REQUEST_TAG_ID 3
#define FID_READER_GET_TAG_ID_LOW_LEVEL 4
#define FID_READER_GET_STATE 5
#define FID_READER_WRITE_NDEF_LOW_LEVEL 6
#define FID_READER_REQUEST_NDEF 7
#define FID_READER_READ_NDEF_LOW_LEVEL 8
#define FID_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE 9
#define FID_READER_WRITE_PAGE_LOW_LEVEL 10
#define FID_READER_REQUEST_PAGE 11
#define FID_READER_READ_PAGE_LOW_LEVEL 12
#define FID_CARDEMU_GET_STATE 14
#define FID_CARDEMU_START_DISCOVERY 15
#define FID_CARDEMU_WRITE_NDEF_LOW_LEVEL 16
#define FID_CARDEMU_START_TRANSFER 17
#define FID_P2P_GET_STATE 19
#define FID_P2P_START_DISCOVERY 20
#define FID_P2P_WRITE_NDEF_LOW_LEVEL 21
#define FID_P2P_START_TRANSFER 22
#define FID_P2P_READ_NDEF_LOW_LEVEL 23
#define FID_SET_DETECTION_LED_CONFIG 25
#define FID_GET_DETECTION_LED_CONFIG 26
#define FID_SET_MAXIMUM_TIMEOUT 27
#define FID_GET_MAXIMUM_TIMEOUT 28
#define FID_SIMPLE_GET_TAG_ID_LOW_LEVEL 29
#define FID_CARDEMU_SET_TAG_ID 30
#define FID_CARDEMU_GET_TAG_ID 31

#define FID_CALLBACK_READER_STATE_CHANGED 13
#define FID_CALLBACK_CARDEMU_STATE_CHANGED 18
#define FID_CALLBACK_P2P_STATE_CHANGED 24

typedef struct {
	TFPMessageHeader header;
	uint8_t mode;
} __attribute__((__packed__)) SetMode;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetMode;

typedef struct {
	TFPMessageHeader header;
	uint8_t mode;
} __attribute__((__packed__)) GetMode_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderRequestTagID;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderGetTagIDLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t tag_type;
	uint8_t tag_id_length;
	uint8_t tag_id_data[32];
} __attribute__((__packed__)) ReaderGetTagIDLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderGetState;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) ReaderGetState_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t ndef_length;
	uint16_t ndef_chunk_offset;
	uint8_t ndef_chunk_data[60];
} __attribute__((__packed__)) ReaderWriteNDEFLowLevel;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderRequestNDEF;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderReadNDEFLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t ndef_length;
	uint16_t ndef_chunk_offset;
	uint8_t ndef_chunk_data[60];
} __attribute__((__packed__)) ReaderReadNDEFLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t page;
	uint8_t key_number;
	uint8_t key[6];
} __attribute__((__packed__)) ReaderAuthenticateMifareClassicPage;

typedef struct {
	TFPMessageHeader header;
	uint16_t page;
	uint16_t data_length;
	uint16_t data_chunk_offset;
	uint8_t data_chunk_data[58];
} __attribute__((__packed__)) ReaderWritePageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t page;
	uint16_t length;
} __attribute__((__packed__)) ReaderRequestPage;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ReaderReadPageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t data_length;
	uint16_t data_chunk_offset;
	uint8_t data_chunk_data[60];
} __attribute__((__packed__)) ReaderReadPageLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) ReaderStateChanged_Callback;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) CardemuGetState;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) CardemuGetState_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) CardemuStartDiscovery;

typedef struct {
	TFPMessageHeader header;
	uint16_t ndef_length;
	uint16_t ndef_chunk_offset;
	uint8_t ndef_chunk_data[60];
} __attribute__((__packed__)) CardemuWriteNDEFLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t transfer;
} __attribute__((__packed__)) CardemuStartTransfer;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) CardemuStateChanged_Callback;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) P2PGetState;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) P2PGetState_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) P2PStartDiscovery;

typedef struct {
	TFPMessageHeader header;
	uint16_t ndef_length;
	uint16_t ndef_chunk_offset;
	uint8_t ndef_chunk_data[60];
} __attribute__((__packed__)) P2PWriteNDEFLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t transfer;
} __attribute__((__packed__)) P2PStartTransfer;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) P2PReadNDEFLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t ndef_length;
	uint16_t ndef_chunk_offset;
	uint8_t ndef_chunk_data[60];
} __attribute__((__packed__)) P2PReadNDEFLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t state;
	bool idle;
} __attribute__((__packed__)) P2PStateChanged_Callback;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) SetDetectionLEDConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetDetectionLEDConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) GetDetectionLEDConfig_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t timeout;
} __attribute__((__packed__)) SetMaximumTimeout;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetMaximumTimeout;

typedef struct {
	TFPMessageHeader header;
	uint16_t timeout;
} __attribute__((__packed__)) GetMaximumTimeout_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t index;
} __attribute__((__packed__)) SimpleGetTagIDLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint8_t tag_type;
	uint8_t tag_id_length;
	uint8_t tag_id_data[10];
	uint32_t last_seen;
} __attribute__((__packed__)) SimpleGetTagIDLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t tag_id_length;
	uint8_t tag_id_data[7];
} __attribute__((__packed__)) CardemuSetTagID;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) CardemuGetTagID;

typedef struct {
	TFPMessageHeader header;
	uint8_t tag_id_length;
	uint8_t tag_id_data[7];
} __attribute__((__packed__)) CardemuGetTagID_Response;


// Function prototypes
BootloaderHandleMessageResponse set_mode(const SetMode *data);
BootloaderHandleMessageResponse get_mode(const GetMode *data, GetMode_Response *response);
BootloaderHandleMessageResponse reader_request_tag_id(const ReaderRequestTagID *data);
BootloaderHandleMessageResponse reader_get_tag_id_low_level(const ReaderGetTagIDLowLevel *data, ReaderGetTagIDLowLevel_Response *response);
BootloaderHandleMessageResponse reader_get_state(const ReaderGetState *data, ReaderGetState_Response *response);
BootloaderHandleMessageResponse reader_write_ndef_low_level(const ReaderWriteNDEFLowLevel *data);
BootloaderHandleMessageResponse reader_request_ndef(const ReaderRequestNDEF *data);
BootloaderHandleMessageResponse reader_read_ndef_low_level(const ReaderReadNDEFLowLevel *data, ReaderReadNDEFLowLevel_Response *response);
BootloaderHandleMessageResponse reader_authenticate_mifare_classic_page(const ReaderAuthenticateMifareClassicPage *data);
BootloaderHandleMessageResponse reader_write_page_low_level(const ReaderWritePageLowLevel *data);
BootloaderHandleMessageResponse reader_request_page(const ReaderRequestPage *data);
BootloaderHandleMessageResponse reader_read_page_low_level(const ReaderReadPageLowLevel *data, ReaderReadPageLowLevel_Response *response);
BootloaderHandleMessageResponse cardemu_get_state(const CardemuGetState *data, CardemuGetState_Response *response);
BootloaderHandleMessageResponse cardemu_start_discovery(const CardemuStartDiscovery *data);
BootloaderHandleMessageResponse cardemu_write_ndef_low_level(const CardemuWriteNDEFLowLevel *data);
BootloaderHandleMessageResponse cardemu_start_transfer(const CardemuStartTransfer *data);
BootloaderHandleMessageResponse p2p_get_state(const P2PGetState *data, P2PGetState_Response *response);
BootloaderHandleMessageResponse p2p_start_discovery(const P2PStartDiscovery *data);
BootloaderHandleMessageResponse p2p_write_ndef_low_level(const P2PWriteNDEFLowLevel *data);
BootloaderHandleMessageResponse p2p_start_transfer(const P2PStartTransfer *data);
BootloaderHandleMessageResponse p2p_read_ndef_low_level(const P2PReadNDEFLowLevel *data, P2PReadNDEFLowLevel_Response *response);
BootloaderHandleMessageResponse set_detection_led_config(const SetDetectionLEDConfig *data);
BootloaderHandleMessageResponse get_detection_led_config(const GetDetectionLEDConfig *data, GetDetectionLEDConfig_Response *response);
BootloaderHandleMessageResponse set_maximum_timeout(const SetMaximumTimeout *data);
BootloaderHandleMessageResponse get_maximum_timeout(const GetMaximumTimeout *data, GetMaximumTimeout_Response *response);
BootloaderHandleMessageResponse simple_get_tag_id_low_level(const SimpleGetTagIDLowLevel *data, SimpleGetTagIDLowLevel_Response *response);
BootloaderHandleMessageResponse cardemu_set_tag_id(const CardemuSetTagID *data);
BootloaderHandleMessageResponse cardemu_get_tag_id(const CardemuGetTagID *data, CardemuGetTagID_Response *response);

// Callbacks
bool handle_reader_state_changed_callback(void);
bool handle_cardemu_state_changed_callback(void);
bool handle_p2p_state_changed_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 3
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_reader_state_changed_callback, \
	handle_cardemu_state_changed_callback, \
	handle_p2p_state_changed_callback, \


#endif
