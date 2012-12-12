                  /**
 * @file hal.h
 *
 * @brief hal board specific functionality
 *
 * This file implements hal board specific functionality.
 *
 * @author    kren
 * @data		Nov 21, 2012
 */
#ifndef _HAL_H_
#define _HAL_H_

/*--------------------------------------------------------------
 * 			Headers
 *--------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "decode.h"
#include "encode.h"
#include "pal.h"
#include "app_config.h"
#include "sio_handler.h"
#include "pal_uart.h"
#include "timer.h"

/*--------------------------------------------------------------
 * 			Export Functions
 *--------------------------------------------------------------*/
void hal_init(void);

#endif 	//_HAL_H_
//eof
