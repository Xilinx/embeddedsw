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

#if ENABLE_HDCP_IN_DESIGN

#if ENABLE_HDCP22_IN_RX
#define RX_ENCRYPTION_OFFSET 0x4C
#endif

#if ENABLE_HDCP22_IN_TX
#define TX_ENCRYPTION_OFFSET 0x4C
#endif

#endif

u8 UpdateBuffer[sizeof(u8) + 16];
u8 WriteBuffer[sizeof(u8) + 16];
u8 ReadBuffer[16];
u16 tx_count_delay = 0;
int tx_aud_started = 0;
int i2s_started = 0;

#if ENABLE_HDCP_IN_DESIGN
u8 hdcp_capable_org = 0;
u8 hdcp_capable = 0;
u8 hdcp_repeater_org = 0;
u8 hdcp_repeater = 0;
u8 internal_rx_tx = 0;
u32 val=0;
#endif
u8 do_not_train_tx = 0;
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

int ConfigFrmbuf_rd(u32 StrideInBytes, XVidC_ColorFormat Cfmt,
		XVidC_VideoStream *StreamPtr);
int ConfigFrmbuf_rd_trunc(u32 offset);
int ConfigFrmbuf_wr(u32 StrideInBytes, XVidC_ColorFormat Cfmt,
		XVidC_VideoStream *StreamPtr);

void DpPt_Main(void);
void operationMenu(void);
void Dppt_DetectAudio (void);
int Dppt_DetectResolution(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);

void frameBuffer_stop(XDpTxSs_MainStreamAttributes Msa[4]);
void frameBuffer_stop_wr(XDpTxSs_MainStreamAttributes Msa[4]);
void frameBuffer_stop_rd(XDpTxSs_MainStreamAttributes Msa[4]);

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

void start_audio_passThrough(u8 LineRate_init_tx);

u8 edid_page;
u8 tx_after_rx = 0;
u8 rx_aud = 0;
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
u8 i2s_tx_started = 0;
u8 status_captured = 0;
u8 aes_sts[24];
int filter_count_b = 0;
int track_msa = 0;
u8 Edid_org[128];
u8 Edid1_org[128];
u8 Edid2_org[128];
int filter_count = 0;

#if ENABLE_HDCP_IN_DESIGN
int mon_is_hdcp22_cap = 0;	//0-hdcp1.3 cap 	1-hdcp2.2 cap
#endif

void DpPt_Main(void){
	u32 Status;
	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;
	u8 retval = 0;

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

		for (i = 0; i < (384 * 4); i = i + (16 * 4)) {
			for (j = i; j < (i + (16 * 4)); j = j + 4) {
				XDp_WriteReg(VID_EDID_BASEADDR, j, edid_monitor[(i / 4) + 1]);
			}
		}
		for (i = 0; i < (384 * 4); i = i + 4) {
			XDp_WriteReg(VID_EDID_BASEADDR, i, edid_monitor[i / 4]);
		}
	}

	Status  = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							XDP_DPCD_MAX_LINK_RATE,  1, LineRate_tx);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr,
							XDP_DPCD_MAX_LANE_COUNT, 1, LaneCount_tx);

#if  ENABLE_HDCP_IN_DESIGN

#if (ENABLE_HDCP22_IN_RX > 0)
	XHdcp22_RxSetRxCaps(DpRxSsInst.Hdcp22Ptr, TRUE);
#endif

#endif

	sink_power_cycle();

#if ENABLE_HDCP_IN_DESIGN
	u32 TxAuthAttempts = 0;
	u8 auxValues_org;
	retval=0;

	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);

#if ENABLE_HDCP22_IN_TX
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x06921D, 1, &auxValues_org);
	retval = auxValues_org & 0x2;
#endif
	if(retval==2)
	{
#if ENABLE_HDCP22_IN_TX
		hdcp_capable_org=1;
		mon_is_hdcp22_cap=1;
		DpTxSsInst.HdcpProtocol=XDPTXSS_HDCP_22;
#endif
	}
	else
	{
#if ENABLE_HDCP1x_IN_TX
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, &auxValues_org);
		retval = auxValues_org & 0x1;
#endif
		if(retval==1)
		{
#if ENABLE_HDCP1x_IN_TX
			hdcp_capable_org=1;
			mon_is_hdcp22_cap=0;
			DpTxSsInst.HdcpProtocol=XDPTXSS_HDCP_1X;
#endif
		}
		else
		{
			hdcp_capable_org = 0;
		}
	}

	hdcp_repeater_org = auxValues_org & 0x2;
	if ((hdcp_capable_org == 0)) {
		hdcp_capable_org = 0;
		xil_printf ("HDCP feature is being disabled in the system\r\n");
	} else {
		xil_printf ("System is capable of displaying HDCP content...\r\n");
	}



#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
	KEYMGMT_Init();
	XHdcp1xExample_Init();

#if ENABLE_HDCP1x_IN_TX
	DpTxSsInst.Hdcp1xPtr->IsRepeater = 0;
#endif
#if ENABLE_HDCP1x_IN_RX
	DpRxSsInst.Hdcp1xPtr->IsRepeater = 0;
#endif
//	XHdcp1xExample_Enable();
#endif
	XDpRxSs_HdcpEnable(&DpRxSsInst);
	XDpTxSs_HdcpEnable(&DpTxSsInst);
#else
        xil_printf ("--->HDCP feature is not enabled in application<---\r\n");
#endif
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

	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

	/* Initializing the Audio related IPs. The AXIS Switches are programmed
	 * based on the "I2S_AUDIO" param in main.h
	 * The Audio Clock Recovery Module is programmed in fixed mode
	 */
	//resetting AUX logic. Needed for some Type based connectors
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);

	pt_help_menu();

	while (1){
		UserInput = XUartPs_RecvByte_NonBlocking();
		if(UserInput!=0){
			xil_printf("UserInput: %c\r\n",UserInput);

			switch(UserInput){
			case '1':
				// This is relevant only when the DP source is DP1.2
				// A DP1.4 source relies on extended capabiltiy bit to decide
				// on the link rate
				DpPt_LaneLinkRateHelpMenu();
				exit = 0;
				while (exit == 0) {
					CmdKey[0] = 0;
					Command = 0;
					CmdKey[0] = inbyte_local();
					if(CmdKey[0]!=0){
						Command = (int)CmdKey[0];

						switch (CmdKey[0]) {
						case 'x':
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
							} else if (Command >= 58 || Command <= 47) {
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
							XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									XDP_TX_INTERRUPT_STATUS);
							// mask out interrupt
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									XDP_TX_INTERRUPT_MASK, 0xFFF);

							frameBuffer_stop(Msa);
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
				xil_printf("==========MCDP6000 Debug Data===========\r\n");
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

				xil_printf("==========RX Debug Data===========\r\n");
				XDpRxSs_ReportLinkInfo(&DpRxSsInst);
				XDpRxSs_ReportMsaInfo(&DpRxSsInst);
				xil_printf("==========TX Debug Data===========\r\n");
				XDpTxSs_ReportMsaInfo(&DpTxSsInst);
				XDpTxSs_ReportLinkInfo(&DpTxSsInst);
				XDpTxSs_ReportVtcInfo(&DpTxSsInst);

				break;

				case '3':
					unplug_proc();
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr,  0x80000000);
					// Disabling TX interrupts
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFF);
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
				user_config.user_format = XVIDC_CSF_RGB;
#if ENABLE_AUDIO
				XGpio_WriteReg(aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
#endif

					//Waking up the monitor
					sink_power_cycle();

					XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
					// This configures the vid_phy for line rate to start with
					//Even though CPLL can be used in limited case,
					//using QPLL is recommended for more coverage.
					set_vphy(LineRate_init_tx);

					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					tx_after_rx = 1;
					DpTxSsInst.no_video_trigger = 1;
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_AUDIO_CONTROL, 0x0);
					frameBuffer_stop_rd(Msa);
					break;

			case 'c':
				xil_printf("========== Rx CRC===========\r\n");
				xil_printf("Rxd Hactive =  %d\r\n",
						((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,
								0xC) & 0xFFFF) + 1)
								* (XDp_ReadReg(VidFrameCRC_rx.Base_Addr, 0x0)));

				xil_printf("Rxd Vactive =  %d\r\n",
				XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0xC) >> 16);
				xil_printf("CRC Cfg     =  0x%x\r\n",
						XDp_ReadReg(VidFrameCRC_rx.Base_Addr, 0x0));
				xil_printf("CRC - R/Y   =  0x%x\r\n",
				XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4) & 0xFFFF);
				xil_printf("CRC - G/Cr  =  0x%x\r\n",
				XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4) >> 16);
				xil_printf("CRC - B/Cb  =  0x%x\r\n",
				XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x8) & 0xFFFF);
				xil_printf("========== Tx CRC===========\r\n");
				xil_printf("Txd Hactive =  %d\r\n",
						((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC) & 0xFFFF)
								+ 1)
								* (XDp_ReadReg(VidFrameCRC_tx.Base_Addr, 0x0)));

					xil_printf ("Txd Vactive =  %d\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)>>16);
					xil_printf ("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0));
					xil_printf ("CRC - R/Y   =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)&0xFFFF);
					xil_printf ("CRC - G/Cr  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x4)>>16);
					xil_printf ("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x8)&0xFFFF);

				break;

			case 'm':
				xil_printf("==========MCDP6000 Debug Data===========\r\n");
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

				xil_printf("0x1294: %08x  0x12BC: %08x  0x12E4: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1294),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x12BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x12E4));
				xil_printf("0x1394: %08x  0x13BC: %08x  0x13E4: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1394),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x13BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x13E4));
				xil_printf("0x1494: %08x  0x14BC: %08x  0x14E4: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1494),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x14BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x14E4));
				xil_printf("0x1594: %08x  0x15BC: %08x  0x15E4: %08x\n\r",
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x1594),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x15BC),
				XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, 0x15E4));

					break;

			case 'n':
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_LINE_RESET_DISABLE,
						~((XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_LINE_RESET_DISABLE)) & 0x1));



					break;

			case 'e':
				//                    XDpRxSs_ReportDp159BitErrCount(&DpRxSsInst);
				ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR,
						XVPHY_RX_SYM_ERR_CNTR_CH1_2_REG);
				xil_printf("Video PHY(8B10B): Error Counts [Lane1, Lane0] "
						"= [%d, %d]\n\r", (ReadVal >> 16), ReadVal & 0xFFFF);
				ReadVal = XVphy_ReadReg(VIDPHY_BASEADDR,
						XVPHY_RX_SYM_ERR_CNTR_CH3_4_REG);
				xil_printf("Video PHY(8B10B): Error Counts [Lane3, Lane2] "
						"= [%d, %d]\n\r", (ReadVal >> 16), ReadVal & 0xFFFF);

					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
					xil_printf ("Lane0 (Lower) is %d,\r\n", DrpVal);
					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
					xil_printf ("Lane0 (Upper) is %d,\r\n", DrpVal);

				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal);
				xil_printf("Lane1 (Lower) is %d,\r\n", DrpVal);
				XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
				XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
				xil_printf("Lane1 (Upper) is %d,\r\n", DrpVal);
				;

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
					XDpRxSs_MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR,
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
				break;

//				case 'q' :
//					if(use_monitor_edid == 1){
//						// change the mode to none-pass-through mdoe
//						use_monitor_edid = 0;
//						xil_printf(
//						"Set as EDID non-pass-through mode\r\n");
//					}else{
//						// This is EDID pass-through mode
//						use_monitor_edid = 1;
//						for(int i=0;i<(384*4);i=i+(16*4)){
//							for(int j=i;j<(i+(16*4));j=j+4){
//								XDp_WriteReg (
//										VID_EDID_BASEADDR,
//								j,edid_monitor[(i/4)+1]);
//							}
//						}
//						for(int i=0;i<(384*4);i=i+4){
//							XDp_WriteReg (
//									VID_EDID_BASEADDR,
//								i, edid_monitor[i/4]);
//						}
//
//						xil_printf(
//							"Set as EDID pass-thorugh mode\r\n");
//					}
//					break;
				case 's':
					xil_printf("DP Link Status --->\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					break;

				case '7':
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
							exit = 1;
							break;

							default :
							xil_printf("You have selected command '%c'\r\n",
														CmdKey[0]);
							if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
								Command = CmdKey[0] -'a' + 10;
								exit = 1;
							}else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								exit = 1;
							}else if (Command >= 58 || Command <= 47) {
								exit = 0;
								break;
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
									ConfigFrmbuf_rd_trunc(
											((dp_msa_hres - 3840) * BPC)
													/ pixel);

									break;

								case 2:
									ConfigFrmbuf_rd_trunc(
											(dp_msa_hres * (dp_msa_vres - 2160))
													* BPC / pixel);
									break;

								case 3:
									ConfigFrmbuf_rd_trunc(
											((dp_msa_hres * (dp_msa_vres - 2160))
													+ (dp_msa_hres - 3840))
													* BPC / pixel);
									break;
								}
							}else{
								xil_printf("!!!Warning: You have selected "
										"wrong option for Quad selection "
										"=%d \n\r", Command);
								break;
							}
							}// end of switch
						} // end of key input wait
					} // end of while

					break;

			case 'u':
				xil_printf("\r\n Give 4 bit Hex value of base register 0x");
				ReadVal = xil_gethex(4);
				xil_printf("\r\n");
				xil_printf("0x%x: %08x\n\r", ReadVal,
						XDpRxSs_MCDP6000_GetRegister(&DpRxSsInst,
						I2C_MCDP6000_ADDR, ReadVal));
				break;

			case 'o':
				xil_printf("\r\n Give 4 bit Hex value of base register 0x");
				ReadVal = xil_gethex(4);
				xil_printf("\r\n");
				xil_printf("\r\n Give 8 bit Hex value of write data 0x");
				//data = xil_gethex(8);
				XDpRxSs_MCDP6000_SetRegister(
						&DpRxSsInst,
				I2C_MCDP6000_ADDR, ReadVal, xil_gethex(8));
				xil_printf("\r\n");

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

#if ENABLE_HDCP_IN_DESIGN
			case 'p':
				xil_printf("****RX regs****\r\n");
				if(DpRxSsInst.HdcpProtocol == XDPRXSS_HDCP_22)
				{
					/*RX HDCP 2.2 encryption status*/
#if ENABLE_HDCP22_IN_RX
					val=XDp_ReadReg(DpRxSsInst.Hdcp22Ptr->Config.BaseAddress,
							RX_ENCRYPTION_OFFSET);
					val=((val & 4)>>2);
					if(val==1)
					{
						xil_printf("RX HDCP 2.2 encryption enabled\r\n");
					}
					else
					{
						xil_printf("RX HDCP 2.2 encryption disabled\r\n");
					}
#endif
				}
				else
				{
#if ENABLE_HDCP1x_IN_RX
					if(DpRxSsInst.HdcpProtocol == XDPRXSS_HDCP_14)
					{
						xil_printf (
								"\r\n==========RX HDCP Debug Data===========\r\n");
									XDpRxSs_ReportHdcpInfo(&DpRxSsInst);

					}
#endif
				}

				xil_printf("****TX regs****\r\n");
				if(DpTxSsInst.HdcpProtocol == XDPTXSS_HDCP_22)
				{
					/*TX HDCP 2.2 encryption status*/
#if ENABLE_HDCP22_IN_TX
					val=XDp_ReadReg(DpTxSsInst.Hdcp22Ptr->Config.BaseAddress,
							TX_ENCRYPTION_OFFSET);
					val=((val & 4)>>2);
					if(val==1)
					{
						xil_printf("TX HDCP 2.2 encryption enabled\r\n");
					}
					else
					{
						xil_printf("TX HDCP 2.2 encryption disabled\r\n");
					}
#endif
				}
				else
				{
#if ENABLE_HDCP1x_IN_TX
					if(DpTxSsInst.HdcpProtocol == XDPTXSS_HDCP_1X)
					{
						xil_printf (
								"\r\n==========TX HDCP Debug Data===========\r\n");
									XDpTxSs_ReportHdcpInfo(&DpTxSsInst);
					}
#endif
				}
				 break;
#endif

			case 'z':
				pt_help_menu();
				break;

			case 'x':
				DpRxSsInst.link_up_trigger = 0;

				// disabling Rx
				XDp_RxDtgDis(DpRxSsInst.DpPtr);
				XDpRxSs_Reset(&DpRxSsInst);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_LINK_ENABLE, 0x0);
				XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
				DpRxSsInst.VBlankCount = 0;

				// disabling Tx
				XDpTxSs_Stop(&DpTxSsInst);
				XScuGic_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
				XScuGic_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

				Vpg_Audio_stop();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE,
						0x0);
				XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x144, 0xFFF);

				track_msa = 0;
				rx_trained = 0;
				tx_done = 0;
#if ENABLE_HDCP22_IN_RX
				XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_SOFT_RESET,
									XDP_RX_SOFT_RESET_HDCP22_MASK);
							XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_SOFT_RESET, 0);
#endif
#if ENABLE_HDCP_IN_DESIGN
				hdcp_capable_org = 0;
				hdcp_capable = 0;
				hdcp_repeater_org = 0;
				hdcp_repeater = 0;
				mon_is_hdcp22_cap=0;
#endif

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
			frameBuffer_stop_rd(Msa);
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			XDpTxSs_Stop(&DpTxSsInst);
			DpTxSsInst.no_video_trigger = 0;
			tx_done = 0;
			i2s_started = 0;

//			XI2s_Rx_Enable(&I2s_rx, 0);
		} else {
#if ENABLE_AUDIO
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
#if ENABLE_HDCP_IN_DESIGN
		//Wait for few frames to ensure valid video is received
		if (tx_after_rx == 1 && rx_trained == 1
				&& DpRxSsInst.link_up_trigger == 1
				&& DpRxSsInst.TmrCtrResetDone
						== 1

						)
#else
						if (tx_after_rx == 1 && rx_trained == 1 && DpRxSsInst.link_up_trigger == 1 )
#endif
			{
		    tx_after_rx = 0;
		    if (track_msa == 1) {
			usleep(20000);
#if ENABLE_HDCP_IN_DESIGN
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst,
						hdcp_capable_org);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
#endif
				start_tx_after_rx();
#if ENABLE_HDCP_IN_DESIGN
				if (hdcp_capable_org == 1) {
					xil_printf("$");
					DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);
					xil_printf(".");
#if ENABLE_HDCP1x_IN_TX
					XDpTxSs_SetLane(&DpTxSsInst,
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount); //LaneCount_init_tx);
					XDpTxSs_SetPhysicalState(&DpTxSsInst, !hdcp_capable_org);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
					XHdcp1xExample_Poll();
#if ENABLE_HDCP1x_IN_TX
					XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
#endif
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
					XHdcp1xExample_Poll();
#endif
				} //hdcp_capable_org check
				else {
					if (DpRxSsInst.TmrCtrResetDone == 1) {
						tx_after_rx = 0;
					}
				}
#endif
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
			xil_printf ("Problem !! : Unable to get RX MSA Values\r\n");
		    }
		}

		// This continuously tracks the Maud, Naud values by reading the
		// registers
		if (rx_trained && rx_aud && tx_done) {
			Dppt_DetectAudio();
		}
#if ENABLE_HDCP_IN_DESIGN
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
		XHdcp1xExample_Poll();
#endif

		if((
#if ENABLE_HDCP1x_IN_RX
				(XHdcp1x_IsEncrypted(DpRxSsInst.Hdcp1xPtr))
#else
				0
#endif
				||

#if ENABLE_HDCP22_IN_RX
				XHdcp22Rx_IsEncryptionEnabled(DpRxSsInst.Hdcp22Ptr)
#else
				0
#endif
		)
				&& rx_trained
				&& XDpTxSs_IsConnected(&DpTxSsInst) && tx_done)
		{
			if((
#if ENABLE_HDCP1x_IN_TX
					(XHdcp1x_IsEncrypted(DpTxSsInst.Hdcp1xPtr)==0)
#else
					0
#endif
					&& !mon_is_hdcp22_cap) ||
					(
#if ENABLE_HDCP22_IN_TX
							((XHdcp22Tx_IsEncryptionEnabled(DpTxSsInst.Hdcp22Ptr))==0)
#else
							(0)
#endif
							&& mon_is_hdcp22_cap ))
			{
				xil_printf("*");
				if(TxAuthAttempts < 5)
				{
					DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 750000);
				}
				/* Waiting for authenticate to complete */
				DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 150000);

				TxAuthAttempts++;
			if (tx_done && TxAuthAttempts > 5) {
				if(mon_is_hdcp22_cap)
				{
					if (XDpTxSs_IsAuthenticated(&DpTxSsInst) == 0)
					{
						if(
#if ENABLE_HDCP22_IN_TX
								XHdcp22Tx_IsInProgress(DpTxSsInst.Hdcp22Ptr)==0
#else
								0
#endif
								) {
									XDpTxSs_Authenticate(&DpTxSsInst);
						}
					}
					else
					{
						DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 75000);
						TxAuthAttempts = 0;
						XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
					}

				} else {
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
					XHdcp1xExample_Poll();
#endif
					if (XDpTxSs_IsAuthenticated(&DpTxSsInst)==0 )
					{
						/* DP TX State 10 : Un-authenticated */
						if(
#if ENABLE_HDCP1x_IN_TX
								DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10
#else
								0
#endif
								) {
										XDpTxSs_Authenticate(&DpTxSsInst);
						} else if (
#if ENABLE_HDCP1x_IN_TX
								DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 0 ||
								DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 11
#else
								0
#endif
								) {
							/* DP TX State 0 : Disabled
							 * DP TX State 11 : Phy-layer-down */

#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
							XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
							XHdcp1xExample_Poll();
#endif
							XDpTxSs_HdcpEnable(&DpTxSsInst);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
							XHdcp1xExample_Poll();
#endif
							XDpTxSs_Authenticate(&DpTxSsInst);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
							XHdcp1xExample_Poll();
#endif
							XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
							XHdcp1xExample_Poll();
#endif
						}
					} else {
						DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 75000);
						TxAuthAttempts = 0;

						XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
					}
				}
			}
				if(TxAuthAttempts == 100)
				{
					xil_printf(">>>> HDCPTX Authentication "
							"failed , stopping passthrough video and starting ColorBar on TX \r\n");
					Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									 0x11);
					TxAuthAttempts = 0;
					break;
				}
			}
		}
		else
		{
			TxAuthAttempts = 0;
			/*
			 * Bring down TX encryption/authentication
			 */
			if ((XDpTxSs_IsAuthenticated(&DpTxSsInst)==1))
			{
				xil_printf(".~\r\n");
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst,
						hdcp_capable_org);
#endif
#if (ENABLE_HDCP1x_IN_RX | ENABLE_HDCP1x_IN_TX)
				XHdcp1xExample_Poll();
#endif
			}
		}
#endif

#if ENABLE_HDCP_IN_DESIGN
#if (ENABLE_HDCP22_IN_RX | ENABLE_HDCP22_IN_TX)
		if (DpRxSsInst.HdcpIsReady || DpTxSsInst.HdcpIsReady) {
			/* Poll HDCP22 */
			XHdcp22_Poll(&Hdcp22Repeater);
		}
#endif
#endif

	}		//end while(1)

}

/* Audio passThrough setting */
void start_audio_passThrough(u8 LineRate_init_tx){

	// Copy the Audi Infoframe data from RX to TX
//	xil_printf("Sending Audio Info frame\r\n");
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
//	usleep(10000);
	XDpTxSs_SendAudioInfoFrame(&(DpTxSsInst),xilInfoFrame);
//	usleep(30000);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CHANNELS, 0x1);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x1);
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

	rx_all_detect = 1;
	VmId = XVidC_GetVideoModeId(Msa[0].Vtm.Timing.HActive,
			Msa[0].Vtm.Timing.VActive, Msa[0].Vtm.FrameRate, 0);

	// check monitor capability
	u8 max_cap_org=0;
	u8 max_cap_lanes=0;
	u8 monitor_8K=0;

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x1, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2, 1, &max_cap_lanes);
	u8 rData = 0;
	// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL, 1, &rData);

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

	downshift4K = 0;
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

	// This block is to use with 4K30 monitor.
	if(max_cap_org <= 0x14 || monitor_8K == 0){
		// 8K resolution will be changed to 4K60
		if (Msa[0].Vtm.Timing.HActive >= 7680
				&& Msa[0].Vtm.Timing.VActive >= 4320) {
			xil_printf("\nMonitor is not capable of displaying 8K resolution."
					   " Displaying only 4K@30 resolution\r\n");
			xil_printf("\nOnly one quad of 4k@30 is displayed.\r\n");

			VmId = XVIDC_VM_3840x2160_30_P;		//_RB;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive /= 2;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive /= 2;

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
		else if (Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive
				* Msa[0].Vtm.Timing.VActive > 4096 * 2160 * 60) {
			xil_printf("\nMonitor is not capable of displaying 4K@120 "
					"resolution. Forcing 4K@30 resolution\r\n");
			// to keep 4Byte mode, it has to be 4K60
			VmId = XVIDC_VM_3840x2160_30_P;//_RB;
			Msa[0].Vtm.FrameRate = 60;
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
			u32 recv_clk_freq = (((int) DpRxSsInst.UsrOpt.LinkRate * 27)
					* mvid_rx) / nvid_rx;

			// Re-calculating Mvid/Nvid based on 5.4Gbps
			u32 nvid_tx = (XDP_TX_LINK_BW_SET_540GBPS * 27);
			u32 mvid_tx = (recv_clk_freq * nvid_tx * 1000)
					/ (XDP_TX_LINK_BW_SET_540GBPS * 27);
			nvid_tx *= 1000;

			// Update MVID and NVID at here with bsed on 5.4Gbps
			Msa[0].MVid = mvid_tx;
			Msa[0].NVid = nvid_tx;
		}
	}

	user_config.VideoMode_local = VmId;

	frameBuffer_stop_rd(Msa);
	//Waking up the monitor
	sink_power_cycle();

	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	// This configures the vid_phy for line rate to start with
	//Even though CPLL can be used in limited case,
	//using QPLL is recommended for more coverage.
	set_vphy(LineRate_init_tx);

	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	if(downshift4K == 0){
		start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, Msa);

	}else{
		start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, 0);
	}


//	frameBuffer_stop_rd(Msa);
	frameBuffer_start_rd(VmId, Msa, downshift4K);
}

void unplug_proc (void) {
	i2s_tx_started = 0;
	tx_done = 0;
	rx_aud = 0;
	tx_after_rx = 0;
	rx_trained = 0;
	tx_started = 0;
    rx_unplugged = 0;
    start_i2s_clk = 0;
    DpRxSsInst.VBlankCount = 0;
	frameBuffer_stop(Msa);

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	XDpTxSs_Stop(&DpTxSsInst);

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


#if ENABLE_AUDIO
	i2s_stop_proc();
	XDpRxSs_AudioDisable(&DpRxSsInst);
#endif
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	DpRxSs_Setup();
}
#if ENABLE_AUDIO
void i2s_stop_proc(void) {
	i2s_started = 0;
	tx_aud_started = 0;
//    XI2s_Rx_Enable(&I2s_rx, 0);
}


void audio_start_rx (void) {

		if (rx_trained && start_i2s_clk) {
			XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
//			XI2s_Rx_Enable(&I2s_rx, 0);
		status_captured = 1;

		// process to start Pass Through Audio and program the Audio pipe
		if (status_captured) {
			// && rx_trained == 1 && DpRxSsInst.link_up_trigger == 1) {
			// && rx_all_detect) {
//				I2cClk_Ps(appx_fs_dup, 768*appx_fs_dup);
			xil_printf("Audio Sampling rate is %d Hz\r\n", appx_fs_dup);
			start_i2s_clk = 0;
			i2s_tx_started = 1;
			status_captured = 0;
			i2s_started = 0;
			filter_count_b = 0;
		}
		}
}

void audio_start_tx(void) {
	if (tx_done == 1 && i2s_tx_started == 1 && i2s_started == 0 && AudioinfoFrame.frame_count > 50) {
		filter_count_b++;
		//Audio may not work properly on some monitors if this is started too early
		//hence the delay here
		if (filter_count_b == 190) {
			start_audio_passThrough(LineRate_init_tx);

		}

		else if (filter_count_b > 200) {
			XGpio_WriteReg(aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x2);
			xil_printf("Starting audio on DP TX..\r\n");
			i2s_started = 1;
			filter_count_b = 0;
		}

}

}
#endif
void dprx_tracking(void) {

	if (rx_unplugged == 1) {
		xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
		unplug_proc();
    } else if (DpRxSsInst.link_up_trigger == 0) { // Link Not trained
		if (rx_trained == 1) {             		// If it was previously trained
			xil_printf ("Training Lost !!\r\n");
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_INTERRUPT_MASK, 0xFFF);
#if ENABLE_HDCP22_IN_RX
			XHdcp22_RxSetLaneCount(DpRxSsInst.Hdcp22Ptr,
					DpRxSsInst.UsrOpt.LaneCount);
#endif
			frameBuffer_stop_wr(Msa);
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			XDpTxSs_Stop(&DpTxSsInst);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
		}
		DpRxSsInst.VBlankCount = 0;
		rx_aud = 0;
		rx_trained = 0;
		tx_after_rx = 0;
		i2s_tx_started = 0;
	} else if (DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger == 1
			&& rx_trained == 0) {
		xil_printf("> Rx Training done !!! (BW: 0x%x, Lanes: 0x%x, Status: "
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
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
				0xFFF);

//			DpRxSsInst.link_up_trigger = 0;
		appx_fs_dup = 0;
		rx_trained = 1;
		rx_aud = 0;
//		frameBuffer_stop(Msa);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		i2s_tx_started = 0;
	}

	if (DpRxSsInst.no_video_trigger == 1) {
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK,
				0xFFF);

		frameBuffer_stop(Msa);
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

		/*
		 * Disable & Enable Audio
		 */
		XDpRxSs_AudioDisable(&DpRxSsInst);
		XDpRxSs_AudioEnable(&DpRxSsInst);

		//move to DPPT resolution function

#if !PHY_COMP
		tx_after_rx = 1;
#endif
		rx_aud = 1;
		track_msa = Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa,
				DpRxSsInst.link_up_trigger);

	}

}

void dptx_tracking (void) {
//	u32 Status;

	// When TX is cable is connected, the application will re-initiate the
	// TX training. Note that EDID is not updated.
	// Hence you should not change the monitors at runtime
	if (tx_is_reconnected != 0 && rx_trained == 1
			&& DpRxSsInst.link_up_trigger == 1) { // If Tx cable is reconnected
		xil_printf("TX Cable Connected !!\r\n");
		tx_done = 0;
		AudioinfoFrame.frame_count = 0;
		hpd_con(&DpTxSsInst, Edid_org, Edid1_org, user_config.VideoMode_local);
		tx_is_reconnected--;
		frameBuffer_stop_rd(Msa);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		i2s_started = 0;
#if ENABLE_AUDIO
		XGpio_WriteReg(aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
#endif
		if (do_not_train_tx == 1) {
			xil_printf("Monitor is not HDCP Capable !!\r\n");
		} else {
			tx_after_rx = 1;
		}
	} else {
		tx_is_reconnected = 0;
	}

	if (hpd_pulse_con_event == 1 && rx_trained == 1
			&& DpRxSsInst.link_up_trigger == 1 && tx_done == 1) {
		//if short HPD pulse detected
		//run a loop for 3000 times to filter HPD pulses on cable unplug
		//this time should be more that the BS IDLE time
//		filter_count++;
//		if (filter_count > 30000) {
			xil_printf ("HPD Pulse detected !!\r\n");
//			filter_count = 0;
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, Msa);
//		}
	} else {
		hpd_pulse_con_event = 0;
		filter_count = 0;
	}

}
