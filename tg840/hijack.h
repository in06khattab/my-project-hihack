/**************************************************************************//**
 * @file hijack.h
 * @brief Hijack demo for EFM32TG_STK3300
 * @version 1.01
 ******************************************************************************
 * @section License
 ******************************************************************************
 *
 * Copyright (c) 2010 The Regents of the University of Michigan, Energy Micro 2012
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * - Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the
 *  distribution.
 * - Neither the name of the copyright holder nor the names of
 *  its contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Thomas Schmid
 * Modified by Energy Micro (2012).
 *****************************************************************************/

#ifndef __HIJACK_H
#define __HIJACK_H

#include <stdint.h>
#include <stdbool.h>

/* Device specific header file(s). */


/* LEOS header file(s). */



#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup App_Notes
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup HiJack
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   MACROS   ************************************
 ******************************************************************************/


/*******************************************************************************
 ****************************   CONFIGURATION   ********************************
 ******************************************************************************/

  
/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

typedef enum
{
  hijackEdgeModeRising = 0,
  hijackEdgeModeFalling = 1,
  hijackEdgeModeBoth = 2
} HIJACK_EdgeMode_t;


typedef enum
{
  hijackOutputModeSet = 0,
  hijackOutputModeClear,
  hijackOutputModeToggle
} HIJACK_OutputMode_t;

/** Task pointer type. */
typedef void (*HIJACK_TxDoneFuncPtr_t)(void);
/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/
void HIJACK_Init(HIJACK_TxDoneFuncPtr_t pTxDone);
bool HIJACK_ByteTx(uint8_t byte);
void HIJACK_ByteRx(uint8_t *pByte);


/** @} (end addtogroup HiJack) */
/** @} (end addtogroup App_Notes) */

#ifdef __cplusplus
}
#endif

#endif /* __HIJACK_H */
