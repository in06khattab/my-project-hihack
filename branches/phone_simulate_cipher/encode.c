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
#include "xplained.h"

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

/** channel 0 */
uint8_t DACC_channel_sine = DACC_CHANNEL_0;
/** frequency */
uint16_t frequency = 0;
/** amplitude */
uint16_t amplitude = 0;
/** offset. */
int8_t bias = 0;
/*----------------------------------------------------------------------------
 *        Static functions
 *----------------------------------------------------------------------------*/
void enc_parser(uint8_t bit_msk);
static void _ConfigureTc0( uint32_t freq );
/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/**
 *  \brief Interrupt handler for DACC.
 *
 * Server routine when DACC complete the convertion.
 */
void DAC_IrqHandler(void)
{
	uint32_t status ;

    status = DACC_GetStatus( DACC ) ;

    /* if conversion is done*/
    if ( (status & DACC_IER_EOC) == DACC_IER_EOC ){
		if ( index_sample >= SAMPLES ){
		  	if(0 == enc.reverse)
				encode_machine();
			index_sample = 0;
			ticker = 0;
		}
		else if( ( (SAMPLES/2) == index_sample ) && (Div2 == enc.factor ) ){
			encode_machine();
			ticker = 0;
		}
		else if( ( (SAMPLES/2) == index_sample ) && ( 1 == enc.reverse ) ){
		  	enc.reverse = 0;
			encode_machine();
			ticker = 0;
		}
		DACC->DACC_IDR = DACC_IER_EOC;
	}

}

/**
 *  \brief Interrupt handler for UART0.
 *
 */
void UART0_IrqHandler(void)
{
    uint32_t status;

    /* Read USART status*/
    status = UART0->UART_SR;

    /* Receive byte is stored in buffer. */
    if ((status & UART_SR_RXRDY) == UART_SR_RXRDY) {
		if(us1.count < US_BUFFER_SIZE){
	    	us1.buff[us1.head++] = UART_GetChar();
			us1.count++;
			if(us1.head >= US_BUFFER_SIZE)
				us1.head = 0;
	  	}
		else{
			us1.buff[us1.head] = UART_GetChar();
		}
    }
}

/**
 *  \brief Interrupt handler for SysTick.
 *
 */
void SysTick_Handler( void )
{
	uint16_t value;

    {
		if ( 0 == ( ticker%enc.factor) ){
			value = sine_data[index_sample++] * amplitude / (MAX_DIGITAL/2)  \
			  		+ MAX_DIGITAL/2 /*- MAX_DIGITAL/50*/ + BIAS_STEP*bias ;
        	DACC_SetConversionData(DACC, value ) ;
			DACC->DACC_IER = DACC_IER_EOC;
		}
		ticker++;
    }
}

/**
 *  \brief Interrupt handler for TC0.
 *
 */
void TC0_IrqHandler( void )
{
  	uint32_t status ;
    status = REG_TC0_SR0 ;

	if ( (status & TC_SR_CPCS) == TC_SR_CPCS ){
#if defined	__SAM4S16C__
		XplnLED_Toggle(1);	//LED1 toggle
#endif
	}

}

/**
 * \brief DAC initialization.
 */
void enc_init(void)
{
	/* initialize amplitude and frequency */
    amplitude = MAX_DIGITAL * 2 / 5;
    frequency = HIJACK_CARRIER_FREQ_CONF;

    /*10 us timer*/
    SysTick_Config( BOARD_MCK / (frequency * SAMPLES) ) ;
	//_ConfigureTc0( HIJACK_CARRIER_FREQ_8KHZ );
	//TC_Start( TC0, 0 ) ;
	
    /* Initialize DACC */
    DACC_Initialize( DACC,
                    ID_DACC,
                    0, /* Hardware triggers are disabled */
                    0, /* External trigger */
                    0, /* Half-Word Transfer */
                    0, /* Normal Mode (not sleep mode) */
                    BOARD_MCK,
                    8, /* refresh period */
                    0, /* Channel 0 selection */
                    0, /* Tag Selection Mode disabled */
                    16 /*  value of the start up time */);

	/* enable NVIC_DACC. */
	NVIC_EnableIRQ( DACC_IRQn ) ;
	NVIC_SetPriority(DACC_IRQn, 5);
	
	/* variable initial. */
	enc.data = 0;
	enc.state = Waiting;
	enc.cur = SET;
	enc.factor = Div1;
	enc.reverse = 0;
	ticker = 0;
	index_sample = 0;
	
	/*Enable  channel for potentiometer*/
    DACC_EnableChannel( DACC, DACC_channel_sine ) ;

    /*initialize the DACC_CDR*/
    DACC_SetConversionData( DACC,sine_data[90]*amplitude/(MAX_DIGITAL/2)+MAX_DIGITAL/2);
	 DACC->DACC_IER = DACC_IER_EOC;	//Enable DACC end-of-convertion interrupt
}

/******************************************************************************
 * @brief  HIJACKPutData function
 *
 *****************************************************************************/
void HIJACKPutData(uint8_t * dataPtr, buffer_t * dstBuf, uint32_t dataLen)
{
  uint8_t i = 0;
  uint8_t len = 0;
  uint8_t round = 0;
  buffer_t* ptr = dstBuf;

  /* Check if buffer is large enough for data */
  if (dataLen > BUFFERSIZE)
  {
    /* Buffer can never fit the requested amount of data */
    return;
  }

  /* Check if buffer has room for new data */
  if ((ptr->pendingBytes + dataLen) > BUFFERSIZE)
  {
    /* Wait until room */
    while ( (ptr->pendingBytes + dataLen) > BUFFERSIZE) ;
  }

  /* Fill dataPtr[0:dataLen-1] into txBuffer */
#if CRITICAL_PROTECTION==1
  __disable_irq();
#endif

  while (i < dataLen){
    ptr->data[ptr->wrI] = *(dataPtr + i);
    ptr->wrI             = (ptr->wrI + 1) % BUFFERSIZE;
    i++;
  }

  /* Increment pending byte counter */
  //ptr->pendingBytes += dataLen;

  // Calculate the round
  //round = 16 - ( ( dataLen % 16 ) == 0 ? 16 : ( dataLen % 16 ) ) ;
  //len = dataLen + round;
  //ptr->wrI = (ptr->wrI + round) % BUFFERSIZE;

#if CRITICAL_PROTECTION==1
  __enable_irq();
#endif

  ptr->pendingBytes += dataLen;
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief TC0 configuration
 *
 * Configures Timer Counter 0 (TC0) to generate an interrupt every second. This
 * interrupt will be used to display the number of bytes received on the USART.
 */

static void _ConfigureTc0( uint32_t freq )
{
    /* Enable TC0 peripheral clock*/
    PMC_EnablePeripheral( ID_TC0 ) ;

    /*
	 	CLK SRC: MCK/2, 64MHz/2 = 32MHz.
	 	RC Compare trigger, RC Compare resets the counter and starts the counter clock.
	 */
    TC_Configure( TC0, 0, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG ) ;

    TC0->TC_CHANNEL[0].TC_RC = 32*100 ;

    /* Configure interrupt on RC compare*/
    TC0->TC_CHANNEL[0].TC_IER = TC_SR_CPCS ;

	NVIC_DisableIRQ( TC0_IRQn ) ;
    NVIC_ClearPendingIRQ( TC0_IRQn ) ;
    NVIC_SetPriority( TC0_IRQn, 0 ) ;
    NVIC_EnableIRQ( TC0_IRQn ) ;

}

/**
 *  \brief UART0 hardware configuration
 *
 * Configures UART0 in hardware handshaking mode, asynchronous, 8 bits, 1 stop
 * bit, no parity, 115200 bauds and enables its transmitter and receiver.
 */
void _ConfigureCom( void )
{
  	/* Enaboe UART0 interrupt*/
    NVIC_EnableIRQ( UART0_IRQn ) ;
	/* Enable UART0 RXREADY interrupt. */
	UART0->UART_IER = UART_IER_RXRDY ;
}
/*
 * Get us1 buffer amount.
*/
uint8_t us1_get_count(void)
{
  	return (us1.count);
}

/*
 * Get a charactors.
*/
uint8_t us1_get_char(void)
{
  	uint8_t temp;
	
	if(us1.count){
#if CRITICAL_PROTECTION==1
		__disable_irq();
#endif
    	temp = us1.buff[us1.tail++];
		us1.count--;
		if(us1.tail >= US_BUFFER_SIZE){
			us1.tail = 0;
		}
#if CRITICAL_PROTECTION==1
		__enable_irq();
#endif
  	}
    return temp;
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
