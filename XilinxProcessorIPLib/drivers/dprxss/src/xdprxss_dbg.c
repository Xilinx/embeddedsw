/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdprxss_dbg.c
*
* This file contains functions to report debug information of DisplayPort RX
* Subsystem sub-cores.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xvidc_dp159.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reports list of sub-cores included in DisplayPort RX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_ReportCoreInfo(XDpRxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\n\rDisplayPort RX Subsystem info:\n\r");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->DpPtr) {
		xil_printf("DisplayPort Receiver(DPRX):Yes\n\r");
	}

	if (InstancePtr->IicPtr) {
		xil_printf("IIC:Yes\n\r");
	}

	xil_printf("Audio enabled:%s\n\r",
			InstancePtr->Config.SecondaryChEn? "Yes": "No");
	xil_printf("Max supported audio channels:%d\n\r",
			InstancePtr->Config.MaxNumAudioCh);
	xil_printf("Max supported bits per color:%d\n\r",
			InstancePtr->Config.MaxBpc);
	xil_printf("Supported color format:%d\n\r",
			InstancePtr->Config.ColorFormat);
	xil_printf("HDCP enabled:%s\n\r",
			InstancePtr->Config.HdcpEnable? "Yes": "No");
	xil_printf("Max supported lane count:%d\n\r",
			InstancePtr->Config.MaxLaneCount);
	xil_printf("Max supported link rate:%d\n\r",
			InstancePtr->DpPtr->Config.MaxLinkRate);
	xil_printf("Multi-Stream Transport mode:%s\n\r",
			InstancePtr->Config.MstSupport? "Yes": "No (SST)");
	xil_printf("Max number of supported streams:%d\n\r",
			InstancePtr->Config.NumMstStreams);
	xil_printf("DP RX Subsystem is running in: %s with streams %d\n\r",
		InstancePtr->UsrOpt.MstSupport? "MST": "SST",
			InstancePtr->UsrOpt.NumOfStreams);

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the link status, selected resolution, link rate /lane
* count symbol error.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_ReportLinkInfo(XDpRxSs *InstancePtr)
{
	XDp_Config *RxConfig = &InstancePtr->DpPtr->Config;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Read link rate and lane count */
	xil_printf("\n\rLINK_BW_SET (0x09C) status in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_OVER_LINK_BW_SET));
	xil_printf("\n\rLANE_COUNT_SET (0x0A0) status in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_OVER_LANE_COUNT_SET));

	/* Read lanes status */
	xil_printf("LANE0_1_STATUS (0x043C) in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_LANE01_STATUS));
	xil_printf("LANE2_3_STATUS (0x440) in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_LANE23_STATUS));

	/* Read symbol error */
	xil_printf("SYM_ERR_CNT01 (0x448) = 0x%x\n\r"
		"SYM_ERR_CNT23 (0x44C) = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_SYM_ERR_CNT01),
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_SYM_ERR_CNT23));

	/* PHY status */
	xil_printf("PHY_STATUS (0x208) = 0x%x\n\r",
		XDpRxSs_ReadReg(RxConfig->BaseAddr, XDP_RX_PHY_STATUS));

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the current main stream attributes from the DisplayPort
* RX core.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_ReportMsaInfo(XDpRxSs *InstancePtr)
{
	XDp_Config *RxConfig = &InstancePtr->DpPtr->Config;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("RX MSA registers:\n\r"
			"\tClocks, H Total                (0x510) : %d\n\r"
			"\tClocks, V Total                (0x524) : %d\n\r"
			"\tHSyncPolarity                  (0x504) : %d\n\r"
			"\tVSyncPolarity                  (0x528) : %d\n\r"
			"\tHSync Width                    (0x508) : %d\n\r"
			"\tVSync Width                    (0x51C) : %d\n\r"
			"\tHorz Resolution                (0x500) : %d\n\r"
			"\tVert Resolution                (0x514) : %d\n\r"
			"\tHorz Start                     (0x50C) : %d\n\r"
			"\tVert Start                     (0x520) : %d\n\r"
			"\tMisc0                          (0x528) : 0x%08X\n\r"
			"\tMisc1                          (0x52C) : 0x%08X\n\r"
			"\tUser Pixel Width               (0x010) : %d\n\r"
			"\tM Vid                          (0x530) : %d\n\r"
			"\tN Vid                          (0x534) : %d\n\r"
			"\tVB-ID                          (0x538) : %d\n\r",
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HTOTAL),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VTOTAL),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSPOL),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSPOL),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSWIDTH),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSWIDTH),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HRES),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VHEIGHT),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSTART),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSTART),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MISC0),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MISC1),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_USER_PIXEL_WIDTH),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MVID),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_NVID),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VBID));

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the bit error encountered in DP159.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_ReportDp159BitErrCount(XDpRxSs *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Select page 0 */
	XVidC_Dp159Write(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0xFF, 0x00);

	/* Read TST_INT/Q */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x17, &Data);
	xil_printf("TST_INT/Q : %d\n\r", Data);

	/* BERT counter0[7:0] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x18, &Data);
	xil_printf("BERT counter0[7:0] : %d\n\r", Data);

	/* BERT counter0[11:8] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x19, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter1[7:0] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1A, &Data);
	xil_printf("BERT counter0[7:0] : %d\n\r", Data);

	/* BERT counter1[11:8] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1B, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter2[7:0] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1C, &Data);
	xil_printf("BERT counter2[7:0] : %d\n\r", Data);

	/* BERT counter2[11:8] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1D, &Data);
	xil_printf("BERT counter2[11:8] : %d\n\r", Data);

	/* BERT counter3[7:0] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1E, &Data);
	xil_printf("BERT counter3[7:0] : %d\n\r", Data);

	/* BERT counter3[11:8] */
	XVidC_Dp159Read(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0x1F, &Data);
	xil_printf("BERT counter3[11:8] : %d\n\r", Data);

	/* Clear BERT counters and TST_INTQ latches - Self-clearing in DP159 */
	XVidC_Dp159Write(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0xFF, 0x00);

	/* Select page 1 */
	XVidC_Dp159Write(InstancePtr->IicPtr, XVIDC_DP159_IIC_SLAVE,
			0xFF, 0x01);
}
