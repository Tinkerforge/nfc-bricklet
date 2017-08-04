/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150_reader.c: Driver for PN7150 NFC IC reader mode
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

#include "pn7150_reader.h"

#include "bricklib2/hal/uartbb/uartbb.h"
#include "bricklib2/utility/crc16.h"

#include "pn7150.h"
#include "communication.h"
#include "NxpNci.h"

extern PN7150 pn7150;

static NxpNci_RfIntf_t pn7150_reader_interface;

void pn7150_reader_push_cb(unsigned char *ndef, unsigned short ndef_length) {
	uartbb_puts("pn7150_reader_push_cb: "); uartbb_putu(ndef_length); uartbb_putnl();
	pn7150.reader_state = NFC_READER_STATE_WRITE_NDEF_READY;
}

void pn7150_reader_pull_cb(unsigned char *ndef, unsigned short ndef_length) {
	memcpy(pn7150.data, ndef, ndef_length);
	pn7150.data_length = ndef_length;
	pn7150.data_chunk_offset = 0;
	pn7150.reader_state = NFC_READER_STATE_REQUEST_NDEF_READY;
	uartbb_puts("pn7150_reader_pull_cb: "); uartbb_putu(ndef_length); uartbb_putnl();
}

static void pn7150_reader_request_tag_id(void) {
	uint8_t discovery_technologies[] = {
		MODE_POLL | TECH_PASSIVE_NFCA,
		MODE_POLL | TECH_PASSIVE_NFCF,
		MODE_POLL | TECH_PASSIVE_NFCB,
		MODE_POLL | TECH_PASSIVE_15693
	};

//	uartbb_puts("NxpNci_StartDiscovery\n\r");
	if(NxpNci_StartDiscovery(discovery_technologies, sizeof(discovery_technologies)) != NFC_SUCCESS) {
		// Discovery failed. Perhaps there was still a discovery running? Lets stop and try again!
		NxpNci_StopDiscovery();
//		uartbb_puts("NxpNci_StartDiscovery 2\n\r");
		if(NxpNci_StartDiscovery(discovery_technologies, sizeof(discovery_technologies)) != NFC_SUCCESS) {
			uartbb_puts("Error: cannot start discovery\n\r");

			pn7150_init_nfc();
			return;
		}
	}

//	uartbb_puts("NxpNci_WaitForDiscoveryNotification\n\r");
	if(NxpNci_WaitForDiscoveryNotification(&pn7150_reader_interface) != NFC_SUCCESS) {
		uartbb_puts("Error: discovery timeout\n\r");
		return;
	}

//	uartbb_puts("rf_interface.ModeTech: "); uartbb_putu(pn7150_reader_interface.ModeTech); uartbb_putnl();
	if((pn7150_reader_interface.ModeTech & MODE_MASK) == MODE_POLL) {
		pn7150.reader_tid_length = pn7150_reader_interface.Info.NFC_APP.NfcIdLen;
		memcpy(pn7150.reader_tid, pn7150_reader_interface.Info.NFC_APP.NfcId, 10);

		switch(pn7150_reader_interface.Protocol) {
			case PROT_T1T: {
				pn7150.reader_tag_type = NFC_TAG_TYPE_TYPE1;
				// If Type 1 Tag is detected there is no identifier in the discovery response
				// (see NCI specification: 7.1 Starting RF Discovery, table 54)

				// We use Read Identifier command instead (see Digital Protocol Specification: 8.6)
				uint8_t t1t_rid[] = {0x78,0x00,0x00,0x00,0x00,0x00,0x00};
				uint8_t response[256];
				uint8_t length;
//				uartbb_puts("read id start\n\r");
				bool status = NxpNci_ReaderTagCmd(t1t_rid, sizeof(t1t_rid), response, &length);
				if((status == NFC_ERROR) || (response[length - 1] != 0)) {
					uartbb_puts("T1T Read block failed with error: "); uartbb_putu(response[length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_REQUEST_TAG_ID_ERROR;
					return;
				}

				pn7150.reader_tid_length  = 4;
				memcpy(pn7150.reader_tid, &response[2], 4);

				break;
			}

			case PROT_T2T: {
				pn7150.reader_tag_type = NFC_TAG_TYPE_TYPE2;
				break;
			}

			case PROT_MIFARE: {
				pn7150.reader_tag_type = NFC_TAG_TYPE_MIFARE_CLASSIC;
				break;
			}

			default: {
				pn7150.reader_state = NFC_READER_STATE_REQUEST_TAG_ID_ERROR;
				return;
			}
		}
//		uartbb_puts("Protocol: "); uartbb_putu(pn7150_reader_interface.Protocol); uartbb_putnl();
	} else {
		pn7150.reader_state = NFC_READER_STATE_REQUEST_TAG_ID_ERROR;
		return;
	}

	pn7150.reader_state = NFC_READER_STATE_REQUEST_TAG_ID_READY;
}

static void pn7150_reader_request_ndef(void) {
	switch(pn7150_reader_interface.Protocol) {
		case PROT_T1T:
		case PROT_T2T:
		case PROT_T3T:
		case PROT_ISODEP: {
			NxpNci_ProcessReaderMode(pn7150_reader_interface, READ_NDEF);
			if(pn7150.reader_state != NFC_READER_STATE_REQUEST_NDEF_READY) {
				pn7150.reader_state = NFC_READER_STATE_REQUEST_NDEF_ERROR;
			}
			break;
		}

		// Read NDEF not supported for Mifare Classic tags
		case PROT_MIFARE:
		default: {
			pn7150.reader_state = NFC_READER_STATE_REQUEST_NDEF_ERROR;
			break;
		}
	}
}

static void pn7150_reader_write_ndef(void) {
	switch(pn7150_reader_interface.Protocol) {
		case PROT_T2T:
		case PROT_ISODEP: {
			NxpNci_ProcessReaderMode(pn7150_reader_interface, WRITE_NDEF);
			if(pn7150.reader_state != NFC_READER_STATE_WRITE_NDEF_READY) {
				pn7150.reader_state = NFC_READER_STATE_WRITE_NDEF_ERROR;
			}
			break;
		}

		// write NDEF not supported for type 1, 3 and mifare tags
		case PROT_T1T:
		case PROT_T3T:
		case PROT_MIFARE:
		default: {
			pn7150.reader_state = NFC_READER_STATE_WRITE_NDEF_ERROR;
			break;
		}
	}
}

static void pn7150_reader_authenticate_mifare_classic_page(void) {
	switch(pn7150_reader_interface.Protocol) {
		// See NXP UM10936 7.1.8 MIFARE Classic REQs & RSPs, Table 38
		case PROT_MIFARE: {
			const uint8_t sector_address = pn7150.reader_mifare_auth_page/4;
			const uint8_t key_selector   = 0x10 | (pn7150.reader_mifare_auth_key_number == 0 ? 0 : 0x80);
			uint8_t auth[] = {0x40, sector_address, key_selector, pn7150.reader_mifare_auth_key[0], pn7150.reader_mifare_auth_key[1], pn7150.reader_mifare_auth_key[2], pn7150.reader_mifare_auth_key[3], pn7150.reader_mifare_auth_key[4], pn7150.reader_mifare_auth_key[5]};

			uint8_t length;
			uint8_t response[256];
			uartbb_puts("Mifare start auth\n\r");
			bool status = NxpNci_ReaderTagCmd(auth, sizeof(auth), response, &length);
			if((status == NFC_ERROR) || (response[length - 1] != 0)) {
				uartbb_puts("Mifare auth failed with error: "); uartbb_putu(response[length - 1]); uartbb_putnl();
				pn7150.reader_state = NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_ERROR;
				return;
			}

			pn7150.reader_state = NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_READY;
			break;
		}

		default: {
			pn7150.reader_state = NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_ERROR;
			break;
		}
	}
}


static void pn7150_reader_request_page(void) {
	switch(pn7150_reader_interface.Protocol) {
		case PROT_MIFARE: {
			uint8_t response[256];
			uint8_t length;
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_request_page;
			do {
				// See NFC Mifare Classic specification 10.2 MIFARE Read, Table 14
				uint8_t read[] = {0x10, 0x30, page};
				bool status = NxpNci_ReaderTagCmd(read, sizeof(read), response, &length);
				uartbb_puts("mifare read length: "); uartbb_putu(length); uartbb_putnl();
				uartbb_puts("mifare read 0: "); uartbb_putu(response[0]); uartbb_putnl();
				uartbb_puts("mifare read 1: "); uartbb_putu(response[1]); uartbb_putnl();
				uartbb_puts("mifare read 2: "); uartbb_putu(response[2]); uartbb_putnl();
				uartbb_puts("mifare read 3: "); uartbb_putu(response[3]); uartbb_putnl();
				if((status == NFC_ERROR) || (length != 18) || (response[length - 1] != 0)) {
					uartbb_puts("Mifare Read block failed with error: "); uartbb_putu(response[length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_ERROR;
					return;
				}

				memcpy(pn7150.data + offset, response+1, 16);

				page += 4;
				offset += 16;
			} while(offset < pn7150.reader_request_length);

			pn7150.data_length       = pn7150.reader_request_length;
			pn7150.data_chunk_offset = 0;

			pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_READY;
			break;
		}

		case PROT_T1T: {
			uint8_t length;
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_request_page;
			uint8_t response[256];

			do {
				if(page < 0xf) { // page num < 0xF are read with RALL (see Type1 spec 5.7)
					uint8_t rall[7] = {0x00, 0x00, 0x00, pn7150.reader_tid[0], pn7150.reader_tid[1], pn7150.reader_tid[2], pn7150.reader_tid[3]};

					uartbb_puts("t1t rall: "); uartbb_putu(page); uartbb_putnl();
					bool status = NxpNci_ReaderTagCmd(rall, sizeof(rall), response, &length);
					if((status == NFC_ERROR) || (response[length - 1] != 0)) {
						uartbb_puts("T1T Read block all failed with error: "); uartbb_putu(response[length - 1]); uartbb_putnl();
						pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_ERROR;
						return;
					}

					memcpy(pn7150.data, response + 2 + page*8, 8*(0xF - page));
					offset = 8*(0xF - page);
					page = 0xf;

				} else { // page num >= 0xF are read with READ8 (see Type1 spec 5.13)
					uint8_t read8[14] = {0x02, page, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, pn7150.reader_tid[0], pn7150.reader_tid[1], pn7150.reader_tid[2], pn7150.reader_tid[3]};

					uartbb_puts("t1t read8: "); uartbb_putu(page); uartbb_putnl();
					bool status = NxpNci_ReaderTagCmd(read8, sizeof(read8), response, &length);
					if((status == NFC_ERROR) || (response[length - 1] != 0)) {
						uartbb_puts("T1T Read block 8 failed with error: "); uartbb_putu(response[length - 1]); uartbb_putnl();
						pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_ERROR;
						return;
					}

					memcpy(&pn7150.data[offset], &response[1], 8);

					page += 1;
					offset += (length-1);
				}
			} while(offset < pn7150.reader_request_length);

			pn7150.data_length       = pn7150.reader_request_length;
			pn7150.data_chunk_offset = 0;

			pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_READY;
			break;
		}

		case PROT_T2T: {
			uint8_t length;
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_request_page;
			do {
				uint8_t read[] = {0x30, page};
				bool status = NxpNci_ReaderTagCmd(read, sizeof(read), pn7150.data + offset, &length);
				if((status == NFC_ERROR) || (pn7150.data[offset + length - 1] != 0)) {
					uartbb_puts("T2T Read block failed with error: "); uartbb_putu(pn7150.data[offset + length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_ERROR;
					return;
				}

				page += 4;
				offset += (length-1);
			} while(offset < pn7150.reader_request_length);

			pn7150.data_length       = pn7150.reader_request_length;
			pn7150.data_chunk_offset = 0;

			pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_READY;
			break;
		}

		default: {
			pn7150.reader_state = NFC_READER_STATE_REQUEST_PAGE_ERROR;
			break;
		}
	}
}

void pn7150_reader_write_page(void) {
	switch(pn7150_reader_interface.Protocol) {
		case PROT_MIFARE: {
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_write_page;
			uint8_t ret[16];
			uint8_t ret_length;
			do {
				// See NFC Mifare Classic specification 10.3 MIFARE Write, Table 16
				uint8_t write_start[3] = {0x10, 0xA0, pn7150.reader_write_page};
				uint8_t write_data[17] = {0x10};
				memcpy(write_data+1, pn7150.data + offset, 16);
				bool status = NxpNci_ReaderTagCmd(write_start, sizeof(write_start), ret, &ret_length);
				uartbb_puts("offset: "); uartbb_putu(offset); uartbb_putnl();
				uartbb_puts("sizeof(write): "); uartbb_putu(sizeof(ret)); uartbb_putnl();
				uartbb_puts("ret_length: "); uartbb_putu(ret_length); uartbb_putnl();
				uartbb_puts("page: "); uartbb_putu(page); uartbb_putnl();
				uartbb_puts("0: "); uartbb_putu(ret[0]); uartbb_putnl();
				uartbb_puts("1: "); uartbb_putu(ret[1]); uartbb_putnl();
				uartbb_puts("2: "); uartbb_putu(ret[2]); uartbb_putnl();
				if((status == NFC_ERROR) || (ret[ret_length - 1] != 0)) {
					uartbb_puts("Write block failed with error: "); uartbb_putu(ret[ret_length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_ERROR;
					return;
				}

				status = NxpNci_ReaderTagCmd(write_data, sizeof(write_data), ret, &ret_length);
				uartbb_puts("ret_length: "); uartbb_putu(ret_length); uartbb_putnl();
				uartbb_puts("0: "); uartbb_putu(ret[0]); uartbb_putnl();
				uartbb_puts("1: "); uartbb_putu(ret[1]); uartbb_putnl();
				uartbb_puts("2: "); uartbb_putu(ret[2]); uartbb_putnl();
				if((status == NFC_ERROR) || (ret[ret_length - 1] != 0)) {
					uartbb_puts("Write block failed with error: "); uartbb_putu(ret[ret_length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_ERROR;
					return;
				}

				page++;
				offset += 16;
			} while(offset < pn7150.reader_write_length);

			pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_READY;
			break;
		}

		case PROT_T1T: {
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_write_page;
			uint8_t ret[16];
			uint8_t ret_length;
			do {
				// TODO: It seems to work to always use a write8 here (tested with several
				//       different tags), according to the datasheet we may have to use
				//       write (one byte instead of one block) for pages 0x0-0xe.
				//       Add special case for pages < 0xF if problems turn up!
				uint8_t write_e8[14] = {0x54, page, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, pn7150.reader_tid[0], pn7150.reader_tid[1], pn7150.reader_tid[2], pn7150.reader_tid[3]};
				memcpy(write_e8+2, pn7150.data + offset, 8);
				bool status = NxpNci_ReaderTagCmd(write_e8, sizeof(write_e8), ret, &ret_length);
				uartbb_puts("offset: "); uartbb_putu(offset); uartbb_putnl();
				uartbb_puts("sizeof(write): "); uartbb_putu(sizeof(write_e8)); uartbb_putnl();
				uartbb_puts("ret_length: "); uartbb_putu(ret_length); uartbb_putnl();
				uartbb_puts("page: "); uartbb_putu(page); uartbb_putnl();
				uartbb_puts("0: "); uartbb_putu(ret[0]); uartbb_putnl();
				uartbb_puts("1: "); uartbb_putu(ret[1]); uartbb_putnl();
				uartbb_puts("2: "); uartbb_putu(ret[2]); uartbb_putnl();
				if((status == NFC_ERROR) || (ret[ret_length - 1] != 0)) {
					uartbb_puts("Write block failed with error: "); uartbb_putu(ret[ret_length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_ERROR;
					return;
				}

				page++;
				offset += 8;
			} while(offset < pn7150.reader_write_length);

			pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_READY;
			break;
		}

		case PROT_T2T: {
			uint16_t offset = 0;
			uint16_t page = pn7150.reader_write_page;
			uint8_t ret[16];
			uint8_t ret_length;
			do {
				uint8_t write[6] = {0xA2, page};
				memcpy(write+2, pn7150.data + offset, 4);
				bool status = NxpNci_ReaderTagCmd(write, sizeof(write), ret, &ret_length);
				uartbb_puts("offset: "); uartbb_putu(offset); uartbb_putnl();
				uartbb_puts("sizeof(write): "); uartbb_putu(sizeof(write)); uartbb_putnl();
				uartbb_puts("ret_length: "); uartbb_putu(ret_length); uartbb_putnl();
				uartbb_puts("page: "); uartbb_putu(page); uartbb_putnl();
				uartbb_puts("0: "); uartbb_putu(ret[0]); uartbb_putnl();
				uartbb_puts("1: "); uartbb_putu(ret[1]); uartbb_putnl();
				uartbb_puts("2: "); uartbb_putu(ret[2]); uartbb_putnl();
				if((status == NFC_ERROR) || (ret[ret_length - 1] != 0)) {
					uartbb_puts("Write block failed with error: "); uartbb_putu(ret[ret_length - 1]); uartbb_putnl();
					pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_ERROR;
					return;
				}

				page++;
				offset += 4;
			} while(offset < pn7150.reader_write_length);

			pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_READY;
			break;
		}

		default: {
			pn7150.reader_state = NFC_READER_STATE_WRITE_PAGE_ERROR;
			break;
		}
	}
}

void uartbb_putarru(char *name, uint8_t *data, uint32_t length) {
	uartbb_puts(name); uartbb_puts(": ");
	for(uint32_t i = 0; i < length; i++) {
		uartbb_putu(data[i]); uartbb_puts(", ");
	}

	uartbb_putnl();
}

void pn7150_reader_update_ndef(void) {
	// TODO: Check validity of ndef record?

	uartbb_puts("update_ndef: "); uartbb_putu(pn7150.reader_ndef_length); uartbb_putnl();

	// We always only do either read or write
	if(pn7150.reader_ndef_length > 0) {
		uartbb_putarru("ndef upd", pn7150.data, pn7150.reader_ndef_length);
		RW_NDEF_SetMessage((unsigned char *) pn7150.data, pn7150.reader_ndef_length, pn7150_reader_push_cb);
		RW_NDEF_RegisterPullCallback(NULL);
	} else {
		RW_NDEF_SetMessage(NULL, 0, NULL);
		RW_NDEF_RegisterPullCallback(pn7150_reader_pull_cb);
	}

}

void pn7150_reader_state_machine(void) {
	switch(pn7150.reader_state) {
		case NFC_READER_STATE_REQUEST_TAG_ID:                   pn7150_reader_request_tag_id();                   break;
		case NFC_READER_STATE_REQUEST_NDEF:                     pn7150_reader_request_ndef();                     break;
		case NFC_READER_STATE_WRITE_NDEF:                       pn7150_reader_write_ndef();                       break;
		case NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE: pn7150_reader_authenticate_mifare_classic_page(); break;
		case NFC_READER_STATE_REQUEST_PAGE:                     pn7150_reader_request_page();                     break;
		case NFC_READER_STATE_WRITE_PAGE:                       pn7150_reader_write_page();                       break;
		default: break;
	}
}
