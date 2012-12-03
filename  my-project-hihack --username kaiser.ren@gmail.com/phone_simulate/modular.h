/* ----------------------------------------------------------------------------
 *         MODULAR SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file modular.h
 *
 * Implements machester modulation.
 *
 */
/*------------------------------------------------------------------------------*/
/*         INCLUDE                                                   			*/
/*------------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

/*------------------------------------------------------------------------------*/
/*         TYPEDEF                                                   			*/
/*------------------------------------------------------------------------------*/
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
	Bit0,
	Bit1,
	Bit2,
	Bit3,
	Bit4,
	Bit5,
	Bit6,
	Bit7
}mod_state_t;

typedef enum prev_bit_tag
{
	Vacant = 0,
	Occupy = 1
}prev_bit_t;

typedef enum factor_tag
{
	Div1 = 1,
	Div2 = 2  	
}factor_t;

typedef struct module_tag
{
  	unsigned char data;
	mod_state_t state;
	prev_bit_t  prev;
    factor_t	factor;
}modulate_t;


extern modulate_t mod;
extern uint8_t index_sample;

void state_switch(void);
uint8_t us1_get_count(void);
uint8_t us1_get_char(void);

//end of file
