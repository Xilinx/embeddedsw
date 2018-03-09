#include "main.h"
#include "tx.h"
#include "rx.h"

u8 UpdateBuffer[sizeof(u8) + 16];
u8 WriteBuffer[sizeof(u8) + 16];
u8 ReadBuffer[16];

XV_frmbufrd_Config frmbufrd_cfg;
XV_frmbufwr_Config frmbufwr_cfg;

int ConfigFrmbuf_rd(u32 StrideInBytes,
					XVidC_ColorFormat Cfmt,
					XVidC_VideoStream *StreamPtr);
int ConfigFrmbuf_rd_trunc(u32 offset);
int ConfigFrmbuf_wr(u32 StrideInBytes,
							XVidC_ColorFormat Cfmt,
							XVidC_VideoStream *StreamPtr);
void remap_start(XDpTxSs_MainStreamAttributes Msa[4]);


void Dplb_Main(void);
void operationMenu(void);
void Dppt_DetectResolution(void *InstancePtr,
							XDpTxSs_MainStreamAttributes Msa[4]);
void frameBuffer_stop(XDpTxSs_MainStreamAttributes Msa[4]);
void frameBuffer_start(XVidC_VideoMode VmId,
				XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);
void resetIp();
void power_down_HLSIPs(void);
void power_up_HLSIPs(void);
void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
						u16 height, u8 color_format);
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);
void edid_change(int page);
char inbyte_local(void);
void pt_help_menu();
void resolution_help_menu(void);
void select_link_lane(void);
void sub_help_menu(void);

//u8 edid_page;

extern lane_link_rate_struct lane_link_table[];
extern XVidC_VideoMode resolution_table[];

void Dplb_Main(void){
	u32 Status;
	u8 UserInput;
	u8 LineRate_init_tx;
	u8 LaneCount_init_tx;
	user_config_struct user_config;
	XDpTxSs_MainStreamAttributes Msa[4];
	u8 Edid_org[128];
	u8 Edid1_org[128];
	u8 exit;

	char CmdKey[2];
	unsigned int Command;


//	edid_page = 0;


	/* Set Link rate and lane count to maximum */
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
	/* Setting EDID to be default 8K capable one */
	edid_change(1);

	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
					&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);

	/* Setting up Tx side */
	DpTxSs_Setup(&LineRate_init_tx, &LaneCount_init_tx, Edid_org, Edid1_org);


	DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
	DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
	DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat = 0x0;

	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));

	// Disabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_MASK, 0xFFF);
	LineRate_init_tx = 0x06;
	LaneCount_init_tx = 4;
	Status = set_vphy(LineRate_init_tx);

	XDpTxSs_Stop(&DpTxSsInst);
	XDpTxSs_Reset(&DpTxSsInst);
	user_config.user_bpc=8;
	user_config.VideoMode_local
					=XVIDC_VM_800x600_60_P;
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;


	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);


	pt_help_menu();
	while (1){
		UserInput = XUartPs_RecvByte_NonBlocking();
		if(UserInput!=0){
			xil_printf("UserInput: %c\r\n",UserInput);

			switch(UserInput){
			// Toggle HPD
			case '0':
				XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
											0xFFF8FFFF);
				XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
											0x80000000);
				// Disabling TX interrupts
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK, 0xFFF);
				XDpTxSs_Stop(&DpTxSsInst);

				XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
				xil_printf("\r\n- HPD Toggled for 5ms! -\n\r");

				break;

			// Change resolutions
			case '1':
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
								exit = 1;
							}

							else if (Command > 47 && Command < 58) {
								Command = Command - 48;
								exit = 1;
							}
							else if (Command >= 58 || Command <= 47) {
								resolution_help_menu();
								exit = 0;
								break;
							}
							xil_printf ("\r\nSetting resolution...\r\n");
							//audio_on = 0;
							user_config.VideoMode_local =
												resolution_table[Command];


							start_tx (LineRate_init_tx,LaneCount_init_tx,
											user_config, 0);

							LineRate_init_tx = get_LineRate();
							LaneCount_init_tx = get_Lanecounts();

							//exit = done;
							break;
						}
					}
				}
				break;
			// Display Debug info
			case '2':
			//	debug_info();
				xil_printf (
			"==========MCDP6000 Debug Data===========\r\n");
				xil_printf("0x0700: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0700));
				xil_printf("0x0704: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0704));

				xil_printf("0x0754: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0754));
				xil_printf("0x0B20: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B20));
				xil_printf("0x0B24: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B24));
				xil_printf("0x0B28: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B28));
				xil_printf("0x0B2C: %08x\n\r",
XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B2C));

				xil_printf (
						"==========RX Debug Data===========\r\n");
				XDpRxSs_ReportLinkInfo(&DpRxSsInst);
				XDpRxSs_ReportMsaInfo(&DpRxSsInst);
				xil_printf (
						"==========TX Debug Data===========\r\n");
				XDpTxSs_ReportMsaInfo(&DpTxSsInst);
				XDpTxSs_ReportLinkInfo(&DpTxSsInst);
				break;

			// Change link setting
			case '3':
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
								LaneCount_init_tx =
										lane_link_table[Command].lane_count;
								LineRate_init_tx =
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
									LaneCount_init_tx =
											DpTxSsInst.Config.MaxLaneCount;
								}
								exit = 1;
							}
							else
							{
								xil_printf(
								"!!!Warning: You have selected wrong option"
								" for lane count and link rate\r\n");
								select_link_lane();
								exit = 0;
								break;
							}

							XDpRxSs_Reset(&DpRxSsInst);

							// Disabling TX interrupts
							XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
											XDP_TX_INTERRUPT_MASK, 0xFFF);
							//LineRate_init_tx = user_tx_LineRate;
							Status = set_vphy(LineRate_init_tx);

							XDpTxSs_Stop(&DpTxSsInst);
							//audio_on = 0;
							xil_printf(
						"TX Link & Lane Capability is set to %x, %x\r\n",
						LineRate_init_tx, LaneCount_init_tx);
							xil_printf(
						"Setting TX to 8 BPC and 800x600 resolution\r\n");
							XDpTxSs_Reset(&DpTxSsInst);
							user_config.user_bpc=8;
							user_config.VideoMode_local
											=XVIDC_VM_800x600_60_P;
							user_config.user_pattern=1;
							user_config.user_format = XVIDC_CSF_RGB;
							start_tx (LineRate_init_tx,LaneCount_init_tx,
													user_config, 0);
							LineRate_init_tx = get_LineRate();
							LaneCount_init_tx = get_Lanecounts();
							//exit = done;
							break;
						}
					}
				}
				sub_help_menu ();
				break;

			// Reset DTG on Rx side
			case 'r':
				xil_printf(
					"Reset Video DTG in DisplayPort Controller...\r\n");
				XDp_RxDtgDis(DpRxSsInst.DpPtr);
				XDp_RxDtgEn(DpRxSsInst.DpPtr);
				break;

			// Start Tx
			case 't':
				start_tx (LineRate_init_tx,LaneCount_init_tx,user_config, 0);
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
				XScuGic_Disable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
				XScuGic_Disable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

				Vpg_Audio_stop();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									XDP_TX_ENABLE, 0x0);
				XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x140);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144, 0xFFF);

				operationMenu();
				return;

			default :
				pt_help_menu();
			break;
			}
		}//end if

		if(DpRxSsInst.VBlankCount >= 2 && DpRxSsInst.link_up_trigger ==1){
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
			DpRxSsInst.link_up_trigger = 0;
		}

		if(DpRxSsInst.no_video_trigger == 1){
			frameBuffer_stop(Msa);
			DpRxSsInst.no_video_trigger = 0;
		}

		//Loopback Handling
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT){
			DpRxSsInst.no_video_trigger = 0;
			//VBLANK Management
			DpRxSsInst.VBlankCount = 0;
			XDp_RxDtgDis(DpRxSsInst.DpPtr);
			XDp_RxDtgEn(DpRxSsInst.DpPtr);
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
											XDP_RX_INTERRUPT_MASK_VBLANK_MASK);

			/*
			 * Disable & Enable Audio
			 */
			XDpRxSs_AudioDisable(&DpRxSsInst);
			XDpRxSs_AudioEnable(&DpRxSsInst);

			CalculateCRC();
			/*
			 * Reset CRC Test Counter in DP DPCD Space
			 */
			XVidFrameCrc_Reset(&VidFrameCRC_rx);
			VidFrameCRC_rx.TEST_CRC_CNT = 0;
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
					VidFrameCRC_rx.TEST_CRC_SUPPORTED<<5 |
					VidFrameCRC_rx.TEST_CRC_CNT);
			/* Set Pixel width in CRC engine*/
			u8 ppc_int = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_USER_PIXEL_WIDTH);
			XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
					VIDEO_FRAME_CRC_CONFIG, ppc_int);


			//Detect incoming video
			Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa);
			DpRxSsInst.VBlankCount = 0;

		}
	}//end while(1)
}
