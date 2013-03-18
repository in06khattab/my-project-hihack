/******************************************************************************
 * @file
 * @brief ADC single conversion example
 * @author Energy Micro AS
 * @version 1.07
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
#include "em_adc.h"
#include "em_lcd.h"
#include "segmentlcd.h"
#include "rtcdrv.h"

/***************************************************************************//**
* @brief
*   Configure ADC usage for this application.
*******************************************************************************/
static void ADCConfig(void)
{
  ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

  /* Init common settings for both single conversion and scan mode */
  init.timebase = ADC_TimebaseCalc(0);
  /* Might as well finish conversion as quickly as possibly since polling */
  /* for completion. */
  /* Set ADC clock to 7 MHz, use default HFPERCLK */
  init.prescale = ADC_PrescaleCalc(7000000, 0);

  /* WARMUPMODE must be set to Normal according to ref manual before */
  /* entering EM2. In this example, the warmup time is not a big problem */
  /* due to relatively infrequent polling. Leave at default NORMAL, */

  ADC_Init(ADC0, &init);

  /* Init for single conversion use, measure VDD/3 with 1.25 reference. */
  singleInit.reference  = adcRef1V25;
  singleInit.input      = adcSingleInpVDDDiv3;
  singleInit.resolution = adcRes12Bit;

  /* The datasheet specifies a minimum aquisition time when sampling vdd/3 */
  /* 32 cycles should be safe for all ADC clock frequencies */
  singleInit.acqTime = adcAcqTime32;

  ADC_InitSingle(ADC0, &singleInit);
}


/******************************************************************************
 * @brief  Main function
 * The ADC is set up in single conversion mode and samples VDD/3 10 times each
 * second, it sleeps in em2 between samples. The supply voltage (VDD) is then
 * calculated.
 *****************************************************************************/
int main(void)
{
  /* Initialize chip */
  CHIP_Init();
  SegmentLCD_Init(false);

  uint32_t sample;
  uint32_t   voltage;

  /* Enable clocks required */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);

  ADCConfig();

  /* Stay in this loop forever at end of program */
  while (1)
  {
    ADC_Start(ADC0, adcStartSingle);

    /* Wait while conversion is active */
    while (ADC0->STATUS & ADC_STATUS_SINGLEACT) ;

    /* Get ADC result */
    sample = ADC_DataSingleGet(ADC0);

    /* Calculate supply voltage relative to 1.25V reference */
    voltage = (sample * 1250 * 3) / 4096;

    /* Write to LCD */
    SegmentLCD_Number(voltage);

    /* wait 100ms in EM2 before next conversion */
    RTCDRV_Trigger(100, NULL);
    EMU_EnterEM2(true);
  }
}



