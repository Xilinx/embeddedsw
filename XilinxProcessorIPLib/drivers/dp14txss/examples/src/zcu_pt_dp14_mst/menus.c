/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 1.00 KU  02/05/19 Initial release.
*
* </pre>
*
******************************************************************************/
#include "xdptxss_zcu102_mst_pt.h"


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

XVidC_VideoMode resolution_table[] =
{
                XVIDC_VM_640x480_60_P,
                XVIDC_VM_480_60_P,
                XVIDC_VM_800x600_60_P,
                XVIDC_VM_1024x768_60_P,
                XVIDC_VM_720_60_P,
                XVIDC_VM_1600x1200_60_P,
                XVIDC_VM_1366x768_60_P,
                XVIDC_VM_1080_60_P
};

static char inbyte_local(void);

static char XUartPs_RecvByte_NonBlocking(void);
extern void Dprx_DetectResolution(void *InstancePtr, u16 offset);
extern void Dprx_ResetVideoOutput(void *InstancePtr);
extern XDpTxSs_MainStreamAttributes Msa[4];
extern void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);

//void start_tx_after_rx( u8 stream_id, u8 only_tx);
void start_audio_passThrough();
XDp_TxAudioInfoFrame *xilInfoFrame;

u8 tx_is_up = 0;
u8 aud_started = 0;
extern XilAudioInfoFrame_rx AudioinfoFrame;
extern XAxis_Switch axis_switch;
extern XDpRxSs DpRxSsInst;	/* The DPTX Subsystem instance.*/
extern XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
extern XVphy VPhyInst;
extern XTmrCtr TmrCtr;
extern int tx_is_reconnected;
extern u8 hpd_pulse_con_event;
extern XScuGic IntcInst;

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
				"3 -->  Send Audio in Stream 3 \n\r"
				"4 -->  Send Audio in Stream 4 \n\r"
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
			   "5 -->  Flat Green screen 			\r\n"
			   "6 -->  Flat Blue screen 			\r\n"
			   "7 -->  Flat Purple screen 			\r\n"
			   "\r\n"
			   "Press 'x' to return to main menu\r\n"
			   "Press any key to show this menu again\r\n");
}

void sub_help_menu(void)
{

  xil_printf(
"- - - - -  -  - - - - - - - - - - - - - - - - - -\r\n"
"-          DisplayPort TX Only Demo Menu        -\r\n"
"- Press 'z' to get this main menu at any point  -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - -\r\n"

   "1 - Change Resolution \n\r"
   "8 - Change Bits Per Color \n\r"
   "2 - Change Number of Lanes, Link Rate \n\r"
   "3 - Change Pattern \n\r"
   "4 - Display MSA Values for Tx\n\r"
   "5 - Display Link Configuration Status and user selected resolution, BPC\n\r"
   "6 - Display DPCD register Configurations\n\r"
   "7 - Read Auxiliary registers \n\r"
   "a - Enable/Disable Audio \n\r"
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
//	  print(" x = Return to Main menu\r\n");
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

// This is for TX only mode

void main_loop(){
	int i;
	u32 Status;
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
	int it = 0;
	u8 stream;

	unsigned char bpc_table[] = {6,8,10,12,16};

//	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_1920x1080_60_P;
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;


	xilInfoFrame = 0; // initialize
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
    set_vphy(DPTXSS_LINK_RATE);
    start_tx_only (DPTXSS_LINK_RATE, DPTXSS_LANE_COUNT,user_config);
	sub_help_menu ();

	exit = 0;
	while (exit == 0) { // for menu loop
		if (tx_is_reconnected == 1) {
			start_tx_only (DPTXSS_LINK_RATE, DPTXSS_LANE_COUNT, user_config);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1){
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, 1);
			Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
			if (Status != XST_SUCCESS) {
				xil_printf ("Link is bad..re initiating training\r\n");
				sink_power_cycle();
				tx_is_reconnected = 1;
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
			   if (audio_on == 0) {

				  audio_stream_help();
				  exit = 0;

				  while (exit == 0) {

								CmdKey[0] = 0;
								Command = 0;
								CmdKey[0] = inbyte_local();
								Command = (int)CmdKey[0];

					  switch  (CmdKey[0])
					  {
						 case '4' :
								   exit = 1;
										stream = 0x4;
										break;

						 case '3' :
								   exit = 1;
										stream = 0x3;
										break;

						 case '2' :
								   exit = 1;
										stream = 0x2;
										break;

						 case '1' :
								   exit = 1;
										stream = 0x1;
								   break;
					  }

				  }
				  if (exit == 1) {
					exit=0;
					CmdKey[0] = 0;
					Command = 0;
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
#if SEND_AIF
					XDpTxSs_SendAudioInfoFrame(&DpTxSsInst, xilInfoFrame);
#endif
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					 XDP_TX_AUDIO_CHANNELS, 0x1);

					usleep(10000);
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x1);
					usleep(10000);
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x2);
					usleep(10000);
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x10, 0x0);//0x0: Sine tone, 0x2: Ping tone
					usleep(10000);
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x20, 0x0);//0x0: Sine tone, 0x2: Ping tone
					usleep(10000);

					//0x04120002 channel status    16.4 release
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA0, 0x10000244);
					usleep(10000);
					//channel statu    16.4 release
					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA4, 0x40000000);
					usleep(10000);

					XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x4, 0x202);
					usleep(10000);

					XDpTxSs_Mst_AudioEnable (&DpTxSsInst , stream);
					xil_printf ("Audio enabled\r\n");
					audio_on = 1;
			   }
			   } else {
	   	   	   	   XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
				   XDpTxSs_Mst_AudioDisable (&DpTxSsInst);
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

						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();

						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x10, 0x0);//0x0: Sine tone, 0x2: Ping tone
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x20, 0x0);//0x0: Sine tone, 0x2: Ping tone
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA0, 0x0);
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0xA4, 0x0000000);
						usleep(10000);
						XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x4, 0x0);
						usleep(10000);

						start_tx_only (LineRate,LaneCount,user_config);
						exit = done;
						break;
					}
				}
			}
            exit = 0;
			sub_help_menu ();
			break;
#if 1
		case '8' :
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

							Status = set_vphy(0x14);
							start_tx_only (LineRate, LaneCount,user_config);
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
			   	   	   	   XDp_WriteReg (XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR + 0x400,  0x0, 0x0);
//						 XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0xF0000);//Mute & Disable

						start_tx_only (user_tx_LineRate, user_tx_LaneCount,
												user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();
						exit = done;
						break;
					}
				}
			}
			exit = 0;
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
			exit = 0;
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '4' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
//			XDpTxSs_ReportVtcInfo(&DpTxSsInst);
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

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			sub_help_menu ();
			break;

		default:
			break;

		} //end of switch (CmdKey[0])
	}
}


u8 rx_unplugged = 0;
u8 rx_trained = 0;
extern u8 aud_info_rcvd;
u8 strm_start = 1;
u8 AudioStream;

void pt_loop(){
	int i;
	u32 Status;
	u8 exit = 0;
	u32 user_lane_count;
	u32 user_link_rate;
	u32 data, addr;
    u32 line_rst = 0;
	int count;

	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 Edid_org[128], Edid1_org[128];

	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate
	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
	user_config.user_pattern= C_VideoUserStreamPattern[1];
	user_config.user_format = XVIDC_CSF_RGB;
	xilInfoFrame = 0; // initialize
    XAxisScr_MiPortEnable (&axis_switch, 0, 0);
    XAxisScr_RegUpdateEnable (&axis_switch);
    DpRxSs_Setup();

	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDpTxSs_Stop(&DpTxSsInst);
	//isue HPD
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_HPD_INTERRUPT,0xFBB80001);
    XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);
      //Clearing the interrupt before starting
//    XDpTxSs_Stop(&DpTxSsInst);
  	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
	sub_help_menu_pt ();

	while (exit == 0) { // for menu loop
		if (tx_is_reconnected == 1 && DpRxSsInst.link_up_trigger == 1) {
			aud_started = 0;
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			start_tx_after_rx(strm_start, 0);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1 && DpRxSsInst.link_up_trigger == 1 && tx_is_up == 1){
			tx_is_up = 0;
			aud_started = 0;
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, 0);
			Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
			if (Status != XST_SUCCESS) {
				tx_is_up = 0;
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK, 0xFFF);

				DpTxSubsystem_Start(&DpTxSsInst, Msa);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK, 0x0);
				tx_is_up = 1;
			}
		} else {
			hpd_pulse_con_event = 0;
		}

		if (tx_is_up == 1 && aud_started == 0 && aud_info_rcvd == 1) {
			count++;
			if (count == 100) {
				// get TX SS Aud out of reset
				Xil_Out32(TX_AUD_RST_BASE, 0x1);
			} else if (count > 200) {
				start_audio_passThrough();
				usleep(100000);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x1);
				usleep(100000);
				Xil_Out32(TX_AUD_RST_BASE, 0x3);
				xil_printf ("Audio Started %d...\r\n",AudioStream);
				aud_started = 1;

			}
		} else {
			count = 0;
		}

        if (rx_unplugged == 1) {
                 xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
                 unplug_proc ();
        } else if(DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0){
        	tx_is_up = 0;
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
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_ENABLE, 0x0);
			XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
					&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);
		}

		if(DpRxSsInst.no_video_trigger == 1){
			frameBuffer_stop(Msa);
			DpRxSsInst.no_video_trigger = 0;
		}

		//Pass-through Handling
		//rx_trained == 1 needed as safety measure to start tx
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT && (rx_trained == 1)){

			//restoring timeout
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CDR_CONTROL_CONFIG,
							XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);

			DpRxSsInst.no_video_trigger = 0;
			//VBLANK Management
			DpRxSsInst.VBlankCount = 0;
			XDp_RxDtgDis(DpRxSsInst.DpPtr);
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
			XDp_RxInterruptDisable1(DpRxSsInst.DpPtr,
					0x00010410);

			XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK |
					XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK);
			XDpRxSs_Mst_AudioDisable (&DpRxSsInst);
			//RX is always in 4 PPC mode
			XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x4);
            XDp_RxDtgEn(DpRxSsInst.DpPtr);
			start_tx_after_rx (strm_start, 0);
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
			exit = 0;
			while (exit == 0) {
			CmdKey[0] = 0;
			Command = 0;
			CmdKey[0] = inbyte_local();
			if(CmdKey[0]!=0){
				Command = (int)CmdKey[0];
				Command = Command -48;
				xil_printf ("Command is %d\r\n", Command);
				switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
//					   sub_help_menu ();
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

				        XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
				                        XDP_RX_INTERRUPT_MASK_TP1_MASK |
				                        XDP_RX_INTERRUPT_MASK_TP2_MASK |
				                        XDP_RX_INTERRUPT_MASK_TP3_MASK|
				                        XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
				                        XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
				                        XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK |
										XDP_RX_INTERRUPT_MASK_VCP_ALLOC_MASK |
										XDP_RX_INTERRUPT_MASK_VCP_DEALLOC_MASK |
										XDP_RX_INTERRUPT_MASK_DOWN_REPLY_MASK |
										XDP_RX_INTERRUPT_MASK_DOWN_REQUEST_MASK);

				        XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
				                        XDP_RX_INTERRUPT_MASK_TP4_MASK|
				                        XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
				                        XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
				                        XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

						// Disabling TX interrupts
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
						XDpTxSs_Stop(&DpTxSsInst);
						XDpRxSs_SetLinkRate(&DpRxSsInst, user_link_rate);
						XDpRxSs_SetLaneCount(&DpRxSsInst, user_lane_count);
						exit = 1;
					}
					}
			}
			}
			exit = 0;
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
			unplug_proc();

			/*HPD is held low for 2sec for gpu to recover*/
	        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
	                     XDP_RX_LINK_ENABLE, 0x0);
	        usleep(2000000);
	        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
	                     XDP_RX_LINK_ENABLE, 0x1);

			// Disabling TX interrupts
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
			XDpTxSs_Stop(&DpTxSsInst);
//			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_HPD_INTERRUPT,0xFFFF0001/*0xFBB80001*/);
			xil_printf("\r\n- HPD Toggled for retraining! -\n\r");
			break;

		case '4':
			line_rst = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_LINE_RESET_DISABLE);
			xil_printf ("Line reset was %d\r\n",line_rst & 0x1);
			if (line_rst & 0x1) {
				line_rst &= 0xFFFFFFFE;
			} else {
				line_rst |= 0xFFFFFFFF;
			}
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_LINE_RESET_DISABLE, line_rst);

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

			tx_is_up = 0;
			strm_start = 1;
			Xil_Out32(TX_AUD_RST_BASE, 0x0);
			start_tx_after_rx (strm_start, 0);
			aud_started = 0;

			break;

		case 'b' :

			tx_is_up = 0;
			strm_start = 2;
			Xil_Out32(TX_AUD_RST_BASE, 0x0);
			start_tx_after_rx (strm_start, 0);
			aud_started = 0;

			break;

		case 'c' :

			tx_is_up = 0;
			strm_start = 3;
			Xil_Out32(TX_AUD_RST_BASE, 0x0);
			start_tx_after_rx (strm_start, 0);
			aud_started = 0;

			break;

		case 'd' :

			tx_is_up = 0;
			strm_start = 4;
			Xil_Out32(TX_AUD_RST_BASE, 0x0);
			start_tx_after_rx (strm_start, 0);
			aud_started = 0;
			break;

		case 'z' :
			sub_help_menu_pt ();
			break;

		default :
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

extern u8 invalid_stream;

void start_tx_after_rx(u8 stream_id, u8 only_tx) {
	u32 mvid_rx;
	u32 nvid_rx;
	u32 Status;
	// Get incoming pixel frequency at here
	u32 recv_clk_freq;
	// Re-calculating Mvid/Nvid based on 5.4Gbps
	u32 nvid_tx;
	u32 mvid_tx;
	user_config_struct user_config;
	u8 max_cap_org = 0;
	u8 max_cap_lanes = 0;
	u8 monitor_8K=0;
	u8 AudioStream_a;
	u8 num_stream = 0;
	int aud_timeout = 0;
	u8 stream1_is_invalid = 0;
	u8 stream2_is_invalid = 0;
	u8 stream3_is_invalid = 0;
	u8 stream4_is_invalid = 0;
	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes);
	u8 rData = 0;

	if (only_tx == 0) {
		XAxisScr_MiPortDisableAll(&axis_switch);
		frameBuffer_stop_wr();
		usleep(1000000);
		//Detect resolution on each stream
		for (num_stream=0;num_stream<4;num_stream++) {
			Dppt_DetectResolution(DpRxSsInst.DpPtr, num_stream+1, Msa, num_stream+1);
			if (invalid_stream && num_stream==0) {
				stream1_is_invalid = 1;
			}
			if (invalid_stream && num_stream==1) {
				stream2_is_invalid = 1;
			}
			if (invalid_stream && num_stream==2) {
				stream3_is_invalid = 1;
			}
			if (invalid_stream && num_stream==3) {
				stream4_is_invalid = 1;
			}
		}
		//Set linereset enable/disable on each stream
		for (num_stream=0;num_stream<4;num_stream++) {
			XDp_RxSetLineReset(DpRxSsInst.DpPtr,num_stream+1);
		}
		usleep (400000);
		//setting a valid stream to start TX
		if (stream1_is_invalid && stream_id==1) {
			if (!stream2_is_invalid) {
				stream_id = 2;
			} else if (!stream3_is_invalid) {
				stream_id = 3;
			} else if (!stream4_is_invalid) {
				stream_id = 4;
			} else {
				xil_printf ("None of the RX streams has a valid video\r\n");
			}
		}

		Dppt_DetectResolution(DpRxSsInst.DpPtr, stream_id, Msa, 1);
		XDpRxSs_Mst_AudioDisable (&DpRxSsInst);
		frameBuffer_start_wr(Msa, 0);
		XAxisScr_MiPortEnable (&axis_switch, 0, stream_id-1);
		XAxisScr_RegUpdateEnable (&axis_switch);
	}

	// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
					1, &rData);

	if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_810GBPS) {
	// if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
	if(rData & 0x80){
			// read maxLineRate
			XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
			if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
					monitor_8K = 1;
					max_cap_org = 0x1E;
//					xil_printf ("Monitor is 8.1 capable\r\n");
			} else {
				    max_cap_org = 0x14;
//					xil_printf ("Monitor is not 8.1 capable\r\n");
			}
	}
	}


// if monitor does not support 8.1G then calc MVID, NVID for 5.4
	if (monitor_8K == 0) {
		if(DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_810GBPS) {

		mvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MVID + (0x40*(stream_id-1)));
		nvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_NVID + (0x40*(stream_id-1)));

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

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
	frameBuffer_stop_rd();



	//Selecting the Audio Stream
#if (DEFAULT_STREAM == 0)
		xil_printf ("Choose the RX stream that has valid audio (1,2,3 or 4?)\r\n");
		while (1) {
			AudioStream_a = XUartPs_RecvByte_NonBlocking();
			AudioStream = AudioStream_a - 48;
			if (((AudioStream) > 0) && ((AudioStream) < 5)) {
				xil_printf ("You selected stream %d\r\n",AudioStream);
				break;
			}
		}
#else
		AudioStream = stream_id;
#endif


	user_config.user_bpc = Msa[0].BitsPerColor;
	user_config.user_pattern = 0; /*pass-through (Default)*/
	user_config.VideoMode_local = 0;//VmId;
	/*Check component Format*/
	if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XVIDC_CSF_YCRCB_422 + 1;
	}else if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XVIDC_CSF_YCRCB_444 + 1;
	}else
		user_config.user_format = XVIDC_CSF_RGB + 1;

	sink_power_cycle();

	//Setting to 0x6 improves the perfomance on some
	//Dell monitors, which otherwise give multiple HPD pulse
	XDpTxSs_SetLinkRate(&DpTxSsInst, XDPTXSS_LINK_BW_SET_162GBPS);
	set_vphy(max_cap_org);
	start_tx(max_cap_org, DpRxSsInst.UsrOpt.LaneCount, user_config, Msa);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0x0);
	frameBuffer_start_rd(Msa, 0);
	tx_is_up = 1;
	aud_started = 0;
	XDpRxSs_Mst_AudioDisable (&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	aud_timeout = 0;
	aud_info_rcvd = 0;
	Xil_Out32(TX_AUD_RST_BASE, 0x0); //all in reset
	usleep(100000);
	XDpRxSs_Mst_AudioEnable (&DpRxSsInst, AudioStream);
	//giving some time to for AudioInfoframe to be available
	while (aud_timeout < 100) {
		//wait till info packet is received
		aud_timeout++;
		usleep(10000);
	}
}


/* Audio passThrough setting */
void start_audio_passThrough(){

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x00000);
	xilInfoFrame->audio_channel_count = AudioinfoFrame.audio_channel_count;
	xilInfoFrame->audio_coding_type = AudioinfoFrame.audio_coding_type;
	xilInfoFrame->channel_allocation = AudioinfoFrame.channel_allocation;
	xilInfoFrame->downmix_inhibit = AudioinfoFrame.downmix_inhibit;
	xilInfoFrame->info_length = AudioinfoFrame.info_length;
	xilInfoFrame->level_shift = AudioinfoFrame.level_shift;
	xilInfoFrame->sample_size = AudioinfoFrame.sample_size;
	xilInfoFrame->sampling_frequency = AudioinfoFrame.sampling_frequency;
	xilInfoFrame->type = AudioinfoFrame.type;
	xilInfoFrame->version = AudioinfoFrame.version;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CHANNELS, 0x1);
#if SEND_AIF
	usleep(10000);
	XDpTxSs_SendAudioInfoFrame(&DpTxSsInst, xilInfoFrame);
	usleep(30000);
#endif
}

void unplug_proc (void) {
        rx_trained = 0;
        rx_unplugged = 0;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
        frameBuffer_stop(Msa);
        XDpRxSs_Mst_AudioDisable (&DpRxSsInst);
        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x00000);
        DpRxSsInst.VBlankCount = 0;
        DpRxSs_Setup();
	DpRxSsInst.link_up_trigger = 0;
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
        AudioinfoFrame.frame_count = 0;
        aud_info_rcvd = 0;
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
}
