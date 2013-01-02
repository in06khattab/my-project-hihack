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
#include "xplained.h"


/*----------------------------------------------------------------------------
 *        Variable
 *----------------------------------------------------------------------------*/
/** PIOs for xplained leds */
static const Pin XplanpinsLeds[] = { XPLN_PINS_LEDS } ;


/*----------------------------------------------------------------------------
 *        Local function
 *----------------------------------------------------------------------------*/
uint32_t XplnLED_Configure( void );
void XplnLED_Set( uint32_t dwLed );
void XplnLED_Clear( uint32_t dwLed );
void XplnLED_Toggle( uint32_t dwLed );
/*----------------------------------------------------------------------------
 *        Functions
 *----------------------------------------------------------------------------*/
/**
 *  Configures the pin associated with the given LED number. If the LED does
 *  not exist on the board, the function does nothing.
 */
uint32_t XplnLED_Configure( void )
{
    return ( PIO_Configure( XplanpinsLeds, sizeof(XplanpinsLeds) ) ) ;
}

/**
 *  Turns the given LED on if it exists; otherwise does nothing.
 *  \param led  Number of the LED to turn on.
 *  \return 1 if the LED has been turned on; 0 otherwise.
 */
void XplnLED_Set( uint32_t dwLed )
{
    /* Turn LED on */
    if ( XplanpinsLeds[dwLed].type == PIO_OUTPUT_0 )
    {

        PIO_Set( &XplanpinsLeds[dwLed] ) ;
    }
    else
    {
        PIO_Clear( &XplanpinsLeds[dwLed] ) ;
    }
}

/**
 *  Turns a LED off.
 *
 *  \param led  Number of the LED to turn off.
 *  \return 1 if the LED has been turned off; 0 otherwise.
 */
void XplnLED_Clear( uint32_t dwLed )
{
    /* Turn LED off */
    if ( XplanpinsLeds[dwLed].type == PIO_OUTPUT_0 )
    {
        PIO_Clear( &XplanpinsLeds[dwLed] ) ;
    }
    else
    {
        PIO_Set( &XplanpinsLeds[dwLed] ) ;
    }
}

/**
 *  Toggles the current state of a LED.
 *
 *  \param led  Number of the LED to toggle.
 *  \return 1 if the LED has been toggled; otherwise 0.
 */
void XplnLED_Toggle( uint32_t dwLed )
{
    /* Toggle LED */
    if ( PIO_GetOutputDataStatus( &XplanpinsLeds[dwLed] ) )
    {
        PIO_Clear( &XplanpinsLeds[dwLed] ) ;
    }
    else
    {
        PIO_Set( &XplanpinsLeds[dwLed] ) ;
    }
}
//end of file
