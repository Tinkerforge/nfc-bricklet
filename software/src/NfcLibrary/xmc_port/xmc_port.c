/* nfc-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * xmc_port.c: Port for XMC microcontroller of the interface functions
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


#include "xmc_port.h"

#include <stdint.h>

#include "configs/config_pn7150.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/os/coop_task.h"
#include "bricklib2/logging/logging.h"
#include "bricklib2/utility/util_definitions.h"

#include "pn7150.h"

#include "xmc_i2c.h"
#include "xmc_gpio.h"
#include "xmc_usic.h"
#include "xmc_scu.h"

typedef enum {
	I2C_TX_STATE_START_TX,
	I2C_TX_STATE_DATA_TX,
	I2C_TX_STATE_STOP_TX,
	I2C_TX_STATE_START_RX,
	I2C_TX_STATE_DATA_RX,
	I2C_TX_STATE_STOP_RX,
	I2C_TX_STATE_DONE
} I2CTXState;


typedef enum XMC_I2C_CH_TDF {
	XMC_I2C_CH_TDF_MASTER_SEND         = 0,
	XMC_I2C_CH_TDF_SLAVE_SEND          = 1 << 8,
	XMC_I2C_CH_TDF_MASTER_RECEIVE_ACK  = 2 << 8,
	XMC_I2C_CH_TDF_MASTER_RECEIVE_NACK = 3 << 8,
	XMC_I2C_CH_TDF_MASTER_START        = 4 << 8,
	XMC_I2C_CH_TDF_MASTER_RESTART      = 5 << 8,
	XMC_I2C_CH_TDF_MASTER_STOP         = 6 << 8
} XMC_I2C_CH_TDF_t;

#define i2c_rx_irq_handler  IRQ_Hdlr_11
#define i2c_tx_irq_handler  IRQ_Hdlr_12

volatile uint8_t *i2c_tx_data = NULL;
volatile uint16_t i2c_tx_data_length = 0;
volatile uint16_t i2c_tx_data_index = 0;
volatile bool i2c_tx_data_send_stop = false;
volatile I2CTXState i2c_tx_state = I2C_TX_STATE_DONE;

volatile uint8_t *i2c_rx_data = NULL;
volatile uint16_t i2c_rx_data_length = 0;
volatile uint16_t i2c_rx_data_index = 0;

uint16_t i2c_max_timeout = 2000;

void i2c_rx_irq_handler(void) {
	while(!XMC_USIC_CH_RXFIFO_IsEmpty(PN7150_I2C)) {
		if(i2c_rx_data_index <= i2c_rx_data_length) {
			i2c_rx_data[i2c_rx_data_index] = XMC_I2C_CH_GetReceivedData(PN7150_I2C);
			i2c_rx_data_index++;
		} else {
			XMC_USIC_CH_RXFIFO_Flush(PN7150_I2C);
			return;
		}
	}
}

void i2c_tx_irq_handler(void) {
	while(!XMC_USIC_CH_TXFIFO_IsFull(PN7150_I2C)) {
		switch(i2c_tx_state) {
			case I2C_TX_STATE_START_TX: {
				PN7150_I2C->IN[0] = (PN7150_I2C_ADDRESS << 1) | XMC_I2C_CH_TDF_MASTER_START;
				i2c_tx_state = I2C_TX_STATE_DATA_TX;
				break;
			}

			case I2C_TX_STATE_DATA_TX: {
				PN7150_I2C->IN[0] = XMC_I2C_CH_TDF_MASTER_SEND | i2c_tx_data[i2c_tx_data_index];
				i2c_tx_data_index++;
				if(i2c_tx_data_index >= i2c_tx_data_length) {
					if(i2c_tx_data_send_stop) {
						i2c_tx_state = I2C_TX_STATE_STOP_TX;
					} else {
						i2c_tx_state = I2C_TX_STATE_DONE;
						XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
						return;
					}
				}
				break;
			}

			case I2C_TX_STATE_STOP_TX: {
				PN7150_I2C->IN[0] = XMC_I2C_CH_TDF_MASTER_STOP;
				i2c_tx_state = I2C_TX_STATE_DONE;
				XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
				return;
			}

			case I2C_TX_STATE_START_RX: {
				PN7150_I2C->IN[0] = (PN7150_I2C_ADDRESS << 1) | XMC_I2C_CH_CMD_READ | XMC_I2C_CH_TDF_MASTER_START;
				i2c_tx_state = I2C_TX_STATE_DATA_RX;
				break;
			}

			case I2C_TX_STATE_DATA_RX: {
				i2c_tx_data_index++;
				if(i2c_tx_data_index < i2c_tx_data_length) {
					PN7150_I2C->IN[0] = XMC_I2C_CH_TDF_MASTER_RECEIVE_ACK;
				} else {
					PN7150_I2C->IN[0] = XMC_I2C_CH_TDF_MASTER_RECEIVE_NACK;
					if(i2c_tx_data_send_stop) {
						i2c_tx_state = I2C_TX_STATE_STOP_RX;
					} else {
						i2c_tx_state = I2C_TX_STATE_DONE;
						XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
						return;
					}
				}

				break;
			}

			case I2C_TX_STATE_STOP_RX: {
				PN7150_I2C->IN[0] = XMC_I2C_CH_TDF_MASTER_STOP;
				i2c_tx_state = I2C_TX_STATE_DONE;
				XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
				return;
			}

			case I2C_TX_STATE_DONE: {
				XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
				return;
			}
		}
	}
}

bool i2c_write(uint8_t *data, const uint16_t length, bool send_stop) {
	if(i2c_tx_state != I2C_TX_STATE_DONE) {
		return false;
	}

	i2c_tx_state = I2C_TX_STATE_START_TX;
	i2c_tx_data = data;
	i2c_tx_data_length = length;
	i2c_tx_data_index = 0;
	i2c_tx_data_send_stop = send_stop;

	uint16_t last_index = 0;
	XMC_USIC_CH_TXFIFO_EnableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
	XMC_USIC_CH_TriggerServiceRequest(PN7150_I2C, PN7150_SERVICE_REQUEST_TX);

	uint32_t time = system_timer_get_ms();
	while(i2c_tx_state != I2C_TX_STATE_DONE) {
		if(last_index != i2c_tx_data_index) {
			last_index = i2c_tx_data_index;
			time = system_timer_get_ms();
		} else if(system_timer_is_time_elapsed_ms(time, 5)) {
			// We wait a maximum of 5ms per byte, otherwise we assume something is broken and return
			XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
			XMC_USIC_CH_TXFIFO_Flush(PN7150_I2C);
			XMC_USIC_CH_RXFIFO_Flush(PN7150_I2C);

			XMC_I2C_CH_ClearStatusFlag(PN7150_I2C, 0xFFFFFFFF);

			// Invalidate transmission (otherwise we can't send anymore)
			WR_REG(PN7150_I2C->FMR, USIC_CH_FMR_MTDV_Msk, USIC_CH_FMR_MTDV_Pos, 2);

			i2c_tx_state = I2C_TX_STATE_DONE;

			return false;
		}
		coop_task_yield();
	}

	time = system_timer_get_ms();
	uint16_t last_fifo_level =  XMC_USIC_CH_TXFIFO_GetLevel(PN7150_I2C);
	uint16_t fifo_level = last_fifo_level;
	while(fifo_level != 0) {
		fifo_level = XMC_USIC_CH_TXFIFO_GetLevel(PN7150_I2C);
		if(fifo_level != last_fifo_level) {
			last_fifo_level = fifo_level;
			time = system_timer_get_ms();
		} else if(system_timer_is_time_elapsed_ms(time, 5)) {
			// We wait a maximum of 5ms per byte, otherwise we assume something is broken and return
			XMC_USIC_CH_TXFIFO_DisableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
			XMC_USIC_CH_TXFIFO_Flush(PN7150_I2C);
			XMC_USIC_CH_RXFIFO_Flush(PN7150_I2C);

			XMC_I2C_CH_ClearStatusFlag(PN7150_I2C, 0xFFFFFFFF);

			// Invalidate transmission (otherwise we can't send anymore)
			WR_REG(PN7150_I2C->FMR, USIC_CH_FMR_MTDV_Msk, USIC_CH_FMR_MTDV_Pos, 2);

			i2c_tx_state = I2C_TX_STATE_DONE;

			return false;
		}
		coop_task_yield();
	}

	return true;
}

bool i2c_read(uint8_t *data, const uint16_t length) {
	i2c_tx_state = I2C_TX_STATE_START_RX;
	i2c_tx_data_length = length;
	i2c_tx_data_index = 0;
	i2c_tx_data_send_stop = true;

	i2c_rx_data = data;
	i2c_rx_data_length = length;
	i2c_rx_data_index = 0;

	uint16_t last_index = 0;
	XMC_USIC_CH_TXFIFO_EnableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
	XMC_USIC_CH_TriggerServiceRequest(PN7150_I2C, PN7150_SERVICE_REQUEST_TX);

	uint32_t time = system_timer_get_ms();
	while(i2c_rx_data_index != i2c_rx_data_length) {
		if(last_index != i2c_rx_data_index) {
			last_index = i2c_rx_data_index;
			time = system_timer_get_ms();
		} else if(system_timer_is_time_elapsed_ms(time, 10)) {
			// We wait a maximum of 10ms per byte, otherwise we assume something is broken and return

			return false;
		}

		// If there is data in the rx fifo we read it here.
		// To save CPU time the rx irq is only triggered if the fifo level
		// goes above 8. For the last 7 bytes this might not happen at all,
		// so we also have to poll the buffer here.
		__disable_irq();
		if(!XMC_USIC_CH_RXFIFO_IsEmpty(PN7150_I2C)) {
			i2c_rx_irq_handler();
		}
		__enable_irq();

		if(i2c_rx_data_index != i2c_rx_data_length) {
			coop_task_yield();
		}
	}

	return true;
}


void tml_Connect(void) {
	tml_Disconnect();

	logd("NCI Connect\n\r");

	const XMC_I2C_CH_CONFIG_t master_channel_config = {
		.baudrate = PN7150_I2C_BAUDRATE,
		.address  = 0
	};

	const XMC_GPIO_CONFIG_t sda_pin_config =  {
		.mode         = PN7150_SDA_PIN_MODE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	const XMC_GPIO_CONFIG_t scl_pin_config = {
		.mode         = PN7150_SCL_PIN_MODE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	const XMC_GPIO_CONFIG_t config_irq =  {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
	};

	const XMC_GPIO_CONFIG_t config_ven =  {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_LOW,
	};

	XMC_I2C_CH_Init(PN7150_I2C, &master_channel_config);

	XMC_USIC_CH_SetInputSource(PN7150_I2C, PN7150_SDA_INPUT, PN7150_SDA_SOURCE);
	XMC_USIC_CH_SetInputSource(PN7150_I2C, PN7150_SCL_INPUT, PN7150_SCL_SOURCE);

	XMC_USIC_CH_TXFIFO_Flush(PN7150_I2C);
	XMC_USIC_CH_RXFIFO_Flush(PN7150_I2C);

	XMC_USIC_CH_TXFIFO_Configure(PN7150_I2C, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 8);
	XMC_USIC_CH_RXFIFO_Configure(PN7150_I2C, 32, XMC_USIC_CH_FIFO_SIZE_16WORDS, 8);

	// Set service request for tx FIFO transmit interrupt
	XMC_USIC_CH_TXFIFO_SetInterruptNodePointer(PN7150_I2C, XMC_USIC_CH_TXFIFO_INTERRUPT_NODE_POINTER_STANDARD, PN7150_SERVICE_REQUEST_TX);

	// Set service request for rx FIFO receive interrupt
	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(PN7150_I2C, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_STANDARD, PN7150_SERVICE_REQUEST_RX);
	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(PN7150_I2C, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_ALTERNATE, PN7150_SERVICE_REQUEST_RX);


	NVIC_SetPriority((IRQn_Type)PN7150_IRQ_RX, PN7150_IRQ_RX_PRIORITY);
	NVIC_EnableIRQ((IRQn_Type)PN7150_IRQ_RX);

	NVIC_SetPriority((IRQn_Type)PN7150_IRQ_TX, PN7150_IRQ_TX_PRIORITY);
	NVIC_EnableIRQ((IRQn_Type)PN7150_IRQ_TX);


	XMC_USIC_CH_EnableEvent(PN7150_I2C, XMC_USIC_CH_EVENT_ALTERNATIVE_RECEIVE);
	XMC_USIC_CH_RXFIFO_EnableEvent(PN7150_I2C, XMC_USIC_CH_RXFIFO_EVENT_CONF_STANDARD | XMC_USIC_CH_RXFIFO_EVENT_CONF_ALTERNATE);

//	XMC_USIC_CH_TXFIFO_EnableEvent(PN7150_I2C, XMC_USIC_CH_TXFIFO_EVENT_CONF_STANDARD);
//	XMC_USIC_CH_TriggerServiceRequest(PN7150_I2C, PN7150_SERVICE_REQUEST_TX);

	XMC_I2C_CH_Start(PN7150_I2C);

	XMC_GPIO_Init(PN7150_SDA_PIN, &sda_pin_config);
	XMC_GPIO_Init(PN7150_SCL_PIN, &scl_pin_config);
	XMC_GPIO_Init(PN7150_IRQ_PIN, &config_irq);
	XMC_GPIO_Init(PN7150_VEN_PIN, &config_ven);

	system_timer_sleep_ms(10);
	XMC_GPIO_SetOutputHigh(PN7150_VEN_PIN);
	system_timer_sleep_ms(10);
}

void tml_Disconnect(void) {
	logd("NCI Disconnect\n\r");
	XMC_I2C_CH_Stop(PN7150_I2C);

	const XMC_GPIO_CONFIG_t config_reset =  {
		.mode             = XMC_GPIO_MODE_INPUT_PULL_UP,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_LARGE,
	};

	XMC_GPIO_Init(PN7150_SDA_PIN, &config_reset);
	XMC_GPIO_Init(PN7150_SCL_PIN, &config_reset);

	XMC_USIC_CH_TXFIFO_Flush(PN7150_I2C);
	XMC_USIC_CH_RXFIFO_Flush(PN7150_I2C);

	WR_REG(PN7150_I2C->FMR, USIC_CH_FMR_MTDV_Msk, USIC_CH_FMR_MTDV_Pos, 2);

	const XMC_GPIO_CONFIG_t config_input =  {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
	};

	XMC_GPIO_Init(PN7150_IRQ_PIN, &config_input);
	XMC_GPIO_Init(PN7150_VEN_PIN, &config_input);
}

void tml_Send(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytesSent) {
    if(!i2c_write(pBuffer, BufferLen, true)) {
    	coop_task_sleep_ms(5);
    	if(!i2c_write(pBuffer, BufferLen, true)) {
    		*pBytesSent = 0;
    		return;
    	}
    }

    *pBytesSent = BufferLen;
}

void tml_Receive(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytes, uint16_t timeout) {
	uint32_t start = system_timer_get_ms();
	bool elapsed = true;
	if(timeout == 0) {
		timeout = 10000;
	} else {
		timeout = MIN(timeout, i2c_max_timeout);
	}

	while(!system_timer_is_time_elapsed_ms(start, timeout)) {
		if(XMC_GPIO_GetInput(PN7150_IRQ_PIN)) {
			elapsed = false;
			break;
		}
		coop_task_yield();
	}

	if(!XMC_GPIO_GetInput(PN7150_IRQ_PIN)) {
		if(elapsed) {
			*pBytes = 0;
			return;
		}
	}

	if(i2c_read(pBuffer, 3)) {
		if((pBuffer[2] + 3) <= BufferLen) {
			if (pBuffer[2] > 0) {
				if(i2c_read(&pBuffer[3], pBuffer[2])) {
					*pBytes = pBuffer[2] + 3;
					return;
				}
			} else {
				*pBytes = 3;
				return;
			}
		}
	}

	*pBytes = 0;
}

void Sleep(unsigned int ms) {
	coop_task_sleep_ms(ms);
}
