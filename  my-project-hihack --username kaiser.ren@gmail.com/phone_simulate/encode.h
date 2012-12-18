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

/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "board.h"

/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
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
}mod_state_t;

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
	mod_state_t state;
	next_bit_t  cur;
    factor_t	factor;
	uint8_t		reverse;
}modulate_t;

typedef struct _usart_rx_tag
{
  	uint8_t count;
	uint8_t head;
	uint8_t tail;
	uint8_t buff[20];
}us_rx_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
/** Reference voltage for DACC,in mv*/
#define VOLT_REF   (3300)

/** The maximal digital value*/
#define MAX_DIGITAL (4095)

/** SAMPLES per cycle*/
#define SAMPLES (100)

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern modulate_t mod;
extern uint8_t index_sample;
extern uint8_t ticker;
extern us_rx_t us1;
extern const int16_t sine_data[SAMPLES];
extern uint16_t amplitude ;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void encode_switch(void);
uint8_t us1_get_count(void);
uint8_t us1_get_char(void);
void _ConfigureUsart( void );
//end of file
