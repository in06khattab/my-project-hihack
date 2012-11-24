/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "pmc.h"
#include "AT91SAM7X256.h"
//#include <board.h>
//#include <assert.h>
//#include <trace.h>

#ifdef CP15_PRESENT
#include <cp15/cp15.h>
#endif

#define MASK_STATUS 0x3FFFFFFC

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Disables the processor clock
//------------------------------------------------------------------------------
void PMC_DisableProcessorClock(void)
{
    AT91C_BASE_PMC->PMC_SCDR = AT91C_PMC_PCK;
    while ((AT91C_BASE_PMC->PMC_SCSR & AT91C_PMC_PCK) != AT91C_PMC_PCK);
}

//------------------------------------------------------------------------------
/// Enables the clock of a peripheral. The peripheral ID (AT91C_ID_xxx) is used
/// to identify which peripheral is targetted.
/// Note that the ID must NOT be shifted (i.e. 1 << AT91C_ID_xxx).
/// \param id  Peripheral ID (AT91C_ID_xxx).
//------------------------------------------------------------------------------
void PMC_EnablePeripheral(unsigned int id)
{
    //SANITY_CHECK(id < 32);

    if ((AT91C_BASE_PMC->PMC_PCSR & (1 << id)) == (1 << id)) {

        //TRACE_INFO("PMC_EnablePeripheral: clock of peripheral"
        //           " %u is already enabled\n\r",
         //          id);
    }
    else {

        AT91C_BASE_PMC->PMC_PCER = 1 << id;
    }
}

//------------------------------------------------------------------------------
/// Disables the clock of a peripheral. The peripheral ID (AT91C_ID_xxx) is used
/// to identify which peripheral is targetted.
/// Note that the ID must NOT be shifted (i.e. 1 << AT91C_ID_xxx).
/// \param id  Peripheral ID (AT91C_ID_xxx).
//------------------------------------------------------------------------------
void PMC_DisablePeripheral(unsigned int id)
{
    //SANITY_CHECK(id < 32);

    if ((AT91C_BASE_PMC->PMC_PCSR & (1 << id)) != (1 << id)) {

        //TRACE_INFO("PMC_DisablePeripheral: clock of peripheral"
        //           " %u is not enabled\n\r",
        //           id);
    }
    else {

        AT91C_BASE_PMC->PMC_PCDR = 1 << id;
    }
}

//------------------------------------------------------------------------------
/// Enable all the periph clock via PMC
/// (Becareful of the last 2 bits, it is not periph clock)
//------------------------------------------------------------------------------
void PMC_EnableAllPeripherals(void)
{
    AT91C_BASE_PMC->PMC_PCER = MASK_STATUS;
    while( (AT91C_BASE_PMC->PMC_PCSR & MASK_STATUS) != MASK_STATUS);
    //TRACE_INFO("Enable all periph clocks\n\r");
}

//------------------------------------------------------------------------------
/// Disable all the periph clock via PMC
/// (Becareful of the last 2 bits, it is not periph clock)
//------------------------------------------------------------------------------
void PMC_DisableAllPeripherals(void)
{
    AT91C_BASE_PMC->PMC_PCDR = MASK_STATUS;
    while((AT91C_BASE_PMC->PMC_PCSR & MASK_STATUS) != 0);
    //TRACE_INFO("Disable all periph clocks\n\r");
}

//-----------------------------------------------------------------------------
/// Get Periph Status
//-----------------------------------------------------------------------------
unsigned int PMC_IsAllPeriphEnabled(void)
{
    return (AT91C_BASE_PMC->PMC_PCSR == MASK_STATUS);
}

//-----------------------------------------------------------------------------
/// Get Periph Status
//-----------------------------------------------------------------------------
unsigned int PMC_IsPeriphEnabled(unsigned int id)
{
    return (AT91C_BASE_PMC->PMC_PCSR & (1 << id));
}
//------------------------------------------------------------------------------
/// Put the CPU in Idle Mode for lower consumption
//------------------------------------------------------------------------------
void PMC_CPUInIdleMode(void)
{
    PMC_DisableProcessorClock();
#ifdef CP15_PRESENT
    _waitForInterrupt();
#endif
}
//------------------------------------------------------------------------------
/// Put the CPU in 32kHz, disable PLL, main oscillator
/// Put voltage regulator in standby mode
//------------------------------------------------------------------------------
void LowPowerMode(void)
{
  	 PMC_DisableAllPeripherals();
	
    // MCK=48MHz to MCK=32kHz
    // MCK = SLCK/2 : change source first from 48 000 000 to 18. / 2 = 9M
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_64;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    // MCK=SLCK : then change prescaler
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    // disable PLL
    AT91C_BASE_PMC->PMC_PLLR = 0;
    // Disable Main Oscillator
    AT91C_BASE_PMC->PMC_MOR = 0;

    // Voltage regulator in standby mode : Enable VREG Low Power Mode
    AT91C_BASE_VREG->VREG_MR |= AT91C_VREG_PSTDBY;

    PMC_DisableProcessorClock();
}

