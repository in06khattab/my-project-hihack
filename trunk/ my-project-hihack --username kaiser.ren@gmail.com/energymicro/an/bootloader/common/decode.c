/* ----------------------------------------------------------------------------
 *         ENCODE SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file decode.c
 *
 * Implements machester modulation.
 *
 */
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdio.h>
/* Chip specific header file(s). */
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_int.h"
#include "em_acmp.h"
#include "em_prs.h"
/* Driver header file(s). */
#include "board.h"
//#include "segmentlcd.h"
#include "main.h"
#include "decode.h"
#include "encode.h"
//#include "com.h"
#include "usart.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define HIJACK_DEC_NUM_TICKS_MAX	(HIJACK_NUM_TICKS_PER_FULL_CYCLE + HIJACK_NUM_TICKS_PER_20_PCNT)
#define HIJACK_DEC_NUM_TICKS_MIN	(HIJACK_NUM_TICKS_PER_FULL_CYCLE - HIJACK_NUM_TICKS_PER_20_PCNT)

/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
decode_t dec;
bool edge_occur = false;
uint32_t cur_stamp = 0 ;
uint32_t prv_stamp = 0 ;
edge_t 	cur_edge = none ;
static uint32_t offset = 0;
static uint32_t inv = 0;
buffer_t decBuf = { 0, 0, 0, false, NULL};
/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
static chk_result_t IsTime2Detect(uint32_t inv);
static void dec_parser(uint8_t bit_msk, state_t state);
static void ACMP_setup(void);
static void TIMER_setup(void) ;
/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/***************************************************************************//**
 * @brief
 *   RxTimer, Timer0 IRQHandler.
 ******************************************************************************/
void TIMER0_IRQHandler(void)
{
  	uint32_t irqFlags;

  	irqFlags = TIMER_IntGet(HIJACK_RX_TIMER);
  	TIMER_IntClear(HIJACK_RX_TIMER, irqFlags);

 	if (TIMER_IF_CC0 & irqFlags)
  	{
		cur_stamp = TIMER_CaptureGet(HIJACK_RX_TIMER, 0);
		
		/* Check what transition it was. */
    	if(ACMP0->STATUS & ACMP_STATUS_ACMPOUT)
    	{
       	cur_edge = rising;
			//BSP_LedSet( 0 );
         //GPIO_PinOutSet(HIJACK_SMBUS_GPIO_PORT, HIJACK_SMBUS_GPIO_SCL);
			//Debug_Print( "%d ", cur_stamp ) ;
    	}
    	else
    	{
      	cur_edge = falling;
			//BSP_LedClear( 0 );
         //GPIO_PinOutClear(HIJACK_SMBUS_GPIO_PORT, HIJACK_SMBUS_GPIO_SCL);
			//Debug_Print( "%d ", cur_stamp ) ;
    	}
		decode_machine();
  	}
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/


/**
 * \brief decode initial.
 */
void dec_init(void)
{
	/* Initialise the ACMP */
  	ACMP_setup();

  	/* Initialise the TIMER */
  	TIMER_setup();
	
	//------------------------------------------------------------
  	// THE INTERRUPT IS SIMPLY TO DISPLAY THE CAPTURE ON THE LCD
  	//------------------------------------------------------------
  	/* Enable CC0 interrupt */
  	TIMER_IntEnable(HIJACK_RX_TIMER, TIMER_IF_CC0);

  	/* Enable TIMER0 interrupt vector in NVIC */
  	NVIC_EnableIRQ(TIMER0_IRQn);
}

/**************************************************************************//**
 * @brief  TIMER0_setup
 * Configures the TIMER
 *****************************************************************************/
static void TIMER_setup(void)
{
  /* Enable necessary clocks */
  CMU_ClockEnable(cmuClock_TIMER0, true);
  CMU_ClockEnable(cmuClock_PRS, true);

  /* Select CC channel parameters */
  const TIMER_InitCC_TypeDef timerCCInit =
  {
    .eventCtrl  = timerEventEveryEdge,      /* Input capture event control */
    .edge       = timerEdgeBoth,       /* Input capture on falling edge */
    .prsSel     = timerPRSSELCh5,         /* Prs channel select channel 5*/
    .cufoa      = timerOutputActionNone,  /* No action on counter underflow */
    .cofoa      = timerOutputActionNone,  /* No action on counter overflow */
    .cmoa       = timerOutputActionNone,  /* No action on counter match */
    .mode       = timerCCModeCapture,     /* CC channel mode capture */
    .filter     = false,                  /* No filter */
    .prsInput   = true,                   /* CC channel PRS input */
    .coist      = false,                  /* Comparator output initial state */
    .outInvert  = false,                  /* No output invert */
  };

  /* Initialize TIMER0 CC0 channel */
  TIMER_InitCC(HIJACK_RX_TIMER, 0, &timerCCInit);

  /* Select timer parameters */
  const TIMER_Init_TypeDef timerInit =
  {
    .enable     = false,                        /* Do not start counting when init complete */
    .debugRun   = false,                        /* Counter not running on debug halt */
    .prescale   = HIJACK_TIMER_RESOLUTION,      /* Prescaler of 1 */
    .clkSel     = timerClkSelHFPerClk,          /* TIMER0 clocked by the HFPERCLK */
    .fallAction = timerInputActionReloadStart,         /* Stop counter on falling edge */
    .riseAction = timerInputActionReloadStart,  /* Reload and start on rising edge */
    .mode       = timerModeUp,                  /* Counting up */
    .dmaClrAct  = false,                        /* No DMA */
    .quadModeX4 = false,                        /* No quad decoding */
    .oneShot    = false,                        /* Counting up constinuously */
    .sync       = false,                        /* No start/stop/reload by other timers */
  };

  /* Initialize TIMER0 */
  TIMER_Init(HIJACK_RX_TIMER, &timerInit);

  /* PRS setup */
  /* Select ACMP as source and ACMP0OUT (ACMP0 OUTPUT) as signal */
  PRS_SourceSignalSet(5, PRS_CH_CTRL_SOURCESEL_ACMP0, PRS_CH_CTRL_SIGSEL_ACMP0OUT, prsEdgeOff);
}

/**************************************************************************//**
 * @brief  ACMP_setup
 * Configures and starts the ACMP
 *****************************************************************************/
static void ACMP_setup(void)
{
  /* Enable necessary clocks */
  CMU_ClockEnable(HIJACK_RX_ACMPCLK, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure ACMP input pin. */
  GPIO_PinModeSet(HIJACK_RX_GPIO_PORT, HIJACK_RX_GPIO_PIN, gpioModeInput, 0);

  /* Analog comparator parameters */
  const ACMP_Init_TypeDef acmpInit =
  {
    .fullBias                 = false,                  /* No full bias current*/
    .halfBias                 = true,                  /* No half bias current */
    .biasProg                 = 2,                      /* Biasprog current 1.4 uA */
    .interruptOnFallingEdge   = false,                  /* Disable interrupt for falling edge */
    .interruptOnRisingEdge    = false,                  /* Disable interrupt for rising edge */
    .warmTime                 = acmpWarmTime256,        /* Warm-up time in clock cycles, should be >140 cycles for >10us warm-up @ 14MHz */
    .hysteresisLevel          = acmpHysteresisLevel7,   /* Hysteresis level 0  - no hysteresis */
    .inactiveValue            = 1,                      /* Inactive comparator output value */
    .lowPowerReferenceEnabled = false,                  /* Low power reference mode disabled */
    .vddLevel                 = HIJACK_RX_ACMP_LEVEL,                     /* Vdd reference scaling of 32 */
  };

  /* Use ACMP0 output, PD6 . */
  //GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
  //ACMP_GPIOSetup(ACMP0, 2, true, false);

  /* Init ACMP and set ACMP channel 4 as positive input
     and scaled Vdd as negative input */
  ACMP_Init(HIJACK_RX_ACMP, &acmpInit);
  ACMP_ChannelSet(HIJACK_RX_ACMP, HIJACK_RX_ACMP_NEG, HIJACK_RX_ACMP_CH);
  ACMP_Enable(HIJACK_RX_ACMP);
}

/**
 * @brief calculate whether cnt is in 500us region
 * ticker = 64Mhz/128 = 2us
 * 475us < cnt < 510us
 *
 */
static chk_result_t IsTime2Detect(uint32_t inv)
{
  	chk_result_t ret;
	
  	if( inv < HIJACK_DEC_NUM_TICKS_MIN){
    	offset = inv;
	    ret = pass;
  	}
	else if ( ( inv <= HIJACK_DEC_NUM_TICKS_MAX ) && ( inv >= HIJACK_DEC_NUM_TICKS_MIN ) ) {
		offset = 0;
		inv = 0;
	  	ret = suit;
	}
	else{
		offset = 0;
		inv = 0;
		ret = error;
	}
	return ret;
}

/*
 * Find phase remain or phase reversal.
*/

static void dec_parser(uint8_t bit_msk, state_t state)
{
  	if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
		if( falling == cur_edge ){
	  		dec.data &= ~(1 << bit_msk);
#if DEC_DEBUG == 1
			uartPutChar( '+' ) ;
			uartPutChar( '_' ) ;
#endif//DEC_DEBUG == 1
		}
		else{
		   	dec.data |= (1 << bit_msk);
#if DEC_DEBUG == 1
			uartPutChar( '_' ) ;
			uartPutChar( '+' ) ;
#endif//DEC_DEBUG == 1
			dec.odd++;
		}
		dec.state = state;   //state switch
	}
	else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
		dec.state = Waiting;   //state switch
	}
}

/**
 * @brief decode state machine.
 *
 */
void decode_machine(void)
{
   	inv = offset + cur_stamp;  //update offset

	switch (dec.state){
		case Waiting:
         	/* go to start bit if rising edge exist. */
         	if (rising == cur_edge) {
            	dec.state = Sta0;
            	offset = 0;
				inv = 0;
         	}
			break;
			//
		case Sta0:
         	if( ( suit == IsTime2Detect(inv) ) && ( falling == cur_edge ) ){
				dec.data = 0;  //clear data field for store new potential data
				dec.odd = 0;   //clear odd field parity counter
				dec.state = Bit0;
#if DEC_DEBUG == 1
			  	//uartPutChar( 'S' ) ;
				uartPutChar( '+' ) ;
				uartPutChar( '_' ) ;
#endif
         	}
			else{
				dec.state = Waiting;
			}
	   		break;
			//
		case Bit0:
#if DEC_DEBUG == 1
			//uartPutChar( '0' ) ;
#endif
		  	dec_parser(BIT0, Bit1);
	   		break;
			//
		case Bit1:
#if DEC_DEBUG == 1
			//uartPutChar( '1' ) ;
#endif
		  	dec_parser(BIT1, Bit2);
	   		break;
			//
		case Bit2:
#if DEC_DEBUG == 1
			//uartPutChar( '2' ) ;
#endif
		  	dec_parser(BIT2, Bit3);
	   		break;
			//
		case Bit3:
#if DEC_DEBUG == 1
			//uartPutChar( '3' ) ;
#endif
		  	dec_parser(BIT3, Bit4);
	   		break;
			//
		case Bit4:
#if DEC_DEBUG == 1
			//uartPutChar( '4' ) ;
#endif
		  	dec_parser(BIT4, Bit5);
	   		break;
			//
		case Bit5:
#if DEC_DEBUG == 1
			//uartPutChar( '5' ) ;
#endif
		  	dec_parser(BIT5, Bit6);
	   		break;
			//
		case Bit6:
#if DEC_DEBUG == 1
			//uartPutChar( '6' ) ;
#endif
		  	dec_parser(BIT6, Bit7);
	   		break;
			//
		case Bit7:
#if DEC_DEBUG == 1
			//uartPutChar( '7' ) ;
#endif
		  	dec_parser(BIT7, Parity);
	   		break;
			//
		case Parity:		
			if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
				if( rising == cur_edge ){
				   dec.odd++;
#if DEC_DEBUG == 1
				   uartPutChar( '_' ) ;
				   uartPutChar( '+' ) ;
#endif
				}
				else{
#if DEC_DEBUG == 1
				   uartPutChar( '+' ) ;
				 	uartPutChar( '_' ) ;
#endif					
				}
#if DEC_DEBUG == 1
					uartPutChar( dec.odd + 0x30) ;
#endif
				if( 1 == (dec.odd%2)){  //parity pass
				   dec.state = Sto0;
				}
				else{ //parity failed
				   dec.state = Waiting;
				}
			 }
			 else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
				dec.state = Waiting;
			 }
			break;
			//
      	case Sto0:	
         	if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
				if( rising == cur_edge ){  //stop bit is rising edge
				    //USART_txByte(dec.data);
#if DEC_DEBUG == 1
					//uartPutChar( 'P' ) ;
					uartPutChar( '_' ) ;
				    uartPutChar( '+' ) ;
#endif
				    HIJACKPutData(&dec.data, &decBuf, sizeof(uint8_t));
				   //encTmr = 100;
				}
				else{
#if DEC_DEBUG == 1
				  	uartPutChar( '+' ) ;
				 	uartPutChar( '_' ) ;
#endif			
				}
				dec.state = Waiting;
#if DEC_DEBUG == 1
				uartPutChar( '\r' ) ;
				uartPutChar( '\n' ) ;
#endif
         	}
         	else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
				dec.state = Waiting;
			}
         	break;
         	//
		default:
	  		break;
			//
	}
}

/**************************************************************************//**
 * @brief Decode single byte to BOOTLOADER_Hijack
 *****************************************************************************/
__ramfunc uint8_t dec_rxByte(void)
{
  uint8_t ch;

  while ( !decBuf.pendingBytes ) ;
  /* Copy data from buffer */
#if CRITICAL_PROTECTION==1
  INT_Disable();
#endif
  ch        = decBuf.data[decBuf.rdI];
  decBuf.rdI = (decBuf.rdI + 1) % BUFFERSIZE;
  /* Decrement pending byte counter */
  decBuf.pendingBytes--;

#if CRITICAL_PROTECTION==1
  INT_Enable();
#endif
  USART_txByte( ch ) ;
  return( ch );
}
//end of file
