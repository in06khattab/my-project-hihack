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
#include "main.h"
#include "encode.h"
#include "board.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define PIN_TC0_TIOA1    {PIO_PA15, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_TC0_TIOA2    {PIO_PA26, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#if defined	sam3s4
	#define DEC_CAPTURE_INPUT	PIN_TC0_TIOA2
#else
	#define DEC_CAPTURE_INPUT	PIN_TC0_TIOA1
#endif

/* use which timer as decode machine timer stamp generator. */
#define DECODE_USED_TMR_ID  2


/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/

typedef struct decode_tag
{
  	uint32_t prev_stamp;
	uint32_t step;
	state_t state;
   uint8_t odd;
	uint8_t data;
}decode_t;

typedef enum _edge_tag_
{
  	none = 0,
	falling,
	rising
}edge_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HAL_USE_ACC_CAP	1

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern uint32_t	edge_occur;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void TcCaptureInitialize(void);
void decode_machine(void);

#endif 	//_DECODE_H_
//end of file
