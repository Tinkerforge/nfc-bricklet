/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150.h: Driver for PN7150 NFC IC
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

#ifndef PN7150_H
#define PN7150_H

#include <stdint.h>
#include <stdbool.h>

#define PN7150_MAX_DATA_SIZE 4096

// Some of the read functions have data at the end of the read
// that we want to be able to write to the data array to save
// an unnecessary memcpy
#define PN7150_MAX_OVERHEAD 32

typedef struct {
	// General
	bool     new_mode;
	uint8_t  mode;
	uint8_t  data[PN7150_MAX_DATA_SIZE + PN7150_MAX_OVERHEAD];
	uint16_t data_length;
	uint16_t data_chunk_offset;

	// Reader
	uint8_t  reader_state;

	uint8_t  reader_tag_type;
	uint8_t  reader_tid_length;
	uint8_t  reader_tid[10];

	uint16_t reader_mifare_auth_page;
	uint8_t  reader_mifare_auth_key_number;
	uint8_t  reader_mifare_auth_key[6];

	uint16_t reader_request_page;
	uint16_t reader_request_length;

	uint16_t reader_write_page;
	uint16_t reader_write_length;

	// Cardemu
	uint8_t  cardemu_state;
	bool     cardemu_ndef_ready;
	uint16_t cardemu_ndef_length;

	// P2P
	uint8_t  p2p_state;
	bool     p2p_ndef_ready;
	uint16_t p2p_ndef_length;
} PN7150;

void pn7150_tick();
bool pn7150_init_nfc(void);

#endif
