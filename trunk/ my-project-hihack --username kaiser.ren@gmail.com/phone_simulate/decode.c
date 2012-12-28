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
uint32_t edge_occur = 0;
static volatile uint32_t cur_stamp = 0 ;
static volatile edge_t 	 cur_edge = none ;
static volatile uint32_t odd;	//odd parity
static volatile uint32_t half_duty;

/** PIOs for TC0 */
static const Pin pTcPins[] = {PIN_TC0_TIOA2};

/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
static uint32_t IsAbout500us(uint32_t cnt);
static uint32_t IsAbout1000us(uint32_t cnt);
void findPhase(uint8_t bit_msk, mod_state_t state);
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

	if ( (status & TC_SR_LDRAS) == TC_SR_LDRAS ){
	 	cur_stamp = REG_TC0_RA2 ;
		edge_occur = 1;
	  	if ( status & TC_SR_MTIOA ){
		  	LED_Clear(0) ;	//PA19 output high
			cur_edge = rising;
	  	}
		else{
			LED_Set(0) ;	//PA19 output low
			cur_edge = falling;
		}
		//printf( "%u ", cur_stamp) ;
		if ( IsAbout500us(cur_stamp) )
			half_duty++;
		else if( IsAbout1000us(cur_stamp) )
		  	half_duty += 2;
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
	
	/* Clear structure. */
	//memset(&dec, 0, sizeof(decode_t) );
	
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
    REG_TC0_CMR2 = (TC_CMR_TCCLKS_TIMER_CLOCK4    /* Clock Selection, PRE = 128 */
                   | TC_CMR_LDRA_EDGE           /* RA Loading Selection: rising edge of TIOA */
                   /*| TC_CMR_LDRB_FALLING*/          /* RB Loading Selection: falling edge of TIOA */
                   | TC_CMR_ABETRG                /* External Trigger Selection: TIOA */
                   | TC_CMR_ETRGEDG_EDGE );    /* External Trigger Edge Selection: Falling edge */
	
	/* Ext edge trigger, overflow .*/
	REG_TC0_IER2 =  TC_IER_LDRAS /*| TC_IER_LDRBS*/ ;	
	
    /* Reset and enable the tiimer counter for TC0 channel 2 */
    REG_TC0_CCR2 =  TC_CCR_CLKEN | TC_CCR_SWTRG;
}

/**
 * @brief calculate whether cnt is in 500us region
 * ticker = 64Mhz/128 = 2us
 * 475us < cnt < 510us
 *
 */
static uint32_t IsAbout500us(uint32_t cnt)
{
  	uint32_t temp;
	
	temp = cnt * 2;
	if( ( temp > 475 ) && ( temp < 550 ) ) {
		return true;
	} 	
	else{
		return false;
	}
}

/**
 * @brief calculate whether cnt is in 1000us region
 * ticker = 64Mhz/128 = 2us
 * 975us < cnt < 1010us
 *
 */
static uint32_t IsAbout1000us(uint32_t cnt)
{
  	uint32_t temp;
	
	temp = cnt * 2;
	if( ( temp > 975 ) && ( temp < 1050 ) ) {
		return true;
	} 	
	else{
		return false;
	}
}

/*
 * Find phase remain or phase reversal.
*/

void findPhase(uint8_t bit_msk, mod_state_t state)
{
  	if (IsAbout500us(cur_stamp) && !dec.step){
		if( falling == cur_edge ){
	  		dec.data &= ~(1 << bit_msk);
		}
		else{
		    dec.data |= (1 << bit_msk);
			odd++;
		}
		dec.step = 1;
	}
	else if (IsAbout500us(cur_stamp) && dec.step){
		dec.state = state;
		dec.step = 0;
	}
  	else if( IsAbout1000us(cur_stamp) && dec.step ){  //phase reversal
	  	if( falling == cur_edge ){
	  		dec.data &= ~(1 << bit_msk);
		}
		else{
		    dec.data |= (1 << bit_msk);
			odd++;
		}
		dec.state = state;
		dec.step = 1;
  	}
	else if ( IsAbout1000us(cur_stamp) && !dec.step ){
		dec.step = 0;
		dec.state = Waiting;
	}
}


/**
 * @brief decode state machine.
 *
 */
void decode_machine(void)
{
  	//uint16_t inv;	//interval

	//inv = cal_interval();
	switch (dec.state){
		case Waiting:
		  	if( ( falling == cur_edge) && IsAbout1000us(cur_stamp) && dec.step ){  //phase reversal
		  		odd = 0;
				dec.data = 0;
			  	dec.state = Sta0;
				dec.step = 1;
			}
			else if( ( rising == cur_edge) && IsAbout500us(cur_stamp) && !dec.step ){
				dec.step = 1;
				dec.state = Waiting;
			}
			else{
				dec.step = 0;
				dec.state = Waiting;
			}
			break;
			//
		case Sta0:
		  	findPhase(BIT0, Bit0);
	   		break;
			//
		case Bit0:
		  	findPhase(BIT1, Bit1);
	   		break;
			//
		case Bit1:
		  	findPhase(BIT2, Bit2);
	   		break;
			//
		case Bit2:
		  	findPhase(BIT3, Bit3);
	   		break;
			//
		case Bit3:
		  	findPhase(BIT4, Bit4);
	   		break;
			//
		case Bit4:
		  	findPhase(BIT5, Bit5);
	   		break;
			//
		case Bit5:
		  	findPhase(BIT6, Bit6);
	   		break;
			//
		case Bit6:
		  	findPhase(BIT7, Bit7);
	   		break;
			//
		case Bit7:
		  	if (IsAbout500us(cur_stamp) && !dec.step){
				if( rising == cur_edge ){
				  	odd++;
				}
				dec.step = 1;
			}
			else if (IsAbout500us(cur_stamp) && dec.step){
			  	if( 1 == (odd%2) ){
			    	dec.state = Parity;
			  	}
				else{
					dec.state = Waiting;
				}
				dec.step = 0;
			}
			else if( IsAbout1000us(cur_stamp) && dec.step ){  //phase reversal
				if( rising == cur_edge ){
				  	odd++;
				}
				if( 1 == (odd%2) ){
			    	dec.state = Parity;
					dec.step = 1;
			  	}
				else{
					dec.state = Waiting;
					dec.step = 0;
				}
			}
			else if ( IsAbout1000us(cur_stamp) && !dec.step ){
				dec.step = 0;
				dec.state = Waiting;
			}
	   		break;
			//
		case Parity:
		  	if( IsAbout1000us(cur_stamp) && dec.step ){  //phase reversal
				if( rising == cur_edge ){
				  	UART_PutChar(dec.data);
					dec.step = 1;
				}
				else{
					dec.step = 0;
				}
				dec.state = Waiting;
			}
			else if ( IsAbout500us(cur_stamp) && !dec.step ){
				if( rising == cur_edge ){
					dec.step = 1;
					UART_PutChar(dec.data);
				}
				else{
					dec.step = 0;
					dec.state = Waiting;
				}
			}
			else if ( IsAbout500us(cur_stamp) && dec.step ){
			  	//if( rising == cur_edge ){
				//  	UART_PutChar(dec.data);
				//	dec.step = 1;
				//}
				//else{
				//	dec.step = 0;
				//}
			  	if( 1 == (odd%2) ){
					UART_PutChar(dec.data);	
				}
			  	dec.step = 0;
				dec.state = Waiting;
			}
			else if ( IsAbout1000us(cur_stamp) && !dec.step ){
				dec.step = 0;	
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
