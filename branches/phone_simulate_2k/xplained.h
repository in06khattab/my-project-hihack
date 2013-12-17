/* ----------------------------------------------------------------------------
 *         XPLAINED HEADER
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file xplained.h
 *
 * sam4s-xplained bsp driver
 *
 */
#ifndef _XPLAINED_H_
#define _XPLAINED_H_
/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "board.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
/** LED #0 pin definition FOR xplained. */
#define XPLN_PIN_LED_0   {PIO_PC10, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
/** LED #1 pin definition FOR xplained. */
#define XPLN_PIN_LED_1   {PIO_PC17, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}

/** List of all LEDs definitions. */
#define XPLN_PINS_LEDS   XPLN_PIN_LED_0, XPLN_PIN_LED_1

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
uint32_t XplnLED_Configure( void );
void XplnLED_Set( uint32_t dwLed );
void XplnLED_Clear( uint32_t dwLed );
void XplnLED_Toggle( uint32_t dwLed );

#endif 	//_XPLAINED_H_
//end of file
