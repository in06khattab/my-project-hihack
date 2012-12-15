/**
 * @file hal.c
 *
 * @brief hal board specific functionality
 *
 * This file implements hal board specific functionality.
 *
 * @author    kren
 * @data		Nov 21, 2012
 */

/* === Includes ============================================================ */
#include "hal.h"


/* === Implementation ====================================================== */
/**
 * @brief Initialize the system clock
 *
 * This function sets the system clock by enabling the internal RC oscillator
 * with the proper prescaler (if available).
 * Ensure that CKDIV8 fuse does not affect the system clock prescaler.
 *
 */
static void mcu_clock_init(void)
{
/*
 * This is only allowed if the AVR 8-bit MCU already has a clock prescaler.
 * For older devices this function does not make sense.
 * Therefore the existence of register CLKPR is checked.
 */
//#ifdef CLKPR    /* Is clock prescaler existing? */

#if (F_CPU == (8000000UL))
#ifdef __ICCAVR__
    ENTER_CRITICAL_REGION();
    CLKPR = 0x80;
    CLKPR = 0x00;
    LEAVE_CRITICAL_REGION();
#else
    clock_prescale_set(clock_div_1);
#endif  /* __ICCAVR__ */
#endif  /* (F_CPU == (8000000UL) */

#if (F_CPU == (4000000UL))
#ifdef __ICCAVR__
    ENTER_CRITICAL_REGION();
    CLKPR = 0x80;
    CLKPR = 0x01;
    LEAVE_CRITICAL_REGION();
#else
    clock_prescale_set(clock_div_2);
#endif  /* __ICCAVR__ */
#endif  /* (F_CPU == (4000000UL) */

#if (F_CPU == (2000000UL))
#ifdef __ICCAVR__
    ENTER_CRITICAL_REGION();
    CLKPR = 0x80;
    CLKPR = 0x02;
    LEAVE_CRITICAL_REGION();
#else
    clock_prescale_set(clock_div_4);
#endif  /* __ICCAVR__ */
#endif  /* (F_CPU == (2000000UL) */

#if (F_CPU == (1000000UL))
#ifdef __ICCAVR__
    ENTER_CRITICAL_REGION();
    CLKPR = 0x80;
    CLKPR = 0x03;
    LEAVE_CRITICAL_REGION();
#else
    clock_prescale_set(clock_div_8);
#endif  /* __ICCAVR__ */
#endif  /* (F_CPU == (1000000UL) */

//#endif  /* CLKPR */
}

/**
 * @brief Initialization of wdt
 */
static void wdt_init(void)
{
  	__watchdog_reset();
	/* Clear WDRF in MCUSR */
		MCUSR &= ~(1<<WDRF);
		/* Write logical one to WDCE and WDE */
		/* Keep old prescaler setting to prevent unintentional time-out
		*/
		WDTCSR |= (1<<WDCE) | (1<<WDE);
		/* Turn off WDT */
		WDTCSR = 0x00;
}

/**
 * @brief Initialization of hal
 */
void hal_init(void)
{
	/* Config mcu clock. */
   mcu_clock_init();
	
	/* Calibrate MCU's RC oscillator */
    //pal_calibrate_rc_osc();
	
	/* wdt initial. */
	wdt_init();
	
	/*
		analog initial.
		AIN0, positive, is PE2 on mcu, module pin 40, which connect with
		battery+ for reference.
	*/
	ac_init();
	
	/*
		initial TMR2 to generate waveform.
	*/
	pwm_init();
	
	/* Initialize the serial interface used for communication with terminal program */
    pal_sio_init(SIO_CHANNEL);

	/*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();
	
	 /* Initialize LEDs */
   	pal_led_init();
}

//eof
