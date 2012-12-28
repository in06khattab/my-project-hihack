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
static volatile uint32_t offset_us = 0;
//static volatile uint32_t odd;	//odd parity

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
  	if ( IsAbout1000us(offset_us) ){ //it's time to determine
		if( falling == cur_edge ){
	  		dec.data &= ~(1 << bit_msk);
		}
		else{
		   dec.data |= (1 << bit_msk);
			dec.odd++;
		}
      offset_us = 0;
		dec.state = state;   //state switch
	}
	else if ( IsAbout500us(offset_us) && (0 == offset_us) ){   //wait for edge detection time
		offset_us = 500;  //update 500us
	}
  	else {
      dec.state = Waiting;
      offset_us = 0;
	}
}


/**
 * @brief decode state machine.
 *
 */
void decode_machine(void)
{
   offset_us += cur_stamp; //update offset
   
	switch (dec.state){
		case Waiting:
         /* go to start bit if rising edge exist. */
         if (rising == cur_edge) {
            dec.state = Sta0;
            offset_us = 0;
         }
			break;
			//
		case Sta0:
         if( IsAbout1000us(offset_us) && ( falling == cur_edge ) ){
            offset_us = 0; //start from edge field
            dec.data = 0;  //clear data field for store new potential data
            dec.odd = 0;   //clear odd field parity counter
            dec.state = Bit0;
         } 
         else{
            offset_us = 0; //start from edge field
            dec.state = Waiting;
         }
	   	break;
			//
		case Bit0:
		  	findPhase(BIT0, Bit1);
	   	break;
			//
		case Bit1:
		  	findPhase(BIT1, Bit2);
	   	break;
			//
		case Bit2:
		  	findPhase(BIT2, Bit3);
	   		break;
			//
		case Bit3:
		  	findPhase(BIT3, Bit4);
	   		break;
			//
		case Bit4:
		  	findPhase(BIT4, Bit5);
	   	break;
			//
		case Bit5:
		  	findPhase(BIT5, Bit6);
	   	break;
			//
		case Bit6:
		  	findPhase(BIT6, Bit7);
	   	break;
			//
		case Bit7:
		  	findPhase(BIT7, Parity);
	   	break;
			//
		case Parity:
		  	if ( IsAbout1000us(offset_us) ){ //it's time to determine
            if( rising == cur_edge ){
               dec.odd++;
            }
            if( 1 == (dec.odd%2)){  //parity pass
               dec.state = Sto0;
            }
            else{ //parity failed
               dec.state = Waiting;
            }
            offset_us = 0;
         }
         else if ( IsAbout500us(offset_us) && (0 == offset_us) ){   //wait for edge detection time
            offset_us = 500;  //update 500us
         }
         else {
            dec.state = Waiting;
            offset_us = 0;
         }
	   	break;
			//
      case Sto0:
         if ( IsAbout1000us(offset_us) ){ //it's time to determine
            if( rising == cur_edge ){  //stop bit is rising edge
               UART_PutChar(dec.data);
            }
            dec.state = Waiting;
            offset_us = 0;
         }
         else if ( IsAbout500us(offset_us) && (0 == offset_us) ){   //wait for edge detection time
            offset_us = 500;  //update 500us
         }
         else {
            dec.state = Waiting;
            offset_us = 0;
         }
         break;
         //
		default:
	  		break;
			//
	}
}
//end of file
