/*****************************************************************************
 * @file
 * @brief PRS Demo Application, GPIO input signal enables UART TX
 * @author Energy Micro AS
 * @version 1.04
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
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
#include "bsp.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_usart.h"


/**************************************************************************//**
 * @brief  GPIO_setup
 * Configures the GPIO
 *****************************************************************************/
void GPIO_setup(void)
{
  /* Enable necessary clocks */
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  /* Configure PC0 as output for UART TX 
     The UART will override this pin for data transmission */
  GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 0); 
  
  /* Configure PD0 as input with filter and pull-down */
  GPIO_PinModeSet(gpioPortD, 0, gpioModeInputPullFilter, 0); 
  
  /* Configure interrupt on Pin PD0 for rising edge but not enabled - PRS sensing instead */
  GPIO_IntConfig(gpioPortD, 0, true, false, false);
  
  /* Enable PRS sense on GPIO and disable interrupt sense */
  GPIO_InputSenseSet(GPIO_INSENSE_PRS, _GPIO_INSENSE_RESETVALUE);
}

/**************************************************************************//** 
 * @brief  UART_setup
 * Configures the UART
 *****************************************************************************/
void USART_setup(void)
{
  /* Enable necessary clocks */
  CMU_ClockEnable(cmuClock_USART1, true);
  CMU_ClockEnable(cmuClock_PRS, true);
  
  /* USART parameters for asynchronous mode */
  USART_InitAsync_TypeDef usartInit =
  {
  .enable       = usartDisable,   /* Usart disable */
  .refFreq      = 0,              /* Currently configured reference clock */
  .baudrate     = 57600,          /* Baudrate of 57600 */
  .oversampling = usartOVS8,      /* 8x oversampling */
  .databits     = usartDatabits8, /* 8 databits */
  .parity       = usartNoParity,  /* No parity bit */
  .stopbits     = usartStopbits1, /* 1 stop bit */ 
  };
  
  /* Initialize USART */
  USART_InitAsync(USART1, &usartInit);
  
  /* Enable TX to location 0 (PC0) */
  USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC0;
  
  /* Select PRS channel 5 to set TXEN enabling the transmiter */
  USART1->TRIGCTRL |= USART_TRIGCTRL_TSEL_PRSCH5 | USART_TRIGCTRL_TXTEN;
    
  /* Put a char in the buffer - it won't be transmited because UART TX is disabled */
  USART_Tx(USART1, 'X');
  
  /* PRS setup */
  /* Select GPIO as source and PD0 as signal 
     The GPIO generates a level signal and the
     UART consumes a pulse so we have to use
     the edge detector to generate the pulse */
  PRS_SourceSignalSet(5, PRS_CH_CTRL_SOURCESEL_GPIOL, PRS_CH_CTRL_SIGSEL_GPIOPIN0, prsEdgePos);
}

/**************************************************************************//**
 * @brief  Main function
 * Main is called from __iar_program_start, see assembly startup file
 *****************************************************************************/
int main(void)
{  
  /* Align different chip revisions */
  CHIP_Init();
  
  /* Initialize DK */
  BSP_Init(BSP_INIT_DEFAULT);
  
  /* Enable RS232A connection on the DK */
  BSP_PeripheralAccess(BSP_RS232A, true);
  
  /* Init GPIO */
  GPIO_setup();
  
  /* Init USART */
  USART_setup();
    
  while(1)
  {
    /* Enter EM1 */ 
    EMU_EnterEM1();
  }
}
