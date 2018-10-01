/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * main.c: Initialization for NFC Bricklet
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

#include <stdio.h>
#include <stdbool.h>

#include "configs/config.h"

#include "bricklib2/bootloader/bootloader.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"
#include "communication.h"
#include "pn7150.h"

#include "xmc_gpio.h"

void HardFault_Handler(void) {
	__asm("MOVS   R0, #4  \n"
	      "MOV    R1, LR  \n"
	      "TST    R0, R1  \n"
		  "BEQ HardFault_M \n"
	      "MRS    R0, PSP \n"
	      "B      HardFault_reg      \n"
		  "HardFault_M: \n"
		  "MRS    R0, MSP \n"
		  "HardFault_reg: \n"
	      "B      HardFault_HandlerC      \n");
}

void HardFault_HandlerC(unsigned int *hardfault_args) {
#if LOGGING_LEVEL != LOGGING_NONE
    volatile unsigned long stacked_r0;
    volatile unsigned long stacked_r1;
    volatile unsigned long stacked_r2;
    volatile unsigned long stacked_r3;
    volatile unsigned long stacked_r12;
    volatile unsigned long stacked_lr;
    volatile unsigned long stacked_pc;
    volatile unsigned long stacked_psr;
    volatile unsigned long _CFSR;
    volatile unsigned long _HFSR;
    volatile unsigned long _DFSR;
    volatile unsigned long _AFSR;
    volatile unsigned long _BFAR;
    volatile unsigned long _MMAR;

    stacked_r0  = ((unsigned long)hardfault_args[0]);
    stacked_r1  = ((unsigned long)hardfault_args[1]);
    stacked_r2  = ((unsigned long)hardfault_args[2]);
    stacked_r3  = ((unsigned long)hardfault_args[3]);
    stacked_r12 = ((unsigned long)hardfault_args[4]);
    stacked_lr  = ((unsigned long)hardfault_args[5]);
    stacked_pc  = ((unsigned long)hardfault_args[6]);
    stacked_psr = ((unsigned long)hardfault_args[7]);

    // Configurable Fault Status Register
    // Consists of MMSR, BFSR and UFSR
    _CFSR = (*((volatile unsigned long *)(0xE000ED28)));

    // Hard Fault Status Register
    _HFSR = (*((volatile unsigned long *)(0xE000ED2C)));

    // Debug Fault Status Register
    _DFSR = (*((volatile unsigned long *)(0xE000ED30)));

    // Auxiliary Fault Status Register
    _AFSR = (*((volatile unsigned long *)(0xE000ED3C)));

    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
    _MMAR = (*((volatile unsigned long *)(0xE000ED34)));
    // Bus Fault Address Register
    _BFAR = (*((volatile unsigned long *)(0xE000ED38)));

	uint32_t *pc = (uint32_t*)stacked_pc;

	uartbb_printf("\n\n[Hard fault handler]\n\r");
	uartbb_printf("R0 = %x\n\r", stacked_r0);
	uartbb_printf("R1 = %x\n\r", stacked_r1);
	uartbb_printf("R2 = %x\n\r", stacked_r2);
	uartbb_printf("R3 = %x\n\r", stacked_r3);
	uartbb_printf("R12 = %x\n\r", stacked_r12);
	uartbb_printf("LR [R14] = %x  subroutine call return address\n\r", stacked_lr);
	uartbb_printf("PC [R15] = %x (%x)  program counter\n\r", stacked_pc, *pc);
	uartbb_printf("PSR = %x\n\r", stacked_psr);
	uartbb_printf("BFAR = %x\n\r", _BFAR);
	uartbb_printf("MMAR = %x\n\r", _MMAR);
	uartbb_printf("CFSR = %x\n\r", _CFSR);
	uartbb_printf("HFSR = %x\n\r", _HFSR);
	uartbb_printf("DFSR = %x\n\r", _DFSR);
	uartbb_printf("AFSR = %x\n\r", _AFSR);

	stack_debug_update();
	uartbb_printf("in coop: %d, boot in tx/rx: %d/%d coop in tx/rx: %d/%d\n\r", 
	              in_coop, bootloader_in_tx, bootloader_in_rx, coop_in_tx, coop_in_rx);
	uartbb_printf("coop free: %d, stack free: %d\n\r", pn7150_task.stack_low_watermark, stack_debug_get_low_watermark());
	while(true) {
		XMC_WDT_Service();
	}

#else
	while(true);
#endif
}

int main(void) {
	logging_init();
	logd("Start NFC Bricklet\n\r");

	communication_init();
	pn7150_init();

	while(true) {
		bootloader_tick();
		communication_tick();
		pn7150_tick();
	}
}
