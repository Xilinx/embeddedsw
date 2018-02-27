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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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

};

extern XVidC_VideoMode resolution_table[];

static char inbyte_local(void);
static u32 xil_gethex(u8 num_chars);
static char XUartPs_RecvByte_NonBlocking(void);


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
"f 1920x1440_60_P  |   i 3840x2160_60_P_RB \r\n"
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
	   "8 --> Set TX capability @  5.4G 4 lanes\r\n");
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
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;


	xilInfoFrame = 0; // initialize



	sub_help_menu ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1) {
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			tx_is_reconnected = 0;
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
				xilInfoFrame->audio_channel_count = 0;
				xilInfoFrame->audio_coding_type = 0;
				xilInfoFrame->channel_allocation = 0;
				xilInfoFrame->downmix_inhibit = 0;
				xilInfoFrame->info_length = 27;
				xilInfoFrame->level_shift = 0;
				xilInfoFrame->sample_size = 1;//16 bits
				xilInfoFrame->sampling_frequency = 3; //48 Hz
				xilInfoFrame->type = 4;
				xilInfoFrame->version = 1;
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				sendAudioInfoFrame(xilInfoFrame);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CHANNELS, 0x1);
				switch(LineRate)
				{
					case  6:m_aud = 24576; n_aud = 162000; break;
					case 10:m_aud = 24576; n_aud = 270000; break;
					case 20:m_aud = 24576; n_aud = 540000; break;
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
						if((Command>=0)&&(Command<9))
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
