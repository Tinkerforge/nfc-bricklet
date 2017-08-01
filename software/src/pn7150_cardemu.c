/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150_cardemu.c: Driver for PN7150 NFC IC cardemu mode
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

#include "pn7150_cardemu.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/system_timer/system_timer.h"

#include "pn7150.h"
#include "communication.h"
#include "NxpNci.h"

extern PN7150 pn7150;

static NxpNci_RfIntf_t pn7150_cardemu_interface;

void pn7150_cardemu_cb(unsigned char *pNdefRecord, unsigned short NdefRecordSize) {
	pn7150.cardemu_state = NFC_CARDEMU_STATE_TRANSFER_NDEF_READY;
    uartbb_puts("pn7150_cardemu_cb\n\r");
}


void pn7150_cardemu_discover(void) {
	uint8_t discovery_technologies[] = {
		MODE_LISTEN | TECH_PASSIVE_NFCA,
		MODE_LISTEN | TECH_PASSIVE_NFCB,
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
	if(NxpNci_WaitForDiscoveryNotification(&pn7150_cardemu_interface) != NFC_SUCCESS) {
		uartbb_puts("Error: discovery timeout\n\r");
		return;
	}

	pn7150.cardemu_state = NFC_CARDEMU_STATE_DISCOVER_READY;
}

void pn7150_cardemu_transfer_ndef(void) {
    if(pn7150.cardemu_ndef_ready && (pn7150_cardemu_interface.Interface == INTF_ISODEP) && ((pn7150_cardemu_interface.ModeTech & MODE_MASK) == MODE_LISTEN)) {
    	uartbb_puts("pn7150_cardemu_process_card start\n\r");

    	// We try to start the data transfer for 250ms
    	uint32_t time = system_timer_get_ms();
        while(NxpNci_ProcessCardMode(pn7150_cardemu_interface) == 0) {
        	if(system_timer_is_time_elapsed_ms(time, 250)) {
        		pn7150.cardemu_state = NFC_CARDEMU_STATE_TRANSFER_NDEF_ERROR;
        		return;
        	}
        }

        // Wait for at most 5 seconds for the data to be send
        for(uint8_t i = 0; i < 100; i++) {
        	coop_task_sleep_ms(50);
        	if(pn7150.cardemu_state == NFC_CARDEMU_STATE_TRANSFER_NDEF_READY) {
        		uartbb_puts("pn7150_cardemu_process_card end ok\n\r");
        		return;
        	}
        }

    	uartbb_puts("pn7150_cardemu_process_card end timeout\n\r");
    }

    uartbb_puts("pn7150_cardemu_process_card end error\n\r");
   	pn7150.cardemu_state = NFC_CARDEMU_STATE_TRANSFER_NDEF_ERROR;
}


void pn7150_cardemu_update_ndef(void) {
	// TODO: Check validity of ndef record?

	uartbb_puts("update_ndef: "); uartbb_putu(pn7150.cardemu_ndef_length); uartbb_putnl();

	pn7150.cardemu_ndef_ready = true;
	T4T_NDEF_EMU_SetMessage((unsigned char *) pn7150.data, pn7150.cardemu_ndef_length, pn7150_cardemu_cb);

}

void pn7150_cardemu_state_machine(void) {
	switch(pn7150.cardemu_state) {
		case NFC_CARDEMU_STATE_DISCOVER:      pn7150_cardemu_discover();      break;
		case NFC_CARDEMU_STATE_TRANSFER_NDEF: pn7150_cardemu_transfer_ndef(); break;
		default: break;
	}
}
