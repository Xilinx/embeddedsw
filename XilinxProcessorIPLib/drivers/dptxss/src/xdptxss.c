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
* @file xdptxss.c
*
* This is the main file for Xilinx DisplayPort Transmitter Subsystem driver.
* This file contains a minimal set of functions for the XDpTxSs driver that
* allow access to all of the DisplayPort Transmitter Subsystem core's
* functionality. Please see xdptxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed sub-cores functions with prefix XDpTxSs_*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"
#include "string.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Subsystem sub-core's structure includes instances of each sub-cores */
typedef struct {
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	XDualSplitter DsInst;
#endif
	XDp DpInst;
	XVtc VtcInst[XDPTXSS_NUM_STREAMS];
} XDpTxSs_SubCores;

/************************** Function Prototypes ******************************/

static void DpTxSs_GetIncludedSubCores(XDpTxSs *InstancePtr);

/************************** Variable Definitions *****************************/

XDpTxSs_SubCores DpTxSsSubCores;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the DisplayPort Transmitter Subsystem core. This
* function must be called prior to using the core. Initialization of the core
* includes setting up the instance data and ensuring the hardware is in a
* quiescent state.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	CfgPtr points to the configuration structure associated with
*		the DisplayPort TX Subsystem core.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_DEVICE_NOT_FOUND if sub-core not found.
*		- XST_FAILURE if sub-core initialization failed.
*		- XST_SUCCESS if XDpTxSs_CfgInitialize successful.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_CfgInitialize(XDpTxSs *InstancePtr, XDpTxSs_Config *CfgPtr,
				u32 EffectiveAddr)
{
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	XDualSplitter_Config DualConfig;
#endif
	XDp_Config DpConfig;
	XVtc_Config VtcConfig;
	u32 Status;
	u32 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XDpTxSs));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XDpTxSs_Config));

	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Get included sub cores in the DisplayPort TX Subsystem */
	DpTxSs_GetIncludedSubCores(InstancePtr);

	/* Check for DisplayPort availability */
	if (InstancePtr->DpPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Initializing "
			"DisplayPort Transmitter IP\n\r");

		/* Assign number of streams to one when MST is not enabled */
		if (InstancePtr->Config.MstSupport) {
			InstancePtr->UsrOpt.NumOfStreams =
					InstancePtr->Config.NumMstStreams;
		}
		else {
			InstancePtr->Config.DpSubCore.DpConfig.NumMstStreams =
				1;
			InstancePtr->UsrOpt.NumOfStreams = 1;
			InstancePtr->Config.NumMstStreams = 1;
		}

		/* Calculate absolute base address of DP sub-core */
		InstancePtr->Config.DpSubCore.DpConfig.BaseAddr +=
					InstancePtr->Config.BaseAddress;
		(void)memcpy((void *)&(DpConfig),
			(const void *)&CfgPtr->DpSubCore.DpConfig,
				sizeof(XDp_Config));

		/* DisplayPort config initialize */
		DpConfig.BaseAddr += InstancePtr->Config.BaseAddress;
		XDp_CfgInitialize(InstancePtr->DpPtr, &DpConfig,
				DpConfig.BaseAddr);
		Status = XDp_Initialize(InstancePtr->DpPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:: DP TX "
				"initialization failed!\n\r");
			return XST_FAILURE;
		}

		/* Initialize user configurable parameters */
		InstancePtr->UsrOpt.VmId = XVIDC_VM_USE_EDID_PREFERRED;
		InstancePtr->UsrOpt.Bpc = InstancePtr->Config.MaxBpc;
		InstancePtr->UsrOpt.MstSupport =
				InstancePtr->Config.MstSupport;
	}

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	/* Check for Dual Splitter availability */
	if ((InstancePtr->DsPtr != NULL) && (InstancePtr->Config.MstSupport)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Initializing Dual "
				"Splitter IP\n\r");

		/* Calculate absolute base address of Dual Splitter sub-core */
		InstancePtr->Config.DsSubCore.DsConfig.BaseAddress +=
					InstancePtr->Config.BaseAddress;

		(void)memcpy((void *)&(DualConfig),
			(const void *)&CfgPtr->DsSubCore.DsConfig,
				sizeof(XDualSplitter_Config));

		/* Dual Splitter config initialize */
		DualConfig.BaseAddress += InstancePtr->Config.BaseAddress;
		Status = XDualSplitter_CfgInitialize(InstancePtr->DsPtr,
				&DualConfig, DualConfig.BaseAddress);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:: Dual "
				"Splitter initialization failed \n\r");
			return XST_FAILURE;
		}
	}
#endif

	/* Initialize VTC equal to number of streams */
	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: "
				"Initializing VTC%d IP \n\r", Index);

			/* Calculate absolute base address of VTC sub-core */
			InstancePtr->Config.VtcSubCore[
				Index].VtcConfig.BaseAddress +=
					InstancePtr->Config.BaseAddress;

			(void)memcpy((void *)&(VtcConfig),
			(const void *)&CfgPtr->VtcSubCore[Index].VtcConfig,
					sizeof(XVtc_Config));

			/* VTC config initialize */
			VtcConfig.BaseAddress +=
					InstancePtr->Config.BaseAddress;
			Status = XVtc_CfgInitialize(InstancePtr->VtcPtr[Index],
					&VtcConfig, VtcConfig.BaseAddress);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC%d initialization failed!\n\r",
						Index);
				return XST_FAILURE;
			}
		}
	}

	/* Reset the hardware and set the flag to indicate the
	 * subsystem is ready
	 */
	XDpTxSs_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets the DisplayPort Transmitter Subsystem including all
* sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_Reset(XDpTxSs *InstancePtr)
{
	u32 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Reset the DisplayPort. */
	XDpTxSs_WriteReg(InstancePtr->Config.DpSubCore.DpConfig.BaseAddr,
		XDP_TX_SOFT_RESET, XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK);
	XDpTxSs_WriteReg(InstancePtr->Config.DpSubCore.DpConfig.BaseAddr,
		XDP_TX_SOFT_RESET, 0x0);

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	/* Reset Dual Splitter */
	if (InstancePtr->DsPtr) {
		XDualSplitter_Reset(InstancePtr->DsPtr);
	}
#endif

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		/* Reset VTC's */
		if (InstancePtr->VtcPtr[Index]) {
			XVtc_Reset(InstancePtr->VtcPtr[Index]);
		}
	}
}

/*****************************************************************************/
/**
*
* This function starts the DisplayPort Transmitter Subsystem including all
* sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if DP TX Subsystem and its included sub-cores
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_Start(XDpTxSs *InstancePtr)
{
	u32 Status;
	u32 Index;
	u8 SinkTotal;
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	u8 VertSplit;
#endif

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((InstancePtr->UsrOpt.MstSupport == 0) ||
				(InstancePtr->UsrOpt.MstSupport == 1));

	/* Check for downstream device connected */
	if (!XDp_TxIsConnected(InstancePtr->DpPtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: RX device "
				"is not connected!\n\r");
		return XST_FAILURE;
	}

	/* Check RX device is MST capable */
	Status = XDp_TxMstCapable(InstancePtr->DpPtr);
	if ((Status == XST_SUCCESS) && (InstancePtr->Config.MstSupport)) {
		if (InstancePtr->UsrOpt.MstSupport <
					InstancePtr->Config.MstSupport) {
			/* Enable SST mode when RX is MST */
			InstancePtr->UsrOpt.MstSupport = 0;

			/* set maximum number of streams to one */
			InstancePtr->UsrOpt.NumOfStreams = 1;
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Setting "
				"to SST even though RX device is with MST "
					"capable!\n\r");
		}
		else {
			/* Enable MST mode */
			InstancePtr->UsrOpt.MstSupport =
					InstancePtr->Config.MstSupport;

			/* Restore maximum number of supported streams */
			InstancePtr->UsrOpt.NumOfStreams =
					InstancePtr->Config.NumMstStreams;
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: RX device "
					"is with MST capable!\n\r");
		}
	}
	else {
		/* Enable SST mode */
		InstancePtr->UsrOpt.MstSupport = 0;

		/* set maximum number of streams to one */
		InstancePtr->UsrOpt.NumOfStreams = 1;
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: RX device "
			"is with SST capable. OR Design supports only SST "
				"mode.\n\r");
	}

	/* Start DisplayPort sub-core configuration */
	Status = XDpTxSs_DpTxStart(InstancePtr->DpPtr,
			InstancePtr->UsrOpt.MstSupport,
				InstancePtr->UsrOpt.Bpc,
					InstancePtr->UsrOpt.VmId);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: DP Start failed "
			"in %s!\n\r",
				InstancePtr->UsrOpt.MstSupport?"MST":"SST");
		return Status;
	}

	/* Align video mode being set in DisplayPort */
	InstancePtr->UsrOpt.VmId =
			InstancePtr->DpPtr->TxInstance.MsaConfig[0].Vtm.VmId;

	/* Set number of stream to number of sinks found. Make sure that sink
	 * total does not exceed total number supported streams in by Subsystem
	 * configuration.
	 */
	if (InstancePtr->UsrOpt.MstSupport) {
		SinkTotal = InstancePtr->DpPtr->TxInstance.Topology.SinkTotal;
		InstancePtr->UsrOpt.NumOfStreams =
		(SinkTotal > InstancePtr->UsrOpt.NumOfStreams)?
			InstancePtr->UsrOpt.NumOfStreams:SinkTotal;
	}

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	if (InstancePtr->DsPtr) {
		/* Check video mode and MST support */
		if ((InstancePtr->UsrOpt.VmId == XVIDC_VM_UHD2_60_P)
				&& (InstancePtr->UsrOpt.MstSupport)) {

			/* Vertical split mode */
			VertSplit = (TRUE);
		}
		else {
			/* Bypass mode */
			VertSplit = (FALSE);
		}

		/* Setup Dual Splitter in either bypass/vertical split mode */
		Status = XDpTxSs_DsSetup(InstancePtr->DsPtr, VertSplit,
				&InstancePtr->DpPtr->TxInstance.MsaConfig[0]);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: DS start "
				"failed!\n\r");
			return Status;
		}
	}
#endif

	/* Setup VTC */
	for (Index = 0; Index < InstancePtr->UsrOpt.NumOfStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			Status = XDpTxSs_VtcSetup(InstancePtr->VtcPtr[Index],
			&InstancePtr->DpPtr->TxInstance.MsaConfig[Index]);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC%d setup failed!\n\r", Index);
				return Status;
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the DisplayPort Transmitter Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_Stop(XDpTxSs *InstancePtr)
{
	u8 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->DpPtr) {
		/* disable main link */
		XDp_TxDisableMainLink(InstancePtr->DpPtr);
	}

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	if (InstancePtr->DsPtr) {
		/* Disable Dual Splitter */
		XDualSplitter_Disable(InstancePtr->DsPtr);
	}
#endif

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			/* Disable all the VTC sub-cores */
			XVtc_Disable(InstancePtr->VtcPtr[Index]);
		}
	}
}

/*****************************************************************************/
/**
*
* This function sets the bits per color value of the video stream.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	Bpc is the new number of bits per color that needs to be set.
*		- 8 = XVIDC_BPC_8,
*		- 10 = XVIDC_BPC_10,
*		- 12 = XVIDC_BPC_12,
*		- 16 = XVIDC_BPC_16,
*
* @return
*		- XST_SUCCESS, if bits per color set successfully.
*		- XST_FAILURE, if bits per color set failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_SetBpc(XDpTxSs *InstancePtr, u8 Bpc)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Bpc == XVIDC_BPC_8) || (Bpc == XVIDC_BPC_10) ||
			(Bpc == XVIDC_BPC_12) || (Bpc == XVIDC_BPC_16));

	/* Set bits per color */
	InstancePtr->UsrOpt.Bpc = Bpc;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the standard display mode.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	VidMode is one of the enumerated standard video modes that is
*		used to determine the MSA values to be used.
*
* @return
*		- XST_SUCCESS, if video mode set successfully.
*		- XST_FAILURE, if video mode set failed.
*
* @note		Refer xvidc.h for enumerated standard video modes.
*
******************************************************************************/
u32 XDpTxSs_SetVidMode(XDpTxSs *InstancePtr, XVidC_VideoMode VidMode)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((VidMode < XVIDC_VM_NUM_SUPPORTED) ||
			(VidMode == XVIDC_VM_USE_EDID_PREFERRED));

	if ((VidMode == XVIDC_VM_UHD_60_P) &&
				(InstancePtr->UsrOpt.MstSupport)) {
		InstancePtr->UsrOpt.VmId = XVIDC_VM_UHD2_60_P;
	}
	else {
		/* Set video mode */
		InstancePtr->UsrOpt.VmId = VidMode;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the data rate to be used by the DisplayPort TX Subsystem
* core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	LinkRate is the rate at which link needs to be driven.
*		- XDPTXSS_LINK_BW_SET_162GBPS = 0x06(for a 1.62 Gbps data rate)
*		- XDPTXSS_LINK_BW_SET_270GBPS = 0x0A(for a 2.70 Gbps data rate)
*		- XDPTXSS_LINK_BW_SET_540GBPS = 0x14(for a 5.40 Gbps data rate)
*
* @return
*		- XST_SUCCESS if setting the new lane rate was successful.
*		- XST_FAILURE otherwise.
*
* @note		Maximum supported link rate is used if given link rate is
*		greater than the maximum supported link rate.
*
******************************************************************************/
u32 XDpTxSs_SetLinkRate(XDpTxSs *InstancePtr, u8 LinkRate)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((LinkRate == XDPTXSS_LINK_BW_SET_162GBPS) ||
			(LinkRate == XDPTXSS_LINK_BW_SET_270GBPS) ||
			(LinkRate == XDPTXSS_LINK_BW_SET_540GBPS));

	/* Check for maximum supported link rate */
	if (LinkRate > InstancePtr->DpPtr->Config.MaxLinkRate) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS info: This link rate is "
			"not supported by Source/Sink.\n\rMax Supported link "
			"rate is 0x%x.\n\rSetting maximum supported link "
			"rate.\n\r", InstancePtr->DpPtr->Config.MaxLinkRate);
		LinkRate = InstancePtr->DpPtr->Config.MaxLinkRate;
	}

	/* Set link rate */
	Status = XDp_TxSetLinkRate(InstancePtr->DpPtr, LinkRate);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting link rate "
			"failed.\n\r");
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the number of lanes to be used by DisplayPort TX Subsystem
* core.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	LaneCount is the number of lanes to be used.
*		- 1 = XDPTXSS_LANE_COUNT_SET_1
*		- 2 = XDPTXSS_LANE_COUNT_SET_2
*		- 4 = XDPTXSS_LANE_COUNT_SET_4
* @return
*		- XST_SUCCESS if setting the new lane count was successful.
*		- XST_FAILURE otherwise.
*
* @note		Maximum supported lane count is used if given lane count is
*		greater than the maximum supported lane count.
*
******************************************************************************/
u32 XDpTxSs_SetLaneCount(XDpTxSs *InstancePtr, u8 LaneCount)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((LaneCount == XDPTXSS_LANE_COUNT_SET_1) ||
			(LaneCount == XDPTXSS_LANE_COUNT_SET_2) ||
			(LaneCount == XDPTXSS_LANE_COUNT_SET_4));

	/* Check for maximum supported lane count */
	if (LaneCount > InstancePtr->DpPtr->Config.MaxLaneCount) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS info: This lane count is "
			"not supported by Source/Sink.\n\rMax Supported lane "
			"count is 0x%x.\n\rSetting maximum supported lane "
			"count.\n\r", InstancePtr->DpPtr->Config.MaxLaneCount);
		LaneCount = InstancePtr->DpPtr->Config.MaxLaneCount;
	}

	/* Set lane count */
	Status = XDp_TxSetLaneCount(InstancePtr->DpPtr, LaneCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting lane count "
			"failed.\n\r");
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets transport mode (SST/MST).
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	Mode specifies the type of transport mode that will be set.
*		- 0 = Single-Stream Transport mode,
*		- 1 = Multi-Stream Transport mode,
*
* @return
*		- XST_SUCCESS, if transport mode is set successfully to either
*		 MST or SST when RX device is MST and mode is less than or
*		 equal to supported mode.
*		- XST_FAILURE, if setting to already set mode or mode is
*		greater than supported mode.
*
* @note		Transport mode is set to either MST or SST when system is MST
*		and RX device is MST capable.
*
******************************************************************************/
u32 XDpTxSs_SetTransportMode(XDpTxSs *InstancePtr, u8 Mode)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Mode == 0x0) || (Mode == 0x1));

	/* Check for MST */
	if (Mode == InstancePtr->UsrOpt.MstSupport) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO:Subsystem is "
			"already in %s mode \n\r",Mode?"MST":"SST");
		Status = XST_FAILURE;
	}
	/* Check for mode less than supported mode */
	else if (Mode <= InstancePtr->Config.MstSupport) {
		/* Check RX device is MST capable */
		Status = XDp_TxMstCapable(InstancePtr->DpPtr);
		if ((Status != XST_SUCCESS) && (Mode >
					InstancePtr->UsrOpt.MstSupport)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: RX device "
				"is SST capable. No change in mode.\n\r");
			Status = XST_FAILURE;
		}
		else if ((Status == XST_SUCCESS) && ((Mode <
				InstancePtr->UsrOpt.MstSupport) ||
				(Mode > InstancePtr->UsrOpt.MstSupport))) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO::setting "
				"Subsystem mode from %s to %s mode \n\r",
				(InstancePtr->UsrOpt.MstSupport?"MST":"SST"),
				(Mode?"MST":"SST"));

			InstancePtr->UsrOpt.MstSupport = Mode;
			Status = XST_SUCCESS;
		}
	}
	/* Everything else */
	else {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR::Subsystem does not "
			"support %s \n\r", Mode?"MST":"SST");
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function retrieves the RX device's capabilities from the RX device's
* DisplayPort Configuration Data (DPCD).
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS if the DisplayPort Configuration Data was read
*		 successfully.
*		- XST_DEVICE_NOT_FOUND if no RX device is connected.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_GetRxCapabilities(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Get RX device capabilities */
	Status = XDp_TxGetRxCapabilities(InstancePtr->DpPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function retrieves an immediately connected RX device's Extended Display
* Identification Data (EDID) structure.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	Edid is a pointer to the Edid buffer to save to.
*
* @return
*		- XST_SUCCESS if the I2C transactions to read the EDID were
*		  successful.
*		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
*		- XST_DEVICE_NOT_FOUND if no RX device is connected.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_GetEdid(XDpTxSs *InstancePtr, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Edid != NULL);

	/* Retrieve the EDID */
	Status = XDp_TxGetEdid(InstancePtr->DpPtr, Edid);

	return Status;
}

/*****************************************************************************/
/**
*
* This function retrieves a remote RX device's Extended Display Identification
* Data (EDID) structure.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	SinkNum is the Sink ID in the sink list within the range
*		[0 to 3].
* @param	Edid is a pointer to the Edid buffer to save to.
*
* @return
*		- XST_SUCCESS if the I2C transactions to read the EDID were
*		  successful.
*		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
*		- XST_DEVICE_NOT_FOUND if no RX device is connected.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_GetRemoteEdid(XDpTxSs *InstancePtr, u8 SinkNum, u8 *Edid)
{
	u32 Status;
	u8 TotalSink;
	XDp_TxTopologyNode *Node;
	TotalSink = InstancePtr->DpPtr->TxInstance.Topology.SinkTotal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Edid != NULL);
	Xil_AssertNonvoid(SinkNum < TotalSink);

	Node = InstancePtr->DpPtr->TxInstance.Topology.SinkList[SinkNum];

	/* Retrieve the EDID */
	Status = XDp_TxGetRemoteEdid(InstancePtr->DpPtr, Node->LinkCountTotal,
				Node->RelativeAddress, Edid);

	return Status;
}

/*****************************************************************************/
/**
*
* This function checks if there is a connected RX device to DisplayPort TX
* Subsystem.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- TRUE if there is a connection.
*		- FALSE if there is no connection.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_IsConnected(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for TX connected */
	Status = XDp_TxIsConnected(InstancePtr->DpPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function checks if the receiver's DisplayPort Configuration Data (DPCD)
* indicates the receiver has achieved and maintained clock recovery, channel
* equalization, symbol lock, and interlane alignment for all lanes currently in
* use.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS if the RX device has maintained clock recovery,
*		 channel equalization, symbol lock, and interlane alignment.
*		- XST_DEVICE_NOT_FOUND if no RX device is connected.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_CheckLinkStatus(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check the status of link with lane count */
	Status = XDp_TxCheckLinkStatus(InstancePtr->DpPtr,
			InstancePtr->DpPtr->TxInstance.LinkConfig.LaneCount);

	return Status;
}

/*****************************************************************************/
/**
*
* This function determines whether downstream RX device is MST/SST capable.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS if the RX device is MST enabled.
*		- XST_FAILURE if the RX device is SST enabled.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_IsMstCapable(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check RX device is MST capable */
	Status = XDp_TxMstCapable(InstancePtr->DpPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function reports list of cores included in DisplayPort TX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DpTxSs_GetIncludedSubCores(XDpTxSs *InstancePtr)
{
	u32 Index;

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	/* Assign instance of Dual Splitter core */
	InstancePtr->DsPtr = ((InstancePtr->Config.DsSubCore.IsPresent)?
					(&DpTxSsSubCores.DsInst): NULL);
#endif

	/* Assign instance of DisplayPort core */
	InstancePtr->DpPtr = ((InstancePtr->Config.DpSubCore.IsPresent)?
					(&DpTxSsSubCores.DpInst): NULL);

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {

		/* Assign instances of VTC core */
		InstancePtr->VtcPtr[Index] =
			((InstancePtr->Config.VtcSubCore[Index].IsPresent)?
				(&DpTxSsSubCores.VtcInst[Index]): NULL);
	}
}
