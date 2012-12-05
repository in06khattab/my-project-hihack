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

pwm_out_t pwm_para;

/* === Implementation ====================================================== */


/**
 * @brief Initialization for TMR0 CTC mode
 *
 */
void pwm_init(void)
{
  	/* toggle OC0A output when match, CTC mode */
	TCCR0A = _BV(COM0A0) | _BV(WGM01);
	
	/* make PB7, OC0A, to output wave. */
	DDRB |= _BV(DDB7);
	
	/* initial IE reg. */
	TIFR0 = _BV(OCF0A);
	TIMSK0 = _BV(OCIE0A);
}

/**
 * @brief Set PWM frequency.
 *
 */
void pwm_set_freq(uint16_t freq)
{
  	/* pwm pulse output count. */
  	pwm_para.toggle_cnt = PWM_TMR0_CMP_OUT_PULSE_CNT;
	
  	/* MCK/64 . */
	TCCR0B = PWM_TMR0_CLK_SRC_BIT_DEF;
	
	/* Set compare reg for output. */
	OCR0A = PWM_TMR0_CMP_OUT_OCRA(freq, F_CPU, PWM_TMR0_CLK_SRC_PRE_SCALE) ;
  	
}

/**
 * @brief Un-Initialization for TMR0 CTC mode
 *
 */
void pwm_uninit(void)
{
	/* no clock source provided. */
	TCCR0B = 0;
}

/**
 * @brief ISR for TMR2 interrupt handler
 *
 * This service routine is executed TMR2 interrupt.
 */
ISR(TIMER0_COMPA_vect)
{
  	pwm_para.toggle_cnt--;
	if(!pwm_para.toggle_cnt)
		pwm_uninit();  	
}

/**
 * @brief TMR1 CAP ISR
 */
#if HAL_USE_ACC_CAP==0
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
#endif


//eof

