/* ----------------------------------------------------------------------------
 *         ENCODE SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file encode.c
 *
 * Implements machester modulation.
 *
 */
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "encode.h"

/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
/** encode structure */
static encode_t enc;
/** store odd parity consequence */
static uint8_t odd;	
/** current index_sample */
static uint8_t index_sample = 0;
/** ticker counter. */
static uint8_t ticker = 0;

/*----------------------------------------------------------------------------
 *        local functions
 *----------------------------------------------------------------------------*/
static void HIJACK_CompareConfig(HIJACK_OutputMode_t outputMode);

/*----------------------------------------------------------------------------
 *        global functions
 *----------------------------------------------------------------------------*/
void enc_parser(uint8_t bit_msk);

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/***************************************************************************//**
 * @brief
 *   Timer1 IRQHandler.
 ******************************************************************************/
void TIMER1_IRQHandler(void)
{
  static volatile uint8_t currentPin = 1;
  static volatile uint8_t currentSym = 1;
  static volatile uint8_t txParity = 0;
  uint8_t tmp;
  uint32_t irqFlags;

  /* Clear all pending IRQ flags. */
  irqFlags = TIMER_IntGet(HIJACK_TX_TIMER);
  TIMER_IntClear(HIJACK_TX_TIMER, irqFlags);

  /* Reset the counter and the compare value. */
  TIMER_CounterSet(HIJACK_TX_TIMER, 0);
  TIMER_CompareSet(HIJACK_TX_TIMER, 0, HIJACK_NUM_TICKS_PER_1US*500);

  if (currentPin == 1)
  {
    /* First iteration check for symbol. */
    if( currentSym == 0 )
    {
      /* Have to set the output pin */
      HIJACK_CompareConfig(hijackOutputModeSet);
      currentPin = 2;
    }
    else
    {
      /* Have to reset the pin */
      HIJACK_CompareConfig(hijackOutputModeClear);
      currentPin = 2;
    }
  }
  else
  {
    /* Second time, just toggle the pin */
    HIJACK_CompareConfig(hijackOutputModeToggle);


    currentPin = 1;
#if 0
    switch(txState)
    {
    case STARTBIT:
      currentSym = 0;
      txState = BYTE;
      txBit = 0;
      txParity = 0;
      break;

    case BYTE:

      if (txBit < 8)
      {
        uint8_t tempsendingByteTx = sendingByteTx;
        currentSym = (tempsendingByteTx >> txBit) & 0x01;
        txBit++;
        tmp = txParity;
        txParity = tmp + currentSym;
      }
      else if (txBit == 8)
      {
        currentSym = txParity & 0x01;
        txBit++;
      }
      else if (txBit > 8)
      {
        /* Next bit is the stop bit. */
        currentSym = 1;
        txState = STOPBIT;
      }
      break;

    case STOPBIT:
      /* This is where an application can be signaled that transmit is done. */ 
      if (pTxDoneFunc != NULL)
      {
        pTxDoneFunc();
      }
    case IDLE:
      currentSym = 1;
      txState = IDLE;
      break;

    /* These states are not used in TX and default is to do nothing. */
    case STARTBIT_FALL:
    case DECODE:
    default:
      break;
    }
#endif
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


/**
 * \brief encode part initial.
 */
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
  TIMER_CompareSet(HIJACK_TX_TIMER, 0, HIJACK_NUM_TICKS_PER_1US*500);

  /* Route the capture channels to the correct pins, enable CC0. */
  HIJACK_TX_TIMER->ROUTE = TIMER_ROUTE_LOCATION_LOC4 | TIMER_ROUTE_CC0PEN;
 
  /* Tx: Configure the corresponding GPIO pin (PortD, Ch6) as an input. */
  GPIO_PinModeSet(HIJACK_TX_GPIO_PORT, HIJACK_TX_GPIO_PIN, gpioModePushPull, 0);  

  /* Enable Tx timer CC0 interrupt. */
  NVIC_EnableIRQ(TIMER1_IRQn);
  TIMER_IntEnable(HIJACK_TX_TIMER, TIMER_IF_CC0);

  /* Enable the timer. */
  TIMER_Enable(HIJACK_TX_TIMER, true);
}



/*
 * Find proper factor depending on state and bit mask.
*/

void enc_parser(uint8_t bit_msk)
{
  	if( ( enc.data & ( 1 << bit_msk) ) && (SET == enc.cur) ){
	  	odd++;	//bit is one, odd cnt increacement
  		enc.factor = Div1;
		if( 50 == index_sample )
		  	enc.reverse = 1;
	}
	else if( ( enc.data & ( 1 << bit_msk) ) && (CLR == enc.cur) ){
	  	odd++;	//bit is one, odd cnt increacement
		enc.factor = Div2;
		enc.cur = SET;
	}
	else if( !( enc.data & ( 1 << bit_msk) ) && (SET == enc.cur) ){
		enc.factor = Div2;
		enc.cur = CLR;
	}
	else if( !( enc.data & ( 1 << bit_msk) ) && (CLR == enc.cur) ){
		enc.factor = Div1;
		if( 50 == index_sample )
		  	enc.reverse = 1;
	}
}

/*
 * State machine for machester encoding.
 * Achieve wave phase.
*/

void encode_machine(void)
{
  	state_t sta = enc.state;
	
	switch(sta){
		case Waiting:
			if(us1_get_count() ){ 	//prepare for sta0
				enc.state = Sta0; 	//next state is start bit
				enc.factor = Div2;  //start bit is zero
				enc.cur = CLR;
				enc.data = us1_get_char();
				index_sample = 0;
			}
			break;
			//
		case Sta0: 	//prepare for bit0
		  	odd = 0;	//clear odd cnt
		  	enc_parser(BIT0);
	  		enc.state = Bit0;
			break;
			//
		case Sta1:	//prepare for sta2
		  	enc.state = Sta2;
			break;
			//
	  	case Sta2:	//prepare for sta3
		  	enc.state = Sta3;
			break;
			//
		case Sta3:	//prepare for bit7, sta3 is SET
		  	if( enc.data & (1 << BIT7) ){
		  		enc.factor = Div1;
				enc.cur = SET;
				if( 50 == index_sample )
		  			enc.reverse = 1;
		  	}
			else{
				enc.factor = Div2;
				enc.cur = CLR;	
			}
		  	enc.state = Bit7;
			break;
			//	
		case Bit0: 	//prepare for Bit1
		  	enc_parser(BIT1);
			enc.state = Bit1;
	    	break;
			//
		case Bit1: 	//prepare for Bit2
		  	enc_parser(BIT2);
			enc.state = Bit2;
	    	break;
			//
		case Bit2: 	//prepare for Bit3
		  	enc_parser(BIT3);
			enc.state = Bit3;
	    	break;
			//
		case Bit3: 	//prepare for Bit4
		  	enc_parser(BIT4);
			enc.state = Bit4;
	    	break;
			//
		case Bit4: 	//prepare for Bit5
		  	enc_parser(BIT5);
			enc.state = Bit5;
	    	break;
			//
		case Bit5: 	//prepare for Bit6
		  	enc_parser(BIT6);
			enc.state = Bit6;
	    	break;
			//
		case Bit6: 	//prepare for Bit7
		  	enc_parser(BIT7);
			enc.state = Bit7;
	    	break;
			//
		case Bit7: 	//prepare for Parity
		  	if( ( 0 == ( odd % 2 ) ) && (SET == enc.cur) ){//there is even 1(s), output 1, cur is 1 	
				enc.factor = Div1;
				enc.cur = SET;
				if( 50 == index_sample )
					enc.reverse = 1;
			}
			else if( ( 0 == ( odd % 2 ) ) && (CLR == enc.cur) ){//there is even 1(s), output 1, cur is 0 	
				enc.factor = Div2;
				enc.cur = SET;
			}
			else if( ( 1 == ( odd % 2 ) ) && (CLR == enc.cur) ){//there is odd 1(s), output 0, cur is 0 	
				enc.factor = Div1;
				enc.cur = CLR;
				if( 50 == index_sample )
					enc.reverse = 1;
			}
			else if( ( 1 == ( odd % 2 ) ) && (SET == enc.cur) ){//there is odd 1(s), output 0, cur is 1
				enc.factor = Div2;
				enc.cur = CLR;
			}
			enc.state = Parity;
	    	break;
			//
		case Parity:	//prepare for stop bit
		  	if(SET == enc.cur){
		  		enc.factor = Div1;
				enc.cur = SET;
				if( 50 == index_sample )
					enc.reverse = 1;	
			}
			else{
				enc.factor = Div2;
				enc.cur = CLR;
			}
			enc.state = Sto0;
		  	break;
            //
		case Sto0:     //prepare for Waiting
			enc.state = Waiting;
			enc.factor = Div1;
			enc.cur = SET;
			break;
			//
		default:
	  		break;
	  		//
	}
}


//end of file
