/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
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

/**
 * \file
 *
 * Provides the low-level initialization function that called on chip startup.
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/* Clock settings at 48MHz */
#if (BOARD_MCK == 48000000)
#define BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8))
#define BOARD_PLLAR     (CKGR_PLLAR_STUCKTO1 \
                       | CKGR_PLLAR_MULA(0x7) \
                       | CKGR_PLLAR_PLLACOUNT(0x1) \
                       | CKGR_PLLAR_DIVA(0x1))
#define BOARD_MCKR      (PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK)

/* Clock settings at 64MHz */
#elif (BOARD_MCK == 64000000)
#define BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8))
#define BOARD_PLLAR     (CKGR_PLLAR_STUCKTO1 \
                       | CKGR_PLLAR_MULA(0x0f) \
                       | CKGR_PLLAR_PLLACOUNT(0x1) \
                       | CKGR_PLLAR_DIVA(0x3))
#define BOARD_MCKR      (PMC_MCKR_PRES_CLK | PMC_MCKR_CSS_PLLA_CLK)

#else
    #error "No settings for current BOARD_MCK."
#endif

/* Define clock timeout */
#define CLOCK_TIMEOUT    0xFFFFFFFF

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Performs the low-level initialization of the chip.
 * This includes EFC and master clock configuration.
 * It also enable a low level on the pin NRST triggers a user reset.
 */
extern WEAK void LowLevelInit( void )
{
    uint32_t timeout = 0;

    /* Set 3 FWS for Embedded Flash Access */
   EFC->EEFC_FMR = 6 << 8;

    /* Enable RC oscillator and wait for stabilization */
  PMC->CKGR_MOR = (PMC->CKGR_MOR & ~(0x7 << 4)) | CKGR_MOR_MOSCRCEN | (2 << 4) | (0x37ul << 16);
  while (!(PMC->PMC_SR & PMC_SR_MOSCRCS));

  /* Configure Master clock to work from Main clock source, not PLL */
  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

  /* Disable Master clock prescaler. Disable PLL/2 clock dividers by the way */
  PMC->PMC_MCKR &= ~(PMC_MCKR_PRES_Msk | PMC_MCKR_PLLADIV2 | PMC_MCKR_PLLBDIV2);
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

  /* Switch Main clock to RC */
  PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCSEL) | (0x37ul << 16);
  while (!(PMC->PMC_SR & PMC_SR_MOSCSELS));

  /* Disable PLLA */
  PMC->CKGR_PLLAR =  (1 << 29);
  while (PMC->PMC_SR & PMC_SR_LOCKA);

  /* Disable PLLB */
  PMC->CKGR_PLLBR = (1 << 29);
  while (PMC->PMC_SR & PMC_SR_LOCKB);

  /* Disable potentially enabled xtal */
  PMC->CKGR_MOR =  (PMC->CKGR_MOR & ~CKGR_MOR_MOSCXTEN) | (0x37ul << 16);
  while (PMC->PMC_SR & PMC_SR_MOSCXTS);

  /* Set minimum allowed number of waitstates for 4MHz RC */
  EFC->EEFC_FMR = 0 << 8;

  /* Set 3 FWS for Embedded Flash Access */
   EFC->EEFC_FMR = 6 << 8;
	
	/* Clocking from xtal */
  /* Start oscillator. Typically startup time 1.4 ms for crystals from 12 to 16 MHz. 1 ms for 18 MHz.
     Slow clock = 1/32768 = 30.51 us approximately.
     Start up time = 8 * 32 / SCK = 256 / 32768 = 7.8 ms */
  PMC->CKGR_MOR |= CKGR_MOR_MOSCXTEN | (32ul << 8) | (0x37ul << 16);
  while (!(PMC->PMC_SR & PMC_SR_MOSCXTS));

  /* Select crystal oscilator as the Main Clock. */
  PMC->CKGR_MOR |= CKGR_MOR_MOSCSEL | (0x37ul << 16);
  while (!(PMC->PMC_SR & PMC_SR_MOSCSELS));

  /* PLLA is required to nail required CPU frequency */
  /* Actual multiplication ratio is is f/DIV * (MUL + 1), thus (PLLA_MUL - 1) */
  PMC->CKGR_PLLAR = BOARD_PLLAR;
  while (!(PMC->PMC_SR & PMC_SR_LOCKA));

  /* Set master clock prescaler and PLL/2 prescaler if required */
  //PMC->PMC_MCKR = BOARD_MCKR;
  //while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

  /* Switch to PLLA as master and processor clock source */
  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_PRES_Msk) | PMC_MCKR_PRES_CLK;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_PLLA_CLK;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

  /* Switch off internal RC oscillator */
  PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCRCEN) | (0x37ul << 16);
  while (PMC->PMC_SR & PMC_SR_MOSCRCS);

  /* Setup minimum allowed number of waitstates for given CPU frequency */
  EFC->EEFC_FMR = (0x03 << 8);
	
}
