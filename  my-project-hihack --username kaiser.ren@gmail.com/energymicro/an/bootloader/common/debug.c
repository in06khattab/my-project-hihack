/**************************************************************************//**
 * @file
 * @brief Debug printf on DEBUG_USART with a fixed baud rate of 115200, useful
 *        for debugging purposes. When Debug is not set this will not be
 *        compiled in. Note that __write is overridden in iar_write.c,
 *        which makes this work.
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
#include <stdint.h>
#include "em_device.h"
#include "debug.h"
#include "config.h"

#ifndef NDEBUG
/**************************************************************************//**
 * @brief Initialize DEBUG_USART@115200 for use for debugging purposes.
 *****************************************************************************/
void DEBUG_init(void)
{
  /* Enable clock to DEBUG_USART */
  CMU->HFPERCLKEN0 |= DEBUG_USART_CLOCK;

  /* Set clock division. We need 38400 bps, while the core is running
   * at 14 MHz. Using equation 16.2 the CLKDIV must be set to 5573 */
  DEBUG_USART->CLKDIV = 7000000*16/38400 - 256;;

  /* Use default location 0: TX - pin C0, RX - pin C1 */
  DEBUG_USART->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN
                  | DEBUG_USART_LOCATION;

  CONFIG_DebugGpioSetup();

  /* Clear RX/TX buffers */
  DEBUG_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  /* Enable RX/TX */
  DEBUG_USART->CMD = USART_CMD_RXEN | USART_CMD_TXEN;
}

/**************************************************************************//**
 * @brief Transmit a single byte on usart1
 * @param data Character to transmit.
 *****************************************************************************/
int DEBUG_TxByte(uint8_t data)
{
  /* Check that transmit buffer is empty */
  while (!(DEBUG_USART->STATUS & USART_STATUS_TXBL)) ;

  DEBUG_USART->TXDATA = (uint32_t) data;
  return (int) data;
}

/**************************************************************************//**
 * @brief Transmit buffer to USART1
 * @param buffer Array of characters to send
 * @param nbytes Number of bytes to transmit
 * @return Number of bytes sent
 *****************************************************************************/
int DEBUG_TxBuf(uint8_t *buffer, int nbytes)
{
  int i;

  for (i = 0; i < nbytes; i++)
  {
    DEBUG_TxByte(*buffer++);
  }
  return nbytes;
}

#endif
