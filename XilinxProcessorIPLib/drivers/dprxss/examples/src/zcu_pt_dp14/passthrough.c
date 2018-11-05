/*******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
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

XV_frmbufrd_Config frmbufrd_cfg;
XV_frmbufwr_Config frmbufwr_cfg;

int ConfigFrmbuf_rd(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr);
int ConfigFrmbuf_rd_trunc(u32 offset);
int ConfigFrmbuf_wr(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr);

void remap_start(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);


typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16 FormatBits;
} VideoFormats;

#define NUM_TEST_FORMATS 15
VideoFormats ColorFormats[NUM_TEST_FORMATS] =
{
	//memory format            stream format        bits per component
	{XVIDC_CSF_MEM_RGBX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUVX8,      XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_YUYV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_RGBX10,     XVIDC_CSF_RGB,       10},
	{XVIDC_CSF_MEM_YUVX10,     XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_Y_UV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_Y_UV8_420,  XVIDC_CSF_YCRCB_420, 8},
	{XVIDC_CSF_MEM_RGB8,       XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUV8,       XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y_UV10,     XVIDC_CSF_YCRCB_422, 10},
	{XVIDC_CSF_MEM_Y_UV10_420, XVIDC_CSF_YCRCB_420, 10},
	{XVIDC_CSF_MEM_Y8,         XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y10,        XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_BGRX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8}
};

void DpPt_Main(void);
void operationMenu(void);
void Dppt_DetectResolution(void *InstancePtr,
		XDpTxSs_MainStreamAttributes Msa[4]);

void frameBuffer_stop(XDpTxSs_MainStreamAttributes Msa[4]);
void frameBuffer_start(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void resetIp(XDpTxSs_MainStreamAttributes Msa[4]);
void power_down_HLSIPs(void);
void power_up_HLSIPs(void);
void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format);

void DpPt_TxSetMsaValuesImmediate(void *InstancePtr);
void edid_change(int page);
char inbyte_local(void);
void pt_help_menu();
void select_rx_quad(void);
u32 xil_gethex(u8 num_chars);
void DpPt_LaneLinkRateHelpMenu(void);

u8 edid_page;

extern lane_link_rate_struct lane_link_table[];
extern u32 StreamOffset[4];


void DpPt_Main(void){
	u32 Status;
	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;
	XVidC_VideoMode VmId;
	u8 LineRate_init_tx;
	u8 LaneCount_init_tx;
	user_config_struct user_config;
	XDpTxSs_MainStreamAttributes Msa[4];
	u8 Edid_org[128];
	u8 Edid1_org[128];
	u8 Edid2_org[128];
	u8 edid_monitor[384];
	u8 use_monitor_edid = 1;
	u32 tmp;
	u8 exit;


//	char CommandKey;
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

	//Waking up the monitor
//	sink_power_cycle(400);

//	DpPt_CustomWaitUs(DpTxSsInst.DpPtr, 400000);
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
	/* Setting EDID to be default value*/
	edid_change(edid_page);


	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_SET_MSA),
					&DpPt_TxSetMsaValuesImmediate, &DpTxSsInst);

	XScuGic_Enable(&IntcInst, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);



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

							// setting new capability at here
								// clear the interrupt status
							XDp_ReadReg(
							DpTxSsInst.DpPtr->Config.BaseAddr,
							XDP_TX_INTERRUPT_STATUS);
								// mask out interrupt
							XDp_WriteReg(
							  DpTxSsInst.DpPtr->Config.BaseAddr,
							  XDP_TX_INTERRUPT_MASK, 0xFFF);

//							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
//										C_VideoUserStreamPattern[1]);
							frameBuffer_stop(Msa);
//							reconfig_clkwiz();
//							start_tracking = 0;
//							change_detected = 0;
//							IsRxTrained = 0;
//							rx_link_change_requested = 1;
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

				case '3':
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

				case '4': // re-start Tx side
					LineRate_init_tx = DpRxSsInst.UsrOpt.LinkRate;
					LaneCount_init_tx = DpRxSsInst.UsrOpt.LaneCount;

					user_config.user_bpc = Msa[0].BitsPerColor;
					user_config.VideoMode_local = VmId;
					user_config.user_pattern = 0; /*pass-through (Default)*/
					user_config.user_format = XVIDC_CSF_RGB;

					//Waking up the monitor
					sink_power_cycle();

					XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
					// This configures the vid_phy for line rate to start with
					//Even though CPLL can be used in limited case,
					//using QPLL is recommended for more coverage.
					set_vphy(LineRate_init_tx);

					LaneCount_init_tx = LaneCount_init_tx & 0x7;
					start_tx (LineRate_init_tx, LaneCount_init_tx,user_config,
								Msa);

					frameBuffer_start(VmId, Msa, 0);
					break;

				case 'c':
					xil_printf ("========== Rx CRC===========\r\n");
					xil_printf ("Rxd Hactive =  %d\r\n",
							((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,
								0xC)&0xFFFF) + 1) *
								(XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x0)));

					xil_printf ("Rxd Vactive =  %d\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0xC)>>16);
					xil_printf ("CRC Cfg     =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x0));
					xil_printf ("CRC - R/Y   =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4)&0xFFFF);
					xil_printf ("CRC - G/Cr  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x4)>>16);
					xil_printf ("CRC - B/Cb  =  0x%x\r\n",
							XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x8)&0xFFFF);
					xil_printf ("========== Tx CRC===========\r\n");
					xil_printf ("Txd Hactive =  %d\r\n",
						((XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0xC)&0xFFFF)+ 1)
						  * (XDp_ReadReg(VidFrameCRC_tx.Base_Addr,0x0)));

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
					xil_printf (
				"==========MCDP6000 Debug Data===========\r\n");
					xil_printf("0x0700: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0700));
					xil_printf("0x0704: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0704));

					xil_printf("0x0754: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0754));
					xil_printf("0x0B20: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0B20));
					xil_printf("0x0B24: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0B24));
					xil_printf("0x0B28: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0B28));
					xil_printf("0x0B2C: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x0B2C));

					xil_printf(
					"0x1294: %08x  0x12BC: %08x  0x12E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x1294),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x12BC),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x12E4));
					xil_printf(
					"0x1394: %08x  0x13BC: %08x  0x13E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x1394),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x13BC),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x13E4));
					xil_printf(
					"0x1494: %08x  0x14BC: %08x  0x14E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x1494),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x14BC),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x14E4));
					xil_printf(
					"0x1594: %08x  0x15BC: %08x  0x15E4: %08x\n\r",
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x1594),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x15BC),
				XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
						I2C_MCDP6000_ADDR, 0x15E4));

					break;

				case 'n':
					if(edid_page == 6){
						edid_page = 2;
					}else{
						edid_page++;
					}
					edid_change(edid_page);

					break;

				case 'e':
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
					XDpRxSs_MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR,
							I2C_MCDP6000_ADDR);
					xil_printf("0x0754: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0754));
					xil_printf("0x0B20: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B20));
					xil_printf("0x0B24: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B24));
					xil_printf("0x0B28: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B28));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0B2C));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x061C));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0504));
					xil_printf("0x0B2C: %08x\n\r",XDpRxSs_MCDP6000_GetRegister(
							XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0604));
				break;

//				case 'm':
//					xil_printf(" XDP_RX_USER_FIFO_OVERFLOW (0x110) = 0x%x\n\r",
//							XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//												XDP_RX_USER_FIFO_OVERFLOW));
//					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
//					ReportVideoCRC();
//					xil_printf(" XDP_RX_LINE_RESET_DISABLE (0x008) = 0x%x\n\r",
//					  XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//							  XDP_RX_LINE_RESET_DISABLE));
//					break;
					// EDID pass-thorugh changer
				case 'q' :
					if(use_monitor_edid == 1){
						// change the mode to none-pass-through mdoe
						use_monitor_edid = 0;
						xil_printf(
						"Set as EDID non-pass-through mode\r\n");
					}else{
						// This is EDID pass-through mode
						use_monitor_edid = 1;
						for(int i=0;i<(384*4);i=i+(16*4)){
							for(int j=i;j<(i+(16*4));j=j+4){
								XDp_WriteReg (
										VID_EDID_BASEADDR,
								j,edid_monitor[(i/4)+1]);
							}
						}
						for(int i=0;i<(384*4);i=i+4){
							XDp_WriteReg (
									VID_EDID_BASEADDR,
								i, edid_monitor[i/4]);
						}

						xil_printf(
							"Set as EDID pass-thorugh mode\r\n");
					}
					break;
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
								DpPt_LaneLinkRateHelpMenu();
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
								xil_printf(
		"!!!Warning: You have selected wrong option for Quad selection =%d \n\r"
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
							XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR,
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
							XPAR_IIC_0_BASEADDR,
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

//                case 'c':
//                    XDpRxSs_ReportCoreInfo(&DpRxSsInst);
//                    break;

				case '.':
					pt_help_menu();
					break;

				case ',':
						tmp = 	Xil_In32(0xFD1A0100);
						xil_printf("tmp:%x\r\n", tmp);
						u32 tmp2 = tmp | 0x600;
						Xil_Out32(0xFD1A0100, tmp2);
						usleep(10000);
						Xil_Out32(0xFD1A0100, tmp);
						tmp = 	Xil_In32(0xFD1A0100);
						xil_printf("tmp:%x\r\n", tmp);
					break;

				case '/':
						tmp = 	Xil_In32(0xFD380014);
						xil_printf("tmp:%x\r\n", tmp);
						u32 tmp3 = tmp | 0x1000;
						Xil_Out32(0xFD380014, tmp3);
	//                		usleep(10000);
	//                		Xil_Out32(0xFD380014, tmp);
	//                		tmp = 	Xil_In32(0xFD380014);
	//                		printf("tmp:%x\r\n", tmp);
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
					XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,0x144,0xFFF);


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

		//Pass-through Handling
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
					VIDEO_FRAME_CRC_CONFIG,ppc_int);

			//Wait for few frames to ensure valid video is received
			Dppt_DetectResolution(DpRxSsInst.DpPtr, Msa);
			VmId = XVidC_GetVideoModeId(
					Msa[0].Vtm.Timing.HActive,
					Msa[0].Vtm.Timing.VActive,
					Msa[0].Vtm.FrameRate,0);
			frameBuffer_stop(Msa);

			// check monitor capability
			u8 max_cap_org=0;
			u8 max_cap_lanes=0;
			u8 monitor_8K=0;
			u8 downshift4K = 0;
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


			// This block is to use with 4K60 monitor.
			if(max_cap_org <= 0x14 || monitor_8K == 0){
				// 8K resolution will be changed to 4K60
				if(Msa[0].Vtm.Timing.HActive >= 7680 &&
						Msa[0].Vtm.Timing.VActive >= 4320){
					xil_printf("\nForcing Tx to use 4K60\r\n");

					// to keep 4Byte mode, it has to be 4K60
					VmId = XVIDC_VM_3840x2160_60_P;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive /= 2;
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive /= 2;
			DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
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

					xil_printf("\nForcing Tx to use 4K60\r\n");
					// to keep 4Byte mode, it has to be 4K60
					VmId = XVIDC_VM_3840x2160_60_P;
					Msa[0].Vtm.FrameRate = 60;
					DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
					DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 =
						DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Misc0 & 0xFE;
					downshift4K = 1;
					// overwrite Rate and Lane counts
					LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
					LaneCount_init_tx = XDPTXSS_LANE_COUNT_SET_4;
				}else if(DpRxSsInst.UsrOpt.LinkRate ==
								XDP_TX_LINK_BW_SET_810GBPS){
					LineRate_init_tx = XDP_TX_LINK_BW_SET_540GBPS;
					downshift4K = 1;
				}
			}

			user_config.VideoMode_local = VmId;

			//Waking up the monitor
			sink_power_cycle();

			XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
			// This configures the vid_phy for line rate to start with
			//Even though CPLL can be used in limited case,
			//using QPLL is recommended for more coverage.
			set_vphy(LineRate_init_tx);

			LaneCount_init_tx = LaneCount_init_tx & 0x7;
			//xil_printf("downshift4K:%d\r\n", downshift4K);
			if(downshift4K == 0){
				start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, Msa);
				frameBuffer_start(VmId, Msa, downshift4K);
			}else{
				start_tx (LineRate_init_tx, LaneCount_init_tx,user_config, 0);
				frameBuffer_start(VmId, Msa, downshift4K);
			}

			DpRxSsInst.VBlankCount = 0;
		}

		// Rx cable disconnect check. If Rx is not connected,
		// then shutdown Tx as well as shutdown frameBuffer.
		u8 not_linked_up = 0;
		switch(DpRxSsInst.UsrOpt.LaneCount){
		case '4':
			if(
			(XDpRxSs_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_DPCD_LANE01_STATUS) & 0xFF) != 0x77
			||
			(XDpRxSs_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_DPCD_LANE23_STATUS) & 0xFF) != 0x77
			)
				not_linked_up = 1;
			break;
		case '2':
			if(
			(XDpRxSs_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_DPCD_LANE01_STATUS) & 0xFF) != 0x77
			)
				not_linked_up = 1;
			break;
		case '1':
			if(
			(XDpRxSs_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_DPCD_LANE01_STATUS) & 0xF) != 0x7
			)
				not_linked_up = 1;
			break;
		}

		if(not_linked_up == 1){
			frameBuffer_stop(Msa);
			XDpTxSs_Stop(&DpTxSsInst);
			Vpg_Audio_stop();
			XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
								XDP_TX_ENABLE, 0x0);
		}
	}//end while(1)
}

// This process takes in all the MSA values and find out resolution, BPC,
// refresh rate. Further this sets the pixel_width based on the pixel_clock and
// lane set. This is to ensure that it matches the values in TX driver. Else
// video cannot be passthrough. Approximation is implemented for refresh rates.
// Sometimes a refresh rate of 60 is detected as 59
// and vice-versa. Approximation is done for single digit.

void Dppt_DetectResolution(void *InstancePtr,
							XDpTxSs_MainStreamAttributes Msa[4]){

	u32 DpHres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HRES);
	u32 DpVres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VHEIGHT);
	u32 DpHres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HTOTAL);
	u32 DpVres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VTOTAL);
	u32 rxMsamisc0 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC0);
	u32 rxMsamisc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC1);
	u32 rxMsaMVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MVID);
	u32 rxMsaNVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_NVID);

	Msa[0].Misc0 = rxMsamisc0;
	Msa[0].Misc1 = rxMsamisc1;
	rxMsamisc0 = ((rxMsamisc0 >> 5) & 0x00000007);
//	u8 comp = ((rxMsamisc0 >> 1) & 0x00000003);


	u8 Bpc[] = {6, 8, 10, 12, 16};


	Msa[0].Vtm.Timing.HActive = DpHres;
	Msa[0].Vtm.Timing.VActive = DpVres;
	Msa[0].Vtm.Timing.HTotal = DpHres_total;
	Msa[0].Vtm.Timing.F0PVTotal = DpVres_total;
	Msa[0].MVid = rxMsaMVid;
	Msa[0].NVid = rxMsaNVid;
	Msa[0].HStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSTART);
	Msa[0].VStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSTART);

	Msa[0].Vtm.Timing.HSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSWIDTH);
	Msa[0].Vtm.Timing.F0PVSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSWIDTH);

	Msa[0].Vtm.Timing.HSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL);
	Msa[0].Vtm.Timing.VSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSPOL);


	Msa[0].SynchronousClockMode = rxMsamisc0 & 1;
	u8 bpc = Bpc[rxMsamisc0];
	Msa[0].BitsPerColor = bpc;
//	Msa[0].Misc0 = rxMsamisc0;
//	Msa[0].Misc1 = rxMsamisc1;

	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( (Msa[0].Misc0 & 0x6 ) == 0x2  ) {
	//YUV422
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if( (Msa[0].Misc0 & 0x6 ) == 0x4  ) {
	//RGB or YUV444
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;


	u32 recv_clk_freq =
		(((int)DpRxSsInst.UsrOpt.LinkRate*27)*rxMsaMVid)/rxMsaNVid;

	float recv_frame_clk =
		(int)( (recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) < 0.0 ?
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) :
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total)+0.9
				);

	XVidC_FrameRate recv_frame_clk_int = recv_frame_clk;
	//Doing Approximation here
	if (recv_frame_clk_int == 59 || recv_frame_clk_int == 61) {
		recv_frame_clk_int = 60;
	} else if (recv_frame_clk_int == 29 || recv_frame_clk_int == 31) {
		recv_frame_clk_int = 30;
	} else if (recv_frame_clk_int == 76 || recv_frame_clk_int == 74) {
		recv_frame_clk_int = 75;
	} else if (recv_frame_clk_int == 121 || recv_frame_clk_int == 119) {
		recv_frame_clk_int = 120;
	}

	Msa[0].Vtm.FrameRate = recv_frame_clk_int;


	Msa[0].PixelClockHz = DpHres_total * DpVres_total * recv_frame_clk_int;
	Msa[0].DynamicRange = XDP_DR_CEA;
	Msa[0].YCbCrColorimetry = XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_BT601;

	if((recv_clk_freq*1000000)>540000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount==4){
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x04);
		Msa[0].UserPixelWidth = 0x4;
	}
	else if((recv_clk_freq*1000000)>270000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount!=1){
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x02);
		Msa[0].UserPixelWidth = 0x2;
	}
	else{
		XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, 0x01);
		Msa[0].UserPixelWidth = 0x1;
	}

	//Setting CRC checker for Rx
	XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
							VIDEO_FRAME_CRC_CONFIG, Msa[0].UserPixelWidth);

		xil_printf(
			"*** Detected resolution: "
				"%lu x %lu @ %luHz, BPC = %lu, PPC = %d***\n\r",
			DpHres, DpVres,recv_frame_clk_int,bpc,Msa[0].UserPixelWidth
		);

}

/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
static u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive;
	u16 MMWidthBytes = AXIMMDataWidth/8;

	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
	  || (Cfmt == XVIDC_CSF_MEM_Y10)) {
	// 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
	stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	}
	else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
		   || (Cfmt == XVIDC_CSF_MEM_Y8)) {
	// 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
	stride = ((width+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	}
	else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)) {
	// 3 bytes per pixel (RGB8, YUV8)
	stride = (((width*3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	}
	else {
	// 4 bytes per pixel
	stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	}

	return(stride);
}

/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_wr(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr){
	int Status;

	/* Stop Frame Buffers */
	Status = XVFrmbufWr_Stop(&frmbufwr);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufWr\r\n");
	}
	XVFRMBUFWR_BUFFER_BASEADDR = (0 + (0x10000000) + 0x08000000);

	Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write "
			"buffer address\r\n");
		return(XST_FAILURE);
	}


	/* Enable Interrupt */
	XVFrmbufWr_InterruptEnable(&frmbufwr);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	/* Start Frame Buffers */
	XVFrmbufWr_Start(&frmbufwr);

	xil_printf("INFO: FRMBUFwr configured\r\n");
	return(Status);
}


/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_rd(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr)
	{

	int Status;

	/* Stop Frame Buffers */
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufRd\r\n");
	}

	XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000));

	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
	XVFrmbufRd_InterruptEnable(&frmbufrd);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
}


/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_rd_trunc(u32 offset){

	int Status;

	/* Stop Frame Buffers */
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufRd\r\n");
	}

	XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000)) + offset;

	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd,
				XV_frmbufrd_Get_HwReg_stride(&frmbufrd.FrmbufRd),
				XV_frmbufrd_Get_HwReg_video_format(&frmbufrd.FrmbufRd),
				XVFrmbufRd_GetVideoStream(&frmbufrd)
			);

	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
	XVFrmbufRd_InterruptEnable(&frmbufrd);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
}





void frameBuffer_stop(XDpTxSs_MainStreamAttributes Msa[4]) {
	resetIp(Msa);
}

void frameBuffer_start(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K) {

	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

	resetIp(Msa);

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		Cfmt = ColorFormats[7].MemFormat;
		VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_8;
	}else if(Msa[0].BitsPerColor == 10){
		Cfmt = ColorFormats[3].MemFormat;
		VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_10;
	}

	VidStream.PixPerClk  = Msa[0].UserPixelWidth;
//	VidStream.VmId = VmId;
//	TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;


	remap_start(Msa, downshift4K);


	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
					frmbufwr.FrmbufWr.Config.AXIMMDataWidth,
					&VidStream);
	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);



	// Tx side may change due to sink monitor capability
	if(downshift4K == 1){ // if sink is 4K monitor,
		VidStream.VmId = VmId; // This will be set as 4K60
		TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
		VidStream.Timing = *TimingPtr;
		VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);
	}

	ConfigFrmbuf_rd(stride, Cfmt, &VidStream);


}


/*****************************************************************************/
/**
 * This function toggles HW reset line for all IP's
 *
 * @return None
 *
 *****************************************************************************/
void resetIp(XDpTxSs_MainStreamAttributes Msa[4])
{

	u32 Status;
	/* Stop Frame Buffer and wait for IDLE */
	Status = XVFrmbufWr_Stop(&frmbufwr);
	Status |= XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS){
		xil_printf("Failed to stop FrameBuffer in resetIP\r\n");
	}

	//xil_printf("\r\nReset HLS IP \r\n");
	power_down_HLSIPs();
	usleep(10000);          //hold reset line
	power_up_HLSIPs();
	usleep(10000);          //hold reset line
	power_down_HLSIPs();
	usleep(10000);          //hold reset line
	power_up_HLSIPs();
	usleep(10000);          //hold reset line

	// ToDo   Delete this block, since it only required for FB hang issue
	// This is a work around to prevent Zynq to keep previous value in buffer
//	u32 tmp = 	Xil_In32(0xFD1A0100);
//	u32 tmp2 = tmp | 0x600; // bit10, 9 is the hp_fpd 0/1
//	Xil_Out32(0xFD1A0100, tmp2);
//	usleep(10000);
//	Xil_Out32(0xFD1A0100, tmp);
//	usleep(10000);

}

void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format){
	XV_axi4s_remap_Set_width(remap, width);
	XV_axi4s_remap_Set_height(remap, height);
	XV_axi4s_remap_Set_ColorFormat(remap, color_format);
	XV_axi4s_remap_Set_inPixClk(remap, in_ppc);
	XV_axi4s_remap_Set_outPixClk(remap, out_ppc);
}


void remap_start(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{
	remap_set(&rx_remap, Msa[0].UserPixelWidth, 4,
			Msa[0].Vtm.Timing.HActive,  Msa[0].Vtm.Timing.VActive
			, 0);


	if(downshift4K == 1 && (Msa[0].Vtm.Timing.HActive >= 7680 &&
			Msa[0].Vtm.Timing.VActive >= 4320)){
		remap_set(&tx_remap, 4, Msa[0].UserPixelWidth,
			3840,
			2160
			, 0);
	}
	// 4K120 will be changed to 4K60
	else if(downshift4K == 1 &&
			(Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive
			* Msa[0].Vtm.Timing.VActive > 4096*2160*60)){

		remap_set(&tx_remap, 4, Msa[0].UserPixelWidth,
			3840,
			2160
			, 0);

	}else{
		remap_set(&tx_remap, 4, Msa[0].UserPixelWidth,
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.HActive,
			DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing.VActive
			, 0);

	}



	XV_axi4s_remap_EnableAutoRestart(&rx_remap);
	XV_axi4s_remap_EnableAutoRestart(&tx_remap);

	XV_axi4s_remap_Start(&rx_remap);
	XV_axi4s_remap_Start(&tx_remap);
}


void power_down_HLSIPs(void){

  Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0);
//  usleep(10000);          //hold reset line
}

void power_up_HLSIPs(void){

  Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 1);
//  usleep(10000);          //hold reset line
}



void bufferWr_callback(void *InstancePtr){
	u32 Status;
	if(XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x10000000) + (0x08000000 * 3))){
		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000) + (0x08000000 * 2));
		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x10000000);
	}else{
		XVFRMBUFRD_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR;
		XVFRMBUFWR_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + 0x08000000;
	}



	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Write buffer address\r\n");
	}
	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
	}

}




/*
 * This function is a call back to write the MSA values to Tx as they are
 * read from the Rx, instead of reading them from the Video common library
 */
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr){

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HTOTAL +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_HTOTAL));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VTOTAL +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_VTOTAL));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_MAIN_STREAM_POLARITY+
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL)|
			(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_VSPOL) <<
			XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSWIDTH+
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSWIDTH +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HRES +
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HRES));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VRES +
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VHEIGHT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSTART +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSTART +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0 +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC0));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC1 +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC1));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH +
		StreamOffset[0],
		XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_USER_PIXEL_WIDTH)
			);



	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( ( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_MISC0))
			 & 0x6 ) == 0x2  ) {
	//YUV422
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if(( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MISC0))
			 & 0x6 ) == 0x4){
	// YUV444
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
	// RGB
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
}
