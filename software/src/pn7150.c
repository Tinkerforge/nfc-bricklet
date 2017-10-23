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
#include "bricklib2/utility/led_flicker.h"

#include "communication.h"

#include "xmc_i2c.h"
#include "xmc_gpio.h"
#include "xmc_usic.h"

#include "pn7150_reader.h"
#include "pn7150_cardemu.h"
#include "pn7150_p2p.h"

#include "NxpNci.h"

#define PN7150_DETECT_LED_ONTIME 1000 // in ms

PN7150 pn7150;

CoopTask pn7150_task;

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

void pn7150_tick_task(void) {
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

void pn7150_init(void) {
	memset(&pn7150, 0, sizeof(PN7150));

	pn7150.new_mode      = false;
	pn7150.mode          = NFC_MODE_OFF;
	pn7150.reader_state  = NFC_READER_STATE_INITIALIZATION;
	pn7150.cardemu_state = NFC_CARDEMU_STATE_INITIALIZATION;
	pn7150.p2p_state     = NFC_P2P_STATE_INITIALIZATION;

	// LED init
	pn7150.detection_led_state.config = LED_FLICKER_CONFIG_EXTERNAL;
	pn7150.led_state_change_time      = -PN7150_DETECT_LED_ONTIME-1;
	XMC_GPIO_CONFIG_t led_pin_config = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(PN7150_DETECT_LED_PIN, &led_pin_config);

	coop_task_init(&pn7150_task, pn7150_tick_task);
}

void pn7150_tick(void) {
	coop_task_tick(&pn7150_task);
	led_flicker_tick(&pn7150.detection_led_state, system_timer_get_ms(), PN7150_DETECT_LED_PIN);

	if(pn7150.detection_led_state.config == LED_FLICKER_CONFIG_EXTERNAL) {
		// pn7150.led_state_change_time is updated any time there is communication with a card/reader
		// we enable the LED for PN7150_DETECT_LED_ONTIME ms if there was communication
		if(system_timer_is_time_elapsed_ms(pn7150.led_state_change_time, PN7150_DETECT_LED_ONTIME)) {
			XMC_GPIO_SetOutputHigh(PN7150_DETECT_LED_PIN);
		} else {
			XMC_GPIO_SetOutputLow(PN7150_DETECT_LED_PIN);
		}
	}
}
