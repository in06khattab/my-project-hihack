/* ----------------------------------------------------------------------------
 *         MODULAR SOURCE CODE
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, kren
 */

/**
 * \file modular.c
 *
 * Implements machester modulation.
 *
 */

#include "modular.h"

modulate_t mod;

void findParam(uint8_t bit_msk)
{
  	if( ( mod.data & ( 1 << bit_msk) ) && (SET == mod.cur) ){
  		mod.factor = Div1;
		if( 50 == index_sample )
		  	mod.reverse = 1;
	}
	else if( ( mod.data & ( 1 << bit_msk) ) && (CLR == mod.cur) ){
		mod.factor = Div2;
		mod.cur = SET;
	}
	else if( !( mod.data & ( 1 << bit_msk) ) && (SET == mod.cur) ){
		mod.factor = Div2;
		mod.cur = CLR;
	}
	else if( !( mod.data & ( 1 << bit_msk) ) && (CLR == mod.cur) ){
		mod.factor = Div1;
		if( 50 == index_sample )
		  	mod.reverse = 1;
	}
}

void state_switch(void)
{
  	mod_state_t sta = mod.state;
	
	switch(sta){
		case Waiting:
			if(us1_get_count() ){ 	//prepare for sta0
				mod.state = Sta0;
				mod.factor = Div2;
				mod.data = us1_get_char();
				index_sample = 0;
			}
			break;
			//
		case Sta0: 	//prepare for sta1
	  		mod.state = Sta1;
			break;
			//
		case Sta1:	//prepare for sta2
		  	mod.state = Sta2;
			break;
			//
	  	case Sta2:	//prepare for sta3
		  	mod.state = Sta3;
			break;
			//
		case Sta3:	//prepare for bit7, sta3 is SET
		  	if( mod.data & (1 << BIT7) ){
		  		mod.factor = Div1;
				mod.cur = SET;
				if( 50 == index_sample )
		  			mod.reverse = 1;
		  	}
			else{
				mod.factor = Div2;
				mod.cur = CLR;	
			}
		  	mod.state = Bit7;
			break;
			//	
		case Bit7: 	//prepare for Bit6
		  	findParam(BIT6);
			mod.state = Bit6;
	    	break;
			//
		case Bit6: 	//prepare for Bit5
		  	findParam(BIT5);
			mod.state = Bit5;
	    	break;
			//
		case Bit5: 	//prepare for Bit4
		  	findParam(BIT4);
			mod.state = Bit4;
	    	break;
			//
		case Bit4: 	//prepare for Bit3
		  	findParam(BIT3);
			mod.state = Bit3;
	    	break;
			//
		case Bit3: 	//prepare for Bit2
		  	findParam(BIT2);
			mod.state = Bit2;
	    	break;
			//
		case Bit2: 	//prepare for Bit1
		  	findParam(BIT1);
			mod.state = Bit1;
	    	break;
			//
		case Bit1: 	//prepare for Bit0
		  	findParam(BIT0);
			mod.state = Bit0;
	    	break;
			//
		case Bit0: 	//new byte is exist?
		  	mod.state = Sto0; 	//to stop state 0.
								//output bit is one.
			if(SET == mod.cur){
				mod.factor = Div1;
				if( 50 == index_sample )
		  			mod.reverse = 1;
			}
			else{
				mod.factor = Div2;
			}
			mod.cur = SET;
	    	break;
			//

		case Sto0:     //prepare for sto1
			mod.state = Waiting;
			mod.factor = Div1;
			mod.cur = SET;
			break;
			//
#if 0
		case Sto1:		//go to waiting mode
			mod.state = Waiting;
			break;
			//
#endif
		default:
	  		break;
	  		//
	}
}


//end of file
