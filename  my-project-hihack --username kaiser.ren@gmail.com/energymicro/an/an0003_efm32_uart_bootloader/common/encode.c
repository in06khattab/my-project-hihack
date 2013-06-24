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
//#include "com.h"
#include "board.h"
/*----------------------------------------------------------------------------
 *        local variable
 *----------------------------------------------------------------------------*/
/** encode structure */
static encode_t enc;
static uint8_t enc_odd ;

/*----------------------------------------------------------------------------
 *        global variable
 *----------------------------------------------------------------------------*/
uint32_t ticker = 0 ;
//uint8_t encTmr = 0 ;
volatile buffer_t encBuf = { 0, 0, 0, false, NULL};
/*----------------------------------------------------------------------------
 *        local functions
 *----------------------------------------------------------------------------*/
static void HIJACK_CompareConfig(HIJACK_OutputMode_t outputMode);

/*----------------------------------------------------------------------------
 *        global functions
 *----------------------------------------------------------------------------*/
void enc_parser(uint8_t bit_msk);
void enc_machine(void);
/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
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

  //if (encTmr)
  	//encTmr--;

  ticker++;

  //system timer tick for delay
  sys_tick++;

  /* Reset the counter and the compare value. */
  TIMER_CounterSet(HIJACK_TX_TIMER, 0);
  TIMER_CompareSet(HIJACK_TX_TIMER, 0, HIJACK_NUM_TICKS_PER_HALF_CYCLE);

  enc_machine();
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

/*
 * Find proper factor depending on state and bit mask.
*/

void findParam(uint8_t bit_msk, state_t state)
{
	if( 0 == ticker % 2){
		if( enc.data & ( 1 << bit_msk) ){
			enc.edge = rising;
         HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, for rising
			enc_odd++;
		}
		else{
			enc.edge = falling;
         	HIJACK_CompareConfig(hijackOutputModeSet);	//next is 0x00, for falling
		}
	}
	else{
	  	if( rising == enc.edge ){
		  	HIJACK_CompareConfig(hijackOutputModeSet);	//rising	
	  	}
	  	else{
		  	HIJACK_CompareConfig(hijackOutputModeClear);	//falling
	  	}
		enc.state = state;	//state switch
	}
}

/******************************************************************************
 * @brief  encGetAmount function
 *
 *****************************************************************************/
uint32_t encGetAmount(void)
{
  //return( encTmr? 0 :encBuf.pendingBytes);
  return( encBuf.pendingBytes);
}

/******************************************************************************
 * @brief  encGetChar function
 *
 *****************************************************************************/
uint8_t encGetChar( )
{
  uint8_t ch;

  /* Copy data from buffer */
#if CRITICAL_PROTECTION==1
  INT_Disable();
#endif
  ch        = encBuf.data[encBuf.rdI];
  encBuf.rdI = (encBuf.rdI + 1) % BUFFERSIZE;

#if CRITICAL_PROTECTION==1
  INT_Enable();
#endif

  /* Decrement pending byte counter */
  encBuf.pendingBytes--;

  return ch;
}

/******************************************************************************
 * @brief  uartPutData function
 *
 *****************************************************************************/
void encPutData(uint8_t * dataPtr, uint32_t dataLen)
{
  uint32_t i = 0;

  /* Check if buffer is large enough for data */
  if (dataLen > BUFFERSIZE)
  {
    /* Buffer can never fit the requested amount of data */
    return;
  }

  /* Check if buffer has room for new data */
  if ((encBuf.pendingBytes + dataLen) > BUFFERSIZE)
  {
    /* Wait until room */
    while ((encBuf.pendingBytes + dataLen) > BUFFERSIZE) ;
  }

  /* Fill dataPtr[0:dataLen-1] into txBuffer */
#if CRITICAL_PROTECTION==1
  INT_Disable();
#endif

  while (i < dataLen)
  {
    encBuf.data[encBuf.wrI] = *(dataPtr + i);
    encBuf.wrI             = (encBuf.wrI + 1) % BUFFERSIZE;
    i++;
  }
#if CRITICAL_PROTECTION==1
  INT_Enable();
#endif
  /* Increment pending byte counter */
  encBuf.pendingBytes += dataLen;
}
/*
 * format print.
*/
void enc_print(const char *format, ...)
{
  uint8_t 		appPrintBuf[BUFFERSIZE];
  char *s = (char *)appPrintBuf;
  char nr_of_chars;
  int ans;
  va_list ap;
  _SProutInfo x;

  va_start(ap, format);

  x.s = s;

  ans = _Printf(&_SProut, &x, format, &ap);
        *x.s = '\0';
  va_end(ap);
	
  nr_of_chars = (ans < 0 ? ans : (x.s - s));

  if (nr_of_chars > 0) {
	encPutData(appPrintBuf, nr_of_chars);
  }
}

/*
 * State machine for machester encoding.
 * Achieve wave phase.
*/
void enc_machine(void)
{
  	state_t sta = enc.state;

	switch(sta){
		case Waiting:
		  	if( 0 == ticker % 2){
				if( encGetAmount() ){
					enc.data = encGetChar();
					enc.byte_rev = 1;
					HIJACK_CompareConfig(hijackOutputModeSet);	//next is 0x00, falling
					enc_odd = 0;
				}
				else{
					HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, rising	
					enc.byte_rev = 0;
				}
		  	}
			else{
			  	if(enc.byte_rev){//new byte
			  	 	HIJACK_CompareConfig(hijackOutputModeClear);	//falling
					enc.state = Sta0;	//state switch
				}
				else{	//no byte
					HIJACK_CompareConfig(hijackOutputModeSet);	//rising, keep in waiting state
				}
			}
			break;
			//
		case Sta0: 	//prepare for sta1, rising edge
			findParam(BIT0, Bit0);
			break;
			//
		case Sta1:	//prepare for sta2, falling edge
		  	if( 0 == ticker % 2){
			 	HIJACK_CompareConfig(hijackOutputModeSet);		//next is 0x00, for falling
			}
			else{
				HIJACK_CompareConfig(hijackOutputModeClear);	//falling
				enc.state = Sta2; 	//state switch
			}
			break;
			//
	  	case Sta2:	//prepare for sta3, rising edge
		  	if( 0 == ticker % 2){
			 	HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, for rising
			}
			else{
				HIJACK_CompareConfig(hijackOutputModeSet);	//rising
				enc.state = Sta3;	//state switch
			}
			break;
			//
		case Sta3:	//prepare for bit7
		  	findParam(BIT7, Bit7);
			break;
			//
		case Bit0: 	//prepare for Bit1
		  	findParam(BIT1, Bit1);
	    	break;
			//
		case Bit1: 	//prepare for Bit2
		  	findParam(BIT2, Bit2);
	    	break;
			//
		case Bit2: 	//prepare for Bit3
		  	findParam(BIT3, Bit3);
	    	break;
			//
		case Bit3: 	//prepare for Bit4
		  	findParam(BIT4, Bit4);
	    	break;
			//
		case Bit4: 	//prepare for Bit5
		  	findParam(BIT5, Bit5);
	    	break;
			//
		case Bit5: 	//prepare for Bit6
		  	findParam(BIT6, Bit6);
	    	break;
			//
		case Bit6: 	//prepare for Bit7
		  	findParam(BIT7, Bit7);
	    	break;
			//
		case Bit7: 	//prepare for parity
		  	if( 0 == ticker % 2){
				if( 0 == ( enc_odd%2 ) ){//there is even 1(s), output 1
					enc.edge = rising;
					HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, for rising
				}
				else{//there is odd 1(s), output 0
					enc.edge = falling;
					HIJACK_CompareConfig(hijackOutputModeSet);	//next is 0x00, for falling
				}
			}
			else{
				if( rising == enc.edge ){
					HIJACK_CompareConfig(hijackOutputModeSet);	//rising	
				}
				else{
					HIJACK_CompareConfig(hijackOutputModeClear);	//falling
				}
				enc.state = Parity;	//state switch
			}
	    	break;
			//
		case Parity://prepare for stop bit, it is always 1
		  	if( 0 == ticker % 2){
				HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, for rising
			}
			else{
				HIJACK_CompareConfig(hijackOutputModeSet);	//rising	
				enc.state = Sto0;	//state switch
			}
        	break;
			//
		case Sto0:     //back to waiting
			if( 0 == ticker % 2){
				HIJACK_CompareConfig(hijackOutputModeClear);	//next is 0x80, for rising
			}
			else{
				HIJACK_CompareConfig(hijackOutputModeSet);	//rising	
				enc.state = Waiting;	//state switch
			}
			break;
			//
#if 0
		case Sto1:		//go to waiting mode
			enc.state = Waiting;
			break;
			//
#endif
		default:
	  		break;
	  		//
	}
}


//end of file
