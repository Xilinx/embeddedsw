/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc. All rights reserved.
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
extern XDpTxSs DpTxSsInst;	/* The DPTX Subsystem instance.*/
extern XVphy VPhyInst;	/* The DPRX Subsystem instance.*/
extern XTmrCtr TmrCtr; /* Timer instance.*/
extern Video_CRC_Config VidFrameCRC;
extern int tx_is_reconnected;
extern u8 hpd_pulse_con_event;

static char inbyte_local(void);
static u32 xil_gethex(u8 num_chars);
static char XUartPs_RecvByte_NonBlocking(void);

void resolution_help_menu(void)
{
xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - -"
		" - - - - - - - - - -\r\n"
"-                            Select an Option for Resolutio"
		"n                                      -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - "
		"- - - - - \r\n"
"0 640x480_60_P    |   1 800x600_60_P      |   2 1280x720_60_P  |   3 1920x1080_60_P \r\n"
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
"2 - Change Pattern \r\n"
"3 - Display MSA Values for Tx\r\n"
"z - Display this Menu again\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
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
			set_vphy(XDPTXSS_LINK_BW_SET_540GBPS);
			start_tx(XDPTXSS_LINK_BW_SET_540GBPS, 4, user_config);
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

		case '3' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
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
