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
#ifndef _ENCODE_H_
#define _ENCODE_H_
/*----------------------------------------------------------------------------
 *        Header
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "pal_types.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
#if (PAL_TYPE==ATTINY88)
   #define ENC_DIR   DDRB
   #define ENC_PORT  PORTB
   #define ENC_PIN   PORTB7
#else//ATMEGA1281
   #define ENC_DIR   DDRF
   #define ENC_PORT  PORTF
   #define ENC_PIN   PORTF1
#endif//PAL_TYPE==ATTINY88

/* use which timer as encode machine timer stamp generator. */
#define ENCODE_USED_TMR_ID  0

/* use which prescaler as decode machine clock source. */
#define ENCODE_TMR_CLK_SRC_PRESCALER 8

/* encode timer's prescaler definition. */
#if ENCODE_USED_TMR_ID==0
    #if ENCODE_TMR_CLK_SRC_PRESCALER==0
		#define ENCODE_TMR_CLK_SRC_PRESCALER_REG 0//timer stop
    #elif ENCODE_TMR_CLK_SRC_PRESCALER==1
        #define ENCODE_TMR_CLK_SRC_PRESCALER_REG 1//no prescaler
    #elif ENCODE_TMR_CLK_SRC_PRESCALER==8
        #define ENCODE_TMR_CLK_SRC_PRESCALER_REG 2//prescaler 8
    #elif ENCODE_TMR_CLK_SRC_PRESCALER==64
        #define ENCODE_TMR_CLK_SRC_PRESCALER_REG 3//prescaler 64
    #elif ENCODE_TMR_CLK_SRC_PRESCALER==256
        #define ENCODE_TMR_CLK_SRC_PRESCALER_REG 4//prescaler 256
    #elif ENCODE_TMR_CLK_SRC_PRESCALER==1024
        #define ENCODE_TMR_CLK_SRC_PRESCALER_REG 5//prescaler 1024
    #else
        #error "Unsupported Decode Timer Prescaler."
    #endif//ENCODE_TMR_CLK_SRC_PRESCALER
#endif//ENCODE_USED_TMR_ID

/* F_CPU == (4000000UL)   */
#if F_CPU==4000000UL
    #if ENCODE_TMR_CLK_SRC_PRESCALER==8    //prescale: 8
		/**
         *	PRESCALSE: 8
         *	F_CPU: 4MHz
         * Tick:  4MHz/8 = 2us.
         * TMR overflow interrupt, 256*2us = 512us
	     * 250us need 125 tick.
        **/
        #define ENCODE_TMR_OCR_CNT	125
    #else
        #error "Unsupported Decode Timer Prescaler."
    #endif//ENCODE_TMR_CLK_SRC_PRESCALER
#elif F_CPU==2000000UL	//F_CPU = 2MHz
	#if ENCODE_TMR_CLK_SRC_PRESCALER==8    //prescale: 8
		/**
         *	PRESCALSE: 8
         *	F_CPU: 2MHz
         * Tick:  2MHz/8 = 4us.
         * TMR overflow interrupt, 256*4us = 1024us
         * 500us need 125 tick.
        **/
        #define ENCODE_TMR_OCR_CNT	125
	#else
        #error "Unsupported Decode Timer Prescaler."
    #endif//ENCODE_TMR_CLK_SRC_PRESCALER
#else
    #error "Unsupported F_CPU value"
#endif
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
	Parity,
	Sto0,
	Sto1
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

typedef enum edge_tag
{
  	falling = 0xf0,
	rising = 0x0f
}edge_t;

typedef struct encode_tag
{
  	uint8_t 	data;
	mod_state_t state;
	uint8_t port;
	uint8_t byte_rev;
	edge_t edge;
}encode_t;



/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern encode_t enc;
extern uint8_t ticker;
extern uint8_t tmr0_occur;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void encode_machine(void);
void tmr0_init(void);

#endif //_ENCODE_H_
//end of file
