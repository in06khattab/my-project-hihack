/**************************************************************************//**
 * @file
 * @brief USART code for the EFM32 bootloader
 * @author Energy Micro AS
 * @version 1.64
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/

#ifndef _UART_H
#define _UART_H

#include <stdint.h>
#include "em_device.h"
#include "compiler.h"

__ramfunc void USART_printHex(uint32_t integer);
__ramfunc int USART_txByte(uint8_t data);
__ramfunc uint8_t USART_rxByte(void);
__ramfunc void USART_printString(uint8_t *string);
void USART_init(uint32_t clkdiv);
void uartPutChar(char c);
__ramfunc void USART_printHexBy16u(uint16_t integer);
#endif
