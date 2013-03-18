/******************************************************************************
 * @file
 * @brief Analog Comparator Window Interrupt Example
 * @author Energy Micro AS
 * @version 1.06
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

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_acmp.h"
#include "em_lcd.h"
#include "segmentlcd.h"


/***************************************************************************//**
* @brief
*   Init analog comparator.
*******************************************************************************/
static void ACMPInit(void)
{
  const ACMP_Init_TypeDef acmp_init =
  {
    false,                              /* Full bias current*/
    false,                              /* Half bias current */
    7,                                  /* Biasprog current configuration */
    true,                               /* Enable interrupt for falling edge */
    true,                               /* Enable interrupt for rising edge */
    acmpWarmTime256,                    /* Warm-up time in clock cycles, >140 cycles for 10us with 14MHz */
    acmpHysteresisLevel0,               /* Hysteresis configuration */
    0,                                  /* Inactive comparator output value */
    false,                              /* Enable low power mode */
    0,                                  /* Vdd reference scaling */
    true,                               /* Enable ACMP */
  };

  /* Init and set ACMP channel */
  ACMP_Init(ACMP0, &acmp_init);
  ACMP_ChannelSet(ACMP0, acmpChannel2V5, acmpChannel4);

  ACMP_IntEnable(ACMP0, ACMP_IEN_EDGE);   /* Enable edge interrupt */


  /* Wait for warmup */
  while (!(ACMP0->STATUS & ACMP_STATUS_ACMPACT)) ;

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(ACMP0_IRQn);
  NVIC_EnableIRQ(ACMP0_IRQn);
}


/**************************************************************************//**
 * @brief ACMP0 Interrupt handler
 *****************************************************************************/
void ACMP0_IRQHandler(void)
{
  /* Clear interrupt flag */
  ACMP0->IFC = ACMP_IFC_EDGE;
  
  /* Write ACMP status to LCD */
  SegmentLCD_Number((int) ((ACMP0->STATUS & ACMP_STATUS_ACMPOUT) >> _ACMP_STATUS_ACMPOUT_SHIFT));
}

/******************************************************************************
 * @brief  Main function, enables analog comparator 0 and waits in em2 for
 * comparator interrupt, both rising and falling edges.
 * Main is called from _program_start, see assembly startup file
 *****************************************************************************/
int main(void)
{
  /* Initialize chip */
  CHIP_Init();
  SegmentLCD_Init(false);

  /* Enable clocks required */
  CMU_ClockEnable(cmuClock_ACMP0, true);

  ACMPInit();
  
  /* Write ACMP status to LCD */
  SegmentLCD_Number((int) ((ACMP0->STATUS & ACMP_STATUS_ACMPOUT) >> _ACMP_STATUS_ACMPOUT_SHIFT));

  /* Stay in this loop forever at end of program */
  while (1)
  {
    /* Enter em2 and wait for acmp interrupt */
    EMU_EnterEM2(true);
  }
}

