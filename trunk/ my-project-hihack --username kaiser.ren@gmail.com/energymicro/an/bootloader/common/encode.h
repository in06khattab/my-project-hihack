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

#define CRITICAL_PROTECTION 1

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
extern buffer_t encBuf;
/*
 * a delay counter, unit is 500ms, enc machine work only if counter is zero.
 */
extern uint32_t enc_delay_tmr_cnt;
/*----------------------------------------------------------------------------
 *        External Function
 *----------------------------------------------------------------------------*/
/******************************************************************************
 * @brief  encode peripheral part initial.
 *
 *****************************************************************************/
void enc_init(void);
/******************************************************************************
 * @brief  uartPutData function
 *
 *****************************************************************************/
__ramfunc void HIJACKPutData(uint8_t * dataPtr, buffer_t * dstBuf, uint32_t dataLen);

/******************************************************************************
 * @brief  format print.
 *****************************************************************************/
void enc_print(const char *format, ...);
#endif /* ENCODE_H */
//end of file
