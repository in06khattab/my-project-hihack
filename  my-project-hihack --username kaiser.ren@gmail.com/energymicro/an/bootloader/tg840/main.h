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
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "xstdio.h"
#include "em_i2c.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#define VER_MAJOR 0
#define VER_MINOR 8
#define VER_PATCH ' '


#define BOARD_MCK 7000000ul
#define MLX90615

/* PRESCALER: 64 */
#define HIJACK_TIMER_RESOLUTION     timerPrescale16
#define HIJACK_TMR_CLK_SRC_PRESCALER ( 0x01 << HIJACK_TIMER_RESOLUTION)

/* how many ticker per us. */
//#define HIJACK_NUM_TICKS_PER_1US (BOARD_MCK/HIJACK_TMR_CLK_SRC_PRESCALER/1000000ul)

/* how many ticks per half cycle. */
#define HIJACK_NUM_TICKS_PER_HALF_CYCLE (BOARD_MCK / HIJACK_TMR_CLK_SRC_PRESCALER / HIJACK_DEC_CARRIER / 2)

/* how many ticks per full cycle . */
#define HIJACK_NUM_TICKS_PER_FULL_CYCLE (BOARD_MCK / HIJACK_TMR_CLK_SRC_PRESCALER / HIJACK_DEC_CARRIER)

/* how manu ticks when precision is about 5%. */
#define HIJACK_NUM_TICKS_PER_20_PCNT	(HIJACK_NUM_TICKS_PER_FULL_CYCLE / 10)

/* Declare a circular buffer structure to use for Rx and Tx queues */
#define BUFFERSIZE          200

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

typedef struct _circularBuffer_
{

  uint32_t rdI;               /* read index */
  uint32_t wrI;               /* write index */
  uint32_t pendingBytes;      /* count of how many bytes are not yet handled */
  bool     overflow;          /* buffer overflow indicator */
  uint8_t  data[BUFFERSIZE];  /* data buffer */
}buffer_t;

extern uint32_t sys_tick;
#endif//_MAIN_H_
