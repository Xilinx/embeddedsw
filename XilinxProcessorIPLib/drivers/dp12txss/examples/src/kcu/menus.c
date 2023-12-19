/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file menus.c
*
* This file contains functions to configure Video Pattern Generator core.
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


#include "dppt.h"



void DpPt_LaneLinkRateHelpMenu(void)
{
	xil_printf("Choose test option for Lane count and Link rate change\n\r"
	"0 --> train link @ 1.62G 1 lane\n\r"
	"1 --> train link @ 1.62G 2 lanes\n\r"
	"2 --> train link @ 1.62G 4 lanes\n\r"
	"3 --> train link @  2.7G 1 lane\n\r"
	"4 --> train link @  2.7G 2 lanes\n\r"
	"5 --> train link @  2.7G 4 lanes\n\r"
	"6 --> train link @  5.4G 1 lane\n\r"
	"7 --> train link @  5.4G 2 lanes\n\r"
	"8 --> train link @  5.4G 4 lanes\n\r"
	"\n\r"
	"Press 'x' to return to main menu\n\r"
	"Press any key to display this menu again\n\r"

	);
}



void bpc_help_menu(void)
{
xil_printf("Choose Video Bits per color option\n\r"
			"1 -->  8 bpc (24bpp)\n\r"
			"2 --> 10 bpc (30bpp)\n\r"
//			"3 --> 12 bpc (36bpp)\n\r"
//			"4 --> 16 bpc (48bpp)\n\r"
			"\n\r"
			"Press 'x' to return to main menu\n\r"
			"Press any key to display this menu again\n\r"
);
}

void resolution_help_menu(void)
{
xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - - "
		"- - - - - - - - - -\n\r"
"-                            Select an Option for Resolutio"
"n                                      -\n\r"
"- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - -"
"- - - - \n\r"
"0 640x480_60_P    |   1 720x480_60_P      |   2 800x600_60_P  \r\n"
"3 1024x768_60_P   |   4 1280x720_60_P     |   5 1600x1200_60_P \r\n"
"6 1366x768_60_P   |   7 1920x1080_60_P    |   8 3840x2160_30_P\r\n"
"9 3840x2160_60_P  |   a 2560x1600_60_P    |   b 1280x1024_60_P\r\n"
"c 1792x1344_60_P  |   d 848x480_60_P      |   e 1280x960\r\n"
"f 1920x1440_60_P   \r\n"
"- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - -"
"- - - - - -\n\r"

"Press 'x' to return to main menu \n\r"
"Press any key to show this menu again\n\r");
}

void test_pattern_gen_help()
{
	xil_printf("Choose Video pattern\n\r"
			   "1 -->  Vesa LLC pattern 			\n\r"
			   "2 -->  Black & White Vertical Lines \n\r"
			   "3 -->  Vesa Color Squares			\n\r"
			   "4 -->  Flat Red  screen 			\n\r"
			   "5 -->  Flat Green screen 			\n\r"
			   "6 -->  Flat Blue screen 			\n\r"
			   "7 -->  Flat Yellow screen 			\n\r"
			   "\r\n"
			   "Press 'x' to return to main menu\n\r"
			   "Press any key to show this menu again\n\r");
}

void app_help()
 {
	print("\n\n-----------------------------------------------------\n\r");
	print("--                       Menu                      --\n\r");
	print("-----------------------------------------------------\n\r");
	print("\n\r");
	print(" Select option\n\r");
#if !BUFFER_BYPASS
	  print(" r = Activate Rx-Tx passthrough (RX, TX use CPLL) \n\r");
#endif
	print(" s = Activate Rx-Tx passthrough (RX uses CPLL, TX use QPLL)  \n\r");
	print(" t = Activate Tx Only path (TX uses QPLL) \n\r");
	print("\n\r");
	print("-----------------------------------------------------\n\r");
 }

void sub_help_menu(void)
{
  xil_printf(
	 "- - - - -  -  - - - - - - - - - - - - - - - - - -\n\r"
	 "-          DisplayPort TX Only Demo Menu        -\n\r"
	 "- Press 'z' to get this main menu at any point  -\n\r"
	 "- - - - - - - - - - - - - - - - - - - - - - - - -\n\r"
	 "1 - Change Resolution \n\r"
	 "2 - Change Bits Per Color \n\r"
	 "3 - Change Number of Lanes, Link Rate \n\r"
	 "4 - Change Pattern \n\r"
	 "5 - Display MSA Values for Tx\n\r"
	 "6 - Display EDID values\n\r"
	 "7 - Display Link Configuration Status and user selected "
			  "resolution, BPC\n\r"
	 "8 - Display DPCD register Configurations\n\r"
	 "9 - Read Auxiliary registers \n\r"
	 "a - Enable/Disable Audio\r\n"
#if ENABLE_HDCP_IN_DESIGN
	 "i - Start/Stop TX HDCP\r\n"
	 "p - Displays the TX HDCP Debug data\n\r"
#endif
	 "z - Display this Menu again\r\n"
	 "x - Main menu \r\n\r\n"
	 "- - - - - - - - - - - - - - - - - - - - - - - - - \n\r");
}

void rx_help_menu()
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
//	  print(" 5 = Enable/disable DTG\n\r");
//	  print(" 6 = Re-initialize VDMA\n\r");
	  print(" 5 = Switch TX data to internal pattern generator\n\r");
	  print(" 6 = Switch TX back to RX video data\n\r");
	  print(" c = Display CRC value\n\r");
//	  print(" 9 = Report RX, TX Frame stats\n\r");
	  print(" w = Sink register write\n\r");
	  print(" r = Sink register read\n\r");
#if ENABLE_HDCP_IN_DESIGN
	  print(" p = RX & TX HDCP debug info\r\n");
#endif
	  print(" z = Display this menu again\r\n");
	  print(" x = Return to Main menu\r\n");
	  print("\n\r");
	  print("-----------------------------------------------------\n\r");

#if COMPLIANCE
	  print("-----------------------------------------------------\n\r");
	  print("To manually start the video press 'x' after training \n\r");
	  print("is done and then press 'm'                           \n\r");
	  print("-----------------------------------------------------\n\r");
#endif
 }

void select_rx_link_lane(void)
 {
	  print("-----------------------------------------------------\n\r");
	  print("--    Select the Link and Line Capabilities        --\n\r");
	  print("-----------------------------------------------------\n\r");
	  xil_printf("Choose RX capability for Lane count and Link rate\n\r"
           "0 --> Set RX capability @ 1.62G 1 lane\n\r"
           "1 --> Set RX capability @ 1.62G 2 lanes\n\r"
           "2 --> Set RX capability @ 1.62G 4 lanes\n\r"
           "3 --> Set RX capability @  2.7G 1 lane\n\r"
           "4 --> Set RX capability @  2.7G 2 lanes\n\r"
           "5 --> Set RX capability @  2.7G 4 lanes\n\r"
           "6 --> Set RX capability @  5.4G 1 lane\n\r"
           "7 --> Set RX capability @  5.4G 2 lanes\n\r"
           "8 --> Set RX capability @  5.4G 4 lanes\n\r");
	  print("\n\r");
	  print("-----------------------------------------------------\n\r");
 }


void select_link_lane(void)
 {

	  print("-----------------------------------------------------\n\r");
	  print("--    Select the Link and Line Capabilities        --\n\r");
	  print("-----------------------------------------------------\n\r");
	  xil_printf("Choose TX capability for Lane count and Link rate\n\r"
	   "0 --> Set TX capability @ 1.62G 1 lane\n\r"
	   "1 --> Set TX capability @ 1.62G 2 lanes\n\r"
	   "2 --> Set TX capability @ 1.62G 4 lanes\n\r"
	   "3 --> Set TX capability @  2.7G 1 lane\n\r"
	   "4 --> Set TX capability @  2.7G 2 lanes\n\r"
	   "5 --> Set TX capability @  2.7G 4 lanes\n\r"
	   "6 --> Set TX capability @  5.4G 1 lane\n\r"
	   "7 --> Set TX capability @  5.4G 2 lanes\n\r"
	   "8 --> Set TX capability @  5.4G 4 lanes\n\r");
	  print("\n\r");
	  print("Press 'x' to return to main menu\n\r");
	  print("Press any key to display this menu\n\r");
	  print("-----------------------------------------------------\n\r");
 }
