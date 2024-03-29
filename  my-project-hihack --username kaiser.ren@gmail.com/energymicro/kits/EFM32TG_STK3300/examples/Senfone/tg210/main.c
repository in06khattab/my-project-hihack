/**************************************************************************//**
 * @file main.c
 * @brief sensorfone
 * Author: kaiser ren
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_timer.h"

/* Driver header file(s). */
#include "segmentlcd.h"

/* HiJack header file. */
#include "com.h"
#include "decode.h"
#include "encode.h"

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
  //HIJACK_ByteTx(0x55);
}


/**************************************************************************//**
 * @brief GPIO Interrupt handler (PD8)
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 8);
  //HIJACK_ByteTx(0xAA);

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
 * @brief output CLK_OUT0, PC12 .
 *****************************************************************************/
void CMU_Clkout(void)
{
  /* Set divider */
  /* Configure GPIO pins */
  GPIO_PinModeSet(gpioPortC, 12, gpioModePushPull, 0);
  CMU->ROUTE = CMU_ROUTE_CLKOUT0PEN | CMU_ROUTE_LOCATION_LOC1;
  CMU->CTRL = (CMU->CTRL & ~_CMU_CTRL_CLKOUTSEL0_MASK) |
      ((_CMU_CTRL_CLKOUTSEL0_HFRCO) << _CMU_CTRL_CLKOUTSEL0_SHIFT);
}
/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  //BSP_TraceProfilerSetup();

  /* configure SWO output for debugging. */
  setupSWO();

  /* configure HFRCO @ 7MHz. */
  CMU_HFRCOBandSet(	cmuHFRCOBand_7MHz ) 	;

  /* Select clock source for HF clock. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

  /* Enable peripheral clocks. */
  CMU_ClockEnable(cmuClock_HFPER, true);

  /* Configure push button interrupts. */
  //gpioSetup();

  /* Configure clk_out0 for debug. */
  CMU_Clkout();

  /* Initialize LED driver */
  BSP_LedsInit();

  /* Init Segment LCD without boost. */
  //SegmentLCD_Init(false);

  /* Turn on relevant symbols on the LCD. */
  //SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
  //SegmentLCD_Symbol(LCD_SYMBOL_ANT, true);

  /* Print welcome text on the LCD. */
  //SegmentLCD_Write("senfone");

  /* dec part initial. */
  dec_init();
  enc_init();

  /* Configure COM port, USART1. */
  COM_Init();

  /* print startup. */
  /* Output example information */
  Debug_Print( "-- senfone V%d.%02d%c --\r\n", VER_MAJOR, VER_MINOR, VER_PATCH ) ;
  Debug_Print( "-- Compiled: %s %s --\r\n", __DATE__, __TIME__ ) ;
  Debug_Print( "-- Platform: EFM32 %s %uHz --\r\n", DEV, CMU_ClockFreqGet(cmuClock_HFPER) ) ;

  /* While loop sending a range of values upon wakeup.  */
  while(1){
	if(edge_occur){
		edge_occur = false;
		decode_machine();
	}
	EMU_EnterEM1();
  }
}
