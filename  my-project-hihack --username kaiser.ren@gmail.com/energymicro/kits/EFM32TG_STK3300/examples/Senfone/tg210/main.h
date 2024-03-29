/* ----------------------------------------------------------------------------
 *         MAIN HEADER
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren, renkaikaiser@hotmail.com
 */
/**
 * \file main.h
 *
 * Various configuration for this project.
 *
 */
/* ----------------------------------------------------------------------------*/

#ifndef _MAIN_H_
#define _MAIN_H_

/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
//#include "board.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define VER_MAJOR 0
#define VER_MINOR 2
#define VER_PATCH ' '


#define BOARD_MCK 7000000ul

/* PRESCALER: 128 */
#define HIJACK_TMR_CLK_SRC_PRESCALER 1
#define HIJACK_TIMER_RESOLUTION     timerPrescale1

/* how many ticker per us. */
#define HIJACK_NUM_TICKS_PER_1US (BOARD_MCK/HIJACK_TMR_CLK_SRC_PRESCALER/1000000ul)

/* how many ticks per half cycle. */
#define HIJACK_NUM_US_PER_HALF_CYCLE (1000000ul/HIJACK_CARRIER_FREQ_CONF/2)
#define HIJACK_NUM_TICKS_PER_HALF_CYCLE (HIJACK_NUM_US_PER_HALF_CYCLE*HIJACK_NUM_TICKS_PER_1US)

/* how many ticks per full cycle . */
#define HIJACK_NUM_US_PER_FULL_CYCLE (1000000/HIJACK_CARRIER_FREQ_CONF)
#define HIJACK_NUM_TICKS_PER_FULL_CYCLE (HIJACK_NUM_US_PER_FULL_CYCLE*HIJACK_NUM_TICKS_PER_1US)

/* how manu ticks when precision is about 5%. */
#define HIJACK_NUM_TICKS_PER_5_PCNT	(HIJACK_NUM_TICKS_PER_FULL_CYCLE/20)

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
/* bit mask . */
enum{
	BIT0 = 0,
	BIT1 = 1,
	BIT2 = 2,
	BIT3 = 3,
	BIT4 = 4,
	BIT5 = 5,
	BIT6 = 6,
	BIT7 = 7
};

/* state definition. */
typedef enum modulation_state_tag
{
  	Waiting = 0,
	Sta0,
	Sta1,
	Sta2,
	Sta3,
	Bit7,
	Bit6,
	Bit5,
	Bit4,
	Bit3,
	Bit2,
	Bit1,
	Bit0,
	Sto0,
	Sto1,
	Parity
}state_t;

#endif//_MAIN_H_
