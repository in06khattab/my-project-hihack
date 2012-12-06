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
#include "hal.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
//F_CPU == (4000000UL)
#define DECODE_TMR2_FREQ_2KHZ_MIN	61
#define DECODE_TMR2_FREQ_2KHZ_MAX	63

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
enum{
  	falling = 0,
	rising
};

typedef struct decode_tag
{
  	uint8_t prev_stamp;
	uint8_t prev_ovfw;
	uint8_t skip;
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

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void ac_init(void);
void decode_machine(void);

#endif 	//_DECODE_H_
//end of file