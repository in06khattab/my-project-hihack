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
#include "decode.h"
/* Chip specific header file(s). */
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_int.h"

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HIJACK_TX_TIMER             TIMER1
#define HIJACK_TX_TIMERCLK          cmuClock_TIMER1
#define HIJACK_TX_GPIO_PORT         gpioPortB
#define HIJACK_TX_GPIO_PIN          7


/* several carrier frequency definition. */
#define HIJACK_ENC_CARRIER_FREQ_1KHZ	1000ul
#define HIJACK_ENC_CARRIER_FREQ_2KHZ	2000ul
#define HIJACK_ENC_CARRIER_FREQ_4KHZ	4000ul
#define HIJACK_ENC_CARRIER_FREQ_8KHZ	8000ul

/* current carrier frequency definition. */
#define HIJACK_ENC_CARRIER_FREQ_CONF HIJACK_ENC_CARRIER_FREQ_1KHZ
/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/

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

typedef struct module_tag
{
  	uint8_t 	data;
	state_t state;
	uint8_t port;
	uint8_t byte_rev;
	edge_t edge;
}encode_t;

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
