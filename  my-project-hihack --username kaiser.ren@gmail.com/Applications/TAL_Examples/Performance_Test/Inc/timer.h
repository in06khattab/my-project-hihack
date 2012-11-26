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

/* === MACROS ============================================================== */
//MCK:4MHz, Pre Scale: 62.5KHz
#define	PWM_TMR0_CLK_SRC_PRE_SCALE  (64ul)
#define	PWM_TMR0_CLK_SRC_BIT_DEF ( _BV(CS01) | _BV(CS00) )
#define 	PWM_TMR0_CMP_OUT_FREQ (500ul)
#define 	PWM_TMR0_CMP_OUT_OCRA(freq, cpu, prescale) (cpu/freq/prescale/2)

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
void pwm_init(uint16_t freq);
void pwm_uninit(void);
//eof

