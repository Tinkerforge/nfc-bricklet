/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150_p2p.c: Driver for PN7150 NFC IC p2p mode
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

#include "pn7150_p2p.h"
#include "pn7150_simple.h"

#include "bricklib2/os/coop_task.h"
#include "bricklib2/hal/system_timer/system_timer.h"

#include "pn7150.h"
#include "communication.h"
#include "NxpNci.h"

extern PN7150 pn7150;

static NxpNci_RfIntf_t pn7150_p2p_interface;

void pn7150_p2p_push_cb(unsigned char *ndef, unsigned short ndef_length) {
	pn7150.p2p_state = NFC_P2P_STATE_TRANSFER_NDEF_READY;
}

void pn7150_p2p_pull_cb(unsigned char *ndef, unsigned short ndef_length) {
	memcpy(pn7150.data, ndef, ndef_length);
	pn7150.data_length = ndef_length;
	pn7150.data_chunk_offset = 0;
	pn7150.p2p_state = NFC_P2P_STATE_TRANSFER_NDEF_READY;
}

void pn7150_p2p_discover(void) {
	// TODO: According to the example code we can also use MODE_POLL here,
	//       but it seems that there is only handling code for MODE_LISTEN.
	//       Can we implement the MODE_POLL case?
	uint8_t discovery_technologies[] = {
//		MODE_POLL | TECH_PASSIVE_NFCA,
//		MODE_POLL | TECH_PASSIVE_NFCF,
	    // Only one POLL ACTIVE mode can be enabled, if both are defined only NFCF applies
//	    MODE_POLL | TECH_ACTIVE_NFCA,
	    //MODE_POLL | TECH_ACTIVE_NFCF,
		MODE_LISTEN | TECH_PASSIVE_NFCA,
	    MODE_LISTEN | TECH_PASSIVE_NFCF,
//	    MODE_LISTEN | TECH_ACTIVE_NFCA,
//	    MODE_LISTEN | TECH_ACTIVE_NFCF,
	};

	if(!pn7150_simple_discovery(discovery_technologies, &pn7150_p2p_interface)) {
		return;
	}

	pn7150.led_state_change_time = system_timer_get_ms();
	pn7150.p2p_state = NFC_P2P_STATE_DISCOVER_READY;
}

void pn7150_p2p_transfer_ndef(void) {
    if(pn7150_p2p_interface.Interface == INTF_NFCDEP) {
        NxpNci_ProcessP2pMode(pn7150_p2p_interface);

        if(pn7150.p2p_state == NFC_P2P_STATE_TRANSFER_NDEF_READY) {
        	pn7150.led_state_change_time = system_timer_get_ms();
        	return;
        }
    }

    pn7150.p2p_state = NFC_P2P_STATE_TRANSFER_NDEF_ERROR;
}

void pn7150_p2p_update_ndef(void) {
	// TODO: Check validity of ndef record?

	pn7150.cardemu_ndef_ready = true;

	// We always only do either read or write
	if(pn7150.p2p_ndef_length > 0) {
		P2P_NDEF_SetMessage((unsigned char *) pn7150.data, pn7150.p2p_ndef_length, pn7150_p2p_push_cb);
		P2P_NDEF_RegisterPullCallback(NULL);
	} else {
		P2P_NDEF_SetMessage(NULL, 0, NULL);
		P2P_NDEF_RegisterPullCallback(pn7150_p2p_pull_cb);
	}

}

void pn7150_p2p_state_machine(void) {
	switch(pn7150.p2p_state) {
		case NFC_P2P_STATE_DISCOVER:      pn7150_p2p_discover();      break;
		case NFC_P2P_STATE_TRANSFER_NDEF: pn7150_p2p_transfer_ndef(); break;
		default: break;
	}
}
