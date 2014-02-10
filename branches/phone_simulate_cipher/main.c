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
#include "aes.h"

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

/** Pushbutton \#1 pin instance. */
const Pin pinPB1 = PIN_PUSHBUTTON_1;
const Pin pinPB2 = PIN_PUSHBUTTON_2;

/** Pushbutton \#1 pin event flag. */
volatile bool button1Evt = false;
volatile bool button2Evt = false;

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

/* a sample key, key must be located in RAM */
uint8_t key[]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
/* sample data, you can encrypt what you want but keep in mind that only 128 bits (not less not more) get encrypted*/
uint8_t data[] = { 0x01, 0x02, 0x03, 0x04,
                   0x05, 0x06, 0x07, 0x08,
                   0x09, 0x0A, 0x0B, 0x0C,
                   0x0D, 0x0E, 0x0F, 0x00 };
aes128_ctx_t ctx; /* the context where the round keys are stored */

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
/**
 *  \brief Handler for Button 1 rising edge interrupt.
 *
 *  Set button1 event flag (button1Evt).
 */
static void _Button1_Handler( const Pin *pin )
{
    if ( pin->mask == pinPB1.mask && pin->id == pinPB1.id )
    {
        button1Evt = true ;
		bias++;
    }
    else
    {
        button1Evt = false ;
    }
}

static void _Button2_Handler( const Pin *pin )
{
    if ( pin->mask == pinPB2.mask && pin->id == pinPB2.id )
    {
        button2Evt = true ;
		bias--;
    }
    else
    {
        button2Evt = false ;
    }
}


/**
 *  \brief Configure the Pushbutton
 *
 *  Configure the PIO as inputs and generate corresponding interrupt when
 *  pressed or released.
 */
static void _ConfigureButton( void )
{
    /* Configure pios as inputs. */
    PIO_Configure(&pinPB1, 1 );
	PIO_Configure(&pinPB2, 1 );
    /* Adjust pio debounce filter parameters, uses 10 Hz filter. */
    PIO_SetDebounceFilter(&pinPB1, 10);
	PIO_SetDebounceFilter(&pinPB2, 10);
    /* Initialize pios interrupt handlers, see PIO definition in board.h. */
    PIO_ConfigureIt( &pinPB1, _Button1_Handler); /* Interrupt on rising edge  */
	PIO_ConfigureIt( &pinPB2, _Button2_Handler); /* Interrupt on rising edge  */
    /* Enable PIO controller IRQs. */
    NVIC_EnableIRQ(PIOB_IRQn);
	NVIC_EnableIRQ(PIOC_IRQn);
    /* Enable PIO line interrupts. */
    PIO_EnableIt(&pinPB1);
	PIO_EnableIt(&pinPB2);
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
    /* Disable watchdog */
    WDT_Disable( WDT ) ;
	
	/* Configure pins*/
    PIO_Configure( pins, PIO_LISTSIZE( pins ) ) ;
	
	/* Configure buttion. */
	_ConfigureButton();

#if defined	sam3s4	
	/* Configure leds . */
	LED_Configure(0) ;
	LED_Configure(1) ;
#endif
	
#if defined	__SAM4S16C__
    XplnLED_Configure();
#endif
	
	UART_Configure(1200, BOARD_MCK);
	
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
	
	 aes128_init(key, &ctx); /* generating the round keys from the 128 bit key */
	
    /* main menu*/
    //_DisplayMenuChoices() ;

    while( 1 )
    {
		//aes128_enc(data, &ctx); /* encrypting the data block */

		//aes128_dec(data, &ctx); /* decrypting the data block */
		
		dec_stream_process() ;
	  	/*if(edge_occur){
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
		} */
    }
}

