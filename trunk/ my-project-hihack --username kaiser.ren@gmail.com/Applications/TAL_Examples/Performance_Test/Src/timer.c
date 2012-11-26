/**
 * @file timer.c
 *
 * @brief hal board specific functionality
 *
 * This file implements hal board specific functionality.
 *
 * @author    kren
 * @data		Nov 21, 2012
 */


/* === Includes ============================================================ */
#include "hal.h"

/* === Globals ============================================================== */
ac_cap_t ac_cap_para = {.occur = 0,
								.interval = 0,
								.cur_stamp = 0};

/* === Implementation ====================================================== */
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
	
	/* Config AC's setting, ACO enable and make it connect with Tmr1's
	capture input, toggle trigger. */
#if 1
	ACSR = _BV(ACIC);
	//noise canceller, falling edge detection, prescaler is clkio/8
	TCCR1B = _BV(ICNC1) | _BV(ICES1) | _BV(CS11);
	//clear potential capture int flag
	TIFR1 = _BV(ICF1);
	//enable capture int
	TIMSK1 = _BV(ICIE1);
#else
	ACSR = _BV(ACIE) | _BV(ACI) | _BV(ACIS1);
	TCCR1B = _BV(CS11);
#endif
}

/**
 * @brief Initialization for AC compare vector
 *
 */
void pwm_init(void)
{
  	/* toggle OC0A output when match, CTC mode */
	TCCR0A = _BV(COM0A0) | _BV(WGM01);
	
	/* MCK/8 . */
	TCCR0B = _BV(CS01);
	
	/*
	 * Set CTC OCR2A to generate 5000Hz wave.
	 * 4M/8/5000Hz = 100 tick.
	*/
	OCR0A = 100;
	
	/* make PB7, OC0A, to output wave. */
	DDRB |= _BV(DDB7);
	
	/* initial IE reg. */
	TIFR0 = _BV(OCF0A);
	TIMSK0 = _BV(OCIE0A);
}

/**
 * @brief ISR for TMR2 interrupt handler
 *
 * This service routine is executed TMR2 interrupt.
 */
ISR(TIMER2_COMPA_vect)
{
  	nop();
}

/**
 * @brief ISR for AC compare vector
 *
 * This service routine is executed AC pin edge detection.
 */
#if 1
ISR(TIMER1_CAPT_vect)
{
  	uint8_t sreg;
	uint16_t temp;
	
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	__disable_interrupt();
	
	
#if 0
  	if( ACSR & _BV( ACO ) ){
	  	sio_uart_1_tx("f", 1) ;
  	}
	else{
	  	sio_uart_1_tx("r", 1);
	}
#else
	/* auto adding for times int handling . */
	ac_cap_para.occur++;	
	
	/* Read ICR1 into current stamp. */
	temp = ICR1;
	if(temp < ac_cap_para.cur_stamp){
		nop();
	}
	ac_cap_para.interval = temp - ac_cap_para.cur_stamp;
	ac_cap_para.cur_stamp = temp;
#endif
	/* Restore global interrupt flag */
	SREG = sreg;
}
#else
ISR(ANALOG_COMP_vect)
{  	
  	ACSR |=  _BV(ACI);
  	if( ACSR & _BV( ACO ) ){
	  	printf("f\r\n");
  	}
	else{
	  	printf("r\r\n");
	}
}
#endif


//eof

