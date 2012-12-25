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
uint16_t cur_stamp = 0;
uint8_t cur_ovfw = 0;
uint8_t	ovfw = 0;
uint8_t	acc_occur = 0;
static uint8_t odd;	//odd parity

static void dec_update_tmr(void);

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/**
 * @brief ACC_ISR, load TMR2 cnt and ACSR for further operation, decode_state()
 *
 */
#if (PAL_TYPE==ATTINY88)
ISR(ANALOG_COMP_vect)
{
  	uint8_t sreg;

	sreg = SREG; 	// Save global interrupt flag
	__disable_interrupt();	// Disable interrupts
#if CURRENT_TEST==0
	pal_led(LED_1, LED_TOGGLE);
#endif

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
	cli();	// Disable interrupts

  	acc_occur = 1;		// set the flag for further operation in while(1)
  	cur_stamp = TCNT1;	// load TMR2 count
	cur_ovfw = ovfw;
	dec.acsr  = ACSR;   // load ACSR status

	SREG = sreg;	// Restore global interrupt flag
	
#if CURRENT_TEST==0
	if(dec.acsr & 0x20)
		pal_led(LED_1, LED_ON);
	else
	  	pal_led(LED_1, LED_OFF);
#endif
}
#endif//HAL_USE_ACC_CAP

#if DECODE_USED_TMR_ID==2
/**
 * @brief TMR2 Overflow interrupt.
 *
 */
ISR(TIMER2_OVF_vect)
{
	ovfw++;
}

#elif DECODE_USED_TMR_ID==1
/**
 * @brief TMR1 Overflow interrupt.
 *
 */
ISR(TIMER1_OVF_vect)
{
	ovfw++;
}
#else
    #error "Unsupported Decode Timer Id"
#endif//DECODE_USED_TMR_ID
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
	 *	PRESCALSE: 128
	 *	F_CPU: 4MHz
	 * Tick:  4MHz/128 = 32us.
	 *	enable TMR0 overflow interrupt, 256*32us = 8192us
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

#if DECODE_USED_TMR_ID==2
	TCCR2B = DECODE_TMR_CLK_SRC_PRESCALER_REG ;
	//TIMSK2 = _BV(TOIE2) ;
#elif DECODE_USED_TMR_ID==1
    TCCR1B = DECODE_TMR_CLK_SRC_PRESCALER_REG ;
    TIMSK1 = _BV(TOIE1) ;
#else
    #error "Unsupported Decode Timer Id"
#endif//DECODE_USED_TMR_ID
	
#endif//HAL_USE_ACC_CAP
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
	inv = 65535 - dec.prev_stamp;
	inv = cur_stamp + inv;
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
		  	if( dec.acsr & ( 1 << ACO) )	{	//rising
				dec.state = Sta0;	//goto start bit
			}
			dec_update_tmr();
			break;
			//
		case Sta0:
		  	if ( !( dec.acsr & ( 1 << ACO) )	)	{//falling
			  	if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
				  	dec.state = Bit0; 	//goto bit0
			  		odd = 0;	//clear odd parity cnt
				}
				else{
				  	dec.state = Waiting;
				}
			}
			else{
				dec.state = Waiting;
			}
			dec_update_tmr();
			break;
			//
		case Sta1:
		  	if ( dec.acsr & ( 1 << ACO) )	{	//rising
		  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX))
				  	dec.state = Sta2;
				else	//falling
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			dec_update_tmr();
			break;
			//
		case Sta2:
		  	if ( !( dec.acsr & ( 1 << ACO) ) )	{	//falling
		  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX))
				  	dec.state = Sta3;
				else
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			dec_update_tmr();
			break;
			//
		case Sta3:
		  	if ( dec.acsr & ( 1 << ACO) )	{ //rising
		  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX))
				  	dec.state = Bit7;
				else									//falling
				  	dec.state = Waiting;
			}
			else{
				dec.state = Waiting;
			}
			dec_update_tmr();
			break;
			//
		case Bit0:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT0 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT0 ) ;
				}
				dec.state = Bit1;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit1:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT1 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT1 ) ;
				}
				dec.state = Bit2;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit2:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT2 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT2 ) ;
				}
				dec.state = Bit3;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit3:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT3 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT3 ) ;
				}
				dec.state = Bit4;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit4:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT4 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT4 ) ;
				}
				dec.state = Bit5;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit5:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT5 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT5 ) ;
				}
				dec.state = Bit6;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit6:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT6 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT6 ) ;
				}
				dec.state = Bit7;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Bit7:
	  		if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO) ) 	{//rising
				  	dec.data |= ( 1 << BIT7 ) ;
					odd++;
				}
				else{							//falling
				  	dec.data &= ~( 1 << BIT7 ) ;
				}
				//sio_putchar(dec.data);
				dec.state = Parity;

			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
			break;
			//
		case Parity:
		  	if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( ( 0 == (odd % 2 ) ) && ( dec.acsr & ( 1 << ACO ) ) ) {//there is even 1(s)
					dec.state = Sto0;                                    //rev 1 is ok when odd parity
				}
				else if( ( 1 == (odd % 2 ) ) && !( dec.acsr & ( 1 << ACO ) ) ){ 	//there is odd 1(s)
					dec.state = Sto0;                                      		//rev 0 is ok when odd parity
				}
				else{
					dec.state = Waiting;
				}
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
		  	break;
			//
		case Sto0:
		  	if( ( inv >= DECODE_TMR_FREQ_2KHZ_MIN ) && (inv <= DECODE_TMR_FREQ_2KHZ_MAX)){
		  		dec_update_tmr();
				if ( dec.acsr & ( 1 << ACO ) ) {//stop bit should be always 1
					pal_led(LED_2, LED_ON);
				 	sio_putchar(dec.data);
					pal_led(LED_2, LED_OFF);
				}
				dec.state = Waiting;
			}
			else if (inv > DECODE_TMR_FREQ_2KHZ_MAX ) {
				dec.state = Waiting;
				dec_update_tmr();
			}
		  	break;
			//
		default:
	  		break;
			//

	}
}
//end of file
