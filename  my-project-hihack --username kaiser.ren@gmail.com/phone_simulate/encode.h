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
#include "board.h"

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/

#define CRITICAL_PROTECTION 1

/** Reference voltage for DACC,in mv*/
#define VOLT_REF   (3300)

/** The maximal digital value*/
#define MAX_DIGITAL (4095)

/** SAMPLES per cycle*/
#define SAMPLES (100)

/** uart buffer size*/
#define US_BUFFER_SIZE (200)

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
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

typedef struct _usart_rx_tag
{
  	uint8_t count;
	uint8_t head;
	uint8_t tail;
	uint8_t buff[US_BUFFER_SIZE];
}us_rx_t;

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
//extern encode_t enc;
//extern uint8_t index_sample;
//extern uint8_t ticker;
extern us_rx_t us1;
extern const int16_t sine_data[SAMPLES];
extern uint16_t amplitude ;
extern uint16_t frequency ;

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
