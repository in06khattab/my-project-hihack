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
#include "hal.h"

encode_t enc;
uint8_t tmr0_occur = 0;
uint8_t ticker = 0 ;

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/**
 *  \brief Interrupt handler for TMR0.
 *
 * Server routine when TMR0 Compare Int occur.
 */
ISR(TIMER0_COMPA_vect)
{
	ticker++;
	tmr0_occur = 1; 	
	pal_led(LED_2, LED_TOGGLE);
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief TMR0 initialization
 *
 * F_CPU: 4MHz
 * Prescaler: 8
 * Ticker: 4MHz/8 = 2us,
 * CNT: 250us/2us = 125.
 */
void tmr0_init(void)
{
  	/* variable initialization. */
  	memset(&enc, 0, sizeof(encode_t) );
	
    /* CTC mode, no compare unit */
	TCCR0A = _BV(WGM01);

	/* Clock source and prescaler, 8 */
	TCCR0B = _BV(CS01);

    /* load compare cnt. */
    OCR0A = 125;

    /* make PB7, OC0A, to output wave. */
	DDRB |= _BV(DDB7);

	/* clear int flag and enable OCR0A compare. */
	TIFR0 = 0xFF;
	TIMSK0 = _BV(OCIE0A);
}

/*
 * Get us1 buffer amount.
*/

/*
 * Find proper factor depending on state and bit mask.
*/

void findParam(uint8_t bit_msk)
{
  	if( ( enc.data & ( 1 << bit_msk) ) && (SET == enc.cur) ){
  		enc.factor = Div1;
		//if( 50 == index_sample )
		  	enc.reverse = 1;
	}
	else if( ( enc.data & ( 1 << bit_msk) ) && (CLR == enc.cur) ){
		enc.factor = Div2;
		enc.cur = SET;
	}
	else if( !( enc.data & ( 1 << bit_msk) ) && (SET == enc.cur) ){
		enc.factor = Div2;
		enc.cur = CLR;
	}
	else if( !( enc.data & ( 1 << bit_msk) ) && (CLR == enc.cur) ){
		enc.factor = Div1;
		//if( 50 == index_sample )
		  	enc.reverse = 1;
	}
}

/*
 * State machine for machester encoding.
 * Achieve wave phase.
*/

void encode_machine(void)
{
  	mod_state_t sta = enc.state;

	switch(sta){
		case Waiting:
			//if(us1_get_count() ){ 	//prepare for sta0
				enc.state = Sta0;
				enc.factor = Div2;
				//nc.data = us1_get_char();
				//index_sample = 0;
			//}
			break;
			//
		case Sta0: 	//prepare for sta1
	  		enc.state = Sta1;
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
				//if( 50 == index_sample )
		  			enc.reverse = 1;
		  	}
			else{
				enc.factor = Div2;
				enc.cur = CLR;
			}
		  	enc.state = Bit7;
			break;
			//
		case Bit7: 	//prepare for Bit6
		  	findParam(BIT6);
			enc.state = Bit6;
	    	break;
			//
		case Bit6: 	//prepare for Bit5
		  	findParam(BIT5);
			enc.state = Bit5;
	    	break;
			//
		case Bit5: 	//prepare for Bit4
		  	findParam(BIT4);
			enc.state = Bit4;
	    	break;
			//
		case Bit4: 	//prepare for Bit3
		  	findParam(BIT3);
			enc.state = Bit3;
	    	break;
			//
		case Bit3: 	//prepare for Bit2
		  	findParam(BIT2);
			enc.state = Bit2;
	    	break;
			//
		case Bit2: 	//prepare for Bit1
		  	findParam(BIT1);
			enc.state = Bit1;
	    	break;
			//
		case Bit1: 	//prepare for Bit0
		  	findParam(BIT0);
			enc.state = Bit0;
	    	break;
			//
		case Bit0: 	//new byte is exist?
		  	enc.state = Sto0; 	//to stop state 0.
								//output bit is one.
			if(SET == enc.cur){
				enc.factor = Div1;
				//if( 50 == index_sample )
		  			enc.reverse = 1;
			}
			else{
				enc.factor = Div2;
			}
			enc.cur = SET;
	    	break;
			//

		case Sto0:     //prepare for sto1
			enc.state = Waiting;
			enc.factor = Div1;
			enc.cur = SET;
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
