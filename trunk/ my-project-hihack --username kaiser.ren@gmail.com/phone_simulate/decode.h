/* ----------------------------------------------------------------------------
 *         DECODE SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file decode.h
 *
 * Implements machester modulation.
 *
 */
#ifndef _DNCODE_H_
#define _DNCODE_H_
/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "encode.h"
#include "board.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define PIN_TC0_TIOA1    {PIO_PA15, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC0_TIOA2    {PIO_PA26, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}

/* use which timer as decode machine timer stamp generator. */
#define DECODE_USED_TMR_ID  2


/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/

typedef struct decode_tag
{
  	uint16_t prev_stamp;
	uint8_t prev_ovfw;
	mod_state_t state;
	uint8_t acsr;
	uint8_t data;
}decode_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HAL_USE_ACC_CAP	1

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern uint8_t	acc_occur;
extern uint8_t	ovfw ;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void TcCaptureInitialize(void);
void decode_machine(void);

#endif 	//_DECODE_H_
//end of file
