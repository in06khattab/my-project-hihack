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
#include "decode.h"


/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
decode_t dec;
volatile uint32_t cur_stamp = 0;
volatile uint32_t cur_ovfw = 0;
uint64_t cnt;
static uint8_t odd;	//odd parity
/** Capture status*/
//static uint32_t _dwCaptured_pulses;
//static uint32_t _dwCaptured_ra ;
//static uint32_t _dwCaptured_rb ;
/** PIOs for TC0 */
static const Pin pTcPins[] = {PIN_TC0_TIOA2};

/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
static void dec_update_tmr(void);

/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
/**
 * \brief Interrupt handler for the TC0 channel 2.
 */
void TC2_IrqHandler( void )
{
    uint32_t status ;
    status = REG_TC0_SR2 ;

    if ( (status & TC_SR_ETRGS) == TC_SR_ETRGS )
    {
	  	if ( status & TC_SR_MTIOA ){
		  	LED_Clear(0) ;	//PA19 output high
	  	}
		else{
			LED_Set(0) ;	//PA19 output low
		}
		cur_stamp = REG_TC0_CV2;
		printf( "%ul\r\n", REG_TC0_CV2 - dec.prev_stamp ) ;
		dec.prev_stamp = REG_TC0_CV2;
        //_dwCaptured_pulses++ ;
        //_dwCaptured_ra = REG_TC0_RA2 ;
        //_dwCaptured_rb = REG_TC0_RB2 ;
    }
	else if( (status & TC_SR_COVFS) == TC_SR_COVFS )
	{
	 	cur_ovfw++;
	}
}


/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 * \brief Configure TC0 channel 2 as capture operating mode.
 */
void TcCaptureInitialize(void)
{
    volatile uint32_t dummy;
	/* Configure PIO Pins for TC0 */
    PIO_Configure( pTcPins, PIO_LISTSIZE( pTcPins ) ) ;
	
    /* Configure the PMC to enable the Timer Counter clock TC0 channel 2.*/
    PMC_EnablePeripheral(ID_TC2);
    /*  Disable TC clock */
    REG_TC0_CCR2 = TC_CCR_CLKDIS;
    /*  Disable interrupts */
    REG_TC0_IDR2 = 0xFFFFFFFF;
    /*  Clear status register */
    dummy = REG_TC0_SR2;
    /*  Set channel 2 as capture mode.
	 *  Clock source MCK/8, 8MHz.
	 */
    REG_TC0_CMR2 = (TC_CMR_TCCLKS_TIMER_CLOCK2    /* Clock Selection */
                   /*| TC_CMR_LDRA_RISING*/           /* RA Loading Selection: rising edge of TIOA */
                   /*| TC_CMR_LDRB_FALLING*/          /* RB Loading Selection: falling edge of TIOA */
                   | TC_CMR_ABETRG                /* External Trigger Selection: TIOA */
                   | TC_CMR_ETRGEDG_EDGE );    /* External Trigger Edge Selection: Falling edge */
	
	/* Ext edge trigger, overflow .*/
	REG_TC0_IER2 = TC_IER_ETRGS | TC_IER_COVFS;	
	
    /* Reset and enable the tiimer counter for TC0 channel 2 */
    REG_TC0_CCR2 =  TC_CCR_CLKEN | TC_CCR_SWTRG;
}

/**
 * @brief Calculate the interval between ACC_ISR.
 *
 */
uint64_t cal_interval(void)
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
	
#if 0
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
#endif
}
//end of file
