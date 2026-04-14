/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 1.6   GM    03/06/26  Initial release.
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
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_UHBR10},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_UHBR10},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_UHBR10},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_UHBR135},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_UHBR135},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_UHBR135},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_UHBR20},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_UHBR20},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_UHBR20},
};

void Dp21RxSs_PtMst_TestPatternGenHelp()
{
	xil_printf("Choose Video pattern\r\n"
			   "1 -->  Vesa LLC pattern 			\r\n"
			   "3 -->  Vesa Color Squares			\r\n"
			   "4 -->  Flat Red  screen 			\r\n"
			   "5 -->  Flat Green screen 			\r\n"
			   "6 -->  Flat Blue screen 			\r\n"
			   "7 -->  Flat Purple screen 			\r\n"
			   "\r\n"
			   "Press 'x' to return to main menu\r\n"
			   "Press any key to show this menu again\r\n");
}

void Dp21RxSs_PtMst_SelectLinkLane(void)
{
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("--    Select the Link and Line Capabilities        --\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("Choose TX capability for Lane count and Link rate\r\n"
		   "0 --> Set TX capability @ 1.62G   1 lane\r\n"
		   "1 --> Set TX capability @ 1.62G   2 lanes\r\n"
		   "2 --> Set TX capability @ 1.62G   4 lanes\r\n"
		   "3 --> Set TX capability @  2.7G   1 lane\r\n"
		   "4 --> Set TX capability @  2.7G   2 lanes\r\n"
		   "5 --> Set TX capability @  2.7G   4 lanes\r\n"
		   "6 --> Set TX capability @  5.4G   1 lane\r\n"
		   "7 --> Set TX capability @  5.4G   2 lanes\r\n"
		   "8 --> Set TX capability @  5.4G   4 lanes\r\n"
		   "9 --> Set TX capability @  8.1G   1 lane\r\n"
		   "a --> Set TX capability @  8.1G   2 lanes\r\n"
		   "b --> Set TX capability @  8.1G   4 lanes\r\n"
		   "c --> Set TX capability @  10G    1 lanes\r\n"
		   "d --> Set TX capability @  10G    2 lanes\r\n"
		   "e --> Set TX capability @  10G    4 lanes\r\n"
		   "f --> Set TX capability @  13.5G  1 lanes\r\n"
		   "g --> Set TX capability @  13.5G  2 lanes\r\n"
		   "h --> Set TX capability @  13.5G  4 lanes\r\n"
		   "i --> Set TX capability @  20G    1 lanes\r\n"
		   "j --> Set TX capability @  20G    2 lanes\r\n"
		   "k --> Set TX capability @  20G    4 lanes\r\n");

	xil_printf("\r\n");
	xil_printf("Press 'x' to return to main menu\r\n");
	xil_printf("Press any key to display this menu\r\n");
	xil_printf("-----------------------------------------------------\r\n");
}

void Dp21RxSs_PtMst_BpcHelpMenu(int DPTXSS_BPC_int)
{
	xil_printf("Choose Video Bits per color option\r\n"
		   "1 -->  8 bpc (24bpp)\r\n");

	if (DPTXSS_BPC_int >= 10) {
		xil_printf("2 --> 10 bpc (30bpp)\r\n");

		if (DPTXSS_BPC_int >= 12) {
			xil_printf("3 --> 12 bpc (36bpp)\r\n");

			if (DPTXSS_BPC_int >= 16)
				xil_printf("4 --> 16 bpc (48bpp)\r\n");
		}
	}

	xil_printf("\r\n"
		   "Press 'x' to return to main menu\r\n"
		   "Press any key to display this menu again\r\n");
}

void Dp21RxSs_PtMst_ResolutionHelpMenu(void)
{
	xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - -"
		   " - - - - - - - - - -\r\n"
		   " -                           Select an Option for Resolution -\r\n"
		   "- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - "
		   "- - - - - \r\n"
		   "0 640x480_60_P    |   1 720x480_60_P      |   2 800x600_60_P  \r\n"
		   "3 1024x768_60_P   |   4 1280x720_60_P     |   5 1600x1200_60_P \r\n"
		   "6 1366x768_60_P   |   7 1920x1080_60_P    \r\n"
		   "- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - -"
		   "- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - -"
		   " - - - - - -\r\n"

		   "Press 'x' to return to main menu \r\n"
		   "Press any key to show this menu again\r\n");
}

void Dp21RxSs_PtMst_SubHelpMenu(void)
{
	xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - -\r\n"
		   "-      DisplayPort Rx Only/TX Only Demo Menu -\r\n"
		   "- Press 'z' to get this main menu at any point  -\r\n"
		   "- - - - - - - - - - - - - - - - - - - - - - - - -\r\n"
		   "1 - Change Resolution \n\r"
		   "2 - Change Number of Lanes, Link Rate \n\r"
		   "3 - Change Bits Per Color \n\r"
		   "4 - Change Pattern \n\r"
		   "5 - Display MSA Values for Tx, Rx\n\r"
		   "6 - Display Link Configuration Status and user selected resolution, BPC\n\r"
		   "7 - Display DPCD register Configurations\n\r"
		   "8 - Read Auxiliary registers \n\r"
		   "c = Check SUM on Rx and Tx\n\r"
		   "e - Display EDID values\n\r"
		   "z - Display this Menu again\r\n"
		   "- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
}

char Dp21RxSs_PtMst_InByteLocal(void);
u32 Dp21RxSs_PtMst_Gethex(u8 num_chars);

void Dp21RxSs_PtMst_OperationMenu(void)
{
	xil_printf("\n\n******************************************************\n\r"
		   "This system is capable of working as a PassThrough system,  \r\n"
		   "One MST stream can be chosen to be shown on TX.\r\n"
		   "This system can also works as independent RXO or TX,\r\n"
		   "In this mode, the RX streams are consumed by CRC modules\r\n"
		   "****************************************************\n\r");

	xil_printf("\n\n-----------------------------------------------------\n\r"
		   "--                       Menu                      --\n\r"
		   "-----------------------------------------------------\n\r"
		   "\n\r"
		   " Select option\n\r"
		   "p - Pass-through design\r\n"
		   "i - Independent 4 stream MST Rxo, Txo mode\r\n");
}

void Dp21RxSs_PtMst_LaneLinkRateHelpMenu(void)
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
		   "c --> train link @  10.0G 1 lanes\r\n"
		   "d --> train link @  10.0G 2 lanes\r\n"
		   "e --> train link @  10.0G 4 lanes\r\n"
		   "f --> train link @  13.5G 1 lanes\r\n"
		   "g --> train link @  13.5G 1 lanes\r\n"
		   "h --> train link @  13.5G 1 lanes\r\n"
		   "i --> train link @  20.0G 1 lanes\r\n"
		   "j --> train link @  20.0G 2 lanes\r\n"
		   "k --> train link @  20.0G 4 lanes\r\n"
		   "\r\n"
		   "Press 'x' to return to main menu\r\n"
		   "Press any key to display this menu again\r\n");
}

void Dp21RxSs_PtMst_PtHelpMenu()
{
	print("\n\r");
	print("-----------------------------------------------------\n\r");
	print("-- DisplayPort Pass-through (RX-TX) in MST mode Demo Menu --\n\r");
	print("-----------------------------------------------------\n\r");
	print("\n\r");
	print(" Select option\n\r");
	print(" a = Stream 1\n\r");
	print(" b = Stream 2\n\r");
	print(" c = Stream 3\n\r");
	print(" d = Stream 4\n\r");
	print(" 1 = Change Lane and Link capabilities\n\r");
	print(" 2 = Link, MSA and Error Status\n\r");
	print(" 3 = Toggle HPD to ask for Retraining\n\r");
	print(" 4 = Restart TX path \n\r");
	print(" m = Check SUM on Rx and Tx\n\r");
	print(" n = Clone EDID from Monitor\r\n");
	print(" z = Display this menu again\r\n");
	print(" x = Return to Main menu\r\n");
	print("\n\r");
	print("-----------------------------------------------------\n\r");
}

char Dp21RxSs_PtMst_InByteLocal(void)
{
	char c = 0;

	c = XUartLite_RecvByte(STDIN_BASEADDRESS);
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
*		- received character
*
* @note		None.
*
******************************************************************************/
u32 Dp21RxSs_PtMst_Gethex(u8 num_chars)
{
	u32 data;
	u32 i;
	u8 term_key;
	data = 0;

	for (i = 0; i < num_chars; i++) {
		term_key = Dp21RxSs_PtMst_Getc(0);
		xil_printf("%c",term_key);

		if (term_key >= 'a') {
			term_key = term_key - 'a' + 10;
		} else if (term_key >= 'A') {
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
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received character
*
* @note		None.
*
******************************************************************************/
char Dp21RxSs_PtMst_Getc(u32 timeout_ms)
{
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	/*
	 * Reset and start timer
	 */
	if ( timeout_ms > 0 && timeout_ms != 0xff ) {
		XTmrCtr_Start(&TmrCtr, 0);
	}

#ifndef PLATFORM_MB
	while ((!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) && (timeout == 0)) {
#else
	while (XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS) && (timeout == 0)) {
#endif

		if (timeout_ms == 0) { /**< No timeout - wait for ever */
			timeout = 0;
		} else if ( timeout_ms == 0xff ) { /**< No wait - special case */
			timeout = 1;
		} else if (timeout_ms > 0) {
			if (XTmrCtr_GetValue(&TmrCtr, 0) > ( timeout_ms * (100000000 / 1000) )) {
				timeout = 1;
			}
		}
	}

	if (timeout == 1) {
		c = 0;
	} else {
#ifndef PLATFORM_MB
		c = XUartPs_RecvByte_NonBlocking();
#else
		c = XUartLite_RecvByte(STDIN_BASEADDRESS);
#endif
	}
	return c;
}
