/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * pn7150.c: Driver for PN7150 NFC IC
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

#include "pn7150.h"

#include "configs/config_pn7150.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/uartbb/uartbb.h"
#include "bricklib2/os/coop_task.h"
#include "bricklib2/logging/logging.h"

#include "communication.h"

#include "xmc_i2c.h"
#include "xmc_gpio.h"
#include "xmc_usic.h"

#include "pn7150_reader.h"
#include "pn7150_cardemu.h"
#include "pn7150_p2p.h"

#include "NxpNci.h"

PN7150 pn7150;

bool pn7150_init_nfc(void) {
	uint8_t configure_mask = 0;

	switch(pn7150.mode) {
		case NFC_MODE_CARDEMU: {
			configure_mask = NXPNCI_MODE_CARDEMU;
			break;
		}

		case NFC_MODE_P2P: {
			configure_mask = NXPNCI_MODE_P2P;
			break;
		}

		case NFC_MODE_READER: {
			configure_mask = NXPNCI_MODE_RW;
			break;
		}

		case NFC_MODE_OFF:
		default: {
			NxpNci_Disconnect();

			return true;
		}
	}

	// Open connection to NXPNCI device
	if(NxpNci_Connect() == NFC_ERROR) {
		loge("NxpNci_Connect Error");
		return false;
	}

	while(NxpNci_ConfigureSettings() == NFC_ERROR) {
		loge("NxpNci_ConfigureSettings Error");
		return false;
	}

	while(NxpNci_ConfigureMode(configure_mask) == NFC_ERROR) {
		loge("NxpNci_ConfigureSettings Error");
		return false;
	}

    return true;
}

void pn7150_init(void) {
	memset(&pn7150, 0, sizeof(PN7150));

	pn7150.new_mode      = false;
	pn7150.mode          = NFC_MODE_OFF;
	pn7150.reader_state  = NFC_READER_STATE_INITIALIZATION;
	pn7150.cardemu_state = NFC_CARDEMU_STATE_INITIALIZATION;
	pn7150.p2p_state     = NFC_P2P_STATE_INITIALIZATION;
}

void pn7150_tick(void) {
	pn7150_init();

	while(true) {
		coop_task_yield();

		if(pn7150.new_mode) {
			// We try to initialize nfc until it works.
			// If there is a fatal error we can't do anything else anyway...
			while(!pn7150_init_nfc());

			pn7150.reader_state  = NFC_READER_STATE_IDLE;
			pn7150.cardemu_state = NFC_CARDEMU_STATE_IDLE;
			pn7150.p2p_state     = NFC_P2P_STATE_IDLE;

			pn7150.new_mode = false;
		}

		switch(pn7150.mode) {
			case NFC_MODE_READER:  pn7150_reader_state_machine();  break;
			case NFC_MODE_CARDEMU: pn7150_cardemu_state_machine(); break;
			case NFC_MODE_P2P:     pn7150_p2p_state_machine(); break;
			default: break;
		}
	}
}
