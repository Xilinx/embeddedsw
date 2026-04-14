/*******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.6   GM    03/06/26  Initial release.
*
* </pre>
*
******************************************************************************/
#include "main.h"
#include "tx.h"
#include "rx.h"
#include "xvidframe_crc.h"

volatile u8 hpd_pulse_con_event;		/**< This variable triggers hpd_pulse_con */
extern XDpRxSs DpRxSsInst;			/**< The DPRX Subsystem instance. */
extern XDpTxSs DpTxSsInst;			/**< The DPTX Subsystem instance. */
extern Video_CRC_Config VidFrameCRC_rx;		/**< Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_rx_1;	/**< Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_rx_2;	/**< Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_rx_3;	/**< Video Frame CRC instance */
extern Video_CRC_Config VidFrameCRC_tx;
extern XAxis_Switch axis_switch;
extern volatile int tx_is_reconnected; 	/**< This variable to keep track of the status of Tx link */

extern XVphy VPhyInst;			/**< The DPRX Subsystem instance.*/
extern XIntc IntcInst;

void Dp21RxSs_PtMst_UnplugProc(void);
void Dp21RxSs_PtMst_RxTracking(void);
void Dp21RxSs_PtMst_TxTracking(void);
void Dp21RxSs_PtMst_StartTxAfterRx(void);

int Dp21RxSs_PtMst_ConfigRdFb(u32 StrideInBytes,
		    XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr);
int Dp21RxSs_PtMst_ConfigRdFbTrunc(u32 offset);
int Dp21RxSs_PtMst_ConfigWrFb(u32 StrideInBytes,
		    XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr);

void Dp21RxSs_PtMst_PtSetup(void);
int Dp21RxSs_PtMst_DetectResolution(void *InstancePtr, u8 Stream);
int Dp21RxSs_PtMst_DetectColor(void *InstancePtr,
		     XDpTxSs_MainStreamAttributes Msa[4], u8 plugged);

void Dp21RxSs_PtMst_ResetRdFb(void);
void Dp21RxSs_PtMst_ResetWrFb(void);
char Dp21RxSs_PtMst_InByteLocal(void);
void Dp21RxSs_PtMst_PtHelpMenu();
void Dp21RxSs_PtMst_LaneLinkRateHelpMenu(void);

volatile u8 tx_after_rx = 0;
volatile u8 Video_valid = 0;

u8 LineRate_init_tx;
u8 LaneCount_init_tx;
XDpTxSs_MainStreamAttributes* Msa_test;
XDpTxSs_MainStreamAttributes Msa_test_tx[4];
u8 rx_all_detect = 0;
user_config_struct user_config;
XVidC_VideoMode VmId;

extern volatile u8 rx_unplugged;
volatile u8 rx_trained = 0;

extern lane_link_rate_struct lane_link_table[];
extern u32 StreamOffset[4];
volatile u8 tx_done = 0;
volatile u8 Tx_only = 0;
volatile u8 Tx_only_done = 0;
u8 rx_and_tx_started = 0;
int track_msa = 0;
int track_color = 0;
extern u16 fb_wr_count;
extern u16 fb_rd_count;

u8 SinkMaxLinkrate = 0, SinkMaxLanecount = 0;
u8 SinkCap[16], SinkExtendedCap[16];
u8 config_128_132b;

extern XVphy_PllType VPHY_TX_PLL_TYPE;
extern XVphy_ChannelId VPHY_TX_CHANNEL_TYPE;
u8 Stream = 0;
extern u8 edid_monitor[384];

void Dp21RxSs_PtMst_PtSetup(void)
{
	u32 Status;
	u8 exit;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 connected = 0;

	/*
	 * In Passthrough setting the Stream 0 as the strm to be fwd to TX
	 */
	XAxisScr_MiPortEnable (&axis_switch, 0, 0);
	XAxisScr_MiPortDisable(&axis_switch, 1);
	XAxisScr_MiPortEnable (&axis_switch, 2, 1);
	XAxisScr_MiPortEnable (&axis_switch, 3, 2);
	XAxisScr_MiPortEnable (&axis_switch, 4, 3);
	XAxisScr_RegUpdateEnable (&axis_switch);

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
	/*
	 * Waking up the monitor
	 */
	Dp21RxSs_PtMst_SinkPowerCycle();
	Read_EDID(&DpTxSsInst, 0);
#endif

#ifdef Rx
	xil_printf ("Cloning the sink EDID into RX..\r\n");
	Set_EDID (edid_monitor);
#endif

#ifdef Rx
	/*
	 * Set Link rate and lane count to maximum
	 *
	 * The RX is always set for Max capability of 5.4G and the
	 * extended capability bit is set.
	 * DP1.4 Sources are supposed to read the extended capability bit
	 * and decide whether the sink is 8.1G capable or not.
	 */
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
	DpRxSs_Setup();
#endif

#ifdef Tx
	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;

	/*
	 * Reading and clearing any residual interrupt on TX
	 * Also Masking the interrupts
	 */
	XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
		    0x140);
	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
		      XDP_TX_INTERRUPT_MASK, 0xFFF);
#endif

	/*
	 * Resetting AUX logic. Needed for some Type C based connectors
	 */
	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);

	Dp21RxSs_PtMst_PtHelpMenu();

	while (1) {
		CommandKey = 0;

		CommandKey = Dp21RxSs_PtMst_Getc(0xff);
		Command = atoi(&CommandKey);
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);
			switch(CommandKey) {

				case 'a':
					Stream = 0;
					/*
					 * Selecting Stream 0 to be fwd to TX
					 */
					XAxisScr_MiPortEnable (&axis_switch, 0, 0);
					XAxisScr_MiPortDisable(&axis_switch, 1);
					XAxisScr_MiPortEnable (&axis_switch, 2, 1);
					XAxisScr_MiPortEnable (&axis_switch, 3, 2);
					XAxisScr_MiPortEnable (&axis_switch, 4, 3);
					XAxisScr_RegUpdateEnable (&axis_switch);
					Dp21RxSs_PtMst_DetectResolution(DpRxSsInst.DpPtr, Stream);
					tx_after_rx = 1;
				break;

				case 'b':
					Stream = 1;
					/*
					 * Selecting Stream 1 to be fwd to TX
					 */
					XAxisScr_MiPortEnable (&axis_switch, 0, 1);
					XAxisScr_MiPortEnable (&axis_switch, 1, 0);
					XAxisScr_MiPortDisable(&axis_switch, 2);
					XAxisScr_MiPortEnable (&axis_switch, 3, 2);
					XAxisScr_MiPortEnable (&axis_switch, 4, 3);
					XAxisScr_RegUpdateEnable (&axis_switch);
					Dp21RxSs_PtMst_DetectResolution(DpRxSsInst.DpPtr, Stream);
					tx_after_rx = 1;
				break;

				case 'c':
					Stream = 2;
					/*
					 * Selecting Stream 2 to be fwd to TX
					 */
					XAxisScr_MiPortEnable (&axis_switch, 0, 2);
					XAxisScr_MiPortEnable (&axis_switch, 1, 0);
					XAxisScr_MiPortEnable (&axis_switch, 2, 1);
					XAxisScr_MiPortDisable(&axis_switch, 3);
					XAxisScr_MiPortEnable (&axis_switch, 4, 3);
					XAxisScr_RegUpdateEnable (&axis_switch);
					Dp21RxSs_PtMst_DetectResolution(DpRxSsInst.DpPtr, Stream);
					tx_after_rx = 1;
				break;

				case 'd':
					Stream = 3;
					/*
					 * Selecting Stream 3 to be fwd to TX
					 */
					XAxisScr_MiPortEnable (&axis_switch, 0, 3);
					XAxisScr_MiPortEnable (&axis_switch, 1, 0);
					XAxisScr_MiPortEnable (&axis_switch, 2, 1);
					XAxisScr_MiPortEnable (&axis_switch, 3, 2);
					XAxisScr_MiPortDisable(&axis_switch, 4);
					XAxisScr_RegUpdateEnable (&axis_switch);
					Dp21RxSs_PtMst_DetectResolution(DpRxSsInst.DpPtr, Stream);
					tx_after_rx = 1;
				break;

				case '1':
					Dp21RxSs_PtMst_LaneLinkRateHelpMenu();
					exit = 0;
					while (exit == 0) {
						CmdKey[0] = 0;
						Command = 0;
						CmdKey[0] = Dp21RxSs_PtMst_InByteLocal();
						if (CmdKey[0]!=0) {
							Command = (int)CmdKey[0];

							switch (CmdKey[0]) {
								case 'x':
									exit = 1;
									break;

								default:
									xil_printf("You have selected command '%c'\r\n", CmdKey[0]);
									if (CmdKey[0] >= 'a' && CmdKey[0] <= 'z') {
										Command = CmdKey[0] -'a' + 10;
										exit = 1;
									} else if (Command > 47 && Command < 58) {
										Command = Command - 48;
										exit = 1;
									} else if (Command >= 58 || Command <= 47) {
										Dp21RxSs_PtMst_LaneLinkRateHelpMenu();
										exit = 0;
										break;
									}
									xil_printf("\r\nSetting LineRate:%x  "
										   "LaneCounts:%x\r\n",
										   lane_link_table[Command].link_rate,
										   lane_link_table[Command].lane_count);

#ifdef Rx
									Dp21RxSs_PtMst_UnplugProc();
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
									Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_INTERRUPT_MASK, 0xFFF);

									Dp21RxSs_PtMst_FbStop();
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
#ifdef Rx
					if (DpRxSsInst.link_up_trigger ==1) {
						xil_printf("==========RX Debug Data===========\r\n");
						XDpRxSs_ReportLinkInfo(&DpRxSsInst);
						XDpRxSs_ReportMsaInfo(&DpRxSsInst);
					}
#endif
#ifdef Tx
					xil_printf("==========TX Debug Data===========\r\n");
					XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					XDpTxSs_ReportLinkInfo(&DpTxSsInst);
					XDpTxSs_ReportVtcInfo(&DpTxSsInst);
#endif

#ifdef Rx
					xil_printf("DP RX addr is %x\r\n", DpRxSsInst.DpPtr->Config.BaseAddr);
#endif
#ifdef Tx
					xil_printf("DP TX addr is %x\r\n", DpTxSsInst.DpPtr->Config.BaseAddr);
#endif
					break;

				case '3':
					/*
					 * Disabling TX interrupts
					 */
					Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFF);
#ifdef Rx
					Dp21RxSs_PtMst_UnplugProc();
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

					user_config.user_bpc = Msa_test_tx[0].BitsPerColor;
					user_config.VideoMode_local = VmId;
					user_config.user_pattern = 0; /**< Pass-through (Default) */
					user_config.user_format = Msa_test_tx[0].ComponentFormat;
#ifdef Tx
					Dp21RxSs_PtMst_RdFbStop();

					/*
					 * Waking up the monitor
					 */
					Dp21RxSs_PtMst_SinkPowerCycle();
#endif

					/*
					 * This configures the vid_phy for line rate to start with
					 */
#ifdef Tx
					Dp21RxSs_PtMst_PhyConfig(&VPhyInst, LineRate_init_tx, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
#endif
					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					tx_after_rx = 1;
					DpTxSsInst.no_video_trigger = 1;
					Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
						      XDP_TX_AUDIO_CONTROL, 0x0);
					break;

#ifdef Tx
				case '6':
					Dp21RxSs_PtMst_RdFbStop();
					Dp21RxSs_PtMst_RdFbStart(Msa_test, Stream);
					break;
#endif
				case 'm':
				    if (DpRxSsInst.link_up_trigger ==1) {
						xil_printf("========== Rx CRC===========\r\n");
#ifdef Rx
						ReportVideoCRC(Stream+1);
#endif
					}

					xil_printf ("========== Tx CRC===========\r\n");
					XVidFrameCrc_Report(&VidFrameCRC_tx);

					break;

				case 'n':
#ifdef Tx
					/*
					 * Waking up the monitor
					 */
					Dp21RxSs_PtMst_SinkPowerCycle();
					Read_EDID(&DpTxSsInst,0);
#endif
#ifdef Rx
					Set_EDID(edid_monitor);
#endif
					break;

				case '.':
					Dp21RxSs_PtMst_PtHelpMenu();
					break;

				case 'x':
					DpRxSsInst.link_up_trigger = 0;

					/* Disabling Rx */
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDpRxSs_Reset(&DpRxSsInst);

					Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
						      XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
					Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
						      XDP_RX_LINK_ENABLE, 0x0);
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
					DpRxSsInst.VBlankCount = 0;

					/*
					 * Disabling Tx
					 */
					XDpTxSs_Stop(&DpTxSsInst);

#ifdef XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
					Vpg_Audio_stop();
#endif
					Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_ENABLE, 0x0);
					XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
					Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,0xFFF);


					Dp21RxSs_PtMst_OperationMenu();
					return;

				default :
					Dp21RxSs_PtMst_PtHelpMenu();
					break;
			}
		} /**< end if */


#ifdef Tx
		/*
		 * Tx side process, when there is no video on TX, stop the FB and I2S RX
		 */
		if(DpTxSsInst.no_video_trigger == 1){ /**< Stop frameBuffer if Tx is lost */
			Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			DpTxSsInst.no_video_trigger = 0;
			tx_done = 0;
		}

		/*
		 * Check for HPD and HPD Pulse Interrupt triggers
		 */
		Dp21RxSs_PtMst_TxTracking();
#endif

#ifdef Rx
		/*
		 * Rx and pass-through side process
		 */
		Dp21RxSs_PtMst_RxTracking();
#endif

#ifdef Tx
		/*
		 * Wait for few frames to ensure valid video is received
		 */
		if (tx_after_rx == 1 && rx_trained == 1 &&
				DpRxSsInst.link_up_trigger == 1) {
			tx_after_rx = 0;
			if (track_msa == 1) {
				Dp21RxSs_PtMst_StartTxAfterRx();
				/*
				 * It is observed that some monitors do not give HPD
				 * pulse. Hence checking the link to re-trigger.
				 */
				Status = XDpTxSs_CheckLinkStatus(&DpTxSsInst);
				if (Status != XST_SUCCESS) {
					xil_printf ("^*^");
					hpd_pulse_con(&DpTxSsInst, Msa_test);
				}
				tx_done = 1;
			} else {
				tx_done = 0;
				xil_printf ("Cannot Start TX...\r\n");
			}
		}
#endif
	} /**< End while(1) */
}

#ifdef Tx
void Dp21RxSs_PtMst_StartTxAfterRx (void)
{
	u32 Status;
	rx_all_detect = 1;
	u8 supports_20g = 0;
	u8 supports_135g = 0;
	u8 supports_10g = 0;
	const XVidC_VideoTimingMode *VtmPtr;
	(void)VtmPtr;
	/*
	 * Check monitor capability
	 */
	u8 max_cap_org=0;
	u8 max_cap_lanes=0;

	Status = XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LINK_RATE, 1, &max_cap_org);
	Status |= XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_MAX_LANE_COUNT, 1, &max_cap_lanes);
	u8 rData = 0;

	/*
	 * Check the EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT bit
	 */
	XDp_TxAuxRead(DpTxSsInst.DpPtr, XDP_DPCD_TRAIN_AUX_RD_INTERVAL,
			1, &rData);

	/*
	 * if EXTENDED_RECEIVER_CAPABILITY_FIELD is enabled
	 */
	if(rData & 0x80) {
		/*
		 * Read maxLineRate
		 */
		XDp_TxAuxRead(DpTxSsInst.DpPtr, 0x2201, 1, &rData);
		if (rData == XDP_DPCD_LINK_BW_SET_810GBPS) {
			max_cap_org = 0x1E;
		}
	}

#ifdef Tx
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
			} else if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR20) {
				/*
				 * It can be 10G also, but keeping 20G
				 */
				LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR20;
			} else {
				LineRate_init_tx = SinkMaxLinkrate;
			}
			xil_printf("VCU118 QPLL1 doesn't support 13.5g hence Training TX at 0x%x Linkrate\r\n",
				   LineRate_init_tx);
		} else {
			if (SinkMaxLinkrate == XDP_TX_LINK_BW_SET_UHBR135) {
				/*
				 * QPLL1 cannot run at 13.5
				 * Assuming it supports 10G
				 */
				SinkMaxLinkrate = XDP_TX_LINK_BW_SET_UHBR10;
			}

			if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_UHBR20) {
				if (supports_20g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR20;
				} else if (supports_135g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;      /**< TX QPLL1 not capable */
				} else if (supports_10g == 1) {
					LineRate_init_tx = XDP_TX_LINK_BW_SET_UHBR10;
				} else {
					LineRate_init_tx = SinkMaxLinkrate;
				}
			} else if (DpRxSsInst.UsrOpt.LinkRate == XDP_TX_LINK_BW_SET_UHBR10) {
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

	/*
	 * Assigning the MSA structure of the selected stream to Msa_test_tx
	 * this will be used to configure the single stream of TX
	 */
	Msa_test_tx[0] = Msa_test[Stream];
	Msa_test_tx[1] = Msa_test[Stream];
	Msa_test_tx[2] = Msa_test[Stream];
	Msa_test_tx[3] = Msa_test[Stream];

	user_config.user_bpc = Msa_test_tx[0].BitsPerColor;

	user_config.user_pattern = 0; /**< pass-through (Default) */

	/*
	 * Check component Format
	 */
	if (Msa_test_tx[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422 + 1;
	} else if(Msa_test_tx[0].ComponentFormat ==
			XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444){
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444 + 1;
	} else {
		user_config.user_format = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB + 1;
	}
	user_config.VideoMode_local = VmId;

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_INTERRUPT_MASK, 0xFFF);

	Dp21RxSs_PtMst_RdFbStop();

	/*
	 * Waking up the monitor
	 */
	Dp21RxSs_PtMst_SinkPowerCycle();

	LaneCount_init_tx = LaneCount_init_tx & 0x7;

	Dp21RxSs_PtMst_StartTx (LineRate_init_tx, LaneCount_init_tx, user_config, Msa_test_tx);

	Dp21RxSs_PtMst_RdFbStart(Msa_test_tx, 0);

	/*
	 * Start the data
	 */
	rx_and_tx_started = 1;
}
#endif

extern int tx_started;

#ifdef Rx
void Dp21RxSs_PtMst_UnplugProc (void)
{
	tx_done = 0;
	tx_after_rx = 0;
	rx_trained = 0;
	tx_started = 0;
	rx_unplugged = 0;

	DpRxSsInst.VBlankCount = 0;
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	Dp21RxSs_PtMst_FbStop();

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x80);
	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr, 0x1C, 0x0);
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

	Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	DpRxSs_Setup();
}
#endif

#ifdef Rx
void Dp21RxSs_PtMst_RxTracking (void)
{
	if (rx_unplugged == 1) {
		xil_printf ("Training Lost !! Cable Unplugged !!!\r\n");
		Dp21RxSs_PtMst_UnplugProc();
	} else if (DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1 && rx_trained == 0) {
		if (!Tx_only) {
			XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x140);
			Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr,
				      XDP_TX_INTERRUPT_MASK, 0xFFF);
		}
		Dp21RxSs_PtMst_FbStop();

		tx_after_rx = 0;
		DpRxSsInst.VBlankCount++;
		rx_trained = 1;
#ifdef Tx
		if (!Tx_only) {
			Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			XDpTxSs_Stop(&DpTxSsInst);
		}
#endif
		/*
		 * Restoring unplug counter
		 */
		Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
			      XDP_RX_CDR_CONTROL_CONFIG,
			      XDP_RX_CDR_CONTROL_CONFIG_TDLOCK_DP159);
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
	}

	if (DpRxSsInst.no_video_trigger == 1) {
		Dp21RxSs_PtMst_FbStop();
		if (!Tx_only) {
			Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
			XDpTxSs_Stop(&DpTxSsInst);
		}
		DpRxSsInst.no_video_trigger = 0;
		tx_after_rx = 0;
		rx_all_detect = 0;
	}

	if ((Video_valid == 1) && (rx_trained == 1)) {
		Video_valid = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
				       XDP_RX_INTERRUPT_MASK_VIDEO_MASK);
		DpRxSsInst.no_video_trigger = 0;

		/*
		 * VBLANK Management
		 */
		DpRxSsInst.VBlankCount = 0;
		XDp_RxInterruptDisable(DpRxSsInst.DpPtr, XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
		XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0x00010410);

		XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
				      XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK |
				      XDP_RX_INTERRUPT_MASK_TRAINING_LOST_MASK);

		XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
		XDp_RxDtgDis(DpRxSsInst.DpPtr);
		XDp_RxDtgEn(DpRxSsInst.DpPtr);

		if (!Tx_only) {
			tx_after_rx = 1;
		} else {
			tx_after_rx = 0;
		}
		track_msa = Dp21RxSs_PtMst_DetectResolution(DpRxSsInst.DpPtr, Stream);
	}
}
#endif

extern u8 prog_misc1;
extern u8 prog_fb_rd;

#ifdef Tx
void Dp21RxSs_PtMst_TxTracking(void)
{
	/*
	 * When TX is cable is connected, the application will re-initiate the
	 * TX training. Note that EDID is not updated.
	 * Hence you should not change the monitors at runtime
	 */
	if (tx_is_reconnected != 0 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1) { /**< If Tx cable is reconnected */
		xil_printf ("TX Cable Connected !!\r\n");
		hpd_con(&DpTxSsInst, user_config.VideoMode_local);
		tx_is_reconnected = 0;
		Dp21RxSs_PtMst_RdFbStop();
		Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		tx_done = 0;
		tx_after_rx = 1;
	}
	else if ((Tx_only == 1) && (Tx_only_done == 0)) {
		xil_printf ("TX enabled for Txo mode !!\r\n");
		Tx_only_done = 1;

		Dp21RxSs_PtMst_RdFbStop();
		Dp21RxSs_PtMst_Write(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_ENABLE, 0x0);
		XDpTxSs_Stop(&DpTxSsInst);
		tx_done = 0;

		user_config_struct user_config_txo;

		user_config_txo.VideoMode_local = XVIDC_VM_1920x1080_30_P;
		user_config_txo.user_bpc = 8;
		user_config_txo.user_format = 1;
		user_config_txo.user_pattern = 1;

		Dp21RxSs_PtMst_PhyConfig(&VPhyInst, SinkMaxLinkrate, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
		Dp21RxSs_PtMst_StartTx(SinkMaxLinkrate, SinkMaxLanecount, user_config_txo, 0);
	} else {
		tx_is_reconnected = 0;
	}

	/*
	 * HPD to be considered only when TX & RX is live
	 */
	if (hpd_pulse_con_event == 1 && rx_trained == 1 &&
			DpRxSsInst.link_up_trigger == 1 && tx_done == 1) {
			xil_printf ("HPD Pulse detected !!\r\n");
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst, Msa_test_tx);
	} else {
		hpd_pulse_con_event = 0;
	}
}
#endif
