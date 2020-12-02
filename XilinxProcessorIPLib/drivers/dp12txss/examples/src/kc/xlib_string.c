/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/




#include "stdio.h"
#include "xlib_string.h"
#include "xuartlite_l.h"
#include "xparameters.h"
#include "xtmrctr.h"

char xil_getc(u32 timeout_ms){
	char c;
	u32 timeout = 0;

	  extern XTmrCtr TmrCtr;

	  //dbg_printf ("timeout_ms = %x\n\r",timeout_ms);
	    // Reset and start timer
	  if ( timeout_ms > 0 && timeout_ms != 0xff ){
		  XTmrCtr_Start(&TmrCtr, 0);
		  //dbg_printf ("timeout_ms = %x\n\r",timeout_ms);
	  }
		while(XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS) && (timeout == 0)){
		    if ( timeout_ms == 0 ){ // no timeout - wait for ever
			   timeout = 0;
		    } else if ( timeout_ms == 0xff ) { // no wait - special case
			   timeout = 1;
		    } else if(timeout_ms > 0){
				if(XTmrCtr_GetValue(&TmrCtr, 0) >
				  (timeout_ms * (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000))){
					timeout = 1;
				}
		    }
		}
		if(timeout == 1){
			c = 0;
		} else {
			c = XUartLite_RecvByte(STDIN_BASEADDRESS);
		}

//		c = XUartLite_RecvByte(STDIN_BASEADDRESS);


		return c;
}


u32 xil_gethex(u8 num_chars){
u32 data;
u32 i;
u8 term_key;
data = 0;

for(i=0;i<num_chars;i++){
	term_key = xil_getc(0);
	dbg_printf("%c",term_key);
	if(term_key >= 'a') {
		term_key = term_key - 'a' + 10;
	} else if(term_key >= 'A') {
			term_key = term_key - 'A' + 10;
	} else {
		term_key = term_key - '0';
	}
	data = (data << 4) + term_key;
}


return data;
}

void do_nothing(){

}
