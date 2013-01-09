/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 *  \page dac12_sinewave DAC12 Sinewave Example
 *
 *  \section Purpose
 *
 *  The dac12_sinewave example demonstrates how to use DACC peripheral.
 *
 *  \section Requirements
 *
 *  This package can only be used with sam3s-ek.
 *
 *  \section Description
 *
 *  This application is aimed to demonstrate how to use DACC in free running
 *  mode.
 *
 *  The example allows to configure the frequency and amplitude of output
 *  sinewave. The frequency could be set from 200Hz to 3KHz, and the peak amplitude
 *  could be set from 100 to 2047 in regard to 12bit resolution.
 *
 *  The output could be monitored by connecting PB13/DAC0 to one channel of an
 *  oscilloscope or heard by ear when plugging a headphone to J11 on the EK board.
 *
 *  \section Usage
 *
 *  -# Build the program and download it inside the evaluation board. Please
 *     refer to the
 *     <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6224.pdf">
 *     SAM-BA User Guide</a>, the
 *     <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6310.pdf">
 *     GNU-Based Software Development</a>
 *     application note or to the
 *     <a href="ftp://ftp.iar.se/WWWfiles/arm/Guides/EWARM_UserGuide.ENU.pdf">
 *     IAR EWARM User Guide</a>,
 *     depending on your chosen solution.
 *  -# On the computer, open and configure a terminal application
 *     (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *    - 115200 bauds
 *    - 8 bits of data
 *    - No parity
 *    - 1 stop bit
 *    - No flow control
 *  -# In the terminal window, the
 *     following text should appear (values depend on the board and chip used):
 *     \code
 *      -- DAC12 Sinewave Example xxx --
 *      -- xxxxxx-xx
 *      -- Compiled: xxx xx xxxx xx:xx:xx --
 *      -- Menu Choices for this example--
 *      -- 0: Set frequency(200Hz-3kHz).--
 *      -- 1: Set amplitude(100-2047).--
 *      -- i: Display present frequency and amplitude.--
 *      -- m: Display this menu.--
 *     \endcode
 *  -# Input command according to the menu.
 *
 *  \section References
 *  - dac12_sinewave/main.c
 *  - dacc.h
 */

/** \file
 *
 *  This file contains all the specific code for the dac12_sinewave.
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "main.h"
#include "encode.h"
#include "decode.h"
#include "xplained.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define VER_MAJOR 0
#define VER_MINOR 6
#define VER_PATCH ' '


/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/






/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/**  Pins to configure for the application.*/
const Pin pins[] = {
    BOARD_PIN_USART_RXD,
    BOARD_PIN_USART_TXD,
    BOARD_PIN_USART_CTS,
    BOARD_PIN_USART_RTS,
    BOARD_PIN_USART_EN,
    BOARD_PIN_PA18B_PCK2
};


/** usart1 rx entity. **/
us_rx_t us1 = {.count = 0, .head = 0, .tail = 0};

/** amplitude is MAX_DIGITAL/2,SAMPLES is fixed at 100 */
const int16_t sine_data[SAMPLES]=
{
    0x0,   0x080, 0x100, 0x17f, 0x1fd, 0x278, 0x2f1, 0x367, 0x3da, 0x449,
    0x4b3, 0x519, 0x579, 0x5d4, 0x629, 0x678, 0x6c0, 0x702, 0x73c, 0x76f,
    0x79b, 0x7bf, 0x7db, 0x7ef, 0x7fb, 0x7ff, 0x7fb, 0x7ef, 0x7db, 0x7bf,
    0x79b, 0x76f, 0x73c, 0x702, 0x6c0, 0x678, 0x629, 0x5d4, 0x579, 0x519,
    0x4b3, 0x449, 0x3da, 0x367, 0x2f1, 0x278, 0x1fd, 0x17f, 0x100, 0x080,

    -0x0, -0x080, -0x100, -0x17f, -0x1fd, -0x278, -0x2f1, -0x367,  -0x3da, -0x449,
    -0x4b3, -0x519, -0x579, -0x5d4, -0x629, -0x678, -0x6c0, -0x702, -0x73c, -0x76f,
    -0x79b, -0x7bf, -0x7db, -0x7ef, -0x7fb, -0x7ff, -0x7fb, -0x7ef, -0x7db, -0x7bf,
    -0x79b, -0x76f, -0x73c, -0x702, -0x6c0, -0x678, -0x629, -0x5d4, -0x579, -0x519,
    -0x4b3, -0x449, -0x3da, -0x367, -0x2f1, -0x278, -0x1fd, -0x17f, -0x100, -0x080
};

const int16_t cosine_data[SAMPLES]=
{
    0x7ff, 0x7fb, 0x7ef, 0x7db, 0x7bf,
    0x79b, 0x76f, 0x73c, 0x702, 0x6c0, 0x678, 0x629, 0x5d4, 0x579, 0x519,
    0x4b3, 0x449, 0x3da, 0x367, 0x2f1, 0x278, 0x1fd, 0x17f, 0x100, 0x080,

    -0x0, -0x080, -0x100, -0x17f, -0x1fd, -0x278, -0x2f1, -0x367,  -0x3da, -0x449,
    -0x4b3, -0x519, -0x579, -0x5d4, -0x629, -0x678, -0x6c0, -0x702, -0x73c, -0x76f,
    -0x79b, -0x7bf, -0x7db, -0x7ef, -0x7fb,
	
	 -0x7ff, -0x7fb, -0x7ef, -0x7db, -0x7bf,
    -0x79b, -0x76f, -0x73c, -0x702, -0x6c0, -0x678, -0x629, -0x5d4, -0x579, -0x519,
    -0x4b3, -0x449, -0x3da, -0x367, -0x2f1, -0x278, -0x1fd, -0x17f, -0x100, -0x080,
		
	 0x0,   0x080, 0x100, 0x17f, 0x1fd, 0x278, 0x2f1, 0x367, 0x3da, 0x449,
    0x4b3, 0x519, 0x579, 0x5d4, 0x629, 0x678, 0x6c0, 0x702, 0x73c, 0x76f,
    0x79b, 0x7bf, 0x7db, 0x7ef, 0x7fb
};

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Get input from user, the biggest 4-digit decimal number is allowed
 *
 * \param lower_limit the lower limit of input
 * \param upper_limit the upper limit of input
 */
static int16_t _GetInputValue( int16_t lower_limit, int16_t upper_limit )
{
    int8_t i = 0, c ;
    int16_t value = 0 ;
    int8_t length = 0 ;
    int8_t str_temp[5] = { 0 } ;

    while ( 1 )
    {
        c = UART_GetChar() ;
        if ( c == '\n' || c == '\r' )
        {
            printf( "\r\n" ) ;
            break ;
        }

        if ( '0' <= c && '9' >= c )
        {
            printf( "%c", c ) ;
            str_temp[i++] = c ;

            if ( i >= 4 )
            {
                break ;
            }
        }
    }

    str_temp[i] = '\0' ;
    /* input string length */
    length = i ;
    value = 0 ;

    /* convert string to integer */
    for ( i = 0 ; i < 4 ; i++ )
    {
        if ( str_temp[i] != '0' )
        {
            switch ( length - i - 1 )
            {
                case 0 :
                    value += (str_temp[i] - '0') ;
                break ;

                case 1 :
                    value += (str_temp[i] - '0') * 10 ;
                break ;

                case 2 :
                    value += (str_temp[i] - '0') * 100 ;
                break ;

                case 3 :
                    value += (str_temp[i] - '0') * 1000 ;
                break ;
            }

        }
    }

    if ( value > upper_limit || value < lower_limit )
    {
        printf( "\r\n-F- Input value is invalid!\n" ) ;

        return -1 ;
    }

    return value ;
}

/** Display main menu */
static void _DisplayMenuChoices( void )
{
    printf( "-- Menu Choices for this example--\r\n" ) ;
    printf( "-- 0: Set frequency(200Hz-3kHz).--\r\n" ) ;
    printf( "-- 1: Set amplitude(100-2047).--\r\n" ) ;
    printf( "-- i: Display present frequency and amplitude.--\r\n" ) ;
    printf( "-- m: Display this menu.--\r\n" ) ;
	 printf( "-- s: output sine wave.--\r\n" ) ;
	 printf( "-- c: output cosine wave.--\r\n" ) ;
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief dac12_sinewave Application entry point.
 *
 *
 *  \return Unused (ANSI-C compatibility).
 *  \callgraph
 */
extern int main( void )
{
    uint8_t c ;
    int16_t freq, amp ;

    /* Disable watchdog */
    WDT_Disable( WDT ) ;
	
	/* Configure pins*/
    PIO_Configure( pins, PIO_LISTSIZE( pins ) ) ;

#if defined	sam3s4	
	/* Configure leds . */
	LED_Configure(0) ;
	LED_Configure(1) ;
#endif
	
#if defined	__SAM4S16C__
    XplnLED_Configure();
#endif
	
    /* Output example information */
    printf( "-- Phone Simulator V%d.%02d%c --\r\n", VER_MAJOR, VER_MINOR, VER_PATCH ) ;
    printf( "-- Compiled: %s %s --\r\n", __DATE__, __TIME__ ) ;
#if defined	sam3s4
    printf( "-- Platform: sam3s-ek --\r\n" ) ;
#endif
#if defined	__SAM4S16C__
    printf( "-- Platform: sam4s-xplained --\r\n" ) ;
#endif
	
	/* Initialize encode function. */
   	enc_init();
	
	/* Initialize TC Capture. */
	TcCaptureInitialize();
	
#if defined	sam3s4	
	/* Configure TC interrupts for TC0 channel 2 only */
    NVIC_DisableIRQ( TC2_IRQn ) ;
    NVIC_ClearPendingIRQ( TC2_IRQn ) ;
    NVIC_SetPriority( TC2_IRQn, 0 ) ;
    NVIC_EnableIRQ( TC2_IRQn ) ;
#else //SAM4S-XPLAINED
	/* Configure TC interrupts for TC0 channel 1 only */
    NVIC_DisableIRQ( TC1_IRQn ) ;
    NVIC_ClearPendingIRQ( TC1_IRQn ) ;
    NVIC_SetPriority( TC1_IRQn, 0 ) ;
    NVIC_EnableIRQ( TC1_IRQn ) ;
#endif
	
	/* Initial PMC_PCK2 PA18B as pck output
	 * clock source is PLLB and no prescaler.
	 */
	//PMC->PMC_PCK[2] = PMC_PCK_CSS_PLLB_CLK ;
    //PMC->PMC_SCER = PMC_SCER_PCK2;
	/* Wait for the PCKRDY1 bit to be set in the PMC_SR register */
    //while ( (PMC->PMC_SR & PMC_SR_PCKRDY1) == 0 ) ;
	
	 /* Initial Com port, uart0. */
	 _ConfigureCom();
	
    /* main menu*/
    //_DisplayMenuChoices() ;

    while( 1 )
    {
	  	if(edge_occur){
#if defined	__SAM4S16C__
    		XplnLED_Set(0);	//LED0 on
#endif
			
#if defined	sam3s4	
			LED_Set(0);	
#endif
			
	  		edge_occur = false;
			decode_machine();
			
#if defined	__SAM4S16C__
    		XplnLED_Clear(0); 	//LED0 off
#endif
			
#if defined	sam3s4	
			LED_Clear(0);	
#endif
		}
		
        //c = UART_GetChar() ;
        switch ( c )
        {
            case '0' :
                printf( "Frequency:" ) ;
                freq = _GetInputValue( 200, 3000 ) ;
                printf( "\r\n" ) ;
                printf( "Set frequency to:%dHz\n", freq ) ;

                if ( freq > 0 )
                {
                    SysTick_Config( BOARD_MCK / (freq*SAMPLES) ) ;
                    frequency = freq ;
                }
            break ;

            case '1' :
                printf( "Amplitude:" ) ;
                amp = _GetInputValue( 100, 2047 ) ;
                printf( "\r\n" ) ;
                printf( "Set amplitude to %d \n", amp ) ;
                if ( amp > 0 )
                {
                    amplitude = amp ;
                }
            break ;

            case 'i' :
            case 'I' :
                printf( "-I- Frequency:%d Hz Amplitude:%d\r\n", frequency, amplitude ) ;
			break ;

            case 'm' :
            case 'M' :
                _DisplayMenuChoices() ;
            break ;
				
			case 'G':
			case 'g':
		  		if(us1_get_count()){
		        	printf( "-U- Buffer is active, %c\r\n", us1_get_char() ) ;	
		  		}
		  		else{
		        	printf( "-U- Buffer is empty.\r\n" ) ;
		  		}
		 		break;
		}

        //printf( "Press \'m\' or \'M\' to display the main menu again!!\r\n" ) ;
    }
}

