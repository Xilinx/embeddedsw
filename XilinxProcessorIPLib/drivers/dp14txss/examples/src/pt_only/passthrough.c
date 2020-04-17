/*******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file passthrough.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* </pre>
*
******************************************************************************/


#include "main.h"
#include "tx.h"
#include "rx.h"


u8 UpdateBuffer[sizeof(u8) + 16];
u8 WriteBuffer[sizeof(u8) + 16];
u8 ReadBuffer[16];
u16 tx_count_delay = 0;
int tx_aud_started = 0;
volatile int i2s_started = 0;

extern u8 start_i2s_clk;
extern u32 appx_fs_dup;
XV_frmbufrd_Config frmbufrd_cfg;
XV_frmbufwr_Config frmbufwr_cfg;

void unplug_proc (void);
void i2s_stop_proc (void);
void audio_init (void);
void audio_start_rx (void);
void audio_start_tx (void);
void dprx_tracking (void);
void dptx_tracking (void);
void start_tx_after_rx(void);

int ConfigFrmbuf_rd(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr);
int ConfigFrmbuf_rd_trunc(u32 offset);
int ConfigFrmbuf_wr(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr);

void DpPt_Main(void);
void operationMenu(void);
void Dppt_DetectAudio (void);
int Dppt_DetectResolution(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);
int Dppt_DetectColor(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);



void resetIp_rd(void);
void resetIp_wr(void);

void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format);

void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);
void edid_change(int page);
char inbyte_local(void);
void pt_help_menu();
void select_rx_quad(void);
void DpPt_LaneLinkRateHelpMenu(void);

void start_audio_passThrough();

u8 edid_page;
volatile u8 tx_after_rx = 0;
volatile u8 rx_aud = 0;
u8 downshift4K = 0;
u8 LineRate_init_tx;
u8 LaneCount_init_tx;
XDpTxSs_MainStreamAttributes Msa[4];
u8 rx_all_detect = 0;
user_config_struct user_config;
XVidC_VideoMode VmId;

extern u8 rx_unplugged;
volatile u8 rx_trained = 0;
u8 rx_aud_start = 0;

extern lane_link_rate_struct lane_link_table[];
extern u32 StreamOffset[4];
volatile u8 tx_done = 0;
volatile u8 i2s_tx_started = 0;
u8 rx_and_tx_started = 0;
u8 status_captured = 0;
u8 aes_sts[24];
int filter_count_b = 0;
int track_msa = 0;
u8 Edid_org[128];
u8 Edid1_org[128];
u8 Edid2_org[128];
int filter_count = 0;
extern u8 training_lost;
u8 audio_info_avail = 0;
int track_color = 0;
u32 aud_start_delay = 0;
u32 line_rst = 0;

void DpPt_Main(void){
	u32 Status;
	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;

	u8 edid_monitor[384];
	u8 exit;
    u8 *LaneCount_tx = 0x4;
    u8 *LineRate_tx = 0x14;

	char CmdKey[2];
	unsigned int Command;

	/*Set EDID to be default*/
	edid_page = 0;

	// This is for EDID setup
	u8 connected = 0;
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
		xil_printf("Please connect a DP Monitor to start the "
				"application !!!\r\n");
		connected = 1;
		}
	}
#if PHY_COMP
	/* Load Custom EDID */
	LoadEDID();
#endif



	// disabling this when compliance is enabled
#if !PHY_COMP

	//Waking up the monitor
	sink_power_cycle();

	//reading the first block of EDID
	if (XDpTxSs_IsConnected(&DpTxSsInst)) {
		XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);

		u8 Sum = 0;
		for (int i = 0; i < 128; i++) {
			Sum += Edid_org[i];
		}
		if(Sum != 0){
			xil_printf("Wrong EDID was read\r\n");
		}

		//reading the second block of EDID
		if(Edid_org[126] > 0)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
		if(Edid_org[126] > 1)
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2_org, 2);

		xil_printf("Reading EDID contents of the DP Monitor..\r\n");

		int i, j;

		switch (Edid_org[126]){
		case 0:
			for(i=0; i<128; i++)
				edid_monitor[i] = Edid_org[i];
			for(i=0; i<128; i++)
				edid_monitor[i+128] = 0;
			for(i=0; i<128; i++)
				edid_monitor[i+256] = 0;
			break;
		case 1:
			for(i=0; i<128; i++)
				edid_monitor[i] = Edid_org[i];
			for(i=0; i<128; i++)
				edid_monitor[i+128] = Edid1_org[i];
			for(i=0; i<128; i++)
				edid_monitor[i+256] = 0;
			break;
		case 2:
			for(i=0; i<128; i++)
				edid_monitor[i] = Edid_org[i];
			for(i=0; i<128; i++)
				edid_monitor[i+128] = Edid1_org[i];
			for(i=0; i<128; i++)
				edid_monitor[i+256] = Edid2_org[i];
			break;
		}

		for(i=0;i<(384*4);i=i+(16*4)){
			for(j=i;j<(i+(16*4));j=j+4){
				XDp_WriteReg (VID_EDID_BASEADDR,
				j, edid_monitor[(i/4)+1]);
			}
		}
		for(i=0;i<(384*4);i=i+4){
			XDp_WriteReg (VID_EDID_BASEADDR,
				i, edid_monitor[i/4]);
		}
	}

	Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							XDP_DPCD_MAX_LINK_RATE,  1, LineRate_tx);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
							XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_tx);

//	if (Status == XST_SUCCESS) {
//		xil_printf ("Monitor Capabilities are --> Link rate: %x, "
//				"Lane Count: %d \r\n",LineRate_tx, LaneCount_tx);
//	}

#endif


	/* Set Link rate and lane count to maximum */
	/* The RX is always set for Max capability of 5.4G and the
	 * extended capability bit is set.
	 * DP1.4 Sources are supposed to read the extended capability bit
	 * and decide whether the sink is 8.1G capable or not.
	 */
	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/* Start DPRX Subsystem set */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return;
	}

	/* Setup DPRX SS, left to the user for implementation */
	DpRxSs_Setup();

	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
					&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);

	// Reading and clearing any residual interrupt on TX
	// Also Masking the interrupts
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			0x140);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);


	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

	/* Initializing the Audio related IPs. The AXIS Switches are programmed
	 * based on the "I2S_AUDIO" param in main.h
	 * The Audio Clock Recovery Module is programmed in fixed mode
	 */
#if ENABLE_AUDIO
	audio_init();
#endif
	//resetting AUX logic. Needed for some Type C based connectors
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);

	pt_help_menu();

	while (1){
		UserInput = XUartPs_RecvByte_NonBlocking();
		if(UserInput!=0){
			xil_printf("UserInput: %c\r\n",UserInput);

			switch(UserInput){
			case '1':
				DpPt_LaneLinkRateHelpMenu();
				exit = 0;
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
						   break;

						   default :
						xil_printf("You have selected command '%c'\r\n",
																CmdKey[0]);
							if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
								Command = CmdKey[0] -'a' + 10;
								exit = 1;
							}

							else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								exit = 1;
							}
							else if (Command >= 58 || Command <= 47) {
								DpPt_LaneLinkRateHelpMenu();
								exit = 0;
								break;
							}
							xil_printf ("\r\nSetting LineRate:%x  "
									"LaneCounts:%x\r\n",
									lane_link_table[Command].link_rate,
									lane_link_table[Command].lane_count);

							unplug_proc();
							// setting new capability at here
								// clear the interrupt status
							XDp_ReadReg(
							DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_STATUS);
								// mask out interrupt
							XDp_WriteReg(
							  DpTxSsInst.DpPtr->Config.BaseAddr,
							  XDP_TX_INTERRUPT_MASK, 0xFFF);

							frameBuffer_stop();
							XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
										0x7FF8FFFF);
							// Disabling TX interrupts
							XDpTxSs_Stop(&DpTxSsInst);

								// set new line rate
							DpRxSsInst.DpPtr->Config.MaxLinkRate =
									lane_link_table[Command].link_rate;
							XDpRxSs_SetLinkRate(&DpRxSsInst,
									lane_link_table[Command].link_rate);

								// set new lane counts
							DpRxSsInst.DpPtr->Config.MaxLaneCount =
									lane_link_table[Command].lane_count;
							XDpRxSs_SetLaneCount(&DpRxSsInst,
									lane_link_table[Command].lane_count);

							// issuing HPD for re-training
							XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
							break;
						}
					}
				}
				break;

			case '2':
			//	debug_info();
				xil_printf (
			"==========MCDP6000 Debug Data===========\r\n");
				xil_printf("0x0700: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0700));
				xil_printf("0x0704: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0704));
				xil_printf("0x0754: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0754));
				xil_printf("0x0B20: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0B20));
				xil_printf("0x0B24: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0B24));
				xil_printf("0x0B28: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0B28));
				xil_printf("0x0B2C: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
								I2C_MCDP6000_ADDR, 0x0B2C));

				xil_printf (
					"==========RX Debug Data===========\r\n");
				XDpRxSs_ReportLinkInfo(&DpRxSsInst);
				XDpRxSs_ReportMsaInfo(&DpRxSsInst);
				xil_printf (
				"==========TX Debug Data===========\r\n");
				XDpTxSs_ReportMsaInfo(&DpTxSsInst);
				XDpTxSs_ReportLinkInfo(&DpTxSsInst);
				XDpTxSs_ReportVtcInfo(&DpTxSsInst);

				break;

				case '3':
					// Disabling TX interrupts
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFF);
					unplug_proc();
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr,  0x80000000);
					XDpTxSs_Stop(&DpTxSsInst);
					XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
					xil_printf("\r\n- HPD Toggled for 5ms! -\n\r");
					break;

				case '4': // re-start Tx side
					LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
					LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;

					user_config.user_bpc = Msa[0].BitsPerColor;
					user_config.VideoMode_local = VmId;
					user_config.user_pattern = 0; /*pass-through (Default)*/
					user_config.user_format = Msa[0].ComponentFormat;
#if ENABLE_AUDIO
#if !I2S_AUDIO
					XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
#endif
#endif
					frameBuffer_stop_wr();
					frameBuffer_stop_rd();
					//Waking up the monitor
					sink_power_cycle();

#ifndef versal
					XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
#endif
					// This configures the vid_phy for line rate to start with
					config_phy(LineRate_init_tx);
					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					tx_after_rx = 1;
					DpTxSsInst.no_video_trigger = 1;
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_AUDIO_CONTROL, 0x0);

					break;

				case '5':
					xil_printf ("Stopping WR FB and starting..\r\n ");
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDp_RxDtgEn(DpRxSsInst.DpPtr);
					frameBuffer_stop_wr();
//					Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa,
//									DpRxSsInst.link_up_trigger);
//					frameBuffer_start_wr();
					break;

				case 'c':
					xil_printf ("========== Rx CRC===========\r\n");
					xil_printf ("Rxd Hactive =  %d\r\n",
							((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,
								0xC)&0xFFFF) + 1) *
								((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x0)) & 0x7));

					xil_printf ("Rxd Vactive =  %d\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0xC)>>16);
					xil_printf ("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x0));
					xil_printf ("CRC - R/Cr   =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4)&0xFFFF);
					xil_printf ("CRC - G/Y  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4)>>16);
					xil_printf ("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x8)&0xFFFF);
					xil_printf ("========== Tx CRC===========\r\n");
					xil_printf ("Txd Hactive =  %d\r\n",
						((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)&0xFFFF)+ 1)
						  * ((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0)) & 0x7));

					xil_printf ("Txd Vactive =  %d\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)>>16);
					xil_printf ("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0));
					xil_printf ("CRC - R/Cr   =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)&0xFFFF);
					xil_printf ("CRC - G/Y  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)>>16);
					xil_printf ("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x8)&0xFFFF);

				break;

				case 'm':
					xil_printf (
				"==========MCDP6000 Debug Data===========\r\n");
					xil_printf("0x0700: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0700));
					xil_printf("0x0704: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0704));

					xil_printf("0x0754: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0754));
					xil_printf("0x0B20: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0B20));
					xil_printf("0x0B24: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0B24));
					xil_printf("0x0B28: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0B28));
					xil_printf("0x0B2C: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x0B2C));

					xil_printf(
					"0x1294: %08x  0x12BC: %08x  0x12E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1294),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x12BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x12E4));
					xil_printf(
					"0x1394: %08x  0x13BC: %08x  0x13E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1394),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x13BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x13E4));
					xil_printf(
					"0x1494: %08x  0x14BC: %08x  0x14E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1494),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x14BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x14E4));
					xil_printf(
					"0x1594: %08x  0x15BC: %08x  0x15E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1594),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x15BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x15E4));

					break;

				case 'n':
                                        // This can be used to clone the edid when monitor is changed
                                        // Ensure to unplug/plug RX cable for changes to take effect

					for (int i = 0; i < 128; i++) {
						Edid_org[i] = 0;
						Edid1_org[i] = 0;
						Edid2_org[i] = 0;
					}

					//Waking up the monitor
					sink_power_cycle();

					//reading the first block of EDID
					if (XDpTxSs_IsConnected(&DpTxSsInst)) {
						XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);

						u8 Sum = 0;
						for (int i = 0; i < 128; i++) {
							Sum += Edid_org[i];
						}
						if(Sum != 0){
							xil_printf("Wrong EDID was read\r\n");
						}

						//reading the second block of EDID
						if(Edid_org[126] > 0)
							XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);
						if(Edid_org[126] > 1)
							XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid2_org, 2);

						xil_printf("Reading EDID contents of the DP Monitor..\r\n");

						int i, j;

						switch (Edid_org[126]){
						case 0:
							for(i=0; i<128; i++)
								edid_monitor[i] = Edid_org[i];
							for(i=0; i<128; i++)
								edid_monitor[i+128] = 0;
							for(i=0; i<128; i++)
								edid_monitor[i+256] = 0;
							break;
						case 1:
							for(i=0; i<128; i++)
								edid_monitor[i] = Edid_org[i];
							for(i=0; i<128; i++)
								edid_monitor[i+128] = Edid1_org[i];
							for(i=0; i<128; i++)
								edid_monitor[i+256] = 0;
							break;
						case 2:
							for(i=0; i<128; i++)
								edid_monitor[i] = Edid_org[i];
							for(i=0; i<128; i++)
								edid_monitor[i+128] = Edid1_org[i];
							for(i=0; i<128; i++)
								edid_monitor[i+256] = Edid2_org[i];
							break;
						}

						for(i=0;i<(384*4);i=i+(16*4)){
							for(j=i;j<(i+(16*4));j=j+4){
								XDp_WriteReg (VID_EDID_BASEADDR,
								j, edid_monitor[(i/4)+1]);
							}
						}
						for(i=0;i<(384*4);i=i+4){
							XDp_WriteReg (VID_EDID_BASEADDR,
								i, edid_monitor[i/4]);
						}
					}

					xil_printf ("Please unplug/plug DP RX cable for changes to take effect\r\n");

					break;

				case 'e':
#ifndef versal
	//                    XDpRxSs_ReportDp159BitErrCount(&DpRxSsInst);
					ReadVal = XVphy_ReadReg(
							VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane1, Lane0] "
							"= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);
					ReadVal = XVphy_ReadReg(
							VIDPHY_BASEADDR,XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);
					xil_printf("Video PHY(8B10B): Error Counts [Lane3, Lane2] "
							"= [%d, %d]\n\r", (ReadVal>>16), ReadVal&0xFFFF);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
					xil_printf ("Lane0 (Lower) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
					xil_printf ("Lane0 (Upper) is %d,\r\n", DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
					xil_printf ("Lane1 (Lower) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
					xil_printf ("Lane1 (Upper) is %d,\r\n", DrpVal);;


					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
					xil_printf ("Lane2 (Lower) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
					xil_printf ("Lane2 (Upper) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
					xil_printf ("Lane3 (Lower) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
					xil_printf ("Lane3 (Upper) is %d,\r\n", DrpVal);


					xil_printf ("==========MCDP6000 Debug Data===========\r\n");
					XDpRxSs_MCDP6000_Read_ErrorCounters(&DpRxSsInst,
							I2C_MCDP6000_ADDR);
					xil_printf("0x0754: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0754));
					xil_printf("0x0B20: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0B20));
					xil_printf("0x0B24: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0B24));
					xil_printf("0x0B28: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0B28));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0B2C));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x061C));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0504));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							&DpRxSsInst, I2C_MCDP6000_ADDR, 0x0604));
#endif
				break;

				case 'q' :
                                        line_rst = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_LINE_RESET_DISABLE);
                                        xil_printf ("Line reset was %d\r\n",line_rst & 0x1);
                                        if (line_rst & 0x1) {
                                            line_rst &= 0xFFFFFFFE;
                                        } else {
                                            line_rst |= 0xFFFFFFFF;
                                        }
                                        XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_LINE_RESET_DISABLE, line_rst);

					break;

				case 's':
					xil_printf("DP Link Status --->\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					break;

				case '7':
#ifndef versal
					xil_printf("Video PHY Config/Status --->\r\n");
					xil_printf(" RCS (0x10) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_REF_CLK_SEL_REG));
					xil_printf(" PR  (0x14) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_PLL_RESET_REG));
					xil_printf(" PLS (0x18) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,
							  XVPHY_PLL_LOCK_STATUS_REG));
					xil_printf(" TXI (0x1C) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_TX_INIT_REG));
					xil_printf(" TXIS(0x20) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,
							  XVPHY_TX_INIT_STATUS_REG));
					xil_printf(" RXI (0x24) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_RX_INIT_REG));
					xil_printf(" RXIS(0x28) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,
							  XVPHY_RX_INIT_STATUS_REG));


					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_CPLL_FBDIV,&DrpVal);
					xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) ="
							" 0x%x, Val = 0x%x\r\n",
							XVPHY_DRP_CPLL_FBDIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_CPLL_REFCLK_DIV,&DrpVal);
					xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV) "
				"= 0x%x, Val = 0x%x\r\n",
							XVPHY_DRP_CPLL_REFCLK_DIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_RXOUT_DIV,&DrpVal);
					xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV) = 0x%x, "
				"Val = 0x%x\r\n",
							XVPHY_DRP_RXOUT_DIV,DrpVal);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_TXOUT_DIV,&DrpVal);
					xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV) = 0x%x, "
				"Val = 0x%x\r\n",
							XVPHY_DRP_TXOUT_DIV,DrpVal);
#endif

				case 'd':
					select_rx_quad();
					exit = 0;
					while (exit == 0) {
						CmdKey[0] = 0;
						Command = 0;
						CmdKey[0] = inbyte_local();
						if(CmdKey[0]!=0){
							Command = (int)CmdKey[0];

							switch  (CmdKey[0]){
							case 'x' :
								pt_help_menu();
								exit = 1;
							break;

							default :
							xil_printf("You have selected command '%c'\r\n",
														CmdKey[0]);
							if(CmdKey[0] == 'x'){
								exit = 1;
							}else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								exit = 0;
							}

							if((Command>=0)&&(Command<4)){
								u32 dp_msa_hres = Msa[0].Vtm.Timing.HActive;
								u32 dp_msa_vres = Msa[0].Vtm.Timing.VActive;
								u8 BPC = Msa[0].BitsPerColor;
								u8 pixel = Msa[0].UserPixelWidth;
								switch(Command){
								case 0:
									ConfigFrmbuf_rd_trunc(0);
									break;

								case 1:
									ConfigFrmbuf_rd_trunc(((dp_msa_hres - 3840)
											* BPC) / pixel);

									break;

								case 2:
									ConfigFrmbuf_rd_trunc((dp_msa_hres *
											(dp_msa_vres - 2160))* BPC / pixel);
									break;

								case 3:
									ConfigFrmbuf_rd_trunc(((dp_msa_hres *
											(dp_msa_vres - 2160)) +
											(dp_msa_hres - 3840))* BPC / pixel);
									break;
								}
							}else{
								xil_printf("!!!Warning: You have selected "
										"wrong option for Quad selection "
										"=%d \n\r"
										,Command);
								break;
							}
							}// end of switch
						} // end of key input wait
					} // end of while

					break;

				case 'u':
						xil_printf(
					"\r\n Give 4 bit Hex value of base register 0x");
						ReadVal = xil_gethex(4);
						xil_printf("\r\n");
						xil_printf("0x%x: %08x\n\r", ReadVal,
							XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
							I2C_MCDP6000_ADDR, ReadVal));
						break;

				case 'o':
						xil_printf(
					"\r\n Give 4 bit Hex value of base register 0x");
						ReadVal = xil_gethex(4);
						xil_printf("\r\n");
						xil_printf(
					"\r\n Give 8 bit Hex value of write data 0x");
						//data = xil_gethex(8);
						XDpRxSs_MCDP6000_SetRegister(
								&DpRxSsInst,
							I2C_MCDP6000_ADDR,
							ReadVal,
							xil_gethex(8));
						xil_printf("\r\n");

						break;

				case 'r':
					xil_printf(
						"Reset Video DTG in DisplayPort Controller...\r\n");
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDp_RxDtgEn(DpRxSsInst.DpPtr);
					break;

				case '.':
					pt_help_menu();
					break;
//
//				case ',':
//						tmp = 	Xil_In32(0xFD1A0100);
//						xil_printf("tmp:%x\r\n", tmp);
//						u32 tmp2 = tmp | 0x600;
//						Xil_Out32(0xFD1A0100, tmp2);
//						usleep(10000);
//						Xil_Out32(0xFD1A0100, tmp);
//						tmp = 	Xil_In32(0xFD1A0100);
//						xil_printf("tmp:%x\r\n", tmp);
//					break;
//
//				case '/':
//						tmp = 	Xil_In32(0xFD380014);
//						xil_printf("tmp:%x\r\n", tmp);
//						u32 tmp3 = tmp | 0x1000;
//						Xil_Out32(0xFD380014, tmp3);
//	//                		usleep(10000);
//	//                		Xil_Out32(0xFD380014, tmp);
//	//                		tmp = 	Xil_In32(0xFD380014);
//	//                		printf("tmp:%x\r\n", tmp);
//					break;

#if PHY_COMP
				case 't':
					tx_after_rx = 1;
					break;
#endif
				case 'x':
					DpRxSsInst.link_up_trigger = 0;

					// disabling Rx
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDpRxSs_Reset(&DpRxSsInst);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_LINK_ENABLE, 0x0);
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
												0xFFFFFFFF);
					DpRxSsInst.VBlankCount = 0;

					// disabling Tx
					XDpTxSs_Stop(&DpTxSsInst);
					XScuGic_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
					XScuGic_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

					Vpg_Audio_stop();
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_ENABLE, 0x0);
					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,0xFFF);


					operationMenu();
					return;

				default :
					pt_help_menu();
				break;
			}
		}//end if


		// Tx side process
		// When there is no video on TX, stop the FB and I2S RX
		if(DpTxSsInst.no_video_trigger == 1){ // stop frameBuffer if Tx is lost
			frameBuffer_stop_rd();
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			XDpTxSs_Stop(&DpTxSsInst);
			DpTxSsInst.no_video_trigger = 0;
			tx_done = 0;
			i2s_started = 0;
			aud_start_delay = 0;
#if ENABLE_AUDIO
			XI2s_Rx_Enable(&I2s_rx, 0);
		} else {
			audio_start_tx();
#endif
			}

		// Check for HPD and HPD Pulse Interrupt triggers
		dptx_tracking();


#if ENABLE_AUDIO
		// The I2S Audio is started once the RX is trained
		audio_start_rx();
#endif

		// Rx and pass-through side process
		dprx_tracking();

		//Wait for few frames to ensure valid video is received
		if (tx_after_rx == 1 && rx_trained == 1 &&
				DpRxSsInst.link_up_trigger == 1) {
		    tx_after_rx = 0;
		    if (track_msa == 1) {
				start_tx_after_rx();
				// It is observed that some monitors do not give HPD
				// pulse. Hence checking the link to re-trigger
				Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
				if (Status != XST_SUCCESS) {
					xil_printf ("^*^");
					hpd_pulse_con(&DpTxSsInst, Msa);
				}
					tx_done = 1;
		    } else {
			tx_done = 0;
			aud_start_delay = 0;
			xil_printf ("Cannot Start TX...\r\n");
		    }
		}

		// This continuously tracks the Maud, Naud values by reading the
		// registers
		if (rx_trained && rx_aud && tx_done) {
			Dppt_DetectAudio();
		}
	}//end while(1)
}



/* Audio passThrough setting */
void start_audio_passThrough(){

	// Program the Infoframe data into TX
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x0);
	usleep(30000);
	XDpTxSs_SendAudioInfoFrame(&DpTxSsInst,xilInfoFrame);
	usleep(30000);

}


/*This function starts the TX after RX. It checks for Monitor capability
 * and based on that it will modify the video.
 * For example, if the RX is trained at 8K, but the monitor is not capable of
 * 8.1G, then the application would only display one quad of 8K as 4K@30
 * It is expected that the Monitor would support 4K@30
 * Similarly, is the received video is 4K@120, then it would modified to 4k@60
 */

void start_tx_after_rx (void) {
	u32 Status;
	u32 dptx_sts=0;

	rx_all_detect = 1;
	VmId = XVidC_GetVideoModeId(
			Msa[0].Vtm.Timing.HActive,
			Msa[0].Vtm.Timing.VActive,
			Msa[0].Vtm.FrameRate,0);

	// check monitor capability
	u8 max_cap_org=0;
	u8 max_cap_lanes=0;
	u8 monitor_8K=0;

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
		}
	}

	LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
	LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;

	user_config.user_bpc = Msa[0].BitsPerColor;
	user_config.user_pattern = 0; /*pass-through (Default)*/

	/*Check component Format*/
	if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XVIDC_CSF_YCRCB_422 + 1;
	}else if(Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XVIDC_CSF_YCRCB_444 + 1;
	}else
		user_config.user_format = XVIDC_CSF_RGB + 1;

	downshift4K = 0;
	// This block is to use with 4K30 monitor.
	if (max_cap_org <= 0x14 || monitor_8K == 0) {
		// 8K resolution will be changed to 4K60
		if(Msa[0].Vtm.Timing.HActive >= 7680 &&
				Msa[0].Vtm.Timing.VActive >= 4320){
			xil_printf("\nMonitor is not capable of displaying 8K resolution."
					   " Displaying only 4K@30 resolution\r\n");
			xil_printf("\nOnly one quad of 4k@30 is displayed.\r\n");

			VmId = XVIDC_VM_3840x2160_30_P;//_RB;
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].
						Vtm.Timing.HActive /= 2;
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].
						Vtm.Timing.VActive /= 2;

			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 =
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 & 0xFE;
			downshift4K = 1;
			// overwrite Rate and Lane counts
			LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
			LaneCount_init_tx = XDPTXSS_LANE_COUNT_SET_4;
		}
		// 4K120 will be changed to 4K60
		else if(Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive *
				Msa[0].Vtm.Timing.VActive > 4096*2160*60){
			xil_printf("\nMonitor is not capable of displaying 4K@120 "
					"resolution. Forcing 4K@30 resolution\r\n");
			// to keep 4Byte mode, it has to be 4K60
			VmId = XVIDC_VM_3840x2160_30_P;//_RB;
			Msa[0].Vtm.FrameRate = 30;
			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
			DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 =
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 & 0xFE;
			downshift4K = 1;
			// overwrite Rate and Lane counts
			LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
			LaneCount_init_tx = XDPTXSS_LANE_COUNT_SET_4;
		}else if(DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_810GBPS){
			// Update Tx LineRate as 5.4Gbps
			LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
			LaneCount_init_tx = XDPTXSS_LANE_COUNT_SET_4;
			// Get Rx side Mvid/Nvid to calculate PixelFrequency
			u32 mvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MVID);
			u32 nvid_rx = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_NVID);

			// Get incoming pixel frequency at here
			u32 recv_clk_freq =
				(((int)DpRxSsInst.UsrOpt.LinkRate*27)*mvid_rx)/nvid_rx;

			// Re-calculating Mvid/Nvid based on 5.4Gbps
			u32 nvid_tx = (XDP_TX_LINK_BW_SET_540GBPS * 27);
			u32 mvid_tx = (recv_clk_freq * nvid_tx * 1000) /
							(XDP_TX_LINK_BW_SET_540GBPS*27);
			nvid_tx *= 1000;

			// Update MVID and NVID at here with bsed on 5.4Gbps
			Msa[0].MVid = mvid_tx;
			Msa[0].NVid = nvid_tx;
		}
	}

	user_config.VideoMode_local = VmId;

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);

	frameBuffer_stop_rd();
	//Waking up the monitor
	sink_power_cycle();

	//This is needed for some monitor to make them happy
	//Setting the linerate to 1.62 improves performance with some
	//Dell models, which otherwise give too many HPDs
	//This is needed especially on multiple unplug/plug
	XDpTxSs_SetLinkRate(&DpTxSsInst, XDP_TX_LINK_BW_SET_162GBPS);

	config_phy(LineRate_init_tx);
	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	if(downshift4K == 0){
		start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, Msa);

	}else{
		start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, 0);
	}

	frameBuffer_start_rd(Msa, downshift4K);
	rx_and_tx_started = 1;

}

void unplug_proc (void) {
	u32 vswing;
	u32 postcursor, value;
	u16 preemp = 0;
	u16 diff_swing = 0;
	i2s_tx_started = 0;
	tx_done = 0;
	aud_start_delay = 0;
	rx_aud = 0;
	tx_after_rx = 0;
	rx_trained = 0;
	tx_started = 0;
    rx_unplugged = 0;
    start_i2s_clk = 0;
#ifdef versal
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR_1, 0, 36864000, TRUE);
#else
    I2cClk_Ps(0, 36864000);
#endif
    DpRxSsInst.VBlankCount = 0;
    audio_info_avail = 0;
    AudioinfoFrame.frame_count = 0;
    AudioinfoFrame.all_count = 0;
    appx_fs_dup = 0;
    XDp_RxDtgDis(DpRxSsInst.DpPtr);
	frameBuffer_stop();

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	XDpTxSs_Stop(&DpTxSsInst);

#ifndef versal
    //setting vswing to 0
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);

    //setting preembphasis to 0
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_PREEMP_DP_L0);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x0);
	XACR_WriteReg (RX_ACR_ADDR, RXACR_MODE, 0x0);
#endif

#if ENABLE_AUDIO
	XDpRxSs_AudioDisable(&DpRxSsInst);
	i2s_stop_proc();
	audio_init();
#endif
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	DpRxSs_Setup();
}

void i2s_stop_proc (void) {
#if ENABLE_AUDIO
    i2s_started = 0;
    tx_aud_started = 0;
    XI2s_Rx_Enable(&I2s_rx, 0);
	XACR_WriteReg (RX_ACR_ADDR, RXACR_ENABLE, 0x0);
#endif
}

void audio_init (void) {

#if ENABLE_AUDIO

#if I2S_AUDIO
	// Programming the AXIS switch to route the Audio to I2S TX/RX
	 XAxisScr_MiPortDisable (&axis_switch_rx, 0);
     XAxisScr_MiPortEnable  (&axis_switch_rx, 1, 0);
     XAxisScr_RegUpdateEnable (&axis_switch_rx);
     XAxisScr_MiPortEnable  (&axis_switch_tx, 0, 1);
     XAxisScr_RegUpdateEnable (&axis_switch_tx);

#else
	// Programming the AXIS switch to bypass the I2S TX/RX
	 XAxisScr_MiPortDisable (&axis_switch_rx, 1);
     XAxisScr_MiPortEnable  (&axis_switch_rx, 0, 0);
     XAxisScr_RegUpdateEnable (&axis_switch_rx);
     XAxisScr_MiPortEnable  (&axis_switch_tx, 0, 0);
     XAxisScr_RegUpdateEnable (&axis_switch_tx);
#endif

//#ifndef versal
     XACR_WriteReg (RX_ACR_ADDR, 0x30, 512); // set to half of I2S TX FIFO Depth
     XACR_WriteReg (RX_ACR_ADDR, 0x34, 15);  // Max limit of +/-
     XACR_WriteReg (RX_ACR_ADDR, 0x38, 3);  // incr, decr granularity
     XACR_WriteReg (RX_ACR_ADDR, 0x3C, 8*384);
     XACR_WriteReg (RX_ACR_ADDR, 0x40, 8); // Averaging time

     XACR_WriteReg (RX_ACR_ADDR, RXACR_MODE, 0x1); // 5 - ctrl loop, 1- no loop
     XACR_WriteReg (RX_ACR_ADDR, RXACR_DIV, 0x40); // divider
     XI2s_Tx_SetSclkOutDiv (&I2s_tx, 48000*I2S_CLK_MULT, 48000);
     XI2s_Tx_Enable(&I2s_tx, 1);
     XI2s_Rx_Enable(&I2s_rx, 0);
//#endif
#endif
}

void audio_start_rx (void) {

#if ENABLE_AUDIO
		if (rx_trained && start_i2s_clk) {
			XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
			XI2s_Rx_Enable(&I2s_rx, 0);
#if I2S_AUDIO
		//Poll for no Block Sync Error and capture the STS
		if ((XI2s_Tx_ReadReg(I2s_tx.Config.BaseAddress,0x14)) && 0x2) {
			//clearing block sync until proper block is received
			XI2s_Tx_WriteReg(I2s_tx.Config.BaseAddress,0x14, 0xFFFFFFFF);
		} else {
			if ((XI2s_Tx_ReadReg(I2s_tx.Config.BaseAddress,0x14)) && 0x5) {
			//No block sync error, assuming Channel Status is updated
				XI2s_Tx_GetAesChStatus(&I2s_tx, aes_sts);
				XI2s_Rx_SetAesChStatus(&I2s_rx, aes_sts);
				status_captured = 1;
				XI2s_Tx_Enable(&I2s_tx, 0);
//            xil_printf ("Channel Status captured from I2S TX to I2S RX\r\n");
			}
		}
#else
		status_captured = 1;
#endif

		// process to start Pass Through Audio and program the Audio pipe
		if (status_captured) {
#ifdef versal
			IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR_1, appx_fs_dup, I2S_CLK_MULT*appx_fs_dup, FALSE);
#else
				I2cClk_Ps(appx_fs_dup, I2S_CLK_MULT*appx_fs_dup);
#endif
				usleep(200000);
				//Reset the MMCM that generates the clock to DAC/ADC
				XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x1);
				XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
#if I2S_AUDIO
				//Giving some time for Si5328 to get accustomed to change
				//Not starting I2S TX here
				usleep(200000);
				usleep(200000);
#endif
				start_i2s_clk = 0;
				i2s_tx_started = 1;
				status_captured = 0;
				i2s_started = 0;
		}
		}

#if WAIT_ON_AUD_INFO

		if (AudioinfoFrame.frame_count > AUD_INFO_COUNT) {
			//Disable Infoframe Intr
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			            XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
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
			audio_info_avail = 1;
			AudioinfoFrame.frame_count = 0;
			AudioinfoFrame.all_count = 0;
		} else {
			if (AudioinfoFrame.all_count > 200) {
				//An audio infoframe was not received
				//disabling the interrupt to avoid repeated calls
				xil_printf ("*!*!\r\n");
				XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				            XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
				AudioinfoFrame.all_count = 0;

			}
		}
#else
		audio_info_avail = 1;
#endif
#endif
}

void audio_start_tx (void) {
#if ENABLE_AUDIO

	// Audio on TX will be started only when RX receives Audio Infoframes
	// and after a delay
	if (tx_done == 1 && i2s_tx_started == 1 && i2s_started == 0 && audio_info_avail == 1
			&& aud_start_delay == AUD_START_DELAY) {
	filter_count_b++;
	if (filter_count_b == 50) {

#if WAIT_ON_AUD_INFO
	start_audio_passThrough();
#endif

	} else if (filter_count_b > 200) {

#if I2S_AUDIO
	if (appx_fs_dup != 0) {
		XI2s_Tx_SetSclkOutDiv (&I2s_tx, appx_fs_dup*I2S_CLK_MULT, appx_fs_dup);
		XI2s_Rx_SetSclkOutDiv (&I2s_rx, appx_fs_dup*I2S_CLK_MULT, appx_fs_dup);
		XI2s_Rx_LatchAesChannelStatus (&I2s_rx);
		//Start RX and TX I2S
		//Put the ACR in External N/CTS update mode with loop control
		XI2s_Rx_Enable(&I2s_rx, 1);
		XI2s_Tx_Enable(&I2s_tx, 1);
		XACR_WriteReg (RX_ACR_ADDR, RXACR_MODE, 0x4);
		xil_printf ("Audio Sampling rate is %d Hz\r\n",appx_fs_dup);
	}
#endif
		//Bring the FIFO, I2S TX out of reset
		XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x2);
		usleep(1000);
		//Program num channels and enable the TX audio
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CHANNELS, 0x1);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,	XDP_TX_AUDIO_CONTROL, 0x1);
		xil_printf ("Starting audio....\r\n");
		i2s_started = 1;
		filter_count_b = 0;
		audio_info_avail = 0;
	}

	} else {
		filter_count_b = 0;
	}
#endif
}

void dprx_tracking (void) {

	if (rx_unplugged == 1) {
		unplug_proc();
		xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
	} else if (DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0){
		XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				0x140);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK, 0xFFF);
		frameBuffer_stop();
		tx_after_rx = 0;
		DpRxSsInst.VBlankCount++;
		appx_fs_dup = 0;
		rx_trained = 1;
		rx_aud = 0;
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		i2s_tx_started = 0;
		start_i2s_clk = 0;
	    audio_info_avail = 0;
	    AudioinfoFrame.frame_count = 0;
	    AudioinfoFrame.all_count = 0;
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

	}

	if(DpRxSsInst.no_video_trigger == 1){
		frameBuffer_stop();
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		DpRxSsInst.no_video_trigger = 0;
		tx_after_rx = 0;
		rx_all_detect = 0;
	}

	if((DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT) && (rx_trained == 1)){
		DpRxSsInst.no_video_trigger = 0;
		//VBLANK Management
		DpRxSsInst.VBlankCount = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
		XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK |
				XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK);

		XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
		XDp_RxDtgDis(DpRxSsInst.DpPtr);
		XDp_RxDtgEn(DpRxSsInst.DpPtr);
		i2s_tx_started = 0;
		start_i2s_clk = 0;
		appx_fs_dup = 0;
		/*
		 * Disable & Enable Audio
		 */
		XDpRxSs_AudioDisable(&DpRxSsInst);
		XDpRxSs_AudioEnable(&DpRxSsInst);
	    audio_info_avail = 0;
	    AudioinfoFrame.frame_count = 0;
	    AudioinfoFrame.all_count = 0;


		//move to DPPT resolution function

#if !PHY_COMP
		tx_after_rx = 1;
#endif
		rx_aud = 1;
		track_msa = Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa,
				DpRxSsInst.link_up_trigger);
	}

	if(tx_done == 1) {
		track_color = Dppt_DetectColor(DpRxSsInst.DpPtr, Msa,
		DpRxSsInst.link_up_trigger);
		if (track_color == 1) {
			tx_after_rx = 1;
			i2s_started = 0;
			tx_done = 0;
			aud_start_delay = 0;
		}

		if (aud_start_delay < AUD_START_DELAY) {
			aud_start_delay++;
		}
	} else {
		aud_start_delay = 0;
	}
}

void dptx_tracking (void) {
//	u32 Status;

	// When TX is cable is connected, the application will re-initiate the
	// TX training. Note that EDID is not updated.
	// Hence you should not change the monitors at runtime
	if (tx_is_reconnected != 0 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1) { // If Tx cable is reconnected
		xil_printf ("TX Cable Connected !!\r\n");
		hpd_con(&DpTxSsInst, Edid_org, Edid1_org, user_config.VideoMode_local);
		tx_is_reconnected--;
		frameBuffer_stop_rd();
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		i2s_started = 0;
		aud_start_delay = 0;
		tx_done = 0;
		AudioinfoFrame.all_count = 0;
		//reading the Infoframe buffer to enable it for
		//next interrupt
		int info_rd = 0;
		for(info_rd = 1 ; info_rd < 9 ; info_rd++) {
				XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_AUDIO_INFO_DATA(info_rd));
			}
		XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
		            XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);

#if !I2S_AUDIO && ENABLE_AUDIO
		XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
#endif
		tx_after_rx = 1;
	} else {
		tx_is_reconnected = 0;
	}

	//HPD to be considered only when TX & RX is live
	if(hpd_pulse_con_event == 1 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1 && tx_done == 1) {
			xil_printf ("HPD Pulse detected !!\r\n");
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, Msa);
	} else {
		hpd_pulse_con_event = 0;
		filter_count = 0;
	}

}
