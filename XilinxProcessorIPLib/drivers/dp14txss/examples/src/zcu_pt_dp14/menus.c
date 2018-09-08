/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file menus.c
*
* This file contains application menu information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI   07/13/17 Initial release.
* </pre>
*
******************************************************************************/
#include "main.h"

lane_link_rate_struct lane_link_table[]=
{
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_810GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_810GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_810GBPS},

};

extern XVidC_VideoMode resolution_table[];

char inbyte_local(void);
u32 xil_gethex(u8 num_chars);

void operationMenu(void){
	xil_printf(
	"\n\n****************************************************\n\r"
	"This system is purely a PassThrough system designed to  \r\n"
	"display the video that is received on the RX.\r\n"
	"The TX is non functional in absence of active RX link\r\n"
	"Do not change the Monitor once the application is in run mode\r\n"
	"This system can be used for DisplayPort Sink Compliance\r\n"
	"****************************************************\n\r"
			);

	xil_printf(
	"\n\n-----------------------------------------------------\n\r"
	"--                       Menu                      --\n\r"
	"-----------------------------------------------------\n\r"
	"\n\r"
	" Select option\n\r"
	"p - Pass-through design\r\n"
	);

}

void DpPt_LaneLinkRateHelpMenu(void)
{
	xil_printf("Choose test option for Lane count and Link rate change\r\n"
	"0 --> train link @ 1.62G 1 lane\r\n"
	"1 --> train link @ 1.62G 2 lanes\r\n"
	"2 --> train link @ 1.62G 4 lanes\r\n"
	"3 --> train link @  2.7G 1 lane\r\n"
	"4 --> train link @  2.7G 2 lanes\r\n"
	"5 --> train link @  2.7G 4 lanes\r\n"
	"6 --> train link @  5.4G 1 lane\r\n"
	"7 --> train link @  5.4G 2 lanes\r\n"
	"8 --> train link @  5.4G 4 lanes\r\n"
	"9 --> train link @  8.1G 1 lane\r\n"
	"a --> train link @  8.1G 2 lanes\r\n"
	"b --> train link @  8.1G 4 lanes\r\n"
	"\r\n"
	"Press 'x' to return to main menu\r\n"
	"Press any key to display this menu again\r\n"

	);
}

void pt_help_menu()
{
	print("\n\r");
	print("-----------------------------------------------------\n\r");
	print("--           DisplayPort RX-TX Demo Menu           --\n\r");
	print("-----------------------------------------------------\n\r");
	print("\n\r");
	print(" Select option\n\r");
	print(" 1 = Change Lane and Link capabilities\n\r");
	print(" 2 = Link, MSA and Error Status\n\r");
	print(" 3 = Toggle HPD to ask for Retraining\n\r");
	print(" 4 = Restart TX path \n\r");
	//print(" 5 = Switch TX data to internal pattern generator\n\r");
	//print(" 6 = Switch TX back to RX video data\n\r");
	//print(" 9 = Report RX, TX Frame stats\n\r");
	print(" c = Check SUM on Rx and Tx\n\r");
	print(" d = Quad selection ONLY FOR 8K --> 4K demo\n\r");
	print(" w = Sink register write\n\r");
	print(" r = Sink register read\n\r");
	print(" n = Clone EDID from Monitor\r\n");
	print(" m = Display MCDP6000 stauts\n\r");
	print(" u - Read from MCDP6000\r\n");
	print(" o - Write to MCDP6000\r\n");
	print(" q - EDID pass-through setting\r\n");
	print(" z = Display this menu again\r\n");
	print(" x = Return to Main menu\r\n");
	print("\n\r");
	print("-----------------------------------------------------\n\r");
}

void select_rx_quad(void)
 {
	  print("****  Ensure that the Monitor supports 4K@30  *******\n\r");
	  print("-----------------------------------------------------\n\r");
	  print("--    Select the Quad        --\n\r");
	  print("-----------------------------------------------------\n\r");
	  xil_printf(
           "0 --> Set left upper\n\r"
           "1 --> Set right upper\n\r"
           "2 --> Set left bottom\n\r"
           "3 --> Set right bottom\n\r"
	       "x --> Exit\r\n");
	  print("\n\r");
	  print("-----------------------------------------------------\n\r");
 }



char inbyte_local(void){
	char c=0;
	c = XUartPs_RecvByte_NonBlocking();
	return c;
}


/*****************************************************************************/
/**
*
* This function to convert integer to hex
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
u32 xil_gethex(u8 num_chars){
	u32 data;
	u32 i;
	u8 term_key;
	data = 0;

	for(i=0;i<num_chars;i++){
		term_key = xil_getc(0);
		xil_printf("%c",term_key);
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


/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
//static char XUartPs_RecvByte_NonBlocking(){
//    u32 RecievedByte;
//    RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
//    /* Return the byte received */
//    return (u8)RecievedByte;
//}



/*****************************************************************************/
/**
*
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
char xil_getc(u32 timeout_ms){
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	// Reset and start timer
	if ( timeout_ms > 0 && timeout_ms != 0xff ){
	  XTmrCtr_Start(&TmrCtr, 0);
	}


	while((!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) && (timeout == 0)){
		if ( timeout_ms == 0 ){ // no timeout - wait for ever
		   timeout = 0;
		} else if ( timeout_ms == 0xff ) { // no wait - special case
		   timeout = 1;
		} else if(timeout_ms > 0){
			if(XTmrCtr_GetValue(&TmrCtr, 0)
										> ( timeout_ms * (100000000 / 1000) )){
				timeout = 1;
			}
		}
	}
	if(timeout == 1){
		c = 0;
	} else {
		c = XUartPs_RecvByte_NonBlocking();
	}
	return c;
}
