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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
#include "tx.h"

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
	"\n\n-----------------------------------------------------\n\r"
	"--                       Menu                      --\n\r"
	"-----------------------------------------------------\n\r"
	"\n\r"
	" Select option\n\r"
	"t - Tx Only design\r\n"
	"r - Rx Only design\r\n"
	"p - Pass-through design\r\n"
	"l - LoopBack design\r\n"
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



void bpc_help_menu(int DPTXSS_BPC_int)
{
	xil_printf("Choose Video Bits per color option\r\n"
			"1 -->  8 bpc (24bpp)\r\n");
	if (DPTXSS_BPC_int >= 10){
		xil_printf("2 --> 10 bpc (30bpp)\r\n");

		if (DPTXSS_BPC_int >= 12){
			xil_printf("3 --> 12 bpc (36bpp)\r\n");

			if (DPTXSS_BPC_int >= 16)
				xil_printf("4 --> 16 bpc (48bpp)\r\n");
		}
	}
	xil_printf("\r\n"
		   "Press 'x' to return to main menu\r\n"
		   "Press any key to display this menu again\r\n");
}

void format_help_menu(void)
{
	xil_printf("Choose Video Format \r\n"
		   "For YCbCr - Only Color Square Pattern is Supported \r\n"
		   "1 -->  RGB 			\r\n"
		   "2 -->  YCbCR444		\r\n"
		   "3 -->  YCbCr422		\r\n"
		   "\r\n"
		   "Press 'x' to return to main menu\r\n"
		   "Press any key to show this menu again\r\n");
}

void resolution_help_menu(void)
{
xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - -"
		" - - - - - - - - - -\r\n"
"-                            Select an Option for Resolutio"
		"n                                      -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - "
		"- - - - - \r\n"
"0 640x480_60_P    |   1 720x480_60_P      |   2 800x600_60_P  \r\n"
"3 1024x768_60_P   |   4 1280x720_60_P     |   5 1600x1200_60_P        \r\n"
"6 1366x768_60_P   |   7 1920x1080_60_P    |   8 3840x2160_30_P\r\n"
"9 3840x2160_60_P  |   a 2560x1600_60_P    |   b 1280x1024_60_P\r\n"
"c 1792x1344_60_P  |   d 848x480_60_P      |   e 1280x960\r\n"
"f 1920x1440_60_P  |   i 3840x2160_60_P_RB |   j 3840x2160_120_P_RB\r\n"
	"k 7680x4320_24_P  |   l 7680x4320_30_P    |   m 3840x2160_100_P\r\n"
	"n 7680x4320_30DELL|   o 5120x2880_30      |   p 7680x4320_30_MSTR\r\n"
	"q 5120x2880_MSTR  |   r 3840x2160_120_MSTR\r\n"
"- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - -"
"- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - -"
" - - - - - -\r\n"

"Press 'x' to return to main menu \r\n"
"Press any key to show this menu again\r\n");
}

void test_pattern_gen_help()
{
	xil_printf("Choose Video pattern\r\n"
		   "1 -->  Vesa LLC pattern 			\r\n"
		   "3 -->  Vesa Color Squares			\r\n"
		   "4 -->  Flat Red  screen 			\r\n"
		   "5 -->  Flat Blue screen 			\r\n"
		   "6 -->  Flat Green screen 			\r\n"
		   "7 -->  Flat Purple screen 			\r\n"
		   "\r\n"
		   "Press 'x' to return to main menu\r\n"
		   "Press any key to show this menu again\r\n");
}

void app_help()
{
	xil_printf("\n\n-----------------------------------------------------\r\n");
	xil_printf("--                       Menu                      --\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("\r\n");
	xil_printf(" Select option\r\n");
	xil_printf(" t = Activate Tx Only path (TX uses QPLL) \r\n");
	xil_printf("\r\n");
	xil_printf("-----------------------------------------------------\r\n");
}

void sub_help_menu(void)
{
	xil_printf (
	"- - - - -  -  - - - - - - - - - - - - - - - - - -\r\n"
	"-          DisplayPort TX Only Demo Menu        -\r\n"
	"- Press 'z' to get this main menu at any point  -\r\n"
	"- - - - - - - - - - - - - - - - - - - - - - - - -\r\n"
	"1 - Change Resolution \r\n"
	"2 - Change Bits Per Color \r\n"
	"3 - Change Number of Lanes, Link Rate \r\n"
	"4 - Change Pattern \r\n"
	"5 - Display MSA Values for Tx\r\n"
	"6 - Change Format \r\n"
	"7 - Display Link Configuration Status and user selected resolution\r\n"
	"8 - Display DPCD register Configurations\r\n"
	"9 - Read Auxiliary registers \r\n"
	"a - Enable/Disable Audio\r\n"
	"d - Power Up/Down sink\r\n"
	"e - Read EDID from sink\r\n"
	"m - Read CRC checker value\r\n"
	"z - Display this Menu again\r\n"
	"- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
}



void select_link_lane(void)
{
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("--    Select the Link and Line Capabilities        --\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("Choose TX capability for Lane count and Link rate\r\n"
		   "0 --> Set TX capability @ 1.62G 1 lane\r\n"
		   "1 --> Set TX capability @ 1.62G 2 lanes\r\n"
		   "2 --> Set TX capability @ 1.62G 4 lanes\r\n"
		   "3 --> Set TX capability @  2.7G 1 lane\r\n"
		   "4 --> Set TX capability @  2.7G 2 lanes\r\n"
		   "5 --> Set TX capability @  2.7G 4 lanes\r\n"
		   "6 --> Set TX capability @  5.4G 1 lane\r\n"
		   "7 --> Set TX capability @  5.4G 2 lanes\r\n"
		   "8 --> Set TX capability @  5.4G 4 lanes\r\n"
		   "9 --> Set TX capability @  8.1G 1 lane\r\n"
		   "a --> Set TX capability @  8.1G 2 lanes\r\n"
		   "b --> Set TX capability @  8.1G 4 lanes\r\n");
	xil_printf("\r\n");
	xil_printf("Press 'x' to return to main menu\r\n");
	xil_printf("Press any key to display this menu\r\n");
	xil_printf("-----------------------------------------------------\r\n");
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
	print(" d = Quad selection    ONLY FOR 8K demo\n\r");
	print(" w = Sink register write\n\r");
	print(" r = Sink register read\n\r");
	print(" n = toggle EDID setting between 8K to 4K120\r\n");
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
	  print("-----------------------------------------------------\n\r");
	  print("--    Select the Quad        --\n\r");
	  print("-----------------------------------------------------\n\r");
	  xil_printf(
           "0 --> Set left upper\n\r"
           "1 --> Set right upper\n\r"
           "2 --> Set left bottom\n\r"
           "3 --> Set right bottom\n\r");
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
