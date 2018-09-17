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
#include "xdptxss_vcu118_tx.h"
#include "xuartlite_l.h"


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

#define XVIDC_FR_82HZ 82
#define XVIDC_FR_98HZ 98

//extern XVidC_VideoMode resolution_table[];
// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB ,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_DP_24_P,
	XVIDC_VM_7680x4320_DP_25_P,
	XVIDC_VM_7680x4320_DP_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,

	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR,
	XVIDC_VM_3840x2160_82_ASUS,
	XVIDC_VM_3840x2160_98_ASUS,
	XVIDC_VM_3840x2160_120_ASUS,
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

	{ XVIDC_VM_7680x4320_DP_24_P, "7680x4320@24Hz", XVIDC_FR_24HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_DP_25_P, "7680x4320@25Hz", XVIDC_FR_25HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_DP_30_P, "7680x4320@30Hz", XVIDC_FR_30HZ,
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

		//Asus 4k@82
	{ XVIDC_VM_3840x2160_82_ASUS, "3840x2160@82Hz_ASUS", XVIDC_FR_82HZ,
			{3840, 8, 32, 20, 3900, 1,
			2160, 3, 6, 48, 2217, 0, 0, 0, 0, 1}},

		//Asus 4k@98
	{ XVIDC_VM_3840x2160_98_ASUS, "3840x2160@98Hz_ASUS", XVIDC_FR_98HZ,
			{3840, 8, 32, 20, 3900, 1,
			2160, 3, 6, 56, 2225, 0, 0, 0, 0, 1}},

		//Asus 4k@120
	{ XVIDC_VM_3840x2160_120_ASUS, "3840x2160@120Hz_ASUS", XVIDC_FR_120HZ,
			{3840, 8, 32, 40, 3920, 1,
			2160, 3, 5, 70, 2238, 0, 0, 0, 0, 1}},


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
	XVIDC_VM_7680x4320_DP_24_P,
				XVIDC_VM_7680x4320_DP_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,
	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR,
	XVIDC_VM_3840x2160_82_ASUS,
	XVIDC_VM_3840x2160_98_ASUS,
	XVIDC_VM_3840x2160_120_ASUS

};


char inbyte_local(void);
static u32 xil_gethex(u8 num_chars);
static char XUart_RecvByte_NonBlocking(void);

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
static char XUart_RecvByte_NonBlocking(){
    u32 RecievedByte;
    RecievedByte = XUartLite_ReadReg(STDIN_BASEADDRESS, 0);
    /* Return the byte received */
    return (u8)RecievedByte;
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
"s 3840x2160_82_ASUS|  t 3840x2160_98_ASUS|    u 3840x2160_120_ASUS\r\n"
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
			   "5 -->  Flat Green screen 			\r\n"
			   "6 -->  Flat Blue screen 			\r\n"
			   "7 -->  Flat Yellow screen 			\r\n"
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
	XDp_TxAudioInfoFrame *xilInfoFrame;
	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

	unsigned char bpc_table[] = {6,8,10,12,16};


	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
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



	sub_help_menu ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1) {
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			tx_is_reconnected = 0;
			xil_printf ("HPD connected\r\n");
		}

		if(hpd_pulse_con_event == 1){
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst);
			if(XDpTxSs_CheckLinkStatus(&DpTxSsInst)){
				sink_power_cycle();
			}
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
			audio_on = XDp_ReadReg(
					DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_CONTROL);
			if (audio_on == 0) {
                xilInfoFrame->audio_channel_count = 1;
                xilInfoFrame->audio_coding_type = 0;
                xilInfoFrame->channel_allocation = 0;
                xilInfoFrame->downmix_inhibit = 0;
                xilInfoFrame->info_length = 27;
                xilInfoFrame->level_shift = 0;
                xilInfoFrame->sample_size = 0;//16 bits
                xilInfoFrame->sampling_frequency = 0; //48 Hz
                xilInfoFrame->type = 0x84;
                xilInfoFrame->version = 0x11;
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				XDpTxSs_SendAudioInfoFrame(&DpTxSsInst,xilInfoFrame);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CHANNELS, 0x1);
				switch(LineRate)
				{
                    case  6:m_aud = 512; n_aud = 3375; break;
                    case 10:m_aud = 512; n_aud = 5625; break;
                    case 20:m_aud = 512; n_aud = 11250; break;
                    case 30:m_aud = 512; n_aud = 16875; break;
				}
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_MAUD,  m_aud );
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_NAUD,  n_aud );

				Vpg_Audio_start();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x1);
				xil_printf ("Audio enabled\r\n");
				audio_on = 1;
			} else {
				Vpg_Audio_stop();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				xil_printf ("Audio disabled\r\n");
				audio_on = 0;
			}
			break;
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


						start_tx (LineRate,LaneCount,user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();

						exit = done;
						break;
					}
				}
			}

			sub_help_menu ();
			break;

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

		case '3' :
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
						start_tx (user_tx_LineRate, user_tx_LaneCount,
												user_config);
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

		case '4' :
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

		case '5' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

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

		case '7' :
			//Link config and status
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '8' :
			//Display DPCD reg
			XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '9' :
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
			XVidFrameCrc_Report();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			sub_help_menu ();
			break;

		} //end of switch (CmdKey[0])
	}
}

u8 XUartLite_RecvByte_local(u32 BaseAddress)
{
        do
        {

        } while(XUartLite_IsReceiveEmpty(BaseAddress));
        return (u8)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}


char inbyte_local(void){
//	char c=0;
	return XUartLite_RecvByte_local(STDIN_BASEADDRESS);
//	c = XUart_RecvByte_NonBlocking();
//	return c;
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



/****************************************************************************/
/**
*
* This functions receives a single byte using the UART. It is non-blocking in
* that it returns if there is no data received.
*
* @param           Data will have the received data after this function returns
*                                           XST_SUCCESS
*
* @return            XST_SUCCESS if received a byte, XST_FAILURE otherwise
*
* @note                             None.
*
******************************************************************************/
int UART_RecvByte(u8 *Data)
{

 /* If it is a UartLite device */

   if(XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS)) {
		xil_printf("\n\r 1 ... Is uart empty ::::::: 0x%x\n\r",XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS));
		return XST_FAILURE;
  } else {
		*Data = (u8)Xil_In32(STDIN_BASEADDRESS + XUL_RX_FIFO_OFFSET);
		xil_printf("\n\r 2 ... Is uart empty ::::::: 0x%x\n\r",XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS));
		return XST_SUCCESS;
   }

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
	return c;
}
