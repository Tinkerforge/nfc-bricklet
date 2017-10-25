/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_pn7150.c: Configuration for PN7150 NFC IC
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

#ifndef CONFIG_PN7150_H
#define CONFIG_PN7150_H

#include "xmc_gpio.h"

#define PN7150_I2C_BAUDRATE         400000

#define PN7150_I2C_ADDRESS          0x28 // 0b1000000
#define PN7150_I2C_CHANNEL          USIC0_CH1
#define PN7150_I2C                  XMC_I2C0_CH1

#define PN7150_SCL_PIN              P1_3
#define PN7150_SCL_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT6
#define PN7150_SCL_INPUT            XMC_USIC_CH_INPUT_DX1
#define PN7150_SCL_SOURCE           0 // DX1A

#define PN7150_SDA_PIN              P1_2
#define PN7150_SDA_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT7
#define PN7150_SDA_INPUT            XMC_USIC_CH_INPUT_DX0
#define PN7150_SDA_SOURCE           1 // DX0B

#define PN7150_IRQ_RX               11
#define PN7150_IRQ_RX_PRIORITY      0
#define PN7150_SERVICE_REQUEST_RX   2

#define PN7150_IRQ_TX               12
#define PN7150_IRQ_TX_PRIORITY      1
#define PN7150_SERVICE_REQUEST_TX   3

#define PN7150_VEN_PIN              P0_8
#define PN7150_IRQ_PIN              P0_9

#define PN7150_DETECT_LED_PIN       P1_0

#endif
