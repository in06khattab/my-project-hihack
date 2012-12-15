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
#include "hal.h"
//#include "pal_config.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
/* use which timer as decode machine timer stamp generator. */
#define DECODE_USED_TMR_ID  1

/* use which prescaler as decode machine clock source. */
#define DECODE_TMR_CLK_SRC_PRESCALER 64

#if DECODE_USED_TMR_ID==2
	#if DECODE_TMR_CLK_SRC_PRESCALER==0
		#define DECODE_TMR_CLK_SRC_PRESCALER_REG 0//timer stop
    #elif DECODE_TMR_CLK_SRC_PRESCALER==1
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 1//no prescaler
    #elif DECODE_TMR_CLK_SRC_PRESCALER==8
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 2//prescaler 8
    #elif DECODE_TMR_CLK_SRC_PRESCALER==32
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 3//prescaler 32
    #elif DECODE_TMR_CLK_SRC_PRESCALER==64
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 4//prescaler 64
    #elif DECODE_TMR_CLK_SRC_PRESCALER==128
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 5//prescaler 128
    #elif DECODE_TMR_CLK_SRC_PRESCALER==256
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 6//prescaler 256
    #elif DECODE_TMR_CLK_SRC_PRESCALER==1024
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 7//prescaler 1024
    #else
        #error "Unsupported Decode Timer Prescaler."
    #endif//DECODE_TMR_CLK_SRC_PRESCALER
#elif DECODE_USED_TMR_ID==1
    #if DECODE_TMR_CLK_SRC_PRESCALER==0
		#define DECODE_TMR_CLK_SRC_PRESCALER_REG 0//timer stop
    #elif DECODE_TMR_CLK_SRC_PRESCALER==1
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 1//no prescaler
    #elif DECODE_TMR_CLK_SRC_PRESCALER==8
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 2//prescaler 8
    #elif DECODE_TMR_CLK_SRC_PRESCALER==64
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 3//prescaler 64
    #elif DECODE_TMR_CLK_SRC_PRESCALER==256
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 4//prescaler 256
    #elif DECODE_TMR_CLK_SRC_PRESCALER==1024
        #define DECODE_TMR_CLK_SRC_PRESCALER_REG 5//prescaler 1024
    #else
        #error "Unsupported Decode Timer Prescaler."
    #endif//DECODE_TMR_CLK_SRC_PRESCALER
#endif//DECODE_USED_TMR_ID

/* F_CPU == (4000000UL)   */
#if F_CPU==4000000UL
    #if DECODE_TMR_CLK_SRC_PRESCALER==32    //prescale: 32
		/**
         *	PRESCALSE: 32
         *	F_CPU: 4MHz
         * Tick:  4MHz/32 = 8us.
         * TMR2 overflow interrupt, 256*8us = 2048us
         * TMR1 overflow interrupt, 65536*8us = 524288us, 0.524s
        **/
        #define DECODE_TMR_FREQ_2KHZ_MIN	60
        #define DECODE_TMR_FREQ_2KHZ_MAX	64
	#elif DECODE_TMR_CLK_SRC_PRESCALER==64	//prescals: 64
		/**
         *	PRESCALSE: 64
         *	F_CPU: 4MHz
         * Tick:  4MHz/64 = 16us.
         * TMR2 overflow interrupt, 256*16us = 4096us
         * TMR1 overflow interrupt, 65536*16us = 1048576us, 1.048s
        **/
        #define DECODE_TMR_FREQ_2KHZ_MIN	30
        #define DECODE_TMR_FREQ_2KHZ_MAX	32
    #elif DECODE_TMR_CLK_SRC_PRESCALER==128 //prescals: 128
        /**
         *	PRESCALSE: 128
         *	F_CPU: 4MHz
         * Tick:  4MHz/128 = 32us.
         * TMR2 overflow interrupt, 256*32us = 8192us
         * TMR1 overflow interrupt, 65536*32us = 2097152us, 2.097s
        **/
        #define DECODE_TMR_FREQ_2KHZ_MIN	14
        #define DECODE_TMR_FREQ_2KHZ_MAX	16
    #else
        #error "Unsupported Decode Timer Prescaler."
    #endif//DECODE_TMR_CLK_SRC_PRESCALER
#else
    #error "Unsupported F_CPU value"
#endif
//prescale: 128


/*----------------------------------------------------------------------------
 *        Typedef
 *----------------------------------------------------------------------------*/
enum{
  	falling = 0,
	rising
};

typedef struct decode_tag
{
  	uint16_t prev_stamp;
	uint8_t prev_ovfw;
	mod_state_t state;
	uint8_t acsr;
	uint8_t data;
}decode_t;

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/
#define HAL_USE_ACC_CAP	1

/*----------------------------------------------------------------------------
 *        External Variable
 *----------------------------------------------------------------------------*/
extern uint8_t	acc_occur;
extern uint8_t	ovfw ;

/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
void ac_init(void);
void decode_machine(void);

#endif 	//_DECODE_H_
//end of file
