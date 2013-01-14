/**************************************************************************//**
 * @file main.c
 * @brief Hijack demo for EFM32TG_STK3300
 * @version 1.01
 ******************************************************************************
 * @section License
 ******************************************************************************
 *
 * Copyright (c) 2010 The Regents of the University of Michigan, Energy Micro 2012
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the
 *  distribution.
 * - Neither the name of the copyright holder nor the names of
 *  its contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Thomas Schmid
 * Modified by Energy Micro (2012).
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_emu.h"

/* Driver header file(s). */
#include "segmentlcd.h"

/* HiJack header file. */
#include "hijack.h"

/* Local prototypes */
void gpioSetup(void);
void setupSWO(void);
void Delay(uint32_t delayVal);


void setupSWO(void)
{
  uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
  uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
  uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;

  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  /* Enable Serial wire output pin */
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
#if defined(_EFM32_GIANT_FAMILY)
  /* Set location 0 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

  /* Enable output on pin - GPIO Port F, Pin 2 */
  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
  /* Set location 1 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
  /* Enable output on pin */
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif
  /* Enable debug clock AUXHFRCO */
  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

  while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

  /* Enable trace in core debug */
  CoreDebug->DHCSR |= 1;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Enable PC and IRQ sampling output */
  *dwt_ctrl = 0x400113FF;
  /* Set TPIU prescaler to 16. */
  *tpiu_prescaler = 0xf;
  /* Set protocol to NRZ */
  *tpiu_protocol = 2;
  /* Unlock ITM and output data */
  ITM->LAR = 0xC5ACCE55;
  ITM->TCR = 0x10009;
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB11)
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 11);
  HIJACK_ByteTx(0x55);
}


/**************************************************************************//**
 * @brief GPIO Interrupt handler (PD8)
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 8);
  HIJACK_ByteTx(0xAA);

}


/**************************************************************************//**
 * @brief Setup GPIO interrupt to change demo mode
 *****************************************************************************/
void gpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PD8 and PB11 as input */
  GPIO_PinModeSet(gpioPortD, 8, gpioModeInput, 0);
  GPIO_PinModeSet(gpioPortB, 11, gpioModeInput, 0);

  /* Set falling edge interrupt for both ports */
  GPIO_IntConfig(gpioPortD, 8, false, true, true);
  GPIO_IntConfig(gpioPortB, 11, false, true, true);

  /* Enable interrupt in core for even and odd gpio interrupts */
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
/**************************************************************************//**
 * @brief Delay loop function with optimization turned off.
 *****************************************************************************/
void Delay(uint32_t delayVal)
{
  /* Volatile keeps it from getting optimized away. */
  volatile uint32_t i;
  for(i = 0;i<delayVal;i++);
}
/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  uint32_t j;

  /* Chip errata */
  CHIP_Init();

  /* Select clock source for HFRC0 clock, 14MHz. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

  /* Prescale the core clock -> HFRCO/4 = 14/4 = 3.4Mhz */
  CMU_ClockDivSet(cmuClock_CORE, cmuClkDiv_4);

  /* Configure push button interrupts. */
  gpioSetup();

  /* configure SWO output for debugging. */
  setupSWO();

  /* Init Segment LCD without boost. */
  //SegmentLCD_Init(false);

  /* Turn on relevant symbols on the LCD. */
  //SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Print welcome text on the LCD. */
  //SegmentLCD_Write("HiJack");

  /* Init the HiJack interface. */
  HIJACK_Init(NULL);

  /* While loop sending a range of values upon wakeup.  */
  while(1){
    for(j = 0;j<254;j++){
      HIJACK_ByteTx(j);
      Delay(10000);
    }

  }
}
