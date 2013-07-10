/***************************************************************************//**
 * @file main_adc_em2_interrupt.c
 *
 * @brief This software examples demonstrates how to do low speed (< 2-4KHz) 
 * adc sampling in the most energy efficient way. EM2 is entered between samples.
 * This code is adapted to allow for other interrupts/tasks to run while doing the 
 * ADC sampling. The break even sampling frequency compared to using the autonomous
 * approach with PRS and DMA is about 2-4 KHz. (Please see application note AN0021).
 *
 * @author Energy Micro AS
 * @version 1.07
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
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
 ******************************************************************************/
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_rtc.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_int.h"

#define LFRCOFREQ 32768

/* Change this to increase or decrease sampling frequency. */
#define WAKEUP_US 10000

/* Change this to increase or decrease number of samples. */
#define N_SAMPLES 512

volatile uint16_t sampleBuffer[N_SAMPLES];
volatile uint16_t sampleCount;
volatile uint8_t adcFinished;

/***************************************************************************//**
 * @brief RTC Interrupt Handler.
 ******************************************************************************/
void RTC_IRQHandler(void)
{
  /* Start ADC conversion as soon as we wake up. */
  ADC_Start(ADC0, adcStartSingle);
  
  /* Clear the interrupt flag */
  RTC_IntClear(RTC_IF_COMP0);
  
  /* Wait while conversion is active */
  while (ADC0->STATUS & ADC_STATUS_SINGLEACT);
  
  /* Get ADC result */
  sampleBuffer[sampleCount++] = ADC_DataSingleGet(ADC0);
  
  if(sampleCount >= N_SAMPLES){
    adcFinished = 1;
    RTC_Enable(false);
    RTC_IntDisable(_RTC_IF_MASK);
  }
}

/***************************************************************************//**
 * @brief Enables LFACLK and selects osc as clock source for RTC
 ******************************************************************************/
void RTC_Setup(CMU_Select_TypeDef osc)
{
  RTC_Init_TypeDef init;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable osc as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, osc);

  /* No division prescaler to increase accuracy. */
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = true; /* Count only to top before wrapping */
  RTC_Init(&init);
  
  RTC_CompareSet(0, ((LFRCOFREQ * WAKEUP_US) / 1000000));
    
  /* Disable interrupt generation from RTC0, is enabled before each sample-run. */
  RTC_IntDisable(_RTC_IF_MASK);
  
  /* Enable interrupts in NVIC */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);  
}

/**************************************************************************//**
 * @brief Configure ADC for 12 bit mode, sample channel 0 with Vdd as reference 
 * and use shortest acquisition time.
 *****************************************************************************/
static void ADC_Config(void)
{
  
  CMU_ClockEnable(cmuClock_ADC0, true);
  
  ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

  /* Init common settings for both single conversion and scan mode- */
  /* Set timebase to 10, this gives 11 cycles which equals 1us at 11 MHz. */
  init.timebase = 10;

  /* Set ADC clock prescaler to 0, we are using 11MHz HFRCO, which results in HFPERCLK < 13MHz- */
  init.prescale = 0;

  ADC_Init(ADC0, &init);

  /* Init for single conversion use, measure channel 0 with Vdd as reference. */
  /* Using Vdd as reference removes the 5us warmup time for the bandgap reference. */
  singleInit.reference  = adcRefVDD;
  singleInit.input      = adcSingleInpCh0;
  
  /* Resolution can be set lower for even more energy efficient operation. */
  singleInit.resolution = adcRes12Bit;

  /* Assuming we are mesuring a low impedance source we can safely use the shortest */
  /* acquisition time. */
  singleInit.acqTime = adcAcqTime1;

  ADC_InitSingle(ADC0, &singleInit);
  
}

/**************************************************************************//**
 * @brief A separate function for taking all the samples is preferred since 
 * the whole idea is to stay in EM2 between samples. If other code is added, 
 * it might be more energy efficient to configure the ADC to use DMA while 
 * the cpu can other work. 
 *****************************************************************************/
void startAdcSampling(void)
{  
  sampleCount = 0;
  adcFinished = false;
  
  /* Enable RTC, it can be enabled all the time as well if needed. */
  RTC_Enable(true);
  
  /* Enable interrupt on COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);
  
}

/**************************************************************************//**
 * @brief Main function
 *****************************************************************************/
int main(void)
{
  
  /* Chip revision alignment and errata fixes */
  CHIP_Init();
    
  /* Set the clock frequency to 11MHz so the ADC can run on the undivided HFCLK */
  CMU_HFRCOBandSet(cmuHFRCOBand_11MHz);
  
  /* Configure RTC to use LFRCO as clock source */
  RTC_Setup(cmuSelect_LFRCO);
    
  /* Configure ADC */
  ADC_Config();
 
  /* Start ADC sampling, adcFinished is set when sampling is finished. */
  /* It is safe to do other stuff or go to EM2 while adc sampling is active. */
  /* ADC sampling uses the RTC to trigger new samples. */
  startAdcSampling();
    
  /* Wait in EM2 until adc sampling is finished. */
  /* Disable interrupts until flag is checked in case loop finishes after flag 
  * check but before sleep command. Device will still wake up on any set IRQ 
  * and any pending interrupts will be handled after interrupts are enabled 
  * again. */
  INT_Disable();
  while(!adcFinished)
  {
   EMU_EnterEM2(false); 
   INT_Enable();
   INT_Disable();
  }
  INT_Enable();    
  
  /* Finished */
  while(1);
  
}  
