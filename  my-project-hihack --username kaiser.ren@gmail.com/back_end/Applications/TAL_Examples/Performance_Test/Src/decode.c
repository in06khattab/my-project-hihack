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
#include "hal.h"


/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
decode_t dec;
uint8_t	cur_stamp = 0;
uint8_t 	cur_ovfw = 0;
uint8_t	acc_occur = 0;

static void dec_update_tmr(void);

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/**
 * @brief ACC_ISR, load TMR2 cnt and ACSR for further operation, decode_state()
 *
 */
#if (PAL_TYPE==ATTINY88)
ISR(ANA_COMP_vect)
{  	
  	uint8_t sreg;
	
	sreg = SREG; 	// Save global interrupt flag
	__disable_interrupt();	// Disable interrupts
	
	pal_led(LED_1, LED_TOGGLE);
	
  	acc_occur = 1;		// set the flag for further operation in while(1)
  	cur_stamp = TCNT0;	// load TMR2 count
	dec.acsr  = ACSR;   // load ACSR status
	
	SREG = sreg;	// Restore global interrupt flag
}

/**
 * @brief TMR0 Overflow interrupt.
 *
 */
ISR(TIMER0_OVF_vect)
{
	cur_ovfw++;  	
}

#else //ATMEGA1281
	#if HAL_USE_ACC_CAP>0
ISR(ANALOG_COMP_vect)
{  	
  	uint8_t sreg;
	
	sreg = SREG; 	// Save global interrupt flag
	__disable_interrupt();	// Disable interrupts
	
	pal_led(LED_1, LED_TOGGLE);
	
  	acc_occur = 1;		// set the flag for further operation in while(1)
  	cur_stamp = TCNT2;	// load TMR2 count
	dec.acsr  = ACSR;   // load ACSR status
	
	SREG = sreg;	// Restore global interrupt flag
}
	#endif
/**
 * @brief TMR2 Overflow interrupt.
 *
 */
ISR(TIMER2_OVF_vect)
{
	cur_ovfw++;  	
}
#endif//ATTINY88



/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 * @brief Initialization for AC compare vector
 *
 */
#if (PAL_TYPE==ATTINY88)
void ac_init(void)
{
   /* Select ADC3 as negtive input, mcu pin PF3, module pin 30. */
	ADCSRB = _BV(ACME);
	ADCSRA &= ~_BV(ADEN);
	ADMUX = _BV(MUX0) | _BV(MUX1);
	
	/* initial global variable. */
	memset(&ac_cap_para, 0, sizeof(ac_cap_t));

#if HAL_USE_ACC_CAP==0
	/* Config AC's setting, ACO enable and make it connect with Tmr1's
	capture input, toggle trigger. */
	ACSR = _BV(ACIC);
	//noise canceller, falling edge detection, prescaler is clkio/8
	TCCR1B = _BV(ICNC1) | _BV(ICES1) | _BV(CS11);
	//clear potential capture int flag
	TIFR1 = _BV(ICF1);
	//enable capture int
	TIMSK1 = _BV(ICIE1);
#else
	memset(&dec, 0, sizeof(decode_t) );
	/**
	 *	Acc_Int enable,
	 *	ACC_Int flag clear
	 *  Acc_Int mode is edge sensitive.
	**/
	ACSR = _BV(ACIE) | _BV(ACI);
	
	/**
	 *	PRESCALSE: 32
	 *	F_CPU: 4MHz
	 * Tick:  4MHz/32 = 8us.
	 *	enable TMR0 overflow interrupt, 256*8us = 2048us
	**/
	TCCR0A = _BV(CS01) | _BV(CS00) ;	
	TIMSK0 = _BV(TOIE0) ;
	
#endif
}

#else //ATMEGA1281
void ac_init(void)
{
   /* Select ADC3 as negtive input, mcu pin PF3, module pin 30. */
	ADCSRB = _BV(ACME);
	ADCSRA &= ~_BV(ADEN);
	ADMUX = _BV(MUX0) | _BV(MUX1);
	
	/* initial global variable. */
	memset(&ac_cap_para, 0, sizeof(ac_cap_t));

#if HAL_USE_ACC_CAP==0
	/* Config AC's setting, ACO enable and make it connect with Tmr1's
	capture input, toggle trigger. */
	ACSR = _BV(ACIC);
	//noise canceller, falling edge detection, prescaler is clkio/8
	TCCR1B = _BV(ICNC1) | _BV(ICES1) | _BV(CS11);
	//clear potential capture int flag
	TIFR1 = _BV(ICF1);
	//enable capture int
	TIMSK1 = _BV(ICIE1);
#else
	memset(&dec, 0, sizeof(decode_t) );
	/**
	 *	Acc_Int enable,
	 *	ACC_Int flag clear
	 *  Acc_Int mode is edge sensitive.
	**/
	ACSR = _BV(ACIE) | _BV(ACI);
	
	/**
	 *	PRESCALSE: 32
	 *	F_CPU: 4MHz
	 * Tick:  4MHz/32 = 8us.
	 *	enable TMR2 overflow interrupt, 256*8us = 2048us
	**/
	TCCR2B = _BV(CS21) | _BV(CS20) ;	
	TIMSK2 = _BV(TOIE2) ;
	
#endif
}
#endif//ATTINY88

/**
 * @brief Calculate the interval between ACC_ISR.
 *
 */
uint16_t cal_interval(void)
{
  	uint16_t inv;	//interval
	
	// equal condition, no overflow occur
	if(cur_ovfw == dec.prev_ovfw)
	  	return (cur_stamp - dec.prev_stamp);
	
	inv = cur_ovfw - dec.prev_ovfw;	//get the overflow
	if(inv > 1){
	  	dec_update_tmr();
		return 256;		//this is invalid interval, too huge.
	}
	//only one overflow occur
	inv = cur_stamp + 256 - dec.prev_stamp;
	return (inv);
}

/**
 * @brief update dec's timer stamp.
 *
 */
static void dec_update_tmr(void)
{
	dec.prev_stamp = cur_stamp;		//reload time stamp
	dec.prev_ovfw = cur_ovfw;        //reload overflow cnt
}


/**
 * @brief decode state machine.
 *
 */
void decode_machine(void)
{
  	uint16_t inv;	//interval
	
	inv = cal_interval();
	//printf("%u\r\n", inv);
  	switch (dec.state){
		case Waiting:
		  	if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if( dec.acsr & ( 1 << ACO) )		//rising
				  	dec.state = Sta0;
			}
			else if( inv < DECODE_TMR2_FREQ_2KHZ_MIN ){
			  	dec.skip = 1;
			}
			break;
			//
		case Sta0:
		  	if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( !( dec.acsr & ( 1 << ACO) )	)	//falling
				  	dec.state = Sta1;
				else	//rising
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			break;
			//
		case Sta1:
		  	if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) )		//rising
				  	dec.state = Sta2;
				else	//falling
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			break;
			//
		case Sta2:
		  	if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( !( dec.acsr & ( 1 << ACO) ) )		//falling
				  	dec.state = Sta3;
				else	//rising
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			break;
			//
		case Sta3:
		  	if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) //rising
				  	dec.state = Bit7;
				else									//falling
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			break;
			//
		case Bit7:
	  		if( ( inv >= DECODE_TMR2_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR2_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) //rising
				  	dec.data |= ( 1 << BIT7 ) ;
				else									//falling
				  	dec.data &= ~( 1 << BIT7 ) ;
			}
			else if (inv > DECODE_TMR2_FREQ_2KHZ_MAX ) {
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
