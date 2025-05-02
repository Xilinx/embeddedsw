/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

volatile u8 hpd_pulse_con_event; 	/* This variable triggers hpd_pulse_con */
extern XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
extern XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/
extern Video_CRC_Config VidFrameCRC_rx; /* Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_tx;
extern volatile int tx_is_reconnected; 		/* This variable to keep track
 * of the status of Tx link*/

extern XV_tpg TpgInst;
extern XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
extern XIntc IntcInst;

void unplug_proc (void);
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
int Dppt_DetectResolution(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);
int Dppt_DetectColor(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);

void resetIp_rd(void);
void resetIp_wr(void);
char inbyte_local(void);
void pt_help_menu();
void DpPt_LaneLinkRateHelpMenu(void);

volatile u8 tx_after_rx = 0;
volatile u8 Video_valid = 0;

u8 LineRate_init_tx;
u8 LaneCount_init_tx;
XDp_MainStreamAttributes* Msa_test;
u8 rx_all_detect = 0;
user_config_struct user_config;
XVidC_VideoMode VmId;

extern volatile u8 rx_unplugged;
volatile u8 rx_trained = 0;

extern lane_link_rate_struct lane_link_table[];
extern u32 StreamOffset[4];
volatile u8 tx_done = 0;
volatile u8 Tx_only=0;
volatile u8 Tx_only_done=0;
u8 rx_and_tx_started = 0;
int track_msa = 0;
u8 Edid_org[128];
u8 Edid1_org[128];
u8 Edid2_org[128];
int track_color = 0;
extern u16 fb_wr_count;
extern u16 fb_rd_count;
extern u8 Clip_4k;
extern u32 Clip_4k_Hactive;
extern u32 Clip_4k_Vactive;

u8 SinkMaxLinkrate=0, SinkMaxLanecount=0;
u8 SinkCap[16], SinkExtendedCap[16];
u8 config_128_132b;

void DpPt_Main(void){
	u32 Status;
	u8 edid_monitor[384];
	u8 exit;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	Clip_4k=0;
	Clip_4k_Hactive = 0;
	Clip_4k_Vactive = 0;
//	Tx_only=1; //start tx only mode if rx is not connected.

	// This is for EDID setup
	u8 connected = 0;

#ifdef Tx
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
		xil_printf("Please connect a DP Monitor to start the "
				"application !!!\r\n");
		connected = 1;
		}
	}
#endif

#ifdef Tx
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
#endif

#ifdef Tx

	//Read the sink's max linkrate and lanecount cap
	Status = XDpTxSs_GetSinkCapabilities(&DpTxSsInst, SinkCap, SinkExtendedCap,&config_128_132b);
	if(Status != XST_SUCCESS){
		xil_printf("Error in reading Rx Cap\r\n");
	}

	if(SinkCap[XDP_DPCD_TRAIN_AUX_RD_INTERVAL] &
		    XDP_DPCD_TRAIN_AUX_RD_EXT_RX_CAP_FIELD_PRESENT_MASK){
		if(SinkExtendedCap[XDP_DPCD_ML_CH_CODING_CAP] &
			    XDP_TX_MAIN_LINK_CHANNEL_CODING_SET_128B_132B_MASK){
			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					       XDP_DPCD_128B_132B_SUPPORTED_LINK_RATE,
					       1, &SinkMaxLinkrate);
			if((SinkMaxLinkrate & 0x3) == 0x3){
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR20;
			}else if((SinkMaxLinkrate & 0x5) == 0x5){
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR135;
			}else{
				SinkMaxLinkrate=XDP_TX_LINK_BW_SET_UHBR10;
			}
			SinkMaxLanecount= SinkExtendedCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
		}else{
			SinkMaxLinkrate = SinkExtendedCap[XDP_DPCD_MAX_LINK_RATE];
			SinkMaxLanecount= SinkExtendedCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
		}

	}else{
		SinkMaxLinkrate = SinkCap[XDP_DPCD_MAX_LINK_RATE];
		SinkMaxLanecount= SinkCap[XDP_DPCD_MAX_LANE_COUNT] & XDP_DPCD_MAX_LANE_COUNT_MASK;
	}

#endif

#ifdef Rx
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
	DpRxSs_Setup();
#endif

#ifdef Tx
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	// Reading and clearing any residual interrupt on TX
	// Also Masking the interrupts
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			0x140);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFF);
#endif

	//resetting AUX logic. Needed for some Type C based connectors
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);

	pt_help_menu();

	while (1){
		CommandKey = 0;

		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);
			switch(CommandKey){
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

#ifdef Rx
									unplug_proc();
#endif
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
#ifdef Rx
					xil_printf (
							"==========RX Debug Data===========\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
#endif
#ifdef Tx
					xil_printf (
							"==========TX Debug Data===========\r\n");
					XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					XDpTxSs_ReportLinkInfo(&DpTxSsInst);
					XDpTxSs_ReportVtcInfo(&DpTxSsInst);
#endif

#ifdef Rx
					xil_printf ("DP RX addr is %x\r\n",DpRxSsInst.DpPtr->Config.BaseAddr);
#endif
#ifdef Tx
					xil_printf ("DP TX addr is %x\r\n",DpTxSsInst.DpPtr->Config.BaseAddr);
#endif
					break;

				case '3':
					// Disabling TX interrupts
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFF);
#ifdef Rx
					unplug_proc();
#endif
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr,  0x80000000);
					XDpTxSs_Stop(&DpTxSsInst);
					XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
					xil_printf("\r\n- HPD Toggled for 5ms! -\n\r");
					break;

				case '4': // re-start Tx side
					LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
					LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;

					user_config.user_bpc = Msa_test[0].BitsPerColor;
					user_config.VideoMode_local = VmId;
					user_config.user_pattern = 0; /*pass-through (Default)*/
					user_config.user_format = Msa_test[0].ComponentFormat;

#ifdef Tx
					frameBuffer_stop_rd();

					//Waking up the monitor
					sink_power_cycle();
#endif

					// This configures the vid_phy for line rate to start with
#ifdef Tx
					config_phy(LineRate_init_tx);
#endif
					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					tx_after_rx = 1;
					DpTxSsInst.no_video_trigger = 1;
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								     XDP_TX_AUDIO_CONTROL, 0x0);
					break;

#ifdef Tx
				case '6':
					frameBuffer_stop_rd();
					frameBuffer_start_rd(Msa_test);
					break;
#endif
				case 'c':
					xil_printf ("========== Rx CRC===========\r\n");
					xil_printf ("Rxd Hactive =  %d\r\n",
							(8*((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,
								0xC)&0xFFFF) + 1)));
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
						(8*((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)&0xFFFF)+ 1)));

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

				case 'n':
					// This can be used to clone the edid when monitor is changed
					// Ensure to unplug/plug RX cable for changes to take effect
					for (int i = 0; i < 128; i++) {
						Edid_org[i] = 0;
						Edid1_org[i] = 0;
						Edid2_org[i] = 0;
					}

#ifdef Tx
					//Waking up the monitor
					sink_power_cycle();
#endif
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
					break;
;
				case '.':
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
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
												0xFFFFFFFF);
					DpRxSsInst.VBlankCount = 0;

					// disabling Tx
					XDpTxSs_Stop(&DpTxSsInst);
//					XIntc_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
//					XIntc_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
					Vpg_Audio_stop();
#endif
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_ENABLE, 0x0);
					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,0xFFF);


					operationMenu();
					return;
					break;

				default :
					pt_help_menu();
					break;
			}
		}//end if


#ifdef Tx
		// Tx side process
		// When there is no video on TX, stop the FB and I2S RX
		if(DpTxSsInst.no_video_trigger == 1){ // stop frameBuffer if Tx is lost
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			DpTxSsInst.no_video_trigger = 0;
			tx_done = 0;
		}

		// Check for HPD and HPD Pulse Interrupt triggers
		dptx_tracking();
#endif

#ifdef Rx
		// Rx and pass-through side process
		dprx_tracking();
#endif

#ifdef Tx
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
					hpd_pulse_con(&DpTxSsInst, Msa_test);
				}
				tx_done = 1;
		    }else {
			tx_done = 0;
			xil_printf ("Cannot Start TX...\r\n");
		    }
		}
#endif
	}//end while(1)
}

#ifdef Tx
void start_tx_after_rx (void) {
	u32 Status;
	rx_all_detect = 1;
	const XVidC_VideoTimingMode *VtmPtr;

	// check monitor capability
	u8 max_cap_org=0;
	u8 max_cap_lanes=0;

	//scaling down Tx output to 4k30 if Rx receives above 8k30 due to bw issues
	if (Msa_test[0].Vtm.FrameRate * Msa_test[0].Vtm.Timing.HActive *
			Msa_test[0].Vtm.Timing.VActive > 7680*4320*30){
		xil_printf("DDR BW not sufficient, Scaling the output to 4k30 \r\n");
		Clip_4k=1;
		Clip_4k_Hactive = Msa_test[0].Vtm.Timing.HActive;
		Clip_4k_Vactive = Msa_test[0].Vtm.Timing.VActive;
		u8 Hsync_Pol_org=0,Vsync_Pol_org=0;
		Hsync_Pol_org= Msa_test[0].Vtm.Timing.HSyncPolarity;
		Vsync_Pol_org= Msa_test[0].Vtm.Timing.VSyncPolarity;
		VmId = XVIDC_VM_3840x2160_30_P;
		Msa_test[0].VFreq = 297000000;

		VtmPtr = XVidC_GetVideoModeData(VmId);
		Msa_test[0].Vtm = *VtmPtr;
		Msa_test[0].HStart=384;
		Msa_test[0].VStart=82;
		Msa_test[0].Vtm.Timing.HSyncPolarity=Hsync_Pol_org;
		Vsync_Pol_org = Vsync_Pol_org;
	}else{
		Clip_4k=0;
		Clip_4k_Hactive = 0;
		Clip_4k_Vactive = 0;
	}

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1, &max_cap_lanes);
	u8 rData = 0;
	// check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
			1, &rData);

	// if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
	if(rData & 0x80){
		// read maxLineRate
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
		if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
			max_cap_org = 0x1E;
		}
	}

#ifdef Rx
	LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
	LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;
	if(LineRate_init_tx == XDP_TX_LINK_BW_SET_UHBR135){
		if(SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR135){
			LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;
		}else if(SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR20){
			LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR20;
		}else{
			LineRate_init_tx = SinkMaxLinkrate;
		}
		xil_printf("VCU118 QPLL1 doesnt support 13.5g hence Training Tx at 0x%x Linkrate\r\n",LineRate_init_tx);
	}
#endif

	user_config.user_bpc = Msa_test[0].BitsPerColor;
	user_config.user_pattern = 0; /*pass-through (Default)*/

	/*Check component Format*/
	if(Msa_test[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 + 1;
	}else if(Msa_test[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 + 1;
	}else
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB + 1;

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

	start_tx (LineRate_init_tx, LaneCount_init_tx,user_config,Msa_test);
	resetIp_rd();

	//Programming Tx TPG to start in PT mode
	XV_tpg_Set_height(&TpgInst, Msa_test[0].Vtm.Timing.VActive);
	XV_tpg_Set_width(&TpgInst, Msa_test[0].Vtm.Timing.HActive);
	XV_tpg_Set_bckgndId(&TpgInst, 0x9);
	XV_tpg_Set_ovrlayId(&TpgInst, 0x0);
	if(Msa_test[0].ComponentFormat == XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
	XV_tpg_Set_colorFormat(&TpgInst, 0x2);
	}else if(Msa_test[0].ComponentFormat == XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		XV_tpg_Set_colorFormat(&TpgInst, 0x1);
	}else{
		XV_tpg_Set_colorFormat(&TpgInst, 0x0);
	}
	XV_tpg_Set_enableInput(&TpgInst, 1);
	XV_tpg_Set_passthruStartX(&TpgInst, 0);
	XV_tpg_Set_passthruStartY(&TpgInst, 0);
	XV_tpg_Set_passthruEndX(&TpgInst, Msa_test[0].Vtm.Timing.HActive);
	XV_tpg_Set_passthruEndY(&TpgInst, Msa_test[0].Vtm.Timing.VActive);
	XV_tpg_EnableAutoRestart(&TpgInst);
	XV_tpg_Start(&TpgInst);

	frameBuffer_start_rd(Msa_test);

	//start the data
	rx_and_tx_started = 1;

}
#endif

extern int tx_started;

#ifdef Rx
void unplug_proc (void) {
	tx_done = 0;
	tx_after_rx = 0;
	rx_trained = 0;
	tx_started = 0;
    rx_unplugged = 0;

    DpRxSsInst.VBlankCount = 0;
    XDp_RxDtgDis(DpRxSsInst.DpPtr);
	frameBuffer_stop();

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
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	DpRxSs_Setup();
}
#endif

#ifdef Rx
void dprx_tracking (void) {
	if (rx_unplugged == 1) {
		Tx_only=1;
		Tx_only_done=0;
		xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
		unplug_proc();
	} else if (DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0){
		Tx_only=0;
		XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				0x140);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK, 0xFFF);
		frameBuffer_stop();
		tx_after_rx = 0;
		DpRxSsInst.VBlankCount++;
		rx_trained = 1;
#ifdef Tx
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
#endif
	    // retoring unplug counter
        XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG,
			        XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);
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
//		Tx_only=1;
//		Tx_only_done=0;
		frameBuffer_stop();
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		DpRxSsInst.no_video_trigger = 0;
		tx_after_rx = 0;
		rx_all_detect = 0;
	}

	if((Video_valid == 1) && (rx_trained == 1)){
		Tx_only=0;
		Video_valid = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
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

		//move to DPPT resolution function
		tx_after_rx = 1;
        track_msa = Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa_test,
					DpRxSsInst.link_up_trigger);
	}
}
#endif

extern u8 prog_misc1;
extern u8 prog_fb_rd;

#ifdef Tx
void dptx_tracking(void) {

	// When TX is cable is connected, the application will re-initiate the
	// TX training. Note that EDID is not updated.
	// Hence you should not change the monitors at runtime

	if (tx_is_reconnected != 0 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1) { // If Tx cable is reconnected
		xil_printf ("TX Cable Connected !!\r\n");
		hpd_con(&DpTxSsInst, Edid_org, Edid1_org, user_config.VideoMode_local);
		tx_is_reconnected = 0;
		frameBuffer_stop_rd();
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		tx_done = 0;
		tx_after_rx = 1;
	}
	else if(/*(tx_is_reconnected != 0) &&*/ (Tx_only == 1) && (Tx_only_done == 0)){
		xil_printf ("TX enabled for Txo mode !!\r\n");
		Tx_only_done=1;

//		tx_is_reconnected = 0;
		frameBuffer_stop_rd();
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
				XDpTxSs_Stop(&DpTxSsInst);
		tx_done = 0;

		//configure Tx TPG for start in txo mode for displaying FHD output
		//TPG start
		XV_tpg_DisableAutoRestart(&TpgInst);
		XV_tpg_Set_height(&TpgInst, 1080);
		XV_tpg_Set_width(&TpgInst, 1920);
		XV_tpg_Set_bckgndId(&TpgInst, 0x1/*0x9*/);
		XV_tpg_Set_ovrlayId(&TpgInst, 0x0);
		XV_tpg_Set_colorFormat(&TpgInst, XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB);
		XV_tpg_Set_enableInput(&TpgInst, 0);
		XV_tpg_EnableAutoRestart(&TpgInst);
		XV_tpg_Start(&TpgInst);

		user_config_struct user_config_txo;
		user_config_txo.VideoMode_local=XVIDC_VM_1920x1080_30_P;
		user_config_txo.user_bpc = 8;
		user_config_txo.user_format=1;
		user_config_txo.user_pattern=1;
		config_phy(SinkMaxLinkrate);
		start_tx(SinkMaxLinkrate, SinkMaxLanecount, user_config_txo, 0);
	}
	else {
		tx_is_reconnected = 0;
	}

	//HPD to be considered only when TX & RX is live
	if(hpd_pulse_con_event == 1 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1 && tx_done == 1) {
			xil_printf ("HPD Pulse detected !!\r\n");
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, Msa_test);
	} else {
		hpd_pulse_con_event = 0;
	}
}
#endif
