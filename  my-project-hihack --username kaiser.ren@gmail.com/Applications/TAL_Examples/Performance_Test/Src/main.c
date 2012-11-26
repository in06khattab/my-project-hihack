/**
 * @file main.c
 *
 * @brief  Main of TAL Example - Performance_Test
 *
 * $Id: main.c 22695 2010-07-27 13:13:57Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */
#include "hal.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */
#define VER_MAJOR	0
#define VER_MINOR 3



/* === PROTOTYPES ========================================================== */
static void print_main_menu(void);
static void get_analog_cmp_freq(void);
void data_moduler(uint8_t data);

/* === GLOBALS ============================================================= */
#if (PAL_TYPE==ATMEGA1281)
__root __farflash  	uint32_t		Bf_Appflag 	@0x1DFFC = 0xa5a55a5al;
#endif

/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Main function of the Performance_Test application
 */
int main(void)
{
	/* initial hal layer. */
   hal_init();
	
	printf("\r\n-- Welcome to ATmega Test V%d.%02d --\r\n", VER_MAJOR, VER_MINOR);
	printf("-- Build on: "__TIME__" @"__DATE__"\r\n");
	printf("-- F_CPU: %lu\r\n", F_CPU);

	/* Endless while loop */
   while (1)
   {
		print_main_menu();
   }
}

/*
 * @brief print main menu.
*/
static void print_main_menu(void)
{
  	uint8_t input;
	//uint8_t sreg;
	
	/* Get input from terminal program / user. */
 	input = sio_getchar();
   input = toupper(input);

   /* Handle input from terminal program. */
   switch (input)
   {	
  		case 'F':
       	get_analog_cmp_freq();
         break;
        //
		case 'C':
	  		pwm_uninit();
		  	break;
			//
		case 'E':
		  	pwm_set_freq(PWM_TMR0_CMP_OUT_FREQ);
			break;
			//
		case '1':
		  	pwm_set_freq(1000);
			break;
			//	
		case '2':
		  	pwm_set_freq(2000);
			break;
			//
		case 'M':
	  		data_moduler(0x5a);
			break;
			//
		default:
	  		break;
			//
	}
	/* Restore global interrupt flag */
	//SREG = sreg;
}

void data_moduler(uint8_t data)
{
  	int8_t i;
	
	pwm_para.data = data;
	for(i = 7; i >= 0; i--){
	  	pwm_set_freq( ( (pwm_para.data >> i) & 0x01) * 1000 + 1000 );
	  	while(pwm_para.toggle_cnt);
	}
}

/**
 * @brief Sub-menu to get frequency.
 */
static void get_analog_cmp_freq(void)
{
  	uint16_t freq;
	
	freq = 1000000ul/ac_cap_para.interval/2;
	printf("\r\nFreq:%u\r\n", freq);
}


/**
 * @brief Sub-menu to print sensor data
 */
static void get_sensor_data(void)
{
    //uint16_t bat_mon;

    //bat_mon = tfa_get_batmon_voltage();
    //printf("\r\nBattery monitor: U = %" PRIu16 ".%" PRIu16 " V\r\n", bat_mon/1000, bat_mon%1000);
}



/* EOF */
