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
* @file rx.c
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
#include "rx.h"

void operationMenu(void);
void frameBuffer_start(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K);

void DpRxSs_Main(void){
	u32 Status;
	u8 UserInput;
	u32 ReadVal=0;
	u16 DrpVal;

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
	/* Load Custom EDID */
	LoadEDID();

	XScuGic_Enable(&IntcInst, XINTC_DPRXSS_DP_INTERRUPT_ID);

	/* Need to start FrameBuffer with any setting or Rx won't output anything*/
	/* No need to be specific setting at here. Just need to start */
	/* This is specific to FrameBuffer design. */
	XDpTxSs_MainStreamAttributes Msa_rx[4];
	Msa_rx[0].Vtm.VmId = XVIDC_VM_800x600_60_P;
	Msa_rx[0].BitsPerColor = 8;
	Msa_rx[0].UserPixelWidth = 4;
	Msa_rx[0].Vtm.Timing = *(XVidC_GetTimingInfo(Msa_rx[0].Vtm.VmId));
	Msa_rx[0].Vtm.FrameRate = XVidC_GetFrameRate(Msa_rx[0].Vtm.VmId);
	frameBuffer_start(Msa_rx[0].Vtm.VmId, Msa_rx, 0);

	AppHelp();
	while (1){
		UserInput = XUartPs_RecvByte_NonBlocking();
		if(UserInput!=0){
			xil_printf("UserInput: %c\r\n",UserInput);

			switch(UserInput){
				case '2':
					/* Reset the AUX logic from DP RX */
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_SOFT_RESET, XDP_RX_SOFT_RESET_AUX_MASK);
				break;

				case 's':
					xil_printf("DP Link Status --->\r\n");
					XDpRxSs_ReportLinkInfo(&DpRxSsInst);
					break;

				case 'd':
					xil_printf("Video PHY Config/Status --->\r\n");
					xil_printf(" RCS (0x10) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_REF_CLK_SEL_REG));
					xil_printf(" PR  (0x14) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_PLL_RESET_REG));
					xil_printf(" PLS (0x18) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_PLL_LOCK_STATUS_REG));
					xil_printf(" TXI (0x1C) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_TX_INIT_REG));
					xil_printf(" TXIS(0x20) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_TX_INIT_STATUS_REG));
					xil_printf(" RXI (0x24) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_RX_INIT_REG));
					xil_printf(" RXIS(0x28) = 0x%x\n\r",
					  XVphy_ReadReg(VIDPHY_BASEADDR,XVPHY_RX_INIT_STATUS_REG));


					XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
							XVPHY_DRP_CPLL_FBDIV,&DrpVal);
					xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) = 0x%x, "
								"Val = 0x%x\r\n",
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

					break;

				case 'h':
					XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
					break;

				case 'e':
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

				case 'm':
					xil_printf(" XDP_RX_USER_FIFO_OVERFLOW (0x110) = 0x%x\n\r",
							XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
												XDP_RX_USER_FIFO_OVERFLOW));
					XDpRxSs_ReportMsaInfo(&DpRxSsInst);
//					ReportVideoCRC();

					xil_printf ("========== Rx CRC===========\r\n");
					xil_printf ("Rxd Hactive =  %d\r\n",
						((XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0xC)&0xFFFF)+ 1)
						* (XDp_ReadReg(VidFrameCRC_rx.Base_Addr,0x0)));

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



					xil_printf(" XDP_RX_LINE_RESET_DISABLE (0x008) = 0x%x\n\r",
					  XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							  XDP_RX_LINE_RESET_DISABLE));
					break;

				case 'r':
					xil_printf(
						"Reset Video DTG in DisplayPort Controller...\r\n");
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDp_RxDtgEn(DpRxSsInst.DpPtr);
					break;

				case 'c':
					XDpRxSs_ReportCoreInfo(&DpRxSsInst);
					break;

				case '.':
					AppHelp();
					break;
				case 'x':
					xil_printf("exit from Rx Only mode\r\n");
					XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
											XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
					XDp_RxDtgDis(DpRxSsInst.DpPtr);
					XDpRxSs_Reset(&DpRxSsInst);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK, 0xFFF87FFF);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_LINK_ENABLE, 0x0);
					XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
												0xFFFFFFFF);

					DpRxSsInst.VBlankCount = 0;

					operationMenu();
					return;

				default :
					AppHelp();
				break;
			}
		}//end if
		/* Info Frame Handling
		 * Info frame is sent once per frame and
		 * is static for that config
		 * Capture new Info Frame whenever config changes
		 * */
		if (AudioinfoFrame.frame_count != 0) {
			Print_InfoPkt();
			XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
					XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
			AudioinfoFrame.frame_count=0;
		}

		/* Ext Frame Handling */
		if (SdpExtFrame.Header[1] != SdpExtFrame_q.Header[1]) {
			Print_ExtPkt();
			SdpExtFrame_q = SdpExtFrame;
		}

		//CRC Handling
		if(DpRxSsInst.VBlankCount>VBLANK_WAIT_COUNT){
			//VBLANK Management
			DpRxSsInst.VBlankCount = 0;

			//Wait for few frames to ensure valid video is received
			xil_printf("Video Detected --> Link Config: %dx%d, Frame: %dx%d, "
					"MISC0: 0x%x, Mvid=%d, Nvid=%d \r",
					(int)DpRxSsInst.UsrOpt.LinkRate*270,
					(int)DpRxSsInst.UsrOpt.LaneCount,
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_HRES),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_VHEIGHT),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_MISC0),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_MVID),
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
															XDP_RX_MSA_NVID));

			/*Check for RB Resolution*/
			XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
			xil_printf(" Line Reset=0x%x\n\r",
					(int)XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
							XDP_RX_LINE_RESET_DISABLE));

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

		}
	}//end while(1)
}



/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if Video Phy configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpRxSs_VideoPhyInit(u16 DeviceId)
{

	XVphy_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XVphy_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}


	PLLRefClkSel (&VPhyInst, 0x14);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			ONBOARD_REF_CLK,
			ONBOARD_REF_CLK,
			XVPHY_PLL_TYPE_QPLL1,
			XVPHY_PLL_TYPE_CPLL,
			0x14);

	PHY_Two_byte_set (&VPhyInst, SET_RX_TO_2BYTE, SET_TX_TO_2BYTE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort RX Subsystem.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if DP RX Subsystem configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpRxSs_Setup(void)
{
	u32 ReadVal;

	/*Disable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x0);


	/*Disable All Interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFFFFFFF);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	/*Enable Training related interrupts*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP1_MASK|XDP_RX_INTERRUPT_MASK_TP2_MASK|
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP4_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	/* Setting AUX Defer Count of Link Status Reads to 8 during Link Training
	 * 8 Defer counts is chosen to handle worst case time interrupt service
	 * load (PL system working at 100 MHz) when working with R5
	 * */
	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER);
	ReadVal = ReadVal & 0xF0FF00FF;
	ReadVal = ReadVal | (AUX_DEFER_COUNT<<24);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		XDP_RX_AUX_CLK_DIVIDER, ReadVal);

	/*Setting BS Idle timeout value to long value*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
									XDP_RX_BS_IDLE_TIME, DP_BS_IDLE_TIMEOUT);

	if(LINK_TRAINING_DEBUG==1){
		/*Updating Vswing Iteration Count*/
		RxTrainConfig.ChEqOption = 0;
		RxTrainConfig.ClockRecoveryOption = 1;
		RxTrainConfig.Itr1Premp = 0;
		RxTrainConfig.Itr2Premp = 0;
		RxTrainConfig.Itr3Premp = 0;
		RxTrainConfig.Itr4Premp = 0;
		RxTrainConfig.Itr5Premp = 0;
		RxTrainConfig.MinVoltageSwing = 1;
		RxTrainConfig.SetPreemp = 1;
		RxTrainConfig.SetVswing = 0;
		RxTrainConfig.VswingLoopCount = 3;

		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MIN_VOLTAGE_SWING,
				RxTrainConfig.MinVoltageSwing |
				(RxTrainConfig.ClockRecoveryOption << 2) |
				(RxTrainConfig.VswingLoopCount << 4) |
				(RxTrainConfig.SetVswing << 8) |
				(RxTrainConfig.ChEqOption << 10) |
				(RxTrainConfig.SetPreemp << 12) |
				(RxTrainConfig.Itr1Premp << 14) |
				(RxTrainConfig.Itr2Premp << 16) |
				(RxTrainConfig.Itr3Premp << 18) |
				(RxTrainConfig.Itr4Premp << 20) |
				(RxTrainConfig.Itr5Premp << 22)
				);
	}

	/*Enable CRC Support*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
			VidFrameCRC_rx.TEST_CRC_SUPPORTED<<5);

	/*Enable Rx*/
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_LINK_ENABLE, 0x1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the power state interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_PowerChangeHandler(void *InstancePtr)
{

}

/*****************************************************************************/
/**
*
* This function is the callback function for when a no video interrupt occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_NoVideoHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount = 0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_VBLANK_MASK);
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_NO_VIDEO_MASK);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	/* Reset CRC Test Counter in DP DPCD Space */
	XVidFrameCrc_Reset(&VidFrameCRC_rx);
	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			 XDP_RX_CRC_CONFIG,
			 (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
					 VidFrameCRC_rx.TEST_CRC_CNT));

	DpRxSsInst.no_video_trigger = 1;

	AudioinfoFrame.frame_count=0;
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_INFO_PKT_MASK);
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a vertical blank interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VerticalBlankHandler(void *InstancePtr)
{
	DpRxSsInst.VBlankCount++;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a training lost interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_TrainingLostHandler(void *InstancePtr)
{
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 750);
	XDpRxSs_AudioDisable(&DpRxSsInst);
	DpRxSsInst.link_up_trigger = 0;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when a valid video interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_VideoHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the training done interrupt
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_TrainingDoneHandler(void *InstancePtr)
{
	DpRxSsInst.link_up_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the unplug event occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_UnplugHandler(void *InstancePtr)
{
	/* Disable & Enable Audio */
	XDpRxSs_AudioDisable(&DpRxSsInst);
	AudioinfoFrame.frame_count = 0;
	SdpExtFrame.Header[1] = 0;
	SdpExtFrame_q.Header[1] = 0;
	SdpExtFrame.frame_count = 0;
	SdpExtFrame.frame_count = 0;

	/*Enable Training related interrupts*/
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr,XDP_RX_INTERRUPT_MASK_ALL_MASK);
	XDp_RxInterruptDisable1(DpRxSsInst.DpPtr, 0xFFFFFFFF);

	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP1_MASK|XDP_RX_INTERRUPT_MASK_TP2_MASK|
			XDP_RX_INTERRUPT_MASK_TP3_MASK|
			XDP_RX_INTERRUPT_MASK_POWER_STATE_MASK|
			XDP_RX_INTERRUPT_MASK_CRC_TEST_MASK|
			XDP_RX_INTERRUPT_MASK_BW_CHANGE_MASK);

	XDp_RxInterruptEnable1(DpRxSsInst.DpPtr,
			XDP_RX_INTERRUPT_MASK_TP4_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LANE_SET_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_LINK_QUAL_MASK|
			XDP_RX_INTERRUPT_MASK_ACCESS_ERROR_COUNTER_MASK);

	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 5000);
	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.no_video_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for when the link bandwidth change
* occurs.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_LinkBandwidthHandler(void *InstancePtr)
{
	u32 Status;
	/*Program Video PHY to requested line rate*/
	PLLRefClkSel (&VPhyInst, DpRxSsInst.UsrOpt.LinkRate);

	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));


	XVphy_PllInitialize(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, ONBOARD_REF_CLK,
			ONBOARD_REF_CLK, XVPHY_PLL_TYPE_QPLL1, XVPHY_PLL_TYPE_CPLL);
	Status = XVphy_ClkInitialize(&VPhyInst, 0,
									XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);

	if(Status != XST_SUCCESS)
		xil_printf("XVphy_ClkInitialize failed\r\n");

	DpRxSsInst.link_up_trigger = 0;
	DpRxSsInst.no_video_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_PllResetHandler(void *InstancePtr)
{
/*Issue resets to Video PHY - This API called after line rate is programmed*/
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(FALSE));
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));
	XVphy_WaitForResetDone(&VPhyInst, 0,
						   XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
	XVphy_WaitForPllLock(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA);

	/*Enable all interrupts except Unplug*/
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr,XDP_RX_INTERRUPT_MASK_ALL_MASK);
	DpRxSsInst.no_video_trigger = 1;
}

/*****************************************************************************/
/**
*
* This function is the callback function for PLL reset request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_BWChangeHandler(void *InstancePtr)
{
//    MCDP6000_ResetDpPath(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access lane set request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessLaneSetHandler(void *InstancePtr)
{
}

/*****************************************************************************/
/**
*
* This function is the callback function for Test CRC Event request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_CRCTestEventHandler(void *InstancePtr)
{
	u16 ReadVal;
	u32 TrainingAlgoValue;

	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG);

	/*Record Training Algo Value - to be restored in non-phy test mode*/
	TrainingAlgoValue = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
			XDP_RX_MIN_VOLTAGE_SWING);

	/*Refer to DPCD 0x270 Register*/
	if( (ReadVal&0x8000) == 0x8000){
			/*Enable PHY test mode - Set Min voltage swing to 0*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_MIN_VOLTAGE_SWING,
				(TrainingAlgoValue&0xFFFFFFFC)|0x80000000);

			/*Disable Training timeout*/
			ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_CDR_CONTROL_CONFIG);
					XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG, ReadVal|0x40000000);

	}else{
		/*Disable PHY test mode & Set min voltage swing back to level 1*/
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MIN_VOLTAGE_SWING,
						(TrainingAlgoValue&0x7FFFFFFF)|0x1);

			/*Enable Training timeout*/
			ReadVal = XDp_ReadReg(DpRxSsInst.Config.BaseAddress,
					XDP_RX_CDR_CONTROL_CONFIG);
			XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_CDR_CONTROL_CONFIG, ReadVal&0xBFFFFFFF);
	}
}


/*****************************************************************************/
/**
*
* This function is the callback function for Access link qual request.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessLinkQualHandler(void *InstancePtr)
{
	u32 ReadVal;
	u32 DrpVal;


	ReadVal = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_DPC_LINK_QUAL_CONFIG);

	xil_printf("DpRxSs_AccessLinkQualHandler : 0x%x\r\n", ReadVal);

	/*Check for PRBS Mode*/
	if( (ReadVal&0x00000007) == XDP_RX_DPCD_LINK_QUAL_PRBS){
		/*Enable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x10101010;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);

		/*Reset PRBS7 Counters*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal | 0x08080808;
		XDp_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xF7F7F7F7;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);

		/*Set PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_EnablePrbs7_Rx(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	} else {
		/*Disable PRBS Mode in Video PHY*/
		DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
		DrpVal = DrpVal & 0xEFEFEFEF;
		XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);

		/*Disable PRBS mode in Retimer*/
		XDpRxSs_MCDP6000_DisablePrbs7_Rx(XPAR_IIC_0_BASEADDR,
											I2C_MCDP6000_ADDR);
		XDpRxSs_MCDP6000_ClearCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	//    	MCDP6000_EnableCounter(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
	}
}

/*****************************************************************************/
/**
*
* This function is the callback function for Access prbs error count.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_AccessErrorCounterHandler(void *InstancePtr)
{
//	u16 DrpVal;
//	u16 DrpVal_lower_lane0;
//	u16 DrpVal_lower_lane1;
//	u16 DrpVal_lower_lane2;
//	u16 DrpVal_lower_lane3;
//
//	xil_printf("DpRxSs_AccessErrorCounterHandler\r\n");
//
//
//	/*Read PRBS Error Counter Value from Video PHY*/
//
//	/*Lane 0 - Store only lower 16 bits*/
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane0);
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
//
//	/*Lane 1 - Store only lower 16 bits*/
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane1);
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
//
//	/*Lane 2 - Store only lower 16 bits*/
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane2);
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
//
//	/*Lane 3 - Store only lower 16 bits*/
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_LOWER, &DrpVal_lower_lane3);
//	XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
//			XVPHY_DRP_GTHE4_PRBS_ERR_CNTR_UPPER, &DrpVal);
//
//	/*Write into DP Core - Validity bit and lower 15 bit counter value*/
//	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//			XDP_RX_DPC_L01_PRBS_CNTR,
//			(0x8000|DrpVal_lower_lane0) |
//			((0x8000|DrpVal_lower_lane1)<<16));
//	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//			XDP_RX_DPC_L23_PRBS_CNTR,
//			(0x8000|DrpVal_lower_lane2) |
//			((0x8000|DrpVal_lower_lane3)<<16));
//
//	MCDP6000_Read_ErrorCounters(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR);
//
//	xil_printf("0x061C: %08x\n\r",
//XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x061C));
//	xil_printf("0x0504: %08x\n\r",
//XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0504));
//	xil_printf("0x0604: %08x\n\r",
//XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x0604));
//	xil_printf("0x12BC: %08x\n\r",
//XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x12BC));
//	xil_printf("0x12E4: %08x\n\r",
//XDpRxSs_MCDP6000_GetRegister(XPAR_IIC_0_BASEADDR, I2C_MCDP6000_ADDR, 0x12E4));
//
//	/*Reset PRBS7 Counters*/
//	DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
//	DrpVal = DrpVal | 0x08080808;
//	XDp_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
//	DrpVal = XVphy_ReadReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG);
//	DrpVal = DrpVal & 0xF7F7F7F7;
//	XVphy_WriteReg(VPhyInst.Config.BaseAddr, XVPHY_RX_CONTROL_REG, DrpVal);
}


/*****************************************************************************/
/**
*
* This function prints Menu
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void AppHelp()
{
	xil_printf("\n\n-----------------------------------------------------\n\r");
	xil_printf("--                       Menu                      --\n\r");
	xil_printf("-----------------------------------------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 2 = Reset AUX Logic  \n\r");
	xil_printf(" s = Report DP Link status  \n\r");
	xil_printf(" d = Report VPHY Config/Status  \n\r");
	xil_printf(" h = Assert HPD Pulse (5 ms)  \n\r");
	xil_printf(" e = Report VPHY Error & Status  \n\r");
	xil_printf(" c = Core Info  \n\r");
	xil_printf(" r = Reset DTG  \n\r");
	xil_printf(" m = Report Audio/Video MSA Attributes, Time Stamps, CRC "
					"Values  \n\r");
	xil_printf(" . = Show Menu  \n\r");
	xil_printf("\n\r");
	xil_printf("-----------------------------------------------------\n\r");
}

/*****************************************************************************/
/**
*
* This function reports CRC values of Video components
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void ReportVideoCRC()
{
	XVidFrameCrc_Report(&VidFrameCRC_rx);
}


/*****************************************************************************/
/**
*
* This function load EDID content into EDID Memory. User can change as per
*     their requirement.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void LoadEDID(void)
 {
	int i=0;
	int j=0;

#if(DP12_EDID_ENABLED)
	unsigned char edid[256] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
		0x61, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0E, 0x19, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
		0x3A, 0x4D, 0xD5, 0xA7, 0x55, 0x4A, 0x9D, 0x24,
		0x0E, 0x50, 0x54, 0xBF, 0xEF, 0x00, 0xD1, 0xC0,
		0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xB3, 0x00,
		0x71, 0x4F, 0x81, 0xC0, 0x01, 0x01, 0x4D, 0xD0,
		0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20,
		0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
		0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80,
		0xB0, 0x58, 0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00,
		0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1D,
		0x50, 0x18, 0xA0, 0x3C, 0x04, 0x11, 0x00, 0xF0,
		0xF8, 0x38, 0xF0, 0x3C, 0x00, 0x00, 0x00, 0xFC,
		0x00, 0x58, 0x49, 0x4C, 0x49, 0x4E, 0x58, 0x20,
		0x44, 0x50, 0x0A, 0x20, 0x20, 0x20, 0x01, 0x19,

		0x02, 0x03, 0x27, 0x71, 0x4F, 0x01, 0x02, 0x03,
		0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x1F, 0x90,
		0x0E, 0x0F, 0x1D, 0x1E, 0x23, 0x09, 0x17, 0x07,
		0x83, 0x01, 0x00, 0x00, 0x6A, 0x03, 0x0C, 0x00,
		0x00, 0x00, 0x00, 0x78, 0x20, 0x00, 0x00, 0x56,
		0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30,
		0x20, 0x35, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00,
		0x1E, 0xE2, 0x68, 0x00, 0xA0, 0xA0, 0x40, 0x2E,
		0x60, 0x30, 0x20, 0x36, 0x00, 0x54, 0x4F, 0x21,
		0x00, 0x00, 0x1A, 0x01, 0x1D, 0x00, 0xBC, 0x52,
		0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0x54,
		0x4F, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0,
		0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55,
		0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0
	};
#else
	// 8K30, 8K24, 5K, 4K120, 4K100 + Audio
	unsigned char edid[256] = {
			0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
			0x61, 0x98, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00,
			0x28, 0x1C, 0x01, 0x04, 0xB5, 0x3C, 0x22, 0x78,
			0x26, 0x61, 0x50, 0xA6, 0x56, 0x50, 0xA0, 0x00,
			0x0D, 0x50, 0x54, 0xA5, 0x6B, 0x80, 0xD1, 0xC0,
			0x81, 0xC0, 0x81, 0x00, 0x81, 0x80, 0xA9, 0x00,
			0xB3, 0x00, 0xD1, 0xFC, 0x01, 0x01, 0x04, 0x74,
			0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
			0x8A, 0x00, 0x54, 0x4F, 0x21, 0x00, 0x00, 0x1A,
			0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80,
			0x30, 0x20, 0x35, 0x00, 0x56, 0x50, 0x21, 0x00,
			0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x1E,
			0x3C, 0x32, 0xB4, 0x66, 0x01, 0x0A, 0x20, 0x20,
			0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
			0x00, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20,
			0x73, 0x69, 0x6E, 0x6B, 0x0A, 0x20, 0x01, 0x17,
			0x70, 0x12, 0x6E, 0x00, 0x00, 0x81, 0x00, 0x04,
			0x23, 0x09, 0x03, 0x07, 0x03, 0x00, 0x64, 0xEB,
			0xA0, 0x01, 0x04, 0xFF, 0x0E, 0xA0, 0x00, 0x2F,
			0x80, 0x21, 0x00, 0x6F, 0x08, 0x3E, 0x00, 0x03,
			0x00, 0x05, 0x00, 0xFD, 0x68, 0x01, 0x04, 0xFF,
			0x13, 0x4F, 0x00, 0x27, 0x80, 0x1F, 0x00, 0x3F,
			0x0B, 0x51, 0x00, 0x43, 0x00, 0x07, 0x00, 0x65,
			0x8E, 0x01, 0x04, 0xFF, 0x1D, 0x4F, 0x00, 0x07,
			0x80, 0x1F, 0x00, 0xDF, 0x10, 0x3C, 0x00, 0x2E,
			0x00, 0x07, 0x00, 0x86, 0x3D, 0x01, 0x04, 0xFF,
			0x1D, 0x4F, 0x00, 0x07, 0x80, 0x1F, 0x00, 0xDF,
			0x10, 0x30, 0x00, 0x22, 0x00, 0x07, 0x00, 0x5C,
			0x7F, 0x01, 0x00, 0xFF, 0x0E, 0x4F, 0x00, 0x07,
			0x80, 0x1F, 0x00, 0x6F, 0x08, 0x73, 0x00, 0x65,
			0x00, 0x07, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90
	};
#endif

	for (i = 0 ; i < ( 256 * 4 ) ; i = i + (16 * 4)) {
		for ( j = i ; j < ( i + (16 * 4)) ; j = j + 4 ) {
			XDp_WriteReg (VID_EDID_BASEADDR, j, edid[( i / 4 ) + 1 ] );
		}
	}
	for ( i = 0 ; i < ( 256 * 4 ); i = i + 4 ) {
		XDp_WriteReg (VID_EDID_BASEADDR, i, edid[i / 4]);
	}
}


/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_InfoPacketHandler(void *InstancePtr)
{
	u32 InfoFrame[9];
	int i=1;

	for(i = 1 ; i < 9 ; i++) {
		InfoFrame[i] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_INFO_DATA(i));
	}

	AudioinfoFrame.frame_count++;

	AudioinfoFrame.version = InfoFrame[1]>>26;
	AudioinfoFrame.type = (InfoFrame[1]>>8)&0xFF;
	AudioinfoFrame.sec_id = InfoFrame[1]&0xFF;
	AudioinfoFrame.info_length = (InfoFrame[1]>>16)&0x3FF;

	AudioinfoFrame.audio_channel_count = InfoFrame[2]&0x7;
	AudioinfoFrame.audio_coding_type = (InfoFrame[2]>>4)&0xF;
	AudioinfoFrame.sample_size = (InfoFrame[2]>>8)&0x3;
	AudioinfoFrame.sampling_frequency = (InfoFrame[2]>>10)&0x7;
	AudioinfoFrame.channel_allocation = (InfoFrame[2]>>24)&0xFF;

	AudioinfoFrame.level_shift = (InfoFrame[3]>>3)&0xF;
	AudioinfoFrame.downmix_inhibit = (InfoFrame[3]>>7)&0x1;

//	Print_InfoPkt();
}

/*****************************************************************************/
/**
*
* This function is the callback function for Generic Packet Handling of
* 32-Bytes payload.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void DpRxSs_ExtPacketHandler(void *InstancePtr)
{
	u32 ExtFrame[9];
	int i=1;

	SdpExtFrame.frame_count++;

	/*Header Information*/
	ExtFrame[0] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_AUDIO_EXT_DATA(1));
	SdpExtFrame.Header[0] =  ExtFrame[0]&0xFF;
	SdpExtFrame.Header[1] = (ExtFrame[0]&0xFF00)>>8;
	SdpExtFrame.Header[2] = (ExtFrame[0]&0xFF0000)>>16;
	SdpExtFrame.Header[3] = (ExtFrame[0]&0xFF000000)>>24;

	/*Payload Information*/
	for (i = 0 ; i < 8 ; i++)
	{
		ExtFrame[i+1] = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
				XDP_RX_AUDIO_EXT_DATA(i+2));
		SdpExtFrame.Payload[(i*4)]   =  ExtFrame[i+1]&0xFF;
		SdpExtFrame.Payload[(i*4)+1] = (ExtFrame[i+1]&0xFF00)>>8;
		SdpExtFrame.Payload[(i*4)+2] = (ExtFrame[i+1]&0xFF0000)>>16;
		SdpExtFrame.Payload[(i*4)+3] = (ExtFrame[i+1]&0xFF000000)>>24;
	}

}

/*****************************************************************************/
/**
*
* This function is the callback function for Info Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_InfoPkt()
{
	xil_printf("Received Audio Info Packet::\r\n");
	xil_printf(" -frame_count		 	 : 0x%x \r\n",
			AudioinfoFrame.frame_count);
	xil_printf(" -version			 	 : 0x%x \r\n",
			AudioinfoFrame.version);
	xil_printf(" -type				 	 : 0x%x \r\n",
			AudioinfoFrame.type);
	xil_printf(" -sec_id				 : 0x%x \r\n",
			AudioinfoFrame.sec_id);
	xil_printf(" -info_length			 : 0x%x \r\n",
			AudioinfoFrame.info_length);
	xil_printf(" -audio_channel_count	 : 0x%x \r\n",
			AudioinfoFrame.audio_channel_count);
	xil_printf(" -audio_coding_type		 : 0x%x \r\n",
			AudioinfoFrame.audio_coding_type);
	xil_printf(" -sample_size			 : 0x%x \r\n",
			AudioinfoFrame.sample_size);
	xil_printf(" -sampling_frequency	 : 0x%x \r\n",
			AudioinfoFrame.sampling_frequency);
	xil_printf(" -channel_allocation	 : 0x%x \r\n",
			AudioinfoFrame.channel_allocation);
	xil_printf(" -level_shift			 : 0x%x \r\n",
			AudioinfoFrame.level_shift);
	xil_printf(" -downmix_inhibit		 : 0x%x \r\n",
			AudioinfoFrame.downmix_inhibit);
}

/*****************************************************************************/
/**
*
* This function is the callback function for Ext Packet Handling.
*
* @param    InstancePtr is a pointer to the XDpRxSs instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void Print_ExtPkt()
{
	int i=0;

	xil_printf("Received SDP Packet Type::\r\n");
	switch(SdpExtFrame.Header[1])
	{
		case 0x04: xil_printf(" -> Extension\r\n"); break;
		case 0x05: xil_printf(" -> Audio_CopyManagement\r\n"); break;
		case 0x06: xil_printf(" -> ISRC\r\n"); break;
		case 0x07: xil_printf(" -> Video Stream Configuration (VSC)\r\n");break;
		case 0x20: xil_printf(" -> Video Stream Configuration Extension"
				" for VESA (VSC_EXT_VESA) - Used for HDR Metadata\r\n"); break;
		case 0x21: xil_printf(" -> VSC_EXT_CEA for future CEA INFOFRAME with "
				"payload of more than 28 bytes\r\n"); break;
		default: xil_printf(" -> Reserved/Not Defined\r\n"); break;
	}
	xil_printf(" Header Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
			SdpExtFrame.Header[0],
			SdpExtFrame.Header[1],
			SdpExtFrame.Header[2],
			SdpExtFrame.Header[3]);
	for(i=0;i<8;i++)
	{
		xil_printf(" Payload Bytes : 0x%x, 0x%x, 0x%x, 0x%x \r\n",
				SdpExtFrame.Payload[(i*4)],
				SdpExtFrame.Payload[(i*4)+1],
				SdpExtFrame.Payload[(i*4)+2],
				SdpExtFrame.Payload[(i*4)+3]);
	}
	xil_printf(" Frame Count : %d \r\n",SdpExtFrame.frame_count);
}
