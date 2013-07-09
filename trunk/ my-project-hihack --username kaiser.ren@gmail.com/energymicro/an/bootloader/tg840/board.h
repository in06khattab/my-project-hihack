/* ----------------------------------------------------------------------------
 *         board.h
 * ----------------------------------------------------------------------------
 * Copyright (c) 2013, kren
 */

/**
 * \file board.h
 * Created: Apr 9, 2013
 * tg840 definition
 *
 */

/*----------------------------------------------------------------------------
 *        Includes
 *----------------------------------------------------------------------------*/
#include "bsp.h"

/*----------------------------------------------------------------------------
 *        Macro
 *----------------------------------------------------------------------------*/
/* rx timer definition. */
#define HIJACK_RX_TIMER             (TIMER0)
#define HIJACK_RX_TIMERCLK          (cmuClock_TIMER0)
#define HIJACK_RX_GPIO_PORT         (gpioPortC)
#define HIJACK_RX_GPIO_PIN          4

#define HIJACK_RX_ACMP				(ACMP0)
#define HIJACK_RX_ACMPCLK          (cmuClock_ACMP0)
#define HIJACK_RX_ACMP_CH			(acmpChannel4)
#define HIJACK_RX_ACMP_NEG			(acmpChannelVDD)
#define HIJACK_RX_ACMP_LEVEL		(32)


/* tx timer definition. */
#define HIJACK_TX_TIMER             (TIMER1)
#define HIJACK_TX_TIMERCLK          (cmuClock_TIMER1)
#define HIJACK_TX_GPIO_PORT         (gpioPortC)
#define HIJACK_TX_GPIO_PIN          13
#define HIJACK_TX_LOCATION			(TIMER_ROUTE_LOCATION_LOC0)


/* smbus definition. */
#define HIJACK_SMBUS_I2C	I2C0
#define HIJACK_SMBUS_GPIO_PORT	(gpioPortE)
#define HIJACK_SMBUS_GPIO_SDA	12
#define HIJACK_SMBUS_GPIO_SCL	13
#define HIJACK_SMBUS_LOCATION	(I2C_ROUTE_LOCATION_LOC6)


/* uv definition. */
#define HIJACK_UV_ADC ADC0
#define HIJACK_UV_REF adcRef1V25
#define HIJACK_UV_CH  adcSingleInpCh0
