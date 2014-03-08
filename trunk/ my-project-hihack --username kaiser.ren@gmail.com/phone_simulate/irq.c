/* ----------------------------------------------------------------------------
 *         XPLAINED SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file xplained.c
 *
 * sam4s-xplained bsp driver
 *
 */
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "main.h"
#include "board.h"
#include "encode.h"
#include "com.h"


/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
/** current index_sample */
uint8_t index_sample = 0;


/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *        Functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief Interrupt handler for DACC.
 *
 * Server routine when DACC complete the convertion.
 */
void DAC_IrqHandler(void)
{
	uint32_t status ;

    status = DACC_GetStatus( DACC ) ;

    /* if conversion is done*/
    if ( (status & DACC_IER_EOC) == DACC_IER_EOC ){
		if ( index_sample >= SAMPLES ){
		  	if(0 == enc.reverse)
				encode_machine();
			index_sample = 0;
			ticker = 0;
		}
		else if( ( (SAMPLES/2) == index_sample ) && (Div2 == enc.factor ) ){
			encode_machine();
			ticker = 0;
		}
		else if( ( (SAMPLES/2) == index_sample ) && ( 1 == enc.reverse ) ){
		  	enc.reverse = 0;
			encode_machine();
			ticker = 0;
		}
		DACC->DACC_IDR = DACC_IER_EOC;
	}

}

/**
 *  \brief Interrupt handler for UART0.
 *
 */
void UART0_IrqHandler(void)
{
    uint32_t status;

    /* Read USART status*/
    status = UART0->UART_SR;

    /* Receive byte is stored in buffer. */
    if ((status & UART_SR_RXRDY) == UART_SR_RXRDY) {
		if(us1.count < US_BUFFER_SIZE){
	    	us1.buff[us1.head++] = UART_GetChar();
			us1.count++;
			if(us1.head >= US_BUFFER_SIZE)
				us1.head = 0;
	  	}
		else{
			us1.buff[us1.head] = UART_GetChar();
		}
    }
}

/**
 *  \brief Interrupt handler for SysTick.
 *
 */
void SysTick_Handler( void )
{
	uint16_t value;

    {
		if ( 0 == ( ticker%enc.factor) ){
			value = sine_data[index_sample++] * amplitude / (MAX_DIGITAL/2)  \
			  		+ MAX_DIGITAL/2 /*- MAX_DIGITAL/50*/ + BIAS_STEP*bias ;
        	DACC_SetConversionData(DACC, value ) ;
			DACC->DACC_IER = DACC_IER_EOC;
		}
		ticker++;
    }
}

/**
 *  \brief Interrupt handler for TC0.
 *
 */
void TC0_IrqHandler( void )
{
  	uint32_t status ;
    status = REG_TC0_SR0 ;

	if ( (status & TC_SR_CPCS) == TC_SR_CPCS ){
#if defined	__SAM4S16C__
		XplnLED_Toggle(1);	//LED1 toggle
#endif
	}

}
//end of file
