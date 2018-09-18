/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*
******************************************************************************/
#include "xdptxss_zcu102_tx.h"


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

//extern XVidC_VideoMode resolution_table[];
// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB ,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
	XVIDC_VM_7680x4320_25_P,
	XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,

	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR,
    XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

// CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
					(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
    { XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz (RB)", XVIDC_FR_60HZ,
        {1920, 48, 32, 80, 2080, 1,
		1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}},
    { XVIDC_B_TIMING3_60_P_RB, "2560x1440@60Hz (RB)", XVIDC_FR_60HZ,
         {2560, 48, 32, 80, 2720, 1,
		1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0}},
	{ XVIDC_VM_3840x2160_120_P_RB, "3840x2160@120Hz (RB)", XVIDC_FR_120HZ,
		{3840, 8, 32, 40, 3920, 1,
		2160, 113, 8, 6, 2287, 0, 0, 0, 0, 1} },

	{ XVIDC_VM_7680x4320_24_P, "7680x4320@24Hz", XVIDC_FR_24HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_25_P, "7680x4320@25Hz", XVIDC_FR_25HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_P, "7680x4320@30Hz", XVIDC_FR_30HZ,
		{7680, 8, 32, 40, 7760, 0,
		4320, 47, 8, 6, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_100_P_RB2, "3840x2160@100Hz (RB2)", XVIDC_FR_100HZ,
		{3840, 8, 32, 40, 3920, 0,
		2160, 91, 8, 6, 2265, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
		{7680, 48, 32, 80, 7840, 0,
		4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_P_RB2, "5120x2880@60Hz (RB2)", XVIDC_FR_60HZ,
		{5120, 8, 32, 40, 5200, 0,
		2880, 68, 8, 6, 2962, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_MSTR, "7680x4320_MSTR@30Hz", XVIDC_FR_30HZ,
		{7680, 25, 97, 239, 8041, 0,
		4320, 48, 9, 5, 4382, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_MSTR, "5120x2880@60Hz_MSTR", XVIDC_FR_60HZ,
		{5120, 25, 97, 239, 5481, 0,
		2880, 48, 9, 5, 2942, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_120_MSTR, "3840x2160@120Hz_MSTR", XVIDC_FR_120HZ,
		{3840, 48, 34, 79, 4001, 1,
		2160, 4, 6, 53, 2223, 0, 0, 0, 0, 1}},
};

XVidC_VideoMode resolution_table[] =
{
                XVIDC_VM_640x480_60_P,
                XVIDC_VM_480_60_P,
                XVIDC_VM_800x600_60_P,
                XVIDC_VM_1024x768_60_P,
                XVIDC_VM_720_60_P,
                XVIDC_VM_1600x1200_60_P,
                XVIDC_VM_1366x768_60_P,
                XVIDC_VM_1080_60_P,
                XVIDC_VM_UHD_30_P,
                XVIDC_VM_UHD_60_P,
                XVIDC_VM_2560x1600_60_P,
                XVIDC_VM_1280x1024_60_P,
                XVIDC_VM_1792x1344_60_P,
                XVIDC_VM_848x480_60_P,
                XVIDC_VM_1280x960_60_P,
                XVIDC_VM_1920x1440_60_P,
                XVIDC_VM_USE_EDID_PREFERRED,

				XVIDC_VM_1920x1080_60_P_RB,
				XVIDC_VM_3840x2160_60_P_RB,
				XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
				XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,
	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR

};


static char inbyte_local(void);

static char XUartPs_RecvByte_NonBlocking(void);
extern void Dprx_DetectResolution(void *InstancePtr, u16 offset);
extern void Dprx_ResetVideoOutput(void *InstancePtr);
extern XDpTxSs_MainStreamAttributes Msa[4];
extern void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);

void start_tx_after_rx( u8 stream_id);
void start_audio_passThrough(u8 LineRate_init_tx);
XilAudioInfoFrame *xilInfoFrame;

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

void audio_stream_help()
{
              xil_printf("Choose Audio Stream\n\r"
                                         "1 -->  Send Audio in Stream 1 \n\r"
                                         "2 -->  Send Audio in Stream 2 \n\r"
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
	xil_printf(
			"\r\n"
			"Press 'x' to return to main menu\r\n"
			"Press any key to display this menu again\r\n"
	);
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
#if 0
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
#endif

xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - -"
		" - - - - - - - - - -\r\n"
"-                            Select an Option for Resolutio"
		"n                                      -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - "
		"- - - - - \r\n"
"0 640x480_60_P    |   1 720x480_60_P      |   2 800x600_60_P  \r\n"
"3 1024x768_60_P   |   4 1280x720_60_P     |   5 1600x1200_60_P        \r\n"
"6 1366x768_60_P   |   7 1920x1080_60_P    \r\n"
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
#if 0
  xil_printf(
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
"7 - Display Link Configuration Status and user selected resolution, BPC\r\n"
"8 - Display DPCD register Configurations\r\n"
"9 - Read Auxiliary registers \r\n"
"a - Enable/Disable Audio\r\n"
"d - Power Up/Down sink\r\n"
"e - Read EDID from sink\r\n"
"m - Read CRC checker value\r\n"
"z - Display this Menu again\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
#endif

  xil_printf(
"- - - - -  -  - - - - - - - - - - - - - - - - - -\r\n"
"-          DisplayPort TX Only Demo Menu        -\r\n"
"- Press 'z' to get this main menu at any point  -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - -\r\n"

   "1 - Change Resolution \n\r"
//             "2 - Change Bits Per Color \n\r"
   "2 - Change Number of Lanes, Link Rate \n\r"
   "3 - Change Pattern \n\r"
   "4 - Display MSA Values for Tx\n\r"
   "5 - Display Link Configuration Status and user selected resolution, BPC\n\r"
   "6 - Display DPCD register Configurations\n\r"
   "7 - Read Auxiliary registers \n\r"
   "e - Display EDID values\n\r"
   "z - Display this Menu again\r\n"
		  "- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
}

void sub_help_menu_pt(void)
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
//	  print(" 4 = Restart TX path \n\r");
//	  print(" 5 = Enable/disable DTG\n\r");
//	  print(" 6 = Re-initialize VDMA\n\r");
//	  print(" 7 = Switch TX data to internal pattern generator\n\r");
//	  print(" 8 = Switch TX back to RX video data\n\r");
//	  print(" 9 = Report RX, TX Frame stats\n\r");
	  print(" w = Sink register write\n\r");
	  print(" r = Sink register read\n\r");
	  print(" a = Display RX MST stream 1\r\n");
	  print(" b = Display RX MST stream 2\r\n");
	  print(" c = Display RX MST stream 3\r\n");
	  print(" d = Display RX MST stream 4\r\n");
	  print(" z = Display this menu again\r\n");
	  print(" x = Return to Main menu\r\n");
	  print("\n\r");
	  print("-----------------------------------------------------\n\r");}



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

void operationMenu(void){
	xil_printf(
	"\n\n-----------------------------------------------------\n\r"
	"--                       Menu                      --\n\r"
	"-----------------------------------------------------\n\r"
	"\n\r"
	" Select option\n\r"
	"t - MST Tx Only design\r\n"
	"r - MST RX-TX PassThrough design\r\n"
	);

}

void unplug_proc (void);

void main_loop(){
	int i;
	u32 Status;
//	XDpTxSs_Config *ConfigPtr;
	u8 exit = 0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 LaneCount;
	u8 LineRate;
	u8 LineRate_init_tx = 0;
	u8 Edid_org[128], Edid1_org[128];
	u8 done=0;
	u32 user_tx_LaneCount , user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
	u8 audio_on=0;
	u32 data;

	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

	unsigned char bpc_table[] = {6,8,10,12,16};


	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_1920x1080_60_P;
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;


	xilInfoFrame = 0; // initialize


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}

	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
    set_vphy(0x14);

    start_tx_only (0x14, 0x4,user_config);
	sub_help_menu ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1) {
//			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
//					user_config.VideoMode_local);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1){
			hpd_pulse_con_event = 0;
//			hpd_pulse_con(&DpTxSsInst);
//			Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
//			if (Status != XST_SUCCESS) {
//				//KAPIL Why needed??
//				xil_printf ("Link is bad\r\n");
//				sink_power_cycle();
//			}
		}


		CmdKey[0] = 0;
		CommandKey = 0;


		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);
		if (Command != 0) {
			xil_printf("You have selected command %d\r\n", Command);
		}

		switch (CommandKey){
		case 'e':
			xil_printf ("EDID read is :\r\n");

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid_org[i]);
			}

			xil_printf ("\r\r\n\n");
			//reading the second block of EDID
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);

			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid1_org[i]);
			}
			xil_printf ("\r\nEDID read over =======\r\n");

			break;

		case 'd' :

			if (in_pwr_save == 0) {
				sink_power_down();
				in_pwr_save = 1;
				xil_printf (
					"\r\n==========power down===========\r\n");
			} else {
				sink_power_up();
				in_pwr_save = 0;
				xil_printf (
					"\r\n==========power up===========\r\n");

				hpd_con(&DpTxSsInst, Edid1_org, Edid1_org,
				user_config.VideoMode_local);
			}
			break;


#if ENABLE_AUDIO
        case 'a' :
			   if (audio_on == 0) {

				  audio_stream_help();

				  while (exit == 0) {

								CmdKey[0] = 0;
								Command = 0;
								CmdKey[0] = inbyte_local();
								Command = (int)CmdKey[0];

					  switch  (CmdKey[0])
					  {
						 case '2' :
								   exit = 1;
								   /*Mute unused streams*/
										XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
															XDP_TX_AUDIO_CONTROL, 0x00010);
										xil_printf ("Configured Audio in Stream 2\r\n");
										break;

						 case '1' :
								   exit = 1;
								   /*Mute unused streams*/
										XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
															XDP_TX_AUDIO_CONTROL, 0x00000);
										xil_printf ("Configured Audio in Stream 1\r\n");
								   break;
					  }

				  }
							  exit=0;
//                           xil_printf ("\n\r Infoframe %x\r\n",xilInfoFrame);
xilInfoFrame->audio_channel_count = 1;
xilInfoFrame->audio_coding_type = 0;
xilInfoFrame->channel_allocation = 0;
xilInfoFrame->downmix_inhibit = 0;
xilInfoFrame->info_length = 27;
xilInfoFrame->level_shift = 0;
xilInfoFrame->sample_size = 0;//16 bits
xilInfoFrame->sampling_frequency = 0; //48 Hz
xilInfoFrame->type = 0x4;
xilInfoFrame->version = 0x12;

sendAudioInfoFrame(xilInfoFrame);
XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
 XDP_TX_AUDIO_CHANNELS, 0x1);
xil_printf ("kapil\r\n");
				  switch(LineRate)
				  {
								case  6:m_aud = 512; n_aud = 3375; break;
								case 10:m_aud = 512; n_aud = 5625; break;
								case 20:m_aud = 512; n_aud = 11250; break;
								case 30:m_aud = 512; n_aud = 16875; break;
				  }
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_MAUD,  m_aud );
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_NAUD,  n_aud );
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x1);
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x2);
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x10, 0x0);//0x0: Sine tone, 0x2: Ping tone
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x20, 0x0);//0x0: Sine tone, 0x2: Ping tone
				 xil_printf ("kapil\r\n");
				  //0x04120002 channel status    16.4 release
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA0, 0x10000244);

				  //channel statu    16.4 release
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA4, 0x40000000);
				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x4, 0x202);

				  /*Override Maud/Naud with fixed values - Sync mode*/
//                                                                                                 data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//                                                                                                                              XDP_TX_MAIN_STREAM_MISC0);
//                                                                                                 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//                                                                                                                              XDP_TX_MAIN_STREAM_MISC0, data|0x300);

				  data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,  XDP_TX_AUDIO_CONTROL);

				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, data|0x1);
				  xil_printf ("Audio enabled\r\n");
				  audio_on = 1;
   } else {
				   XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0xF0000);//Mute & Disable
				  xil_printf ("Audio disabled\r\n");
				  audio_on = 0;
   }
 break;



//		case 'a' :
//			audio_on = XDp_ReadReg(
//					DpTxSsInst.DpPtr->Config.BaseAddr,
//					XDP_TX_AUDIO_CONTROL);
//			if (audio_on == 0) {
//				xilInfoFrame->audio_channel_count = 0;
//				xilInfoFrame->audio_coding_type = 0;
//				xilInfoFrame->channel_allocation = 0;
//				xilInfoFrame->downmix_inhibit = 0;
//				xilInfoFrame->info_length = 27;
//				xilInfoFrame->level_shift = 0;
//				xilInfoFrame->sample_size = 1;//16 bits
//				xilInfoFrame->sampling_frequency = 3; //48 Hz
//				xilInfoFrame->type = 4;
//				xilInfoFrame->version = 1;
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x0);
//				sendAudioInfoFrame(xilInfoFrame);
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CHANNELS, 0x1);
//				switch(LineRate)
//				{
//					case  6:m_aud = 24576; n_aud = 162000; break;
//					case 10:m_aud = 24576; n_aud = 270000; break;
//					case 20:m_aud = 24576; n_aud = 540000; break;
//				}
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_MAUD,  m_aud );
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_NAUD,  n_aud );
//
//				Vpg_Audio_start();
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x1);
//				xil_printf ("Audio enabled\r\n");
//				audio_on = 1;
//			} else {
//				Vpg_Audio_stop();
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x0);
//				xil_printf ("Audio disabled\r\n");
//				audio_on = 0;
//			}
//			break;
#endif
		case '1' :
			//resolution menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			resolution_help_menu();
			exit = 0;
//			CmdKey[0] = inbyte_local();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];

					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
					xil_printf("You have selected command '%c'\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
							done = 1;
						}

						else if (Command > 47 && Command < 58) {
							Command = Command - 48;
							done = 1;
						}
						else if (Command >= 58 || Command <= 47) {
							resolution_help_menu();
							done = 0;
							break;
						}
						xil_printf ("\r\nSetting resolution...\r\n");
						audio_on = 0;
						user_config.VideoMode_local =
											resolution_table[Command];

						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();

						start_tx_only (LineRate,LaneCount,user_config);

						exit = done;
						break;
					}
				}
			}

			sub_help_menu ();
			break;
#if 0
		case '2' :
			// BPC menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();

			exit = 0;
			bpc_help_menu(DPTXSS_BPC);
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_bpc = bpc_table[Command];
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command>4) || (Command < 0))
							{
								bpc_help_menu(DPTXSS_BPC);
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting BPC of %d\r\n",
												user_config.user_bpc);
								done = 1;
							}
							start_tx (LineRate, LaneCount,user_config);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
#endif
		case '2' :
			xil_printf("Select the Link and Lane count\r\n");
			exit = 0;
			select_link_lane();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];
					Command = Command - 48;
					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
						xil_printf("You have selected command %c\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
						}

						if((Command>=0)&&(Command<12))
						{
							user_tx_LaneCount =
									lane_link_table[Command].lane_count;
							user_tx_LineRate =
									lane_link_table[Command].link_rate;
							if(lane_link_table[Command].lane_count
									> DpTxSsInst.Config.MaxLaneCount)
							{
								xil_printf(
					"This Lane Count is not supported by Sink \r\n");
								xil_printf(
					"Max Supported Lane Count is 0x%x \r\n",
										DpTxSsInst.Config.MaxLaneCount);
								xil_printf(
					"Training at Supported Lane count  \r\n");
							LaneCount = DpTxSsInst.Config.MaxLaneCount;
							}
							done = 1;
						}
						else
						{
							xil_printf(
							"!!!Warning: You have selected wrong option"
							" for lane count and link rate\r\n");
							select_link_lane();
							done = 0;
							break;
						}
						// Disabling TX interrupts
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_INTERRUPT_MASK, 0xFFF);
						LineRate_init_tx = user_tx_LineRate;
						Status = set_vphy(LineRate_init_tx);

						XDpTxSs_Stop(&DpTxSsInst);
						audio_on = 0;
						xil_printf(
					"TX Link & Lane Capability is set to %x, %x\r\n",
					user_tx_LineRate, user_tx_LaneCount);
						xil_printf(
					"Setting TX to 8 BPC and 800x600 resolution\r\n");
						XDpTxSs_Reset(&DpTxSsInst);
						user_config.user_bpc=8;
						user_config.VideoMode_local
										=XVIDC_VM_800x600_60_P;
						user_config.user_pattern=1;
						user_config.user_format = XVIDC_CSF_RGB;
//						start_tx (user_tx_LineRate, user_tx_LaneCount,
//												user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();
						exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '3' :
			//pattern menu;
			test_pattern_gen_help();
			exit = 0;
			while (exit == 0) {
				CommandKey = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					Command = Command - 48;
					switch  (CommandKey)
					{
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :

							if(Command>0 && Command<8)
							{
								xil_printf(
								"You have selected video pattern %d "
								"from the pattern list \r\n", Command);
								done = 1;
							}
							else
							{
								xil_printf(
						"!!!Warning : Invalid pattern selected \r\n");
								test_pattern_gen_help();
								done = 0;
								break;
							}
							user_config.user_pattern = Command;
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							exit = done;
							break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '4' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
#if 0
		case '6' :
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			exit = 0;
			format_help_menu();
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_format = Command;
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command<=0)||(Command>3))
							{
								format_help_menu();
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting Format of %d\r\n",
											user_config.user_format);
								done = 1;
							}
							if(user_config.user_format!=1)
							{
							//Only Color Square is supported for YCbCr
								user_config.user_pattern = 3;
							}
							else
							{
								//Set Color Ramp for RGB (default)
								user_config.user_pattern = 1;
							}
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							start_tx (LineRate, LaneCount,user_config);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
#endif
		case '5' :
			//Link config and status
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
//			XDpRxSs_ReportLinkInfo(&DpRxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '6' :
			//Display DPCD reg
			XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '7' :
			//"9 - Read Aux registers\r\n"
			xil_printf(
		"\r\n Give 4 bit Hex value of base register 0x");
			aux_reg_address = xil_gethex(4);
			xil_printf(
			  "\r\n Give msb 2 bit Hex value of base register 0x");
			aux_reg_address |= ((xil_gethex(2)<<16) & 0xFFFFFF);
			xil_printf("\r\n Give number of registers that you "
								"want to read (1 to 9): ");
			num_of_aux_registers = xil_gethex(1);
			if((num_of_aux_registers<1)||(num_of_aux_registers>9))
			{
					xil_printf("\r\n!!!Warning: Invalid number "
				   "selected, hence reading only one register\r\n");
					num_of_aux_registers = 1;
			}
			xil_printf("\r\nGiven base address offset is 0x%x\r\n",
										aux_reg_address);
			for(i=0;i<num_of_aux_registers;i++)
			{
					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
						(aux_reg_address+i), 1, &Data);
					if(Status == XST_SUCCESS)
					{
							xil_printf("Value at address offset "
						"0x%x, is = 0x%x\r\n",
										(aux_reg_address+i),
										((Data[0]) & 0xFF));
					} else {
							xil_printf("Aux Read failure\r\n");
							break;
					}
			}
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		// Display VideoPHY status
		case 'b':
			xil_printf("Video PHY Config/Status --->\r\n");
			xil_printf(" RCS (0x10) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_REF_CLK_SEL_REG));
			xil_printf(" PR  (0x14) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_RESET_REG));
			xil_printf(" PLS (0x18) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_LOCK_STATUS_REG));
			xil_printf(" TXI (0x1C) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_REG));
			xil_printf(" TXIS(0x20) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_STATUS_REG));
			xil_printf(" RXI (0x24) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_REG));
			xil_printf(" RXIS(0x28) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_STATUS_REG));

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_FBDIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) "
				"= 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_FBDIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_REFCLK_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_REFCLK_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_RXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_RXOUT_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_TXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_TXOUT_DIV,
				DrpVal
			);

			break;


			// CRC read
		case 'm' :
//			XVidFrameCrc_Report();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			sub_help_menu ();
			break;

		} //end of switch (CmdKey[0])
	}
}


u8 rx_unplugged = 0;
u8 rx_trained = 0;

void pt_loop(){
	int i;
	u32 Status;
	XVidC_VideoMode VmId;
//	XDpTxSs_Config *ConfigPtr;
	u8 exit = 0;
	u32 user_lane_count;
	u32 user_link_rate;
	u32 data, addr;

	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 LaneCount;
	u8 LineRate;
	u8 LineRate_init_tx = 0;
	u8 Edid_org[128], Edid1_org[128];
	u8 done=0;
	u32 user_tx_LaneCount , user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
	u8 audio_on=0;
	XilAudioInfoFrame *xilInfoFrame;
	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

	unsigned char bpc_table[] = {6,8,10,12,16};


	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
	user_config.user_pattern= C_VideoUserStreamPattern[1];
	user_config.user_format = XVIDC_CSF_RGB;


	xilInfoFrame = 0; // initialize


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}

    XAxisScr_MiPortEnable (&axis_switch, 0, 0);
    XAxisScr_RegUpdateEnable (&axis_switch);

//     XAxisScr_MiPortEnable (&axis_switch_tx, 0, 0);
//      XAxisScr_RegUpdateEnable (&axis_switch_tx);

    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x1);

      //Clearing the interrupt before starting
    XDpTxSs_Stop(&DpTxSsInst);
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x144, 0xFFF);

	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);

	sub_help_menu_pt ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1 && DpRxSsInst.link_up_trigger == 1) {
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			start_tx_after_rx(1);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1 && DpRxSsInst.link_up_trigger == 1){
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst);
			Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
			if (Status != XST_SUCCESS) {
				xil_printf ("Link is bad\r\n");
				start_tx_after_rx (1);
			}
		}

        if (rx_unplugged == 1) {
                 xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
                 unplug_proc ();
        } else if (DpRxSsInst.link_up_trigger == 0) {       // Link Not trained
            if (rx_trained == 1) {                                  // If it was previously trained
                    xil_printf ("Training Lost !!\r\n");
//                    unplug_proc ();
            }
            rx_trained = 0;
        } else if(DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0){
			xil_printf(
			"> Rx Training done !!! (BW: 0x%x, Lanes: 0x%x, Status: "
			"0x%x;0x%x).\n\r",
			XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DPCD_LINK_BW_SET),
			XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DPCD_LANE_COUNT_SET),
			XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DPCD_LANE01_STATUS),
			XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_DPCD_LANE23_STATUS));
			DpRxSsInst.VBlankCount++;
			rx_trained = 1;
//			DpRxSsInst.link_up_trigger = 0;

			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_ENABLE, 0x0);

			XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
					&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);

		}

		if(DpRxSsInst.no_video_trigger == 1){
			frameBuffer_stop(Msa);
//			XI2s_Tx_Enable(&I2s_tx, 0);
			DpRxSsInst.no_video_trigger = 0;
		}

		//Pass-through Handling
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT){
			DpRxSsInst.no_video_trigger = 0;
			//VBLANK Management
			DpRxSsInst.VBlankCount = 0;
			XDp_RxDtgDis(DpRxSsInst.DpPtr);
//			XDp_RxDtgEn(DpRxSsInst.DpPtr);
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
			XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK |
					XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK);

            XDpRxSs_AudioDisable(&DpRxSsInst);
            XDp_RxDtgEn(DpRxSsInst.DpPtr);
//            XDpRxSs_AudioEnable(&DpRxSsInst);


			start_tx_after_rx (1);
//			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x144, 0xFFF);

		}

		CmdKey[0] = 0;
		CommandKey = 0;


		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);
		if (Command != 0) {
			xil_printf("You have selected command %d\r\n", Command);
		}

		switch (CommandKey){
		case '1':
			select_rx_link_lane();
			CmdKey[0] = 0;
			Command = 0;
			CmdKey[0] = inbyte_local();
			if(CmdKey[0]!=0){
				Command = (int)CmdKey[0];
				Command = Command -48;
				switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
					xil_printf("You have selected command '%c'\r\n",
															CmdKey[0]);
					if((Command>=0)&&(Command<9)) {
						user_lane_count =  lane_link_table[Command].lane_count;
						user_link_rate =  lane_link_table[Command].link_rate;
						if(lane_link_table[Command].lane_count > DpRxSsInst.DpPtr->Config.MaxLaneCount)
						{
								xil_printf("This Lane Count is not supported by Sink \n\r");
								xil_printf("Max Supported Lane Count is 0x%x \n\r", DpRxSsInst.DpPtr->Config.MaxLaneCount);
								xil_printf("Training at Supported Lane count  \r\n");
								user_lane_count = DpRxSsInst.DpPtr->Config.MaxLaneCount;
						}
						if(lane_link_table[Command].link_rate > DpRxSsInst.DpPtr->Config.MaxLinkRate)
						{
								xil_printf("This link rate is not supported by Sink \n\r");
								xil_printf("Max Supported Link Rate is 0x%x \n\r", DpRxSsInst.DpPtr->Config.MaxLinkRate);
								xil_printf("Training at supported Link Rate\r\n");
								user_link_rate = DpRxSsInst.DpPtr->Config.MaxLinkRate;
						}
						xil_printf("RX Link & Lane Capability is set to %x, %x\r\n", user_link_rate, user_lane_count);
						xil_printf ("\r\n **Important: Please ensure to unplug & plug the cable after the capabilities have been changed **\r\n");
						XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0x7FF8FFFF);
						// Disabling TX interrupts
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
						XDpTxSs_Stop(&DpTxSsInst);
						XDpRxSs_SetLinkRate(&DpRxSsInst, user_link_rate);
						XDpRxSs_SetLaneCount(&DpRxSsInst, user_lane_count);

					}
					}
			}
             break;

		case '2':
		//	debug_info();
			xil_printf ("==========RX Debug Data===========\r\n");
			XDpRxSs_ReportLinkInfo(&DpRxSsInst);
			XDpRxSs_ReportMsaInfo(&DpRxSsInst);
			xil_printf ("==========TX Debug Data===========\r\n");
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
			xil_printf ("==========VTC Debug Data===========\r\n");
			XDpTxSs_ReportVtcInfo(&DpTxSsInst);

			break;

		case '3':
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
			XDp_RxInterruptEnable(DpRxSsInst.DpPtr,0x80000000);
			// Disabling TX interrupts
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
			XDpTxSs_Stop(&DpTxSsInst);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_HPD_INTERRUPT,0xFBB80001);
			xil_printf("\r\n- HPD Toggled for 3ms! -\n\r");
			break;


		case 'w':
			xil_printf("\n\rEnter 4 hex characters: Sink Write address 0x");
			addr = xil_gethex(4);
			xil_printf("\n\rEnter 4 hex characters: Sink Write Data 0x");
			data = xil_gethex(4);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,addr, data);
			break;

		case 'r':
			xil_printf("\n\rEnter 4 hex characters: Sink Read address 0x");
			addr = xil_gethex(4);
			data = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, addr);
			xil_printf("\n\rSink Read Addr %04x Read Data: %04x\n\r", (XPAR_DPRXSS_0_BASEADDR+addr), data);
			break;

		case 'a' :
			// ask for which TX stream
			start_tx_after_rx (1);
//            XDpRxSs_AudioDisable(&DpRxSsInst);
//			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x300, 0x01);
//			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
//			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CHANNELS, 0x1);
//			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x1);
//            XDpRxSs_AudioEnable(&DpRxSsInst);



			break;

		case 'b' :
			start_tx_after_rx (2);
//            XDpRxSs_AudioDisable(&DpRxSsInst);
//			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x300, 0x11);

//            XDpRxSs_AudioEnable(&DpRxSsInst);

			break;

		case 'c' :
//			start_tx_after_rx (3);
            XDpRxSs_AudioDisable(&DpRxSsInst);

			start_audio_passThrough(0x14);

			usleep(50000);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x300, 0x21);
			xil_printf ("Audio Started...\r\n");
//            XDpRxSs_AudioEnable(&DpRxSsInst);

			break;

		case 'd' :
			start_tx_after_rx (4);
//            XDpRxSs_AudioDisable(&DpRxSsInst);
//			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x300, 0x31);

//            XDpRxSs_AudioEnable(&DpRxSsInst);

			break;
//
#if ENABLE_AUDIO
        case 'e' :
			   if (audio_on == 0) {

				  audio_stream_help();

				  while (exit == 0) {

								CmdKey[0] = 0;
								Command = 0;
								CmdKey[0] = inbyte_local();
								Command = (int)CmdKey[0];

					  switch  (CmdKey[0])
					  {
						 case '2' :
								   exit = 1;
								   /*Mute unused streams*/
										XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
															XDP_TX_AUDIO_CONTROL, 0x00010);
										xil_printf ("Configured Audio in Stream 2\r\n");
										break;

						 case '1' :
								   exit = 1;
								   /*Mute unused streams*/
										XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
															XDP_TX_AUDIO_CONTROL, 0x00000);
										xil_printf ("Configured Audio in Stream 1\r\n");
								   break;
					  }

				  }
							  exit=0;
//                           xil_printf ("\n\r Infoframe %x\r\n",xilInfoFrame);
xilInfoFrame->audio_channel_count = 1;
xilInfoFrame->audio_coding_type = 0;
xilInfoFrame->channel_allocation = 0;
xilInfoFrame->downmix_inhibit = 0;
xilInfoFrame->info_length = 27;
xilInfoFrame->level_shift = 0;
xilInfoFrame->sample_size = 0;//16 bits
xilInfoFrame->sampling_frequency = 0; //48 Hz
xilInfoFrame->type = 0x4;
xilInfoFrame->version = 0x12;

sendAudioInfoFrame(xilInfoFrame);
XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
 XDP_TX_AUDIO_CHANNELS, 0x1);
xil_printf ("kapil\r\n");
				  switch(LineRate)
				  {
								case  6:m_aud = 512; n_aud = 3375; break;
								case 10:m_aud = 512; n_aud = 5625; break;
								case 20:m_aud = 512; n_aud = 11250; break;
								case 30:m_aud = 512; n_aud = 16875; break;
				  }
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_MAUD,  m_aud );
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_NAUD,  n_aud );
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x1);
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x2);
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x10, 0x0);//0x0: Sine tone, 0x2: Ping tone
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x20, 0x0);//0x0: Sine tone, 0x2: Ping tone
//				 xil_printf ("kapil\r\n");
//				  //0x04120002 channel status    16.4 release
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA0, 0x10000244);
//
//				  //channel statu    16.4 release
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA4, 0x40000000);
//				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x4, 0x202);

//				  /*Override Maud/Naud with fixed values - Sync mode*/
//				 data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0);
//				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0, data|0x300);

				  data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,  XDP_TX_AUDIO_CONTROL);

				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, data|0x1);
				  xil_printf ("Audio enabled\r\n");
				  audio_on = 1;
   } else {
				   XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
				 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0xF0000);//Mute & Disable
				  xil_printf ("Audio disabled\r\n");
				  audio_on = 0;
   }
 break;



//		case 'a' :
//			audio_on = XDp_ReadReg(
//					DpTxSsInst.DpPtr->Config.BaseAddr,
//					XDP_TX_AUDIO_CONTROL);
//			if (audio_on == 0) {
//				xilInfoFrame->audio_channel_count = 0;
//				xilInfoFrame->audio_coding_type = 0;
//				xilInfoFrame->channel_allocation = 0;
//				xilInfoFrame->downmix_inhibit = 0;
//				xilInfoFrame->info_length = 27;
//				xilInfoFrame->level_shift = 0;
//				xilInfoFrame->sample_size = 1;//16 bits
//				xilInfoFrame->sampling_frequency = 3; //48 Hz
//				xilInfoFrame->type = 4;
//				xilInfoFrame->version = 1;
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x0);
//				sendAudioInfoFrame(xilInfoFrame);
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CHANNELS, 0x1);
//				switch(LineRate)
//				{
//					case  6:m_aud = 24576; n_aud = 162000; break;
//					case 10:m_aud = 24576; n_aud = 270000; break;
//					case 20:m_aud = 24576; n_aud = 540000; break;
//				}
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_MAUD,  m_aud );
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_NAUD,  n_aud );
//
//				Vpg_Audio_start();
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x1);
//				xil_printf ("Audio enabled\r\n");
//				audio_on = 1;
//			} else {
//				Vpg_Audio_stop();
//				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
//						XDP_TX_AUDIO_CONTROL, 0x0);
//				xil_printf ("Audio disabled\r\n");
//				audio_on = 0;
//			}
//			break;
#endif


			break;
		} //end of switch (CmdKey[0])

	}
}


static char inbyte_local(void){
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
static u32 xil_gethex(u8 num_chars){
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
static char XUartPs_RecvByte_NonBlocking(){
    u32 RecievedByte;
    RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
    /* Return the byte received */
    return (u8)RecievedByte;
}



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

void start_tx_after_rx(u8 stream_id) {
	u32 mvid_rx;
	u32 nvid_rx;
	u32 Status;

	// Get incoming pixel frequency at here
	u32 recv_clk_freq;

	// Re-calculating Mvid/Nvid based on 5.4Gbps
	u32 nvid_tx;
	u32 mvid_tx;
	XVidC_VideoMode VmId;
	user_config_struct user_config;

	XDpTxSs_Stop(&DpTxSsInst);
	XDpTxSs_Reset(&DpTxSsInst);
	XAxisScr_MiPortEnable (&axis_switch, 0, stream_id-1);
	XAxisScr_RegUpdateEnable (&axis_switch);

//Dppt_DetectResolution(DpRxSsInst.DpPtr, 1, Msa);
//Dppt_DetectResolution(DpRxSsInst.DpPtr, 2, Msa);
//Dppt_DetectResolution(DpRxSsInst.DpPtr, 3, Msa);
//Dppt_DetectResolution(DpRxSsInst.DpPtr, 4, Msa);

	Dppt_DetectResolution(DpRxSsInst.DpPtr, stream_id, Msa);

	VmId = XVidC_GetVideoModeId(
						Msa[0].Vtm.Timing.HActive,
						Msa[0].Vtm.Timing.VActive,
						Msa[0].Vtm.FrameRate,0);
	user_config.user_bpc = Msa[0].BitsPerColor;
	user_config.user_pattern = 0; /*pass-through (Default)*/
	user_config.VideoMode_local = VmId;
	/*Check component Format*/
	if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XVIDC_CSF_YCRCB_422 + 1;
	}else if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XVIDC_CSF_YCRCB_444 + 1;
	}else
		user_config.user_format = XVIDC_CSF_RGB + 1;


	Dprx_ResetVideoOutput(DpRxSsInst.DpPtr);
	frameBuffer_stop(Msa);

// check monitor capability
//Waking up the monitor
    sink_power_cycle();


	u8 max_cap_org=0;
	u8 max_cap_lanes=0;
	u8 monitor_8K=0;
	u8 downshift4K = 0;
	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes);
	u8 rData = 0;
	// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
					1, &rData);

	// if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
	if(rData & 0x80){
			// read maxLineRate
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
			if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
					monitor_8K = 1;
					max_cap_org = 0x1E;
					xil_printf ("Monitor is 8.1 capable\r\n");
			} else {
					xil_printf ("Monitor is not 8.1 capable\r\n");
			}
	}


// if monitor does not support 8.1G then calc MVID, NVID for 5.4
	if (monitor_8K == 0) {
		if(DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_810GBPS) {

		mvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MVID);
		nvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_NVID);

		// Get incoming pixel frequency at here
		recv_clk_freq =
			(((int)DpRxSsInst.UsrOpt.LinkRate*27)*mvid_rx)/nvid_rx;

		// Re-calculating Mvid/Nvid based on 5.4Gbps
		nvid_tx = (XDP_TX_LINK_BW_SET_540GBPS * 27);
		mvid_tx = (recv_clk_freq * nvid_tx * 1000) / (XDP_TX_LINK_BW_SET_540GBPS*27);
		nvid_tx *= 1000;

		// Update MVID and NVID at here with bsed on 5.4Gbps
		Msa[0].MVid = mvid_tx;
		Msa[0].NVid = nvid_tx;

		}
	}


XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
// This configures the vid_phy for line rate to start with
//Even though CPLL can be used in limited case,
//using QPLL is recommended for more coverage.
//frameBuffer_stop(Msa);
set_vphy(0x14);
start_tx(0x14, 0x4, user_config, Msa);
//start_audio_passThrough(0x14);
frameBuffer_start(VmId, Msa, 0);
}


/* Audio passThrough setting */
void start_audio_passThrough(u8 LineRate_init_tx){
        int m_aud;
        int n_aud;
        u16 data;

        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x00000);

        xilInfoFrame->audio_channel_count = 1;
        xilInfoFrame->audio_coding_type = 0;
        xilInfoFrame->channel_allocation = 0;
        xilInfoFrame->downmix_inhibit = 0;
        xilInfoFrame->info_length = 27;
        xilInfoFrame->level_shift = 0;
        xilInfoFrame->sample_size = 0;//16 bits
        xilInfoFrame->sampling_frequency = 0; //48 Hz
        xilInfoFrame->type = 0x4;
        xilInfoFrame->version = 0x12;

        sendAudioInfoFrame(xilInfoFrame);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
         XDP_TX_AUDIO_CHANNELS, 0x1);
        xil_printf ("kapil\r\n");
					  switch(LineRate_init_tx)
					  {
									case  6:m_aud = 512; n_aud = 3375; break;
									case 10:m_aud = 512; n_aud = 5625; break;
									case 20:m_aud = 512; n_aud = 11250; break;
									case 30:m_aud = 512; n_aud = 16875; break;
					  }
					 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_MAUD,  m_aud );
					 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_NAUD,  n_aud );
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x1);
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x2);
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x10, 0x0);//0x0: Sine tone, 0x2: Ping tone
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x20, 0x0);//0x0: Sine tone, 0x2: Ping tone
//        				 xil_printf ("kapil\r\n");
//        				  //0x04120002 channel status    16.4 release
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA0, 0x10000244);
//
//        				  //channel statu    16.4 release
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA4, 0x40000000);
//        				 XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x4, 0x202);

					  /*Override Maud/Naud with fixed values - Sync mode*/
        //                                                                                                 data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
        //                                                                                                                              XDP_TX_MAIN_STREAM_MISC0);
        //                                                                                                 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
        //                                                                                                                              XDP_TX_MAIN_STREAM_MISC0, data|0x300);

//        				  data = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,  XDP_TX_AUDIO_CONTROL);

					 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x1);
//        		            XDpRxSs_AudioDisable(&DpRxSsInst);
//        		            XDpRxSs_AudioEnable(&DpRxSsInst);
//        					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x300, 0x21);
					  xil_printf ("Audio enabled\r\n");

#if 0

        // stable video is there and ready to start Audio pass-through
        xilInfoFrame->audio_channel_count = 1;
        xilInfoFrame->audio_coding_type = 0;
        xilInfoFrame->channel_allocation = 0;
        xilInfoFrame->downmix_inhibit = 0;
        xilInfoFrame->info_length = 27;
        xilInfoFrame->level_shift = 0;
        xilInfoFrame->sample_size = 0;//16 bits
        xilInfoFrame->sampling_frequency = 0; //48 Hz
        xilInfoFrame->type = 0x84;
        xilInfoFrame->version = 0x12;
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                                XDP_TX_AUDIO_CONTROL, 0x0);
        sendAudioInfoFrame(xilInfoFrame);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                                XDP_TX_AUDIO_CHANNELS, 0x1);
        //switch(dp_conf.LineRate){
        switch(LineRate_init_tx){
                case  6:m_aud = 512; n_aud = 3375; break;
                case 10:m_aud = 512; n_aud = 5625; break;
                case 20:m_aud = 512; n_aud = 11250; break;
                case 30:m_aud = 512; n_aud = 16875; break;
        }
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                        XDP_TX_AUDIO_MAUD,  m_aud );
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                                        XDP_TX_AUDIO_NAUD,  n_aud );
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                        XDP_TX_AUDIO_CONTROL, 0x0);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
                        XDP_TX_AUDIO_CONTROL, 0x1);
//      XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//                      XDP_RX_AUDIO_CONTROL, 0x0);
//      XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//                      XDP_RX_AUDIO_CONTROL, 0x1);
#endif
        xil_printf ("Starting audio on TX..\r\n");
}

void unplug_proc (void) {
        rx_trained = 0;
        rx_unplugged = 0;
        frameBuffer_stop(Msa);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
        DpRxSsInst.VBlankCount = 0;
        DpRxSs_Setup();
}
