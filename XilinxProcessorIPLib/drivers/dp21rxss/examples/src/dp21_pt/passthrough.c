/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * Ver  Who  Date     Changes
 * ---- --- --------  --------------------------------------------------.
 * 1.6  GM  02/18/26  Initial release.
 *
 * </pre>
******************************************************************************/
#include "main.h"
#include "tx.h"
#include "rx.h"

volatile u8 hpd_pulse_con_event;	/**< This variable triggers hpd_pulse_con */
extern XDpRxSs DpRxSsInst;		/**< The DPRX Subsystem instance.*/
extern XDpTxSs DpTxSsInst;		/**< The DPTX Subsystem instance.*/
extern Video_CRC_Config VidFrameCRC_rx;	/**< Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_tx;
extern volatile int tx_is_reconnected;	/**< This variable to keep track of the status of Tx link */

extern XV_tpg TpgInst;
extern XVphy VPhyInst;	/**< The DPRX Subsystem instance.*/

#ifdef PLATFORM_MB
extern XIntc IntcInst;
#endif

extern XilAudioInfoFrame_rx AudioinfoFrame;
extern void Dp21RxSs_Pt_TxStart (XDpTxSs *InstancePtr, u8 Enable);

void Dp21RxSs_Pt_UnplugProc (void);
void Dp21RxSs_Pt_RxTracking (void);
void Dp21RxSs_Pt_TxTracking (void);
void Dp21RxSs_Pt_StartTxAfterRx(void);
void Dp21RxSs_Pt_Hdcp1xExamplePoll(void);
u8 Dp21RxSs_Pt_IsRxEncrypted(XDpRxSs *InstancePtr);
u8 Dp21RxSs_Pt_IsTxEncrypted(XDpTxSs *InstancePtr);

int Dp21RxSs_Pt_ConfigRdFb(u32 StrideInBytes, XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr);
int Dp21RxSs_Pt_ConfigRdFbTrunc(u32 offset);
int Dp21RxSs_Pt_ConfigWrFb(u32 StrideInBytes, XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr);
void Dp21RxSs_Pt_Setup(void);
int Dp21RxSs_Pt_DetectResolution(void *InstancePtr);
void Dp21RxSs_Pt_TxReadEDID(void *InstancePtr, u8 enable_print);
void Dp21RxSs_Pt_RxSetEDID (u8 edid_monitor[384]);
void Dp21RxSs_Pt_TxAudio (XDpTxSs *InstancePtr, u8 Enable);
void Dp21RxSs_Pt_ResetRdFb(void);
void Dp21RxSs_Pt_ResetWrFb(void);
char Dp21RxSs_Pt_InByteLocal(void);
void Dp21RxSs_Pt_HelpMenu(void);
void Dp21RxSs_Pt_LaneLinkRateHelpMenu(void);

volatile u8 tx_after_rx = 0;
volatile u8 Video_valid = 0;

#if ENABLE_HDCP_IN_DESIGN
#if ENABLE_HDCP22_IN_RX
#define RX_ENCRYPTION_OFFSET 0x4C
#endif
#define TX_ENCRYPTION_OFFSET 0x4C
u8 hdcp_capable_org = 0;
int mon_is_hdcp22_cap = 0;	/*
				 * 0-hdcp1.3 cap
				 * 1-hdcp2.2 cap
				 */
u8 hdcp_capable = 0;
u8 hdcp_repeater_org = 0;
u8 hdcp_repeater = 0;
u8 internal_rx_tx = 0;
u32 val=0;
#endif

u8 LineRate_init_tx;
u8 LaneCount_init_tx;
XDp_MainStreamAttributes* Msa_Pt;
u8 rx_all_detect = 0;
user_config_struct user_config;
XVidC_VideoMode VmId;

extern volatile u8 rx_unplugged;
volatile u8 rx_trained = 0;
extern u8 password_valid;

extern lane_link_rate_struct lane_link_table[];
extern u32 StreamOffset[4];
volatile u8 tx_done = 0;
volatile u8 Tx_only=0;
u8 Tx_only_done=0;
u8 rx_and_tx_started = 0;
int track_msa = 0;
int track_color = 0;
extern u16 fb_wr_count;
extern u16 fb_rd_count;
extern u8 Clip_4k;
extern u32 Clip_4k_Hactive;
extern u32 Clip_4k_Vactive;
extern XVphy_PllType VPHY_RX_PLL_TYPE;
extern XVphy_ChannelId VPHY_RX_CHANNEL_TYPE;
extern XVphy_PllType VPHY_TX_PLL_TYPE;
extern XVphy_ChannelId VPHY_TX_CHANNEL_TYPE;

u8 SinkMaxLinkrate=0, SinkMaxLanecount=0;
u16 max_link_lane;
extern u8 edid_monitor[384];
u8 tx_aud_started = 0;
extern u16 vsync_counter;

void Dp21RxSs_Pt_Setup(void)
{
	u32 Status;
	u8 exit;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	/*
	 * This is for EDID setup
	 */
	u8 connected = 0;

	Clip_4k = 0;
	Clip_4k_Hactive = 0;
	Clip_4k_Vactive = 0;
#ifdef Txo
#ifndef Rxo
	Tx_only = 1; /**< start tx only mode if rx is not connected. */
#endif
#endif

#ifdef Txo
	while (!XDpTxSs_IsConnected(&DpTxSsInst)) {
		if (connected == 0) {
			xil_printf("Please connect a DP Monitor to start the "
				   "application !!!\r\n");
			connected = 1;
		}
	}

	/*
	 * Sink Capabilities are read on HPD connect
	 */
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

#if ENABLE_HDCP_IN_DESIGN
	u32 TxAuthAttempts = 0;
	u8 auxValues_org = 0;
	u32 retval=0;

#if ENABLE_HDCP1x_IN_TX
        DpTxSsInst.Hdcp1xPtr->IsRepeater = 0;
#endif

#if (ENABLE_HDCP_TX)
#if ENABLE_HDCP22_IN_TX
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x06921D, 1, &auxValues_org);
	retval = auxValues_org & 0x2;
#endif
#if ENABLE_HDCP1x_IN_TX
	XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x068028, 1, &auxValues_org);
	retval |= (auxValues_org & 0x1);
#endif

	if (XDpTxSs_HdcpIsReady(&DpTxSsInst)) {
		/*
		 * Initialize the HDCP instance
		 */
		XHdcp_Initialize(&Hdcp22Repeater);

		/*
		 * Set HDCP downstream interface(s)
		 */
		XHdcp_SetDownstream(&Hdcp22Repeater, &DpTxSsInst);
	}

	if (retval == 0x3) {
		hdcp_capable_org = 1;
		mon_is_hdcp22_cap = 1;
		XDpTxSs_HdcpSetProtocol(&DpTxSsInst, XDPTXSS_HDCP_22);
	} else if (retval == 0x2) {
		hdcp_capable_org = 0;
		mon_is_hdcp22_cap = 1;
		XDpTxSs_HdcpSetProtocol(&DpTxSsInst, XDPTXSS_HDCP_22);
	} else if (retval == 0x1) {
		hdcp_capable_org = 1;
		mon_is_hdcp22_cap = 0;
		XDpTxSs_HdcpSetProtocol(&DpTxSsInst, XDPTXSS_HDCP_1X);
	} else {
		hdcp_capable_org = 0;
		mon_is_hdcp22_cap = 0;
		XDpTxSs_HdcpSetProtocol(&DpTxSsInst, XDPTXSS_HDCP_NONE);
		xil_printf ("HDCP feature is being disabled in the system\r\n");
	}

	Status = XDpTxSs_HdcpEnable(&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf ("Hdcp Enable failed\r\n");
	}
#endif
#endif
#endif

#ifdef Rxo
	/*
	 * Whatever EDID is read from the sink (Monitor) is cloned into RX
	 */
	Dp21RxSs_Pt_RxSetEDID (edid_monitor);

#if (ENABLE_HDCP22_IN_RX)
	XHdcp22_Dp_RxSetRxCaps(DpRxSsInst.Hdcp22Ptr, TRUE);
#endif

#if ENABLE_HDCP1x_IN_RX
	DpRxSsInst.Hdcp1xPtr->IsRepeater = 0;
#endif

#if (ENABLE_HDCP_RX)
	XDpRxSs_HdcpEnable(&DpRxSsInst);
#endif

	XDpRxSs_SetLinkRate(&DpRxSsInst, DPRXSS_LINK_RATE);
	XDpRxSs_SetLaneCount(&DpRxSsInst, DPRXSS_LANE_COUNT);

	/*
	 * Start DPRX Subsystem set
	 */
	Status = XDpRxSs_Start(&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:DPRX SS start failed\n\r");
		return;
	}
	Dp21RxSs_Pt_RxSetup();
#endif

	/*
	 * Resetting AUX logic. Needed for some Type C based connectors
	 */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);

	Dp21RxSs_Pt_HelpMenu();

	while (1) {
		CommandKey = 0;
		CommandKey = xil_getc(0xff);
		Command = atoi(&CommandKey);

		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);
			switch(CommandKey) {
#if ENABLE_HDCP_IN_DESIGN
			case 'p':
				xil_printf("****RX regs****\r\n");
				if (DpRxSsInst.HdcpProtocol == XDPRXSS_HDCP_22) {
					/*
					 * RX HDCP 2.2 encryption status
					 */
					val = XDp_ReadReg(DpRxSsInst.Hdcp22Ptr->Config.BaseAddress,
							  RX_ENCRYPTION_OFFSET);
					val = ((val & 4)>>2);
					if (val == 1)
					{
						xil_printf("RX HDCP 2.2 encryption enabled\r\n");
					} else {
						xil_printf("RX HDCP 2.2 encryption disabled\r\n");
					}
				} else {
					if(DpRxSsInst.HdcpProtocol == XDPRXSS_HDCP_14) {
						xil_printf ("\r\n==========RX HDCP Debug Data===========\r\n");
						XDpRxSs_ReportHdcpInfo(&DpRxSsInst);
					}
				}

				xil_printf("****TX regs****\r\n");
				if(DpTxSsInst.HdcpProtocol == XDPTXSS_HDCP_22) {
					/*
					 * TX HDCP 2.2 encryption status
					 */
					val = XDp_ReadReg(DpTxSsInst.Hdcp22Ptr->Config.BaseAddress,
							  TX_ENCRYPTION_OFFSET);
					val = ((val & 4)>>2);
					if (val == 1)
					{
						xil_printf("TX HDCP 2.2 encryption enabled\r\n");
					} else {
						xil_printf("TX HDCP 2.2 encryption disabled\r\n");
					}
				} else {
					if(DpTxSsInst.HdcpProtocol == XDPTXSS_HDCP_1X)
					{
						xil_printf("\r\n==========TX HDCP Debug Data===========\r\n");
						XDpTxSs_ReportHdcpInfo(&DpTxSsInst);
					}
				}

				 break;
#endif

				case '1':
					Dp21RxSs_Pt_LaneLinkRateHelpMenu();
					exit = 0;
					while (exit == 0) {
						CmdKey[0] = 0;
						Command = 0;
						CmdKey[0] = Dp21RxSs_Pt_InByteLocal();
						if(CmdKey[0]!=0){
							Command = (int)CmdKey[0];

							switch  (CmdKey[0])
							{
								case 'x':
									exit = 1;
									break;

								default :
									xil_printf("You have selected command '%c'\r\n", CmdKey[0]);

									if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
										Command = CmdKey[0] -'a' + 10;
										exit = 1;
									}
									else if (Command > 47 && Command < 58) {
										Command = Command - 48;
										exit = 1;
									}
									else if (Command >= 58 || Command <= 47) {
										Dp21RxSs_Pt_LaneLinkRateHelpMenu();
										exit = 0;
										break;
									}
									xil_printf("\r\nSetting LineRate:%x LaneCounts:%x\r\n",
										   lane_link_table[Command].link_rate,
										   lane_link_table[Command].lane_count);
#ifdef Rxo
									Dp21RxSs_Pt_UnplugProc();
#endif
									/*
									 * Setting new capability at here
									 * clear the interrupt status
									 */
									XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										    XDP_TX_INTERRUPT_STATUS);

									/*
									 * Mask out interrupt
									 */
									XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										     XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);

									Dp21RxSs_Pt_FbStop();
									XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
											0x7FF8FFFF);
									/*
									 * Disabling TX interrupts
									 */
									XDpTxSs_Stop(&DpTxSsInst);

									/*
									 * Set new line rate
									 */
									DpRxSsInst.DpPtr->Config.MaxLinkRate =
											lane_link_table[Command].link_rate;
									XDpRxSs_SetLinkRate(&DpRxSsInst,
											lane_link_table[Command].link_rate);

									/*
									 * Set new lane counts
									 */
									DpRxSsInst.DpPtr->Config.MaxLaneCount =
											lane_link_table[Command].lane_count;
									XDpRxSs_SetLaneCount(&DpRxSsInst,
											lane_link_table[Command].lane_count);

									/*
									 * Issuing HPD for re-training
									 */
									XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
									break;
							}
						}
					}
					break;

				case '2':
#ifdef Rxo
					xil_printf ("==========RX Debug Data===========\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
#endif
#ifdef Txo
					xil_printf ("==========TX Debug Data===========\r\n");
					XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					XDpTxSs_ReportLinkInfo(&DpTxSsInst);
					XDpTxSs_ReportVtcInfo(&DpTxSsInst);
#endif

#ifdef Rxo
					xil_printf ("DP RX addr is %x\r\n",DpRxSsInst.DpPtr->Config.BaseAddr);
#endif
#ifdef Txo
					xil_printf ("DP TX addr is %x\r\n",DpTxSsInst.DpPtr->Config.BaseAddr);
					xil_printf ("Tx Frame count %d\r\n",vsync_counter);
#endif

					u8 pll = XVphy_GetPllType(&VPhyInst, 0, XVPHY_DIR_TX, VPHY_TX_CHANNEL_TYPE);
					if (pll == 0x3) {
						xil_printf ("TX is using QPLL0\r\n");
					} else if (pll == 0x4) {
						xil_printf ("TX is using QPLL1\r\n");
					}

					pll = XVphy_GetPllType(&VPhyInst, 0, XVPHY_DIR_RX, VPHY_RX_CHANNEL_TYPE);
					if (pll == 0x3) {
						xil_printf ("RX is using QPLL0\r\n");
					} else if (pll == 0x4) {
						xil_printf ("RX is using QPLL1\r\n");
					}
					break;

				case '3':
					/*
					 * Disabling TX interrupts
					 */
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);
#ifdef Rxo
					Dp21RxSs_Pt_UnplugProc();
#endif
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr,  0x80000000);
					XDpTxSs_Stop(&DpTxSsInst);
					XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
					xil_printf("\r\n- HPD Toggled for 5ms! -\n\r");
					break;

				case '4': /**< Re-start Tx side */
					LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
					LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;

					user_config.user_bpc = Msa_Pt[0].BitsPerColor;
					user_config.VideoMode_local = VmId;
					user_config.user_pattern = 0; /**< pass-through (Default) */
					user_config.user_format = Msa_Pt[0].ComponentFormat;

#ifdef Txo
					Dp21RxSs_Pt_FbStopRd();
					/*
					 * Waking up the monitor
					 */
					Dp21RxSs_Pt_SinkPowerCycle();

					/*
					 * This configures the vid_phy for line rate to start with
					 */
					Dp21RxSs_Pt_PhyConfig(&VPhyInst, LineRate_init_tx, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					tx_after_rx = 1;
					DpTxSsInst.no_video_trigger = 1;
					Dp21RxSs_Pt_TxAudio (&DpTxSsInst, 0);
#endif
					break;

#ifdef Txo
				case '6':
					Dp21RxSs_Pt_FbStopRd();
					Dp21RxSs_Pt_FbStartRd(Msa_Pt);
					break;
#endif
				case 'c':
#ifdef Rxo
					xil_printf ("========== Rx CRC===========\r\n");
					XVidFrameCrc_Report(&VidFrameCRC_rx, PPC_RX);
#endif
#ifdef Txo
					xil_printf ("========== Tx CRC===========\r\n");
					XVidFrameCrc_Report(&VidFrameCRC_tx, PPC_TX);
#endif
					break;

				case 'n':
					/*
					 * This can be used to clone the edid when monitor is changed
					 * Ensure to unplug/plug RX cable for changes to take effect
					 */
#ifdef Txo
					/*
					 * Waking up the monitor
					 */
					Dp21RxSs_Pt_SinkPowerCycle();
					Dp21RxSs_Pt_TxReadEDID (&DpTxSsInst, 0);
#endif

#ifdef Rxo
					Dp21RxSs_Pt_RxSetEDID (edid_monitor);
#endif
					break;
				case '.':
					Dp21RxSs_Pt_HelpMenu();
					break;

				case 'x':
					DpRxSsInst.link_up_trigger = 0;

					/*
					 * Disabling Rx
					 */
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDpRxSs_Reset(&DpRxSsInst);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_LINK_ENABLE, 0x0);
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
					DpRxSsInst.VBlankCount = 0;

					XDpTxSs_Stop(&DpTxSsInst);

#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
					Vpg_Audio_stop();
#endif
					Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,0xFFF);
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

					Dp21RxSs_Pt_Menu();
					return;
					break;
				default :
					Dp21RxSs_Pt_HelpMenu();
					break;
			}
		}

#ifdef PT
		u32 ReadVal =0;
		/*
		 * Tx side process
		 * When there is no video on TX, stop the FB and I2S RX
		 */
		if(DpTxSsInst.no_video_trigger == 1){ /**< Stop frameBuffer if Tx is lost */
			Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
			DpTxSsInst.no_video_trigger = 0;
			tx_done = 0;
		} else {
			if (tx_done == 1 && tx_aud_started == 0) {
#if ENABLE_AUDIO
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CHANNELS, 0x1);
				xil_printf ("Starting audio....\r\n");

				/*
				 * Keep the Audio FIFO out of reset state
				 */
				ReadVal = XDp_ReadReg(TX_CLK_RST_ADDR, 0x8);
				ReadVal = (ReadVal & 0x7);
				ReadVal = (ReadVal | 0x2); //Getting FIFO out of reset bit[1]
				XDp_WriteReg(TX_CLK_RST_ADDR, 0x8, ReadVal);
				tx_aud_started = 1;
#endif
			}
		}
#endif
#ifdef Txo
		/*
		 * Check for HPD and HPD Pulse Interrupt triggers
		 */
		Dp21RxSs_Pt_TxTracking();
#endif

#ifdef Rxo
		/*
		 * Rx and pass-through side process
		 */
		Dp21RxSs_Pt_RxTracking();
#endif

#ifdef PT
#if !ENABLE_HDCP_IN_DESIGN
		/*
		 * Wait for few frames to ensure valid video is received
		 */
		if (tx_after_rx == 1 && rx_trained == 1 && DpRxSsInst.link_up_trigger == 1) {
			tx_after_rx = 0;
			if (track_msa == 1) {
				Dp21RxSs_Pt_StartTxAfterRx();

				/*
				 * It is observed that some monitors do not give HPD
				 * pulse. Hence checking the link to re-trigger
				 */
				Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
				if (Status != XST_SUCCESS) {
					xil_printf ("^*^");
					Dp21RxSs_Pt_TxHpdPulseCon(&DpTxSsInst, Msa_Pt);
				}
				tx_done = 1;
			} else {
				tx_done = 0;
				xil_printf ("Cannot Start TX...\r\n");
			}
		}
#else
#if (ENABLE_HDCP_IN_DESIGN && (ENABLE_HDCP_RX))
		/*
		 * Wait for few frames to ensure valid video is received
		 */
#ifdef USE_EEPROM_HDCP_KEYS
		if (!password_valid) {
			DpRxSsInst.TmrCtrResetDone = 1; /**< Done so that tx gets trained in scenario where source starts
							     authentication and encryption just after training even if password is wrong */
		}
#endif
		if (tx_after_rx == 1 && rx_trained == 1
				&& DpRxSsInst.link_up_trigger == 1
				&& DpRxSsInst.TmrCtrResetDone == 1)
#else
		if (tx_after_rx == 1 && rx_trained == 1 && DpRxSsInst.link_up_trigger == 1)
#endif
		{
			tx_after_rx = 0;
			if (track_msa == 1) {
				usleep(20000);
#if (ENABLE_HDCP_TX)
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
#endif
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst,
						hdcp_capable_org);
				Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif
				Dp21RxSs_Pt_StartTxAfterRx();
#if ENABLE_HDCP_IN_DESIGN
				if (hdcp_capable_org == 1) {
					xil_printf("$");
					Dp21RxSs_Pt_CustomWaitUs(DpTxSsInst.DpPtr, 2000000);
					xil_printf(".");
#if ENABLE_HDCP1x_IN_TX
					XDpTxSs_SetLane(&DpTxSsInst,
							DpTxSsInst.DpPtr->TxInstance.LinkConfig.LaneCount); //LaneCount_init_tx);
					XDpTxSs_SetPhysicalState(&DpTxSsInst, !hdcp_capable_org);
					Dp21RxSs_Pt_Hdcp1xExamplePoll();
					XDpTxSs_SetPhysicalState(&DpTxSsInst, hdcp_capable_org);
					Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif
				} /**< hdcp_capable_org check */
				else {
#if (ENABLE_HDCP_RX)
					if (DpRxSsInst.TmrCtrResetDone == 1) {
						tx_after_rx = 0;
					}
#endif
				}
				/*
				 * It is observed that some monitors do not give HPD
				 * pulse. Hence checking the link to re-trigger
				 */
				Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
				if (Status != XST_SUCCESS) {
					xil_printf ("^*^");
					Dp21RxSs_Pt_TxHpdPulseCon(&DpTxSsInst, Msa_Pt);
				}
				tx_done = 1;
			} else {
				tx_done = 0;
				xil_printf ("Problem !! : Unable to get RX MSA Values\r\n");
			}
		}
#endif
#endif

#if ENABLE_HDCP_IN_DESIGN

		Dp21RxSs_Pt_Hdcp1xExamplePoll();

		if(((Dp21RxSs_Pt_IsRxEncrypted(&DpRxSsInst)))
#if (ENABLE_HDCP_IN_DESIGN && (ENABLE_HDCP_RX))
				&& rx_trained
#else
				|| rx_trained
#endif
				&& XDpTxSs_IsConnected(&DpTxSsInst) && tx_done && (hdcp_capable_org || mon_is_hdcp22_cap))
		{
			if((!Dp21RxSs_Pt_IsTxEncrypted(&DpTxSsInst))) {
				xil_printf("*");

				if(TxAuthAttempts < 5) {
					Dp21RxSs_Pt_CustomWaitUs(DpTxSsInst.DpPtr, 750000);
				}

				/*
				 * Waiting for authenticate to complete
				 */
				Dp21RxSs_Pt_CustomWaitUs(DpTxSsInst.DpPtr, 150000);

				TxAuthAttempts++;

				if (tx_done && TxAuthAttempts > 5) {
					if(mon_is_hdcp22_cap) {
						if (!XDpTxSs_IsAuthenticated(&DpTxSsInst)) {
							if(!XHdcp22Tx_Dp_IsInProgress(DpTxSsInst.Hdcp22Ptr)) {
#if (ENABLE_HDCP_TX)
								xil_printf (">>\r\n");
								Status = XDpTxSs_Authenticate(&DpTxSsInst);
								if (Status != XST_SUCCESS) {
									xil_printf ("Hdcp Auth failed\r\n");
								}
#endif
						} else {
							xil_printf ("/\r\n");
						}
					}
					else {
						Dp21RxSs_Pt_CustomWaitUs(DpTxSsInst.DpPtr, 75000);
						TxAuthAttempts = 0;
#if (ENABLE_HDCP_TX)
						xil_printf ("Trying encryption\r\n");
						Status = XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
						if (Status != XST_SUCCESS) {
							xil_printf ("Hdcp EnEncryption failed\r\n");
						}
#endif
					}
				} else if (hdcp_capable_org) { /**< Assuming to be HDCP 1x capable */
					Dp21RxSs_Pt_Hdcp1xExamplePoll();

					if (!XDpTxSs_IsAuthenticated(&DpTxSsInst)) {
						/*
						 * DP TX State 10 : Un-authenticated
						 */
						if(
#if ENABLE_HDCP1x_IN_TX
							DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 10
#else
							0
#endif
						) {
#if (ENABLE_HDCP_TX)
							XDpTxSs_Authenticate(&DpTxSsInst);
#endif
						} else if (
#if ENABLE_HDCP1x_IN_TX
							DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 0 ||
							DpTxSsInst.Hdcp1xPtr->Tx.CurrentState == 11
#else
							0
#endif
							) {
								/*
								 * DP TX State 0 : Disabled
								 * DP TX State 11 : Phy-layer-down
								 */
#if ENABLE_HDCP1x_IN_TX
								XDpTxSs_SetPhysicalState(&DpTxSsInst, TRUE);
#endif

								Dp21RxSs_Pt_Hdcp1xExamplePoll();

#if (ENABLE_HDCP_TX)
								Status = XDpTxSs_HdcpEnable(&DpTxSsInst);
									if (Status != XST_SUCCESS) {
										xil_printf ("Hdcp Enable failed\r\n");
									}
#endif
								Dp21RxSs_Pt_Hdcp1xExamplePoll();

#if (ENABLE_HDCP_TX)
								XDpTxSs_Authenticate(&DpTxSsInst);
#endif
								Dp21RxSs_Pt_Hdcp1xExamplePoll();
							}
					} else {
						Dp21RxSs_Pt_CustomWaitUs(DpTxSsInst.DpPtr, 75000);
						TxAuthAttempts = 0;
#if (ENABLE_HDCP_TX)
						XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
						Dp21RxSs_Pt_Hdcp1xExamplePoll();
#endif
					}
				} else {
					xil_printf ("Monitor does not support HDCP\r\n");
				}
			}
				if (TxAuthAttempts == 100) {
#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
					xil_printf(">>>> HDCPTX Authentication "
							"failed , stopping passthrough video and starting ColorBar on TX \r\n");

					Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									 0x11);
#else
					xil_printf("\r\n>>>> HDCPTX Authentication "
							"failed , stopping passthrough video on TX \r\n");
					/*
					 * Disabling Tx
					 */
					XDpTxSs_Stop(&DpTxSsInst);
#endif
					TxAuthAttempts = 0;
					break;
				}
			}
		} else {
			TxAuthAttempts = 0;
			/*
			 * Bring down TX encryption/authentication
			 */
#if (ENABLE_HDCP_TX)
			if ((XDpTxSs_IsAuthenticated(&DpTxSsInst)==1))
			{
				xil_printf(".~\r\n");
				XDpTxSs_DisableEncryption(&DpTxSsInst,0x1);
				XDpTxSs_HdcpDisable(&DpTxSsInst);
#if ENABLE_HDCP1x_IN_TX
				XDpTxSs_SetPhysicalState(&DpTxSsInst,
						hdcp_capable_org);
#endif

				Dp21RxSs_Pt_Hdcp1xExamplePoll();

			}
#endif
		}
#endif

#if ENABLE_HDCP_IN_DESIGN
	if (DpRxSsInst.HdcpIsReady || DpTxSsInst.HdcpIsReady) {
			/*
			 * Poll HDCP22
			 */
			XHdcp22_Poll(&Hdcp22Repeater);
		}
#endif
#endif
	}/**< End while(1) */
}

#ifdef PT
extern u8 tx_pass;

void Dp21RxSs_Pt_StartTxAfterRx (void)
{
	u32 Status;
	rx_all_detect = 1;
	tx_pass = 0;
	/*
	 * Check monitor capability
	 */
	u8 max_cap_org=0;
	u8 max_cap_lanes=0;
        u8 supports_20g = 0;
        u8 supports_135g = 0;
        u8 supports_10g = 0;

	VmId = 0;
	Clip_4k_Hactive = Msa_Pt[0].Vtm.Timing.HActive;
	Clip_4k_Vactive = Msa_Pt[0].Vtm.Timing.VActive;
	Clip_4k=0;

	/*
	 * Scaling down Tx output to 4k30 if Rx receives above 8k30 due to bw issues
	 */
	if (Msa_Pt[0].Vtm.FrameRate * Msa_Pt[0].Vtm.Timing.HActive *
			Msa_Pt[0].Vtm.Timing.VActive > 7680*4320*30) {
		xil_printf("DDR BW not sufficient, Scaling the output to 4k30 \r\n");
		Clip_4k = 1;
		Clip_4k_Hactive = 3840;
		Clip_4k_Vactive = 2160;

		VmId = XVIDC_VM_3840x2160_30_P;
	}

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1, &max_cap_lanes);
	u8 rData = 0;
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
			1, &rData);

	/*
	 * If EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
	 */
	if (rData & 0x80) {
		/*
		 * Read maxLineRate
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
		if(rData == XDP_DPCD_LINK_BW_SET_810GBPS){
			max_cap_org = 0x1E;
		}
	}

#ifdef Txo
	/*
	 * Determine what should be the TX rate
	 * Assuming 20G will support 20, 13.5, 10
	 */
	if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR20) {
		supports_20g = 1;
		supports_135g = 1;
		supports_10g = 1;
	} else if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR135) {
		supports_20g = 0;
		supports_135g = 1;
		supports_10g = 1;
	} else if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR10) {
		supports_20g = 0;
		supports_135g = 0;
		supports_10g = 1;
        }

	/*
	 * QPLL1 does not support 13.5
	 */
	if (VPHY_TX_CHANNEL_TYPE == XVPHY_CHANNEL_ID_CMN1) {
		/*
		 * If RX is trained at 13.5, TX cannot train at 13.5
		 * TX could run at 10G or 20G
		 */
		if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_UHBR135) {
			if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR135) {
				/*
				 * Assuming it will also support 10G
				 */
				LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;
				xil_printf("VCU118 QPLL1 doesn't support 13.5g hence Training TX at 0x%x Linkrate\r\n",LineRate_init_tx);
			} else if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR20) {
				/*
				 * It can be 10G also, but keeping 20G
				 */
				LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR20;
				xil_printf("VCU118 QPLL1 doesn't support 13.5g hence Training TX at 0x%x Linkrate\r\n",LineRate_init_tx);
			} else {
				LineRate_init_tx = SinkMaxLinkrate;
			}
		} else {
			if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR135) {
				/*
				 * QPLL1 cannot run at 13.5
				 * Assuming it supports 10G
				 */
				SinkMaxLinkrate = XDP_TX_LINK_BW_SET_UHBR10;
			}
                        if ((DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_UHBR20)) {
				if (supports_20g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR20;
				} else if (supports_135g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;	 /**< TX QPLL1 not capable */
				} else if (supports_10g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;
				} else {
					LineRate_init_tx = SinkMaxLinkrate;
				}
                        } else if ((DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_UHBR10)) {
                             if (supports_10g == 1 || supports_20g ==1 || supports_135g == 1) {
				     LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;
                             } else {
				     LineRate_init_tx = SinkMaxLinkrate;
                             }
                        } else {
				LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
                        }
		}
	} else {
		LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
	}
	LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;
#endif
	user_config.user_bpc = Msa_Pt[0].BitsPerColor;
	user_config.user_pattern = 0; /*pass-through (Default)*/

	/*
	 * Check component Format
	 */
	if(Msa_Pt[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 + 1;
	} else if(Msa_Pt[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 + 1;
	} else {
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB + 1;
	}

	user_config.VideoMode_local = VmId;

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);

	Dp21RxSs_Pt_FbStopRd();

	Dp21RxSs_Pt_ResetRdFb();

	/*
	 * Programming Tx TPG to start in PT mode
	 */
	XV_tpg_Set_height(&TpgInst, Clip_4k_Vactive);
	XV_tpg_Set_width(&TpgInst, Clip_4k_Hactive);
	XV_tpg_Set_bckgndId(&TpgInst, 0x9);
	XV_tpg_Set_ovrlayId(&TpgInst, 0x0);

	if(Msa_Pt[0].ComponentFormat == XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		XV_tpg_Set_colorFormat(&TpgInst, 0x2);
	}else if(Msa_Pt[0].ComponentFormat == XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		XV_tpg_Set_colorFormat(&TpgInst, 0x1);
	}else{
		XV_tpg_Set_colorFormat(&TpgInst, 0x0);
	}

	XV_tpg_Set_enableInput(&TpgInst, 1);
	XV_tpg_Set_passthruStartX(&TpgInst, 0);
	XV_tpg_Set_passthruStartY(&TpgInst, 0);
	XV_tpg_Set_passthruEndX(&TpgInst, Clip_4k_Hactive);
	XV_tpg_Set_passthruEndY(&TpgInst, Clip_4k_Vactive);
	XV_tpg_EnableAutoRestart(&TpgInst);

	/*
	 * Waking up the monitor
	 */
	Dp21RxSs_Pt_SinkPowerCycle();

	/*
	 * This is needed for some monitor to make them happy
	 * Setting the linerate to 1.62 improves performance with some
	 * Dell models, which otherwise give too many HPDs
	 * This is needed especially on multiple unplug/plug
	 */
	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	Dp21RxSs_Pt_StartTx (LineRate_init_tx, LaneCount_init_tx,user_config, Msa_Pt);
	XV_tpg_Start(&TpgInst);

	Dp21RxSs_Pt_FbStartRd(Msa_Pt);

	/*
	 * Start the data
	 */
	rx_and_tx_started = 1;
}
#endif

extern int tx_started;

#ifdef Rxo
void Dp21RxSs_Pt_UnplugProc (void)
{
	tx_done = 0;
	tx_after_rx = 0;
	rx_trained = 0;
	tx_started = 0;
	rx_unplugged = 0;
	u32 ReadVal = 0;
	XDpRxSs_AudioDisable(&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	AudioinfoFrame.all_count = 0;

	DpRxSsInst.VBlankCount = 0;
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	Dp21RxSs_Pt_FbStop();
#ifdef Txo
	Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
	XDpTxSs_Stop(&DpTxSsInst);

	/*
	 * Setting vswing to 0
	 */
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
	XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);

	/*
	 * Setting pre-emphasis to 0
	 */
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_PREEMP_DP_L0);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_PREEMP_DP_L0);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_PREEMP_DP_L0);
	XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_PREEMP_DP_L0);

	Dp21RxSs_Pt_TxAudio (&DpTxSsInst, 0);

	/*
	 * Keep the Audio FIFO in reset state
	 */
	ReadVal = XDp_ReadReg(TX_CLK_RST_ADDR, 0x8);
	ReadVal = (ReadVal & 0x7);
	ReadVal = (ReadVal & 0x1); //Putting FIFO in reset bit[1]
	XDp_WriteReg(TX_CLK_RST_ADDR, 0x8, ReadVal);
#endif
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	Dp21RxSs_Pt_RxSetup();

	tx_aud_started = 0;
}
#endif

#ifdef Rxo
void Dp21RxSs_Pt_RxTracking (void)
{
	u32 ReadVal;

	if (rx_unplugged == 1) {
		Tx_only=1;
		Tx_only_done=0;
		xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
		Dp21RxSs_Pt_UnplugProc();
	} else if (DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0){
		Tx_only=0;
#ifdef Txo
		XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				0x140);
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
				XDP_TX_INTERRUPT_MASK, 0xFFFFFFFF);
#endif
		Dp21RxSs_Pt_FbStop();
		tx_after_rx = 0;
		AudioinfoFrame.frame_count = 0;
		AudioinfoFrame.all_count = 0;

		DpRxSsInst.VBlankCount++;
		rx_trained = 1;
#ifdef Txo
		Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
		XDpTxSs_Stop(&DpTxSsInst);
#endif
		/*
		 * Restoring unplug counter.
		 */
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG,
					XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);
		xil_printf("> Rx Training done !!! (BW: 0x%x, Lanes: 0x%x, Status: 0x%x;0x%x).\n\r",
					XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_DPCD_LINK_BW_SET),
					XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_DPCD_LANE_COUNT_SET),
					XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_DPCD_LANE01_STATUS),
					XDpRxSs_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_DPCD_LANE23_STATUS));

		/*
		 * Keep the Audio FIFO in reset state
		 */
		ReadVal = XDp_ReadReg(TX_CLK_RST_ADDR, 0x8);
		ReadVal = (ReadVal & 0x7);
		ReadVal = (ReadVal & 0x1); //Putting FIFO in reset bit[1]
		XDp_WriteReg(TX_CLK_RST_ADDR, 0x8, ReadVal);
		tx_aud_started = 0;
	}

	if(DpRxSsInst.no_video_trigger == 1){
		Dp21RxSs_Pt_FbStop();
#ifdef Txo
		Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
		XDpTxSs_Stop(&DpTxSsInst);
#endif
		DpRxSsInst.no_video_trigger = 0;
		tx_after_rx = 0;
		rx_all_detect = 0;
	}

	if((Video_valid == 1) && (rx_trained == 1) && (DpRxSsInst.VBlankCount > (20+180*ENABLE_HDCP_IN_DESIGN))){
		Tx_only=0;
		Video_valid = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
		DpRxSsInst.no_video_trigger = 0;

		/*
		 * VBLANK Management
		 */
		DpRxSsInst.VBlankCount = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

		XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
				XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK |
				XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK);

		XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
		XDp_RxDtgDis(DpRxSsInst.DpPtr);
		XDp_RxDtgEn(DpRxSsInst.DpPtr);
		AudioinfoFrame.frame_count = 0;
		AudioinfoFrame.all_count = 0;
		XDpRxSs_AudioDisable(&DpRxSsInst);
		XDpRxSs_AudioEnable(&DpRxSsInst);
		tx_after_rx = 0;

#ifdef Txo
		tx_after_rx = 1;
#endif
		track_msa = Dp21RxSs_Pt_DetectResolution(&DpRxSsInst);
	}
}
#endif

extern u8 prog_misc1;
extern u8 prog_fb_rd;

#ifdef Txo
void Dp21RxSs_Pt_TxTracking(void)
{
	/*
	 * When TX is cable is connected, the application will re-initiate the
	 * TX training. Note that EDID is not updated.
	 * Hence you should not change the monitors at runtime
	 */
	if (tx_is_reconnected != 0 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1) { /**< If Tx cable is reconnected */
		xil_printf ("TX Cable Connected !!\r\n");
		tx_aud_started = 0;
		Dp21RxSs_Pt_TxHpdCon(&DpTxSsInst, user_config.VideoMode_local);
		tx_is_reconnected = 0;
		Dp21RxSs_Pt_FbStopRd();

		Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
		XDpTxSs_Stop(&DpTxSsInst);
		AudioinfoFrame.all_count = 0;
		tx_done = 0;
		tx_after_rx = 1;
	} else if(/*(tx_is_reconnected != 0) &&*/ (Tx_only == 1) && (Tx_only_done == 0)) {
		xil_printf ("TX enabled for Txo mode !!\r\n");
		Tx_only_done=1;
		Dp21RxSs_Pt_FbStopRd();

		Dp21RxSs_Pt_TxStart(&DpTxSsInst, 0);
		XDpTxSs_Stop(&DpTxSsInst);
		tx_done = 0;

		/*
		 * Configure Tx TPG for start in txo mode for displaying FHD output
		 * TPG start
		 */
		XV_tpg_DisableAutoRestart(&TpgInst);
		XV_tpg_Set_height(&TpgInst, 1080);
		XV_tpg_Set_width(&TpgInst, 1920);
		XV_tpg_Set_bckgndId(&TpgInst, 0x1/*0x9*/);
		XV_tpg_Set_ovrlayId(&TpgInst, 0x0);
		XV_tpg_Set_colorFormat(&TpgInst, XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB);
		XV_tpg_Set_enableInput(&TpgInst, 0);
		XV_tpg_EnableAutoRestart(&TpgInst);

		user_config_struct user_config_txo;
		user_config_txo.VideoMode_local = XVIDC_VM_1920x1080_30_P;
		user_config_txo.user_bpc = 8;
		user_config_txo.user_format = 1;
		user_config_txo.user_pattern = 1;
		Dp21RxSs_Pt_PhyConfig(&VPhyInst, SinkMaxLinkrate, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
		Dp21RxSs_Pt_StartTx(SinkMaxLinkrate, SinkMaxLanecount, user_config_txo, 0);
		XV_tpg_Start(&TpgInst);
	}
	else {
		tx_is_reconnected = 0;
	}

	/*
	 * HPD to be considered only when TX & RX is live
	 */
	if(hpd_pulse_con_event == 1 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1 && tx_done == 1) {
			xil_printf ("HPD Pulse detected !!\r\n");
			hpd_pulse_con_event = 0;
			Dp21RxSs_Pt_TxHpdPulseCon(&DpTxSsInst, Msa_Pt);
	} else {
		hpd_pulse_con_event = 0;
	}
}
#endif

/*****************************************************************************/
/**
*
* This function polls the hdcp example module
*
* @return
*   void
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
void Dp21RxSs_Pt_Hdcp1xExamplePoll(void)
{
#if ENABLE_HDCP1x_IN_RX
	XDpRxSs_Poll(&DpRxSsInst);
#elif ENABLE_HDCP1x_IN_TX
	XDpTxSs_Poll(&DpTxSsInst);
#endif
}

/*****************************************************************************/
/**
*
* This function returns the state of Encryption
*
* @return
*   void
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
u8 Dp21RxSs_Pt_IsRxEncrypted(XDpRxSs *InstancePtr)
{
	u8 Status = 0;

#if ENABLE_HDCP1x_IN_RX
	Status = XHdcp1x_IsEncrypted(InstancePtr->Hdcp1xPtr);
#elif ENABLE_HDCP22_IN_RX
	Status |= XHdcp22Rx_Dp_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr);
#else
	(void)InstancePtr;
	(void)Status;
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function returns the state of Encryption
*
* @return
*   void
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
u8 Dp21RxSs_Pt_IsTxEncrypted(XDpTxSs *InstancePtr)
{
	u8 Status = 0;

#if ENABLE_HDCP1x_IN_TX
	Status = XHdcp1x_IsEncrypted(InstancePtr->Hdcp1xPtr);
#elif ENABLE_HDCP22_IN_TX
	Status |= XHdcp22Tx_Dp_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr);
#else
	(void)InstancePtr;
	(void)Status;
#endif

	return Status;
}
