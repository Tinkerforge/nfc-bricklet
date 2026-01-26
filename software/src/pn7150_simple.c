/* nfc-bricklet
 * Copyright (C) 2021 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150_simple.c: Driver for PN7150 NFC simple ID discovery mode
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

#include "pn7150_simple.h"

#include <stdlib.h> // qsort

#include "bricklib2/utility/crc16.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/logging/logging.h"
#include "bricklib2/os/coop_task.h"

#include "pn7150.h"
#include "communication.h"
#include "NxpNci.h"

typedef struct {
	unsigned char IDm[8];
	unsigned char BlkNb;
	unsigned short Ptr;
	unsigned short Size;
	unsigned char *p;
} RW_NDEF_T3T_Ndef_t;

bool NxpNci_T3TretrieveIDm(void);
extern RW_NDEF_T3T_Ndef_t RW_NDEF_T3T_Ndef;

extern PN7150 pn7150;

static NxpNci_RfIntf_t pn7150_simple_interface;
SimpleTag pn7150_simple_tags[SIMPLE_TAGS_NUM] = {
	{0, 0, {0}, 0}
};

volatile static uint32_t pn7150_sort_start_time = 0;


// Swap the specified amount of bytes between two memory areas.
static void memswap(void *ptr1, void *ptr2, size_t len) {
	char *a = (char *)ptr1, *aend = a + len, *b = (char *)ptr2, tmp;
	while(a < aend) {
		tmp = *a;
		*a = *b;
		*b = tmp;
		a++;
		b++;
	}
}

// From https://github.com/RedAndBlueEraser/c-sort
void sort_bubblesort(void *arr, size_t count, size_t elesize, int (*cmp)(const void *, const void *)) {
	char *ptrstart = (char *)arr;               // Pointer to start of array.
	char *ptrend = ptrstart + count * elesize;  // Pointer to end of unsorted portion of array.
	char *ptr1;                                 // Pointer to first element to be compared.
	char *ptr2;                                 // Pointer to second element to be compared.
	char *ptrlastswap;                          // Pointer to furthest element that was last swapped.

	// Iterate until the entire array is sorted. After the n'th iteration, at
	// least the n largest elements are correctly positioned at the end of the
	// array. Only the unsorted portion of the array is traversed through.
	while(ptrstart < ptrend) {
		// Store the furthest element that is swapped.
		ptrlastswap = ptrstart;

		// Step through each pair of elements from the start of the array to the
		// end of the unsorted portion of the array, compare them, and swap them
		// if out of order.
		for(ptr1 = ptrstart, ptr2 = ptr1 + elesize; ptr2 < ptrend; ptr1 = ptr2, ptr2 += elesize) {
			if(cmp(ptr1, ptr2) > 0) {
				memswap(ptr1, ptr2, elesize);
				ptrlastswap = ptr2;
			}
		}

		// Mark the furthest element that was last swapped as the end of the
		// unsorted portion of the array.
		ptrend = ptrlastswap;
	}
}

bool pn7150_simple_discovery(uint8_t *discovery_technologies, NxpNci_RfIntf_t *interface) {
	if(NxpNci_StartDiscovery(discovery_technologies, sizeof(discovery_technologies)) != NFC_SUCCESS) {
		// Discovery failed. Perhaps there was still a discovery running? Lets stop and try again!
		NxpNci_StopDiscovery();
		memset(interface, 0, sizeof(NxpNci_RfIntf_t));
		if(NxpNci_StartDiscovery(discovery_technologies, sizeof(discovery_technologies)) != NFC_SUCCESS) {
			pn7150_init_nfc();
			return false;
		}
	}

	if(NxpNci_WaitForDiscoveryNotification(interface) != NFC_SUCCESS) {
		return false;
	}

	return true;
}

// -- Some notes regarding NFC technology, tag type and protocol --

// Mapping between technology, standards and tag types
// Technology   Standard             Tags
// NFCA         ISO 14443-3A         supported by Type 1, Type 2, Type 4
// NFCB         ISO 14443-3B         supported by Type 4
// NFCF         JIS 6319-4[FeliCa]   supported by Type 3
// NFCV         ISO 15693            supported by Type 5
// ISODEP       ISO 14443-4          supported by Type 4 (14443-4 defines the protocol for authentication/encryption)
// MIFARE       Not standardized     supported by Mifare Classic
//
// Hint: The mapping above is done this way in most libraries (e.g. NfcLibrary, Android, etc).
//       This is a bit strange, since the standards that the technology is mapped to are
//       not all on the same abstraction layer (see below).

// Mapping tag types with standards through all layers
// Tag      Example                   Application Protocol   Protocol       Anticollision   RF             Physical
// Type 1   Topaz (Innovsion)         -                      Topaz          -               ISO 14443-2A   ISO 14443-1A
// Type 2   MIFARE Ultralight (NXP)   -                      MIFARE UL      ISO 14443-3A    ISO 14443-2A   ISO 14443-1A
// Type 3   FeliCa (Sony)             JIS 6319-4             JIS 6319-4     JIS 6319-4      JIS 6319-4     JIS 6319-4
// Type 4   MIFARE DESFire (NXP)      ISO 7861-4 (APDUs)     ISO 14443-4A   ISO 14443-3A    ISO 14443-2A   ISO 14443-1A
// Type 4   SLE66CL160S (Infineon)    ISO 7861-4 (APDUs)     ISO 14443-4B   ISO 14443-3B    ISO 14443-2B   ISO 14443-1B
// Type 5   ICODE SLIX (NXP)          ISO 15693              ISO 15693      ISO 15693       ISO 15693      ISO 15693
// Mifare   MIFARE Classic (NXP)      -                      MIFARE         ISO 14443-3A    ISO 14443-2A   ISO 14443-1A
//          P2P                       -                      LLCP           ISO 18092       ISO 18092      ISO 18092
//
// Hint: On the application layer all tags, except Type 3, support NDEF.

bool pn7150_simple_request_tag_id(NxpNci_RfIntf_t *interface) {
	uint8_t discovery_technologies[] = {
		MODE_POLL | TECH_PASSIVE_NFCA,
		MODE_POLL | TECH_PASSIVE_NFCF,
		MODE_POLL | TECH_PASSIVE_NFCB,
		MODE_POLL | TECH_PASSIVE_15693
	};

	memset(interface, 0, sizeof(NxpNci_RfIntf_t));
	if(NxpNci_ReaderActivateNext(interface) == NFC_ERROR) {
		memset(interface, 0, sizeof(NxpNci_RfIntf_t));
		if(!pn7150_simple_discovery(discovery_technologies, interface)) {
			return false;
		}
	}

	pn7150.led_state_change_time = system_timer_get_ms();

	if((interface->ModeTech & MODE_MASK) == MODE_POLL) {

		// If mode is POLL and tech is NFCA we know where the uid is.
		// Note that it is possible for Protocol to be ISODEP, but the tech not to be NFCA.
		// So we can't do the copy below in the cases
		if(interface->ModeTech == (MODE_POLL | TECH_PASSIVE_NFCA)) {
			pn7150.simple_tag_id_length = interface->Info.NFC_APP.NfcIdLen;
			memcpy(pn7150.simple_tag_id, interface->Info.NFC_APP.NfcId, 10);
		}

		switch(interface->Protocol) {
			case PROT_T1T: {
				pn7150.simple_tag_type = NFC_TAG_TYPE_TYPE1;
				// If Type 1 Tag is detected there is no identifier in the discovery response
				// (see NCI specification: 7.1 Starting RF Discovery, table 54)

				// We use Read Identifier command instead (see Digital Protocol Specification: 8.6)
				uint8_t t1t_rid[] = {0x78,0x00,0x00,0x00,0x00,0x00,0x00};
				uint8_t response[256];
				uint8_t length;
				bool status = NxpNci_ReaderTagCmd(t1t_rid, sizeof(t1t_rid), response, &length);
				if((status == NFC_ERROR) || (response[length - 1] != 0)) {
					return false;
				}

				pn7150.simple_tag_id_length = 4;
				memcpy(pn7150.simple_tag_id, &response[2], 4);

				break;
			}

			case PROT_T2T: {
				pn7150.simple_tag_type = NFC_TAG_TYPE_TYPE2;
				break;
			}

			case PROT_T3T: {
				pn7150.simple_tag_type = NFC_TAG_TYPE_TYPE3;
				if(NxpNci_T3TretrieveIDm() == NFC_ERROR) {
					return false;
				}

				pn7150.simple_tag_id_length = 8;
				memcpy(pn7150.simple_tag_id, RW_NDEF_T3T_Ndef.IDm, 8);
				break;
			}

			case PROT_ISODEP: {
				// Try to read phone ID. If successful, it is a compatible phone app
				if(pn7150_simple_try_read_phone_id()) {
					// Phone ID was read successfully, type is already set to NFC_TAG_TYPE_PHONE
					// and simple_tag_id contains the phone ID
					break;
				}

				// Not a compatible phone app, treat as regular Type 4 tag
				pn7150.simple_tag_type = NFC_TAG_TYPE_TYPE4;
				break;
			}

			case PROT_T5T: {
				pn7150.simple_tag_type = NFC_TAG_TYPE_TYPE5;
				if(interface->ModeTech == (MODE_POLL | TECH_PASSIVE_15693)) {
					// With ISO 15693 the ID is in NFC_VPP and the length is 8.
					pn7150.simple_tag_id_length = 8;
					memcpy(pn7150.simple_tag_id, interface->Info.NFC_VPP.ID, 8);
				}
				break;
			}

			case PROT_MIFARE: {
				pn7150.simple_tag_type = NFC_TAG_TYPE_MIFARE_CLASSIC;
				break;
			}

			default: {
				return false;
			}
		}
	} else {
		return false;
	}

	return true;
}

// Try to read phone ID from an ISO-DEP device using Tinkerforge NFC ID protocol.
//
// This should be called after detecting an ISO-DEP device (Type 4).
// If successful, updates pn7150.simple_tag_type to NFC_TAG_TYPE_PHONE
// and stores the first 10 bytes of the phone ID in pn7150.simple_tag_id.
//
// The phone sends 20 bytes, but we only use the first 10 (they contain
// enough entropy for unique identification).
//
// Returns true if this is a compatible phone app, false otherwise.

bool pn7150_simple_try_read_phone_id(void) {
	uint8_t response[256];
	uint8_t length;

	// Tinkerforge NFC ID AID: 54 46 4E 46 43 49 44 ("TFNFCID" in ASCII)
	// SELECT AID command: CLA=00, INS=A4, P1=04 (select by name), P2=00, Lc=07, AID, Le=00
	uint8_t select_aid[] = {0x00, 0xA4, 0x04, 0x00, 0x07,
	                        0x54, 0x46, 0x4E, 0x46, 0x43, 0x49, 0x44,
	                        0x00};

	// GET DATA command: CLA=00, INS=CA, P1=00, P2=01 (get device ID), Le=00
	uint8_t get_id[] = {0x00, 0xCA, 0x00, 0x01, 0x00};

	// Step 1: Select the application
	bool status = NxpNci_ReaderTagCmd(select_aid, sizeof(select_aid), response, &length);
	if(status == NFC_ERROR) {
		return false;
	}

	// Check for success (SW1=90, SW2=00)
	// Response format: [SW1] [SW2] (just 2 bytes for SELECT with no data)
	if(length < 2 || response[length - 2] != 0x90 || response[length - 1] != 0x00) {
		// AID not found - this is not a compatible phone app
		return false;
	}

	// Step 2: Get the device ID
	status = NxpNci_ReaderTagCmd(get_id, sizeof(get_id), response, &length);
	if(status == NFC_ERROR) {
		return false;
	}

	// Expected response: <20 bytes ID> [SW1=90] [SW2=00]
	// Total length: 20 + 2 = 22 bytes
	if(length < 22 || response[length - 2] != 0x90 || response[length - 1] != 0x00) {
		return false;
	}

	// Success!
	pn7150.simple_tag_type = NFC_TAG_TYPE_PHONE;
	pn7150.simple_tag_id_length = SIMPLE_TAG_ID_MAX_LENGTH;
	memcpy(pn7150.simple_tag_id, response, SIMPLE_TAG_ID_MAX_LENGTH);

	return true;
}

bool pn7150_simple_update_last_seen(void) {
	for(uint8_t i = 0; i < SIMPLE_TAGS_NUM; i++) {
		if((pn7150_simple_tags[i].type == pn7150.simple_tag_type) && (memcmp(pn7150_simple_tags[i].id, pn7150.simple_tag_id, pn7150.simple_tag_id_length) == 0)) {
			pn7150_simple_tags[i].last_seen = system_timer_get_ms();
			return true;
		}
	}

	return false;
}

void pn7150_delete_obsolete_tags(void) {
	uint32_t now = system_timer_get_ms();
	for(uint8_t i = 0; i < SIMPLE_TAGS_NUM; i++) {
		uint32_t t = now - pn7150_simple_tags[i].last_seen;
		if(t > 1000*60*60*24) { // delete tags after 24 hours
			memset(&pn7150_simple_tags[i], 0, sizeof(SimpleTag));
		}
	}
}

int pn7150_simple_tags_compare(const void *s1, const void *s2) {
	SimpleTag *tag1 = (SimpleTag *)s1;
	SimpleTag *tag2 = (SimpleTag *)s2;
	if((tag2->last_seen == 0) && (tag1->last_seen == 0)) {
		return 0;
	} else if(tag2->last_seen == 0) {
		return -1;
	} else if(tag1->last_seen == 0) {
		return 1;
	}
	const uint32_t t1_time = pn7150_sort_start_time - tag1->last_seen;
	const uint32_t t2_time = pn7150_sort_start_time - tag2->last_seen;
	return t1_time - t2_time;
}

void pn7150_simple_tags_resort(void) {
	pn7150_sort_start_time = system_timer_get_ms();
	// We don't have enough space for qsort :-(
	sort_bubblesort(pn7150_simple_tags, SIMPLE_TAGS_NUM, sizeof(SimpleTag), pn7150_simple_tags_compare);
}

void pn7150_simple_state_machine(void) {
	pn7150_delete_obsolete_tags();

	if(pn7150_simple_request_tag_id(&pn7150_simple_interface)) {
		if(pn7150.simple_tag_id_length > 0) {
			if(!pn7150_simple_update_last_seen()) {
				SimpleTag *oldest = &pn7150_simple_tags[SIMPLE_TAGS_NUM-1]; // the last one is always the oldest because of resorting
				memcpy(oldest->id, pn7150.simple_tag_id, pn7150.simple_tag_id_length);
				oldest->id_length = pn7150.simple_tag_id_length;
				oldest->type = pn7150.simple_tag_type;
				oldest->last_seen = system_timer_get_ms();
			}

			pn7150_simple_tags_resort();

			pn7150.simple_tag_id_length = 0;
			pn7150.simple_tag_type = 0;
			memset(pn7150.simple_tag_id, 0, SIMPLE_TAG_ID_MAX_LENGTH);
		}
	}
}
