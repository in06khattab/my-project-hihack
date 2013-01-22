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


/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define HIJACK_RX_TIMER             TIMER0
#define HIJACK_RX_TIMERCLK          cmuClock_TIMER0
#define HIJACK_RX_GPIO_PORT         gpioPortD
#define HIJACK_RX_GPIO_PIN          2


/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/

typedef enum _tmr_tag_{
	pass = 0,
	suit,
	error
}chk_result_t;

typedef struct decode_tag
{
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

typedef enum
{
  hijackEdgeModeRising = 0,
  hijackEdgeModeFalling = 1,
  hijackEdgeModeBoth = 2
} HIJACK_EdgeMode_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HAL_USE_ACC_CAP	1

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern bool	edge_occur;
extern uint16_t cur_stamp ;
extern uint16_t prv_stamp ;
extern edge_t 	cur_edge;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void dec_init(void);
void decode_machine(void);

#endif 	//_DECODE_H_
//end of file
