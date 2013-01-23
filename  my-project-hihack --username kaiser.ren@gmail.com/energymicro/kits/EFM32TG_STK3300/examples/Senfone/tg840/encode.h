/* ----------------------------------------------------------------------------
 *         ENCODE SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file encode.h
 *
 * Implements machester modulation.
 *
 */

#ifndef ENCODE_H
#define ENCODE_H
/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "main.h"
/* Chip specific header file(s). */
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_int.h"

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
/* HiJack states. */
typedef enum
{
  STARTBIT = 0U,
  STARTBIT_FALL,
  DECODE,
  STOPBIT,
  BYTE,
  IDLE,
} HIJACK_State_t;

typedef enum
{
  hijackOutputModeSet = 0,
  hijackOutputModeClear,
  hijackOutputModeToggle
} HIJACK_OutputMode_t;

typedef enum next_bit_tag
{
	CLR = 0,
	SET = 1
}next_bit_t;

typedef enum factor_tag
{
	Div1 = 1,
	Div2 = 2  	
}factor_t;

typedef struct module_tag
{
  	uint8_t 	data;
	state_t state;
	next_bit_t  cur;
    factor_t	factor;
	uint8_t		reverse;
}encode_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HIJACK_TX_TIMER             TIMER1
#define HIJACK_TX_TIMERCLK          cmuClock_TIMER1
#define HIJACK_TX_GPIO_PORT         gpioPortD
#define HIJACK_TX_GPIO_PIN          6
#define HIJACK_TX_INTERVAL          (45)

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
//extern encode_t enc;
//extern uint8_t index_sample;
//extern uint8_t ticker;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void encode_machine(void);
uint8_t us1_get_count(void);
uint8_t us1_get_char(void);
void _ConfigureCom( void );
void enc_init(void);
#endif /* ENCODE_H */
//end of file
