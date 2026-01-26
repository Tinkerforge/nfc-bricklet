/* nfc-bricklet
 * Copyright (C) 2021 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150_simple.h: Driver for PN7150 NFC simple ID discovery mode
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

#ifndef PN7150_SIMPLE_H
#define PN7150_SIMPLE_H

#include <stdint.h>
#include <stdbool.h>

#include "Nfc.h"

#define SIMPLE_TAGS_NUM 8
#define SIMPLE_TAG_ID_MAX_LENGTH 10

typedef struct {
	uint8_t type;
	uint8_t id_length;
	uint8_t id[SIMPLE_TAG_ID_MAX_LENGTH];
	uint32_t last_seen;
} SimpleTag;

bool pn7150_simple_discovery(uint8_t *discovery_technologies, NxpNci_RfIntf_t *interface);
bool pn7150_simple_request_tag_id(NxpNci_RfIntf_t *interface);
bool pn7150_simple_try_read_phone_id(void);
void pn7150_simple_state_machine(void);

#endif