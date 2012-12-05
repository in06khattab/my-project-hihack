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

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
enum{
  	falling = 0,
	rising
};

typedef struct decode_tag
{
  	uint8_t data;
	mod_state_t state;
	uint8_t prev_stamp;
}decode_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HAL_USE_ACC_CAP	1

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void ac_init(void);

#endif 	//_DECODE_H_
//end of file
