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
/* Driver header file(s). */
#include "segmentlcd.h"
#include "main.h"
#include "hijack_decode.h"
#include "com.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define HIJACK_DEC_NUM_TICKS_MAX	(HIJACK_NUM_TICKS_PER_1US*1200ul)
#define HIJACK_DEC_NUM_TICKS_MIN	(HIJACK_NUM_TICKS_PER_1US*700ul)

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/


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

/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
static chk_result_t IsTime2Detect(uint32_t inv);
static void dec_parser(uint8_t bit_msk, state_t state);
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

  if (TIMER_IF_CC1 & irqFlags)
  {
	prv_stamp = cur_stamp;
	cur_stamp = TIMER_CaptureGet(HIJACK_RX_TIMER, 1);
    //Debug_Print( "%d\r\n", TIMER_CaptureGet	(HIJACK_RX_TIMER, 1) ) ;
	edge_occur = true;
	BSP_LedToggle(0);
	/* Check what transition it was. */
      if (GPIO_PinInGet(HIJACK_RX_GPIO_PORT, HIJACK_RX_GPIO_PIN))
      {
        cur_edge = rising;
      }
      else
      {
        cur_edge = falling;
      }
  }
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/***************************************************************************//**
 * @brief
 *   Configure the capture channel for the HiJack.
 ******************************************************************************/
static void HIJACK_CaptureConfig(HIJACK_EdgeMode_t edgeMode)
{
  TIMER_InitCC_TypeDef rxTimerCapComChConf =
  { timerEventEveryEdge,      /* Event on every capture. */
    timerEdgeRising,          /* Input capture edge on rising edge. */
    timerPRSSELCh0,           /* Not used by default, select PRS channel 0. */
    timerOutputActionNone,    /* No action on underflow. */
    timerOutputActionNone,    /* No action on overflow. */
    timerOutputActionNone,    /* No action on match. */
    timerCCModeCapture,       /* Configure capture channel. */
    false,                    /* Disable filter. */
    false,                    /* Select TIMERnCCx input. */
    false,                    /* Clear output when counter disabled. */
    false                     /* Do not invert output. */
  };


  if (hijackEdgeModeRising == edgeMode)
  {
    rxTimerCapComChConf.edge = timerEdgeRising;
  }
  else if (hijackEdgeModeFalling == edgeMode)
  {
    rxTimerCapComChConf.edge = timerEdgeFalling;
  }
  else if (hijackEdgeModeBoth == edgeMode)
  {
    rxTimerCapComChConf.edge = timerEdgeBoth;
  }
  else
  {
    /* Config error. */
    rxTimerCapComChConf.edge = timerEdgeNone;
  }

  TIMER_InitCC(HIJACK_RX_TIMER, 1, &rxTimerCapComChConf);
}

/**
 * \brief decode initial.
 */
void dec_init(void)
{
   static const TIMER_Init_TypeDef rxTimerInit =
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

  /* Enable RX_TIMER clock . */
  CMU_ClockEnable(HIJACK_RX_TIMERCLK, true);

  /* Configure Rx timer. */
  TIMER_Init(HIJACK_RX_TIMER, &rxTimerInit);

  /* Configure Rx timer input capture channel 0. */
  HIJACK_CaptureConfig(hijackEdgeModeBoth);

  /* Route the capture channels to the correct pins, enable CC1. */
  HIJACK_RX_TIMER->ROUTE = TIMER_ROUTE_LOCATION_LOC3 | TIMER_ROUTE_CC1PEN;

  /* Rx: Configure the corresponding GPIO pin (PortD, Ch2) as an input. */
  GPIO_PinModeSet(HIJACK_RX_GPIO_PORT, HIJACK_RX_GPIO_PIN, gpioModeInput, 0);

  /* Enable Rx timer CC1 interrupt. */
  NVIC_EnableIRQ(TIMER0_IRQn);
  TIMER_IntEnable(HIJACK_RX_TIMER, TIMER_IF_CC1);

  /* Enable the timer. */
  TIMER_Enable(HIJACK_RX_TIMER, true);
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
		}
		else{
		   dec.data |= (1 << bit_msk);
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
         	}
         	else if( error == IsTime2Detect(inv) ){
				dec.state = Waiting;
         	}
	   		break;
			//
		case Bit0:
		  	dec_parser(BIT0, Bit1);
	   		break;
			//
		case Bit1:
		  	dec_parser(BIT1, Bit2);
	   		break;
			//
		case Bit2:
		  	dec_parser(BIT2, Bit3);
	   		break;
			//
		case Bit3:
		  	dec_parser(BIT3, Bit4);
	   		break;
			//
		case Bit4:
		  	dec_parser(BIT4, Bit5);
	   		break;
			//
		case Bit5:
		  	dec_parser(BIT5, Bit6);
	   		break;
			//
		case Bit6:
		  	dec_parser(BIT6, Bit7);
	   		break;
			//
		case Bit7:
		  	dec_parser(BIT7, Parity);
	   		break;
			//
		case Parity:
			if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
				if( rising == cur_edge ){
				   dec.odd++;
				}
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
				   //UART_PutChar(dec.data);
				}
				dec.state = Waiting;
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
//end of file
