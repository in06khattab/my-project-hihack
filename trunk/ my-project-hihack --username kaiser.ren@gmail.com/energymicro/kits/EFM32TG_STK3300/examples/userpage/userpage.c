/**************************************************************************//**
 * @file
 * @brief Using the userpage in EFM32TG onboard flash.
 * @details
 *   Read/write data to the userpage in flash on the EFM32TG
 *
 * @par Usage
 * @li PB0 Increases the number.
 * @li PB1 Saves the number to the userpage
 *
 *
 * @author Energy Micro AS
 * @version 3.20.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
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

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_msc.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "segmentlcd.h"
#include "rtcdrv.h"
#include "bsp_trace.h"
#include "bsp.h"

#define USERPAGE    (0x00003000) /**< Address of the user page */

/* PRESCALER: 16 */
#define HIJACK_TIMER_RESOLUTION     timerPrescale16
#define HIJACK_TMR_CLK_SRC_PRESCALER ( 0x01 << HIJACK_TIMER_RESOLUTION)

/* tx timer definition. */
#define HIJACK_TX_TIMER             (TIMER1)
#define HIJACK_TX_TIMERCLK          (cmuClock_TIMER1)
#define HIJACK_TX_GPIO_PORT         (gpioPortC)
#define HIJACK_TX_GPIO_PIN          13
#define HIJACK_TX_LOCATION			(TIMER_ROUTE_LOCATION_LOC0)

/* several carrier frequency definition. */
#define HIJACK_ENC_CARRIER_FREQ_1KHZ	1000ul
#define HIJACK_ENC_CARRIER_FREQ_2KHZ	2000ul
#define HIJACK_ENC_CARRIER_FREQ_4KHZ	4000ul
#define HIJACK_ENC_CARRIER_FREQ_8KHZ	8000ul

/* several carrier frequency definition. */
#define HIJACK_DEC_CARRIER_FREQ_1KHZ	1000ul
#define HIJACK_DEC_CARRIER_FREQ_2KHZ	2000ul
#define HIJACK_DEC_CARRIER_FREQ_4KHZ	4000ul
#define HIJACK_DEC_CARRIER_FREQ_8KHZ	8000ul
#define HIJACK_DEC_CARRIER_FREQ_10KHZ   10000ul

/* current carrier frequency definition. */
#define HIJACK_DEC_CARRIER HIJACK_DEC_CARRIER_FREQ_1KHZ

#define BOARD_MCK 14000000ul

/* current carrier frequency definition. */
#define HIJACK_ENC_CARRIER_FREQ_CONF HIJACK_ENC_CARRIER_FREQ_1KHZ

/* how many ticker per us. */
//#define HIJACK_NUM_TICKS_PER_1US (BOARD_MCK/HIJACK_TMR_CLK_SRC_PRESCALER/1000000ul)

/* how many ticks per half cycle. */
#define HIJACK_NUM_TICKS_PER_HALF_CYCLE (BOARD_MCK / HIJACK_TMR_CLK_SRC_PRESCALER / HIJACK_DEC_CARRIER / 2)

/* how many ticks per full cycle . */
#define HIJACK_NUM_TICKS_PER_FULL_CYCLE (BOARD_MCK / HIJACK_TMR_CLK_SRC_PRESCALER / HIJACK_DEC_CARRIER)

/* how manu ticks when precision is about 5%. */
#define HIJACK_NUM_TICKS_PER_20_PCNT	(HIJACK_NUM_TICKS_PER_FULL_CYCLE / 10)

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/

typedef enum
{
  hijackOutputModeSet = 0,
  hijackOutputModeClear,
  hijackOutputModeToggle
} HIJACK_OutputMode_t;

typedef struct
{
  uint32_t number;           /**< Number to display and save to flash */
  uint32_t numWrites;        /**< Number of saves to flash */
  uint8_t payload[256];
} UserData_TypeDef;

UserData_TypeDef userData = {0, 0, NULL};                   /**< User data contents */

volatile bool             rtcFlag;                    /**< Flag used by the RTC timing routines */
volatile bool             recentlySaved;              /**< Flag to indicate successful write */

msc_Return_TypeDef        currentError = mscReturnOk; /** < Latest error encountered */

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB11)
 *        Saves the number to the userpage
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  msc_Return_TypeDef ret;

  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 11);

  /* Initialize the MSC for writing */
  MSC_Init();

  /* Erase the page */
  //ret = MSC_ErasePage((uint32_t *) (USERPAGE + userData.numWrites * 256 ) );

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  /*if (ret != mscReturnOk)
  {
    currentError = ret;
    MSC_Deinit();
    return;
  }*/



  /* Write data to the userpage */
  memset(userData.payload, userData.numWrites, 256);
  ret = MSC_WriteWord((uint32_t *) (USERPAGE + userData.numWrites * 256 ),
					  (void *) &userData.payload,
                      sizeof(UserData_TypeDef));

  /* Increase the number of saves */
  userData.numWrites++;

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  if (ret != mscReturnOk)
  {
    currentError = ret;
    MSC_Deinit();
    return;
  }
  /* Deinitialize the MSC. This disables writing and locks the MSC */
  MSC_Deinit();

  /* Signal completion of save. The number of saves will be displayed */
  recentlySaved = true;
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PD8)
 *        Increases number when PB0 is pressed. Wraps at 10000.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 8);

  /* Increase data - Wrap at 10000 */
  userData.number = (userData.number + 1) % 10000;

  /* Update the display */
  SegmentLCD_Number(userData.number);
}

/**************************************************************************//**
 * @brief Setup GPIO interrupt to set the time
 *****************************************************************************/
void gpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PD8 as input */
  GPIO_PinModeSet(gpioPortD, 8, gpioModeInput, 0);

  /* Set falling edge interrupt */
  GPIO_IntConfig(gpioPortD, 8, false, true, true);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  /* Configure PB11 as input */
  GPIO_PinModeSet(gpioPortB, 11, gpioModeInput, 0);

  /* Set falling edge interrupt */
  GPIO_IntConfig(gpioPortB, 11, false, true, true);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/**************************************************************************//**
 * @brief Callback used for the RTC.
 * @details
 *   Because GPIO interrupts can alswo wake up the CM3 from sleep it is
 *   necessary for the correct timing of the scrolling text to make sure that
 *   the wake-up source is the RTC.
 *****************************************************************************/
void RTC_TimeOutHandler(void)
{
  rtcFlag = false;
}

/**************************************************************************//**
 * @brief Sleeps in EM2 in given time
 * @param msec Time in milliseconds
 *****************************************************************************/
void EM2Sleep(uint32_t msec)
{
  /* Wake us up after msec */
  NVIC_DisableIRQ(LCD_IRQn);
  rtcFlag = true;
  RTCDRV_Trigger(msec, RTC_TimeOutHandler);
  /* The rtcFlag variable is set in the RTC interrupt routine using the callback
   * RTC_TimeOutHandler. This makes sure that the elapsed time is correct. */
  while (rtcFlag)
  {
    EMU_EnterEM1();
  }
  NVIC_EnableIRQ(LCD_IRQn);
}

/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf"
 *****************************************************************************/
void ScrollText(char *scrolltext)
{
  int  i, len;
  char buffer[8];

  buffer[7] = 0x00;
  len       = strlen(scrolltext);
  if (len < 7) return;
  for (i = 0; (!recentlySaved) && (i < (len - 7)); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    SegmentLCD_Write(buffer);
    EM2Sleep(125);
  }
}

/***************************************************************************//**
 * @brief
 *   Configure the compare channel for the HiJack.
 ******************************************************************************/
static void HIJACK_CompareConfig(HIJACK_OutputMode_t outputMode)
{
  TIMER_InitCC_TypeDef txTimerCapComChConf =
  { timerEventEveryEdge,      /* Event on every capture. */
    timerEdgeRising,          /* Input capture edge on rising edge. */
    timerPRSSELCh0,           /* Not used by default, select PRS channel 0. */
    timerOutputActionNone,    /* No action on underflow. */
    timerOutputActionNone,    /* No action on overflow. */
    timerOutputActionSet,     /* No action on match. */
    timerCCModeCompare,       /* Configure capture channel. */
    false,                    /* Disable filter. */
    false,                    /* Select TIMERnCCx input. */
    true,                     /* Output high when counter disabled. */
    false                     /* Do not invert output. */
  };


  if (hijackOutputModeSet == outputMode)
  {
    txTimerCapComChConf.cmoa = timerOutputActionSet;
  }
  else if (hijackOutputModeClear == outputMode)
  {
    txTimerCapComChConf.cmoa = timerOutputActionClear;
  }
  else if (hijackOutputModeToggle == outputMode)
  {
    txTimerCapComChConf.cmoa = timerOutputActionToggle;
  }
  else
  {
    /* Config error. */
    txTimerCapComChConf.cmoa = timerOutputActionNone;
  }

  TIMER_InitCC(HIJACK_TX_TIMER, 0, &txTimerCapComChConf);
}

void enc_init(void)
{
  static const TIMER_Init_TypeDef txTimerInit =
  { false,                  /* Don't enable timer when init complete. */
    false,                  /* Stop counter during debug halt. */
    HIJACK_TIMER_RESOLUTION,/* ... */
    timerClkSelHFPerClk,    /* Select HFPER clock. */
    false,                  /* Not 2x count mode. */
    false,                  /* No ATI. */
    timerInputActionNone,   /* No action on falling input edge. */
    timerInputActionNone,   /* No action on rising input edge. */
    timerModeUp,            /* Up-counting. */
    false,                  /* Do not clear DMA requests when DMA channel is active. */
    false,                  /* Select X2 quadrature decode mode (if used). */
    false,                  /* Disable one shot. */
    false                   /* Not started/stopped/reloaded by other timers. */
  };

  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();

  /* Enable peripheral clocks. */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(HIJACK_TX_TIMERCLK, true);

  /* Configure Rx timer. */
  TIMER_Init(HIJACK_TX_TIMER, &txTimerInit);

  /* Configure Tx timer output compare channel 0. */
  HIJACK_CompareConfig(hijackOutputModeToggle);
  TIMER_CompareSet(HIJACK_TX_TIMER, 0, HIJACK_NUM_TICKS_PER_HALF_CYCLE);

  /* Route the capture channels to the correct pins, enable CC feature. */
  HIJACK_TX_TIMER->ROUTE = HIJACK_TX_LOCATION | TIMER_ROUTE_CC0PEN;

  /* Tx: Configure the corresponding GPIO pin as an input. */
  GPIO_PinModeSet(HIJACK_TX_GPIO_PORT, HIJACK_TX_GPIO_PIN, gpioModePushPull, 0);

  /* Enable Tx timer CC0 interrupt. */
  NVIC_EnableIRQ(TIMER1_IRQn);
  TIMER_IntEnable(HIJACK_TX_TIMER, TIMER_IF_CC0);

  /* Enable the timer. */
  TIMER_Enable(HIJACK_TX_TIMER, true);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initial BSP led driver .  **/
  BSP_LedsInit();
  BSP_LedSet(0);
  BSP_LedClear(0);

  /* timer1 intial to generate wave. */
  enc_init();

  /* Initialize LCD controller without boost */
  SegmentLCD_Init(false);

  /* Disable all segments */
  SegmentLCD_AllOff();

  /* Copy contents of the userpage (flash) into the userData struct */
  //memcpy((void *) &userData, (void *) USERPAGE, sizeof(UserData_TypeDef));

  /* Special case for uninitialized data */
  if (userData.number > 10000)
    userData.number = 0;
  if (userData.numWrites == 0xFFFFFFFF)
    userData.numWrites = 0;

  /* Display the number */
  SegmentLCD_Number(userData.number);

  /* Setup GPIO interrupts. PB0 to increase number, PB1 to save to flash */
  gpioSetup();

  /* No save has occured yet */
  recentlySaved = false;

  /* Main loop - just scroll informative text describing the current state of
   * the system */
  while (1)
  {
	EM2Sleep(125);
#if 0
    switch (currentError)
    {
    case mscReturnInvalidAddr:
      ScrollText("     ERROR: INVALID ADDRESS      ");
      break;
    case mscReturnLocked:
      ScrollText("     ERROR: USER PAGE IS LOCKED      ");
      break;
    case mscReturnTimeOut:
      ScrollText("     ERROR: TIMEOUT OCCURED      ");
      break;
    case mscReturnUnaligned:
      ScrollText("     ERROR: UNALIGNED ACCESS     ");
    default:
      if (recentlySaved)
      {
        recentlySaved = false;
        SegmentLCD_Number(userData.numWrites);
        ScrollText("     SAVE NUMBER       ");
      }
      else
      {
        SegmentLCD_Number(userData.number);
        ScrollText("     PRESS PB0 TO INCREASE NUMBER. PB1 TO SAVE TO INTERNAL FLASH        ");
      }
      break;
    }
#endif
  }
}


/***************************************************************************//**
 * @brief
 *   Timer1 IRQHandler.
 ******************************************************************************/
void TIMER1_IRQHandler(void)
{
  uint32_t irqFlags;

  /* Clear all pending IRQ flags. */
  irqFlags = TIMER_IntGet(HIJACK_TX_TIMER);
  TIMER_IntClear(HIJACK_TX_TIMER, irqFlags);

  /* Reset the counter and the compare value. */
  TIMER_CounterSet(HIJACK_TX_TIMER, 0);
  TIMER_CompareSet(HIJACK_TX_TIMER, 0, HIJACK_NUM_TICKS_PER_HALF_CYCLE);

}

