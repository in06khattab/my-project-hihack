/**
 * @file timer.h
 *
 * @brief hal board specific functionality
 *
 * This file implements hal board specific functionality.
 *
 * @author    kren
 * @data		Nov 21, 2012
 */


/* === Includes ============================================================ */

/* === Types =============================================================== */
typedef struct ac_cap_tag
{
  	uint8_t occur;
	uint16_t interval;
	uint16_t cur_stamp;
}ac_cap_t;

/* === EXTERNALS =========================================================== */
extern ac_cap_t ac_cap_para;

/* === PROTOTYPES ========================================================== */
void ac_init(void);
void pwm_init(void);
//eof

