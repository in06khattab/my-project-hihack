/* ----------------------------------------------------------------------------
 *         COM SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2014, kren
 */

/**
 * \file com.c
 *
 * com port bsp driver
 *
 */
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "com.h"


/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
/** usart1 rx entity. **/
us_rx_t us1 = {.count = 0, .head = 0, .tail = 0};


/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        Functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief UART0 hardware configuration
 *
 * Configures UART0 in hardware handshaking mode, asynchronous, 8 bits, 1 stop
 * bit, no parity, 115200 bauds and enables its transmitter and receiver.
 */
void _ConfigureCom( void )
{
  	/* Enaboe UART0 interrupt*/
    NVIC_EnableIRQ( UART0_IRQn ) ;
	/* Enable UART0 RXREADY interrupt. */
	UART0->UART_IER = UART_IER_RXRDY ;
}
/*
 * Get us1 buffer amount.
*/
uint8_t us1_get_count(void)
{
  	return (us1.count);
}

/*
 * Get a charactors.
*/
uint8_t us1_get_char(void)
{
  	uint8_t temp;
	
	if(us1.count){
#if CRITICAL_PROTECTION==1
		__disable_irq();
#endif
    	temp = us1.buff[us1.tail++];
		us1.count--;
		if(us1.tail >= US_BUFFER_SIZE){
			us1.tail = 0;
		}
#if CRITICAL_PROTECTION==1
		__enable_irq();
#endif
  	}
    return temp;
}
//end of file
