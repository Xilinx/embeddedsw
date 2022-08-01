/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_dbg.c
* @addtogroup dprxss_v8_1
* @{
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
* 2.00 sha 10/05/15 Added HDCP support.
*                   Removed DP159 bit error count code. Used DP159 bit error
*                   count function from Video Common library.
* 4.00 aad 11/14/16 Modified to use DP159 from dprxss
* 4.01 aad 07/06/17 Added MAUD and NAUD to MSA prints.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
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
	if (InstancePtr->Hdcp1xPtr) {
		xil_printf("High-Bandwidth Content protection (HDCP):Yes\n\r");
	}

	if (InstancePtr->TmrCtrPtr) {
		xil_printf("Timer Counter(0):Yes\n\r");
	}

	if (InstancePtr->DpPtr) {
		xil_printf("DisplayPort Receiver(DPRX):Yes\n\r");
	}
#ifdef XPAR_XIIC_NUM_INSTANCES
	if (InstancePtr->Config.IncludeAxiIic && InstancePtr->IicPtr) {
		xil_printf("IIC:Yes\n\r");
	}
	else
#endif
	{
#ifdef XPAR_XIICPS_NUM_INSTANCES
		if (InstancePtr->IicPsPtr) {
			xil_printf("PS IIC:Yes\n\r");
		}
#endif
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
	u32 RegValue;
	u32 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Read link rate and lane count */
	xil_printf("\n\rLINK_BW_SET (0x400) status in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
					XDP_RX_DPCD_LINK_BW_SET));
	xil_printf("LANE_COUNT_SET (0x404) status in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
					XDP_RX_DPCD_LANE_COUNT_SET));

	/* Read lanes status */
	xil_printf("\n\rLANE0_1_STATUS (0x043C) in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_LANE01_STATUS));
	xil_printf("LANE2_3_STATUS (0x440) in DPCD = 0x%x\n\r",
			XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_LANE23_STATUS));

	/* Read symbol error which is RC register. Two times read is required
	 * due to during training if this register is read it gives all F's,
	 * second time read it gives proper value.
	 */
	for (Index = 0; Index < 2; Index++) {
		RegValue = XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_SYM_ERR_CNT01);
	}
	xil_printf("\n\rSYM_ERR_CNT01 (0x448) = 0x%x\n\r", RegValue);

	for (Index = 0; Index < 2; Index++) {
		RegValue = XDpRxSs_ReadReg(RxConfig->BaseAddr,
				XDP_RX_DPCD_SYM_ERR_CNT23);
	}
	xil_printf("SYM_ERR_CNT23 (0x44C) = 0x%x\n\r", RegValue);

	/* PHY status */
	xil_printf("\n\rPHY_STATUS (0x208) = 0x%x\n\r",
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
	u8 Stream;
	u32 StreamOffset[4] = {0, XDP_RX_STREAM2_MSA_START_OFFSET,
					XDP_RX_STREAM3_MSA_START_OFFSET,
					XDP_RX_STREAM4_MSA_START_OFFSET};
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	for (Stream = 1; Stream <= InstancePtr->Config.NumMstStreams; Stream++) {
		xil_printf("RX MSA registers:Stream %x\n\r"
			"\tClocks, H Total                (0x510) : %d\n\r"
			"\tClocks, V Total                (0x524) : %d\n\r"
			"\tHSyncPolarity                  (0x504) : %d\n\r"
			"\tVSyncPolarity                  (0x518) : %d\n\r"
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
			"\tM Aud			  (0x324) : %d\n\r"
			"\tN Aud			  (0x328) : %d\n\r"
			"\tVB-ID                          (0x538) : %d\n\r",

		Stream,
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HTOTAL +
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VTOTAL+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSPOL+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSPOL+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSWIDTH +
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSWIDTH +
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HRES +
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VHEIGHT+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_HSTART+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VSTART+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MISC0+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MISC1+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_USER_PIXEL_WIDTH+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_MVID+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_NVID+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_AUDIO_MAUD+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_AUDIO_NAUD+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(RxConfig->BaseAddr, XDP_RX_MSA_VBID+
				StreamOffset[Stream - 1]));

		xil_printf("\n\r");
	}
}

/*****************************************************************************/
/**
*
* This function prints the debug display info of the HDCP interface.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_ReportHdcpInfo(XDpRxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Hdcp1xPtr)
		XHdcp1x_Info(InstancePtr->Hdcp1xPtr);
	else
		xil_printf("HDCP is not supported in this design.\n\r");
}
/** @} */
