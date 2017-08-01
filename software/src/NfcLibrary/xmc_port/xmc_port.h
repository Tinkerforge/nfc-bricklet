/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * xmc_port.h Port for XMC microcontroller of the interface functions
 *             of the NXP NCI NFC Library
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

#ifndef XMC_PORT_H
#define XMC_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define TIMEOUT_INFINITE	0
#define TIMEOUT_100MS		100
#define TIMEOUT_1S			1000
#define TIMEOUT_2S			2000

void tml_Connect(void);
void tml_Disconnect(void);
void tml_Send(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytesSent);
void tml_Receive(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytes, uint16_t timeout);

extern void Sleep (unsigned int ms);

#endif
