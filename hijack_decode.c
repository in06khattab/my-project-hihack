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
#include "xplained.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define HIJACK_DEC_NUM_TICKS_MAX	(HIJACK_NUM_TICKS_PER_1US*1200ul)
#define HIJACK_DEC_NUM_TICKS_MIN	(HIJACK_NUM_TICKS_PER_1US*700ul)

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
typedef enum _tmr_tag_{
	pass = 0,
	suit,
	error
}chk_result_t;

/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
decode_t dec;
bool edge_occur = false;
static uint32_t cur_stamp = 0 ;
static edge_t 	cur_edge = none ;
static uint32_t offset = 0;
static uint32_t inv = 0;

/** PIOs for TC0 */
static const Pin pTcPins[] = {DEC_CAPTURE_INPUT};

/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
static chk_result_t IsTime2Detect(uint32_t inv);
static void dec_parser(uint8_t bit_msk, state_t state);
/*----------------------------------------------------------------------------
 *        ISR Handler
 *----------------------------------------------------------------------------*/
#if defined	__SAM4S16C__
/**
 * \brief Interrupt handler for the TC0 channel 2.
 */
void TC1_IrqHandler(void)
{
	uint32_t status ;
    status = REG_TC0_SR1 ;
	
#if defined	__SAM4S16C__
		XplnLED_Toggle(1);	//LED1 toggle
#endif

	if ( (status & TC_SR_LDRAS) == TC_SR_LDRAS ){
	 	cur_stamp = REG_TC0_RA1 ;
		
#if defined	sam3s4	
		LED_Toggle(1);	
#endif
			
		//cur_stamp = cur_stamp/10 ;        //420 falling, 351 rising
		edge_occur = true;
	  	if ( status & TC_SR_MTIOA ){
		  	LED_Clear(0) ;	//PA19 output high
			cur_edge = rising;
			//printf( "%u ", cur_stamp) ;
	  	}
		else{
			LED_Set(0) ;	//PA19 output low
			cur_edge = falling;
			//printf( "%u ", cur_stamp) ;
		}
	}
}
#endif

#if defined	sam3s4
/**
 * \brief Interrupt handler for the TC0 channel 2.
 */
void TC2_IrqHandler( void )
{
    uint32_t status ;
    status = REG_TC0_SR2 ;

	if ( (status & TC_SR_LDRAS) == TC_SR_LDRAS ){
	 	cur_stamp = REG_TC0_RA2 ;
		//cur_stamp = cur_stamp/10 ;
		edge_occur = true;
	  	if ( status & TC_SR_MTIOA ){
		  	LED_Clear(0) ;	//PA19 output high
			cur_edge = rising;
			//printf( "%u1 ", cur_stamp) ;
	  	}
		else{
			LED_Set(0) ;	//PA19 output low
			cur_edge = falling;
			//printf( "%u0 ", cur_stamp) ;
		}
	}
}
#endif

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 * \brief Configure TC0 channel 2 as capture operating mode.
 */
void TcCaptureInitialize(void)
{
    volatile uint32_t dummy;
	
	/* Clear structure. */
	//memset(&dec, 0, sizeof(decode_t) );
	
	/* Configure PIO Pins for TC0 */
    PIO_Configure( pTcPins, PIO_LISTSIZE( pTcPins ) ) ;
	
#if defined	sam3s4	
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
    REG_TC0_CMR2 = (HIJACK_TMR_CLK_SRC_PRESCALER_REG    /* Clock Selection, PRE = 8 */
                   | TC_CMR_LDRA_EDGE           /* RA Loading Selection: rising edge of TIOA */
                   /*| TC_CMR_LDRB_FALLING*/          /* RB Loading Selection: falling edge of TIOA */
                   | TC_CMR_ABETRG                /* External Trigger Selection: TIOA */
                   | TC_CMR_ETRGEDG_EDGE );    /* External Trigger Edge Selection: Falling edge */
	
	/* Ext edge trigger, overflow .*/
	REG_TC0_IER2 =  TC_IER_LDRAS /*| TC_IER_LDRBS*/ ;	
	
    /* Reset and enable the tiimer counter for TC0 channel 2 */
    REG_TC0_CCR2 =  TC_CCR_CLKEN | TC_CCR_SWTRG;
#else //SAM4S-XPLAINED
	/* Configure the PMC to enable the Timer Counter clock TC0 channel 1.*/
    PMC_EnablePeripheral(ID_TC1);
    /*  Disable TC clock */
    REG_TC0_CCR1 = TC_CCR_CLKDIS;
    /*  Disable interrupts */
    REG_TC0_IDR1 = 0xFFFFFFFF;
    /*  Clear status register */
    dummy = REG_TC0_SR1;
    /*  Set channel 2 as capture mode.
	 *  Clock source MCK/8, 8MHz.
	 */
    REG_TC0_CMR1 = (HIJACK_TMR_CLK_SRC_PRESCALER_REG    /* Clock Selection, PRE = 8 */
                   | TC_CMR_LDRA_EDGE           /* RA Loading Selection: rising edge of TIOA */
                   /*| TC_CMR_LDRB_FALLING*/          /* RB Loading Selection: falling edge of TIOA */
                   | TC_CMR_ABETRG                /* External Trigger Selection: TIOA */
                   | TC_CMR_ETRGEDG_EDGE );    /* External Trigger Edge Selection: Falling edge */
	
	/* Ext edge trigger, overflow .*/
	REG_TC0_IER1 =  TC_IER_LDRAS /*| TC_IER_LDRBS*/ ;	
	
    /* Reset and enable the tiimer counter for TC0 channel 2 */
    REG_TC0_CCR1 =  TC_CCR_CLKEN | TC_CCR_SWTRG;
#endif
}

/**
 * @brief calculate whether cnt is in 500us region
 * ticker = 64Mhz/128 = 2us
 * 475us < cnt < 510us
 *
 */
static chk_result_t IsTime2Detect(uint32_t inv)
{
  	chk_result_t ret;
	
  	if( inv < HIJACK_DEC_NUM_TICKS_MIN){
    	offset = inv;
	    ret = pass;
  	}
	else if ( ( inv <= HIJACK_DEC_NUM_TICKS_MAX ) && ( inv >= HIJACK_DEC_NUM_TICKS_MIN ) ) {
		offset = 0;
		inv = 0;
	  	ret = suit;
	}
	else{
		offset = 0;
		inv = 0;
		ret = error;
	}
	return ret;
}

/*
 * Find phase remain or phase reversal.
*/

static void dec_parser(uint8_t bit_msk, state_t state)
{
  	if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
		if( falling == cur_edge ){
	  		dec.data &= ~(1 << bit_msk);
		}
		else{
		   dec.data |= (1 << bit_msk);
			dec.odd++;
		}
		dec.state = state;   //state switch
	}
	else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
		dec.state = Waiting;   //state switch
	}
}


/**
 * @brief decode state machine.
 *
 */
void decode_machine(void)
{
   	inv = offset + cur_stamp;  //update offset

	switch (dec.state){
		case Waiting:
         	/* go to start bit if rising edge exist. */
         	if (rising == cur_edge) {
            	dec.state = Sta0;
            	offset = 0;
				inv = 0;
         	}
			break;
			//
		case Sta0:
         	if( ( suit == IsTime2Detect(inv) ) && ( falling == cur_edge ) ){
				dec.data = 0;  //clear data field for store new potential data
				dec.odd = 0;   //clear odd field parity counter
				dec.state = Bit0;
         	}
         	else if( error == IsTime2Detect(inv) ){
				dec.state = Waiting;
         	}
	   		break;
			//
		case Bit0:
		  	dec_parser(BIT0, Bit1);
	   		break;
			//
		case Bit1:
		  	dec_parser(BIT1, Bit2);
	   		break;
			//
		case Bit2:
		  	dec_parser(BIT2, Bit3);
	   		break;
			//
		case Bit3:
		  	dec_parser(BIT3, Bit4);
	   		break;
			//
		case Bit4:
		  	dec_parser(BIT4, Bit5);
	   		break;
			//
		case Bit5:
		  	dec_parser(BIT5, Bit6);
	   		break;
			//
		case Bit6:
		  	dec_parser(BIT6, Bit7);
	   		break;
			//
		case Bit7:
		  	dec_parser(BIT7, Parity);
	   		break;
			//
		case Parity:
			if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
				if( rising == cur_edge ){
				   dec.odd++;
				}
				if( 1 == (dec.odd%2)){  //parity pass
				   dec.state = Sto0;
				}
				else{ //parity failed
				   dec.state = Waiting;
				}
			 }
			 else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
				dec.state = Waiting;
			 }
			break;
			//
      	case Sto0:
         	if ( ( suit == IsTime2Detect(inv) ) ){ //it's time to determine
				if( rising == cur_edge ){  //stop bit is rising edge
				   UART_PutChar(dec.data);
				}
				dec.state = Waiting;
         	}
         	else if ( error == IsTime2Detect(inv) ){   //wait for edge detection time
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
