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
modulate_t mod;

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 * @brief Initialization for AC compare vector
 *
 */
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
	/**
	 *	Acc_Int enable,
	 *	ACC_Int flag clear
	 *  Acc_Int mode is edge sensitive.
	**/
	ACSR = _BV(ACIE) | _BV(ACI);
	
	/**
	 *	prescale is 8, F_CPU is 4MHz
	 * 	Tick: 4MHz/8 = 2us.
	**/
	TCCR2B = CS21 ;	
#endif
}
/**
 * @brief ACC ISR
 *
 */
#if HAL_USE_ACC_CAP>0
ISR(ANALOG_COMP_vect)
{  	
  	//ACSR |=  _BV(ACI); 	// Acc_Int flag clear automatically
  	if( ACSR & _BV( ACO ) ){
	  	printf("f\r\n");
  	}
	else{
	  	printf("r\r\n");
	}
}
#endif
//end of file
