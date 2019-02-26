/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss.c
* @addtogroup dptxss_v6_3
* @{
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
* ---- --- -------- ---------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed sub-cores functions with prefix XDpTxSs_*
* 2.00 sha 08/07/15 Added support for customized main stream attributes.
*                   Added HDCP instance into global sub-cores structure.
* 2.00 sha 09/28/15 Added HDCP and Timer Counter support.
* 3.0  sha 02/05/16 Added support for multiple subsystems in a design.
* 3.0  sha 02/19/16 Added function: XDpTxSs_ReadDownstream,
*                   XDpTxSs_HandleTimeout.
*                   Enabled HDCP in XDpTxSs_Start function.
* 4.1  aad 07/28/16 Enabled VTC before DPTX core enable for better
*		    image stability
* 4.1  als 08/08/16 Synchronize with new HDCP APIs.
*      aad 09/06/16 Updates to support 64-bit base address
* 5.0  tu  07/20/17 Allowing Custom VTM in XDpTxSs_SetVidMode function.
* 5.0  tu  08/10/17 Adjusted BS symbol for equal timing
* 5.0  tu  08/11/17 Removing ceil() to remove dependency on math library.
* 5.0  tu  09/06/17 Set timer callback after HDCP initialization
* 5.0  tu  09/06/17 Added Set UserPixelWidth support on tx side
* 5.0  tu  09/08/17 Set HPD callbacks for HPD event and HPD pulse
* 5.0  jb  02/21/19 Added HDCP22 support. Made the Timer counter available for
* 		    both HDCP1x and 22.
* 6.2  jb  02/14/20 The DP Tx subsystems assumes that the HDCP configuration is
* 		    same for all the instances in multiple subsystems in the
* 		    design. This driver wont support for different configuration
* 		    of the subsystems.
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
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	XHdcp1x Hdcp1xInst;
#endif
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	XTmrCtr TmrCtrInst;
#endif
	XDp DpInst;
	XVtc VtcInst[XDPTXSS_NUM_STREAMS];
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	XHdcp22_Tx Hdcp22Inst;
#endif
} XDpTxSs_SubCores;

/************************** Function Prototypes ******************************/

static void DpTxSs_GetIncludedSubCores(XDpTxSs *InstancePtr);
static void DpTxSs_CalculateMsa(XDpTxSs *InstancePtr, u8 Stream);
static u32 DpTxSs_CheckRxDeviceMode(XDpTxSs *InstancePtr);
static u32 DpTxSs_SetupSubCores(XDpTxSs *InstancePtr);

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
static int DpTxSs_HdcpStartTimer(void *InstancePtr, u16 TimeoutInMs);
static int DpTxSs_HdcpStopTimer(void *InstancePtr);
static int DpTxSs_HdcpBusyDelay(void *InstancePtr, u16 DelayInMs);
static u32 DpTxSs_ConvertUsToTicks(u32 TimeoutInUs, u32 ClkFreq);
static void DpTxSs_TimerCallback(void *InstancePtr, u8 TmrCtrNumber);
#endif

#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
static void DpTxSs_TimerHdcp22Callback(void *InstancePtr, u8 TmrCtrNumber);
#endif

/************************** Variable Definitions *****************************/

XDpTxSs_SubCores DpTxSsSubCores[XPAR_XDPTXSS_NUM_INSTANCES];

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
				UINTPTR EffectiveAddr)
{
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	XDualSplitter_Config DualConfig;
#endif
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	XHdcp1x_Config Hdcp1xConfig;
#endif
	XDp_Config DpConfig;
	XVtc_Config VtcConfig;
	u32 Status;
	u32 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0x0);

	/* Setup the instance */
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

		/* Initialize user configurable parameters */
		InstancePtr->UsrOpt.VmId = XVIDC_VM_USE_EDID_PREFERRED;
		InstancePtr->UsrOpt.Bpc = InstancePtr->Config.MaxBpc;
		InstancePtr->UsrOpt.MstSupport =
				InstancePtr->Config.MstSupport;
		InstancePtr->UsrOpt.VtcAdjustBs = 0;
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
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	/* Check for Timer Counter availability */
	if (InstancePtr->TmrCtrPtr != NULL) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Initializing Timer "
				"Counter IP \n\r");

		/* Calculate absolute base address of Timer Counter sub-core */
		InstancePtr->Config.TmrCtrSubCore.TmrCtrConfig.BaseAddress +=
			InstancePtr->Config.BaseAddress;

		/* Timer Counter config initialize */
		Status = XTmrCtr_Initialize(InstancePtr->TmrCtrPtr,
				InstancePtr->Config.TmrCtrSubCore.
				TmrCtrConfig.DeviceId);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:: Timer "
					"Counter initialization failed\n\r");
			return XST_FAILURE;
		}

		/* Calculate absolute base address of Timer Counter sub-core */
		InstancePtr->TmrCtrPtr->Config.BaseAddress +=
			InstancePtr->Config.BaseAddress;
		InstancePtr->TmrCtrPtr->BaseAddress +=
			InstancePtr->Config.BaseAddress;
	}
#endif /*(XPAR_DPTXSS_0_HDCP_ENABLE > 0)||(XPAR_XHDCP22_TX_NUM_INSTANCES > 0)*/

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Check for HDCP availability */
	if (InstancePtr->Hdcp1xPtr != NULL) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS INFO: Initializing HDCP IP "
				"\n\r");

		/* Calculate absolute base address of HDCP sub-core */
		InstancePtr->Config.Hdcp1xSubCore.Hdcp1xConfig.BaseAddress +=
					InstancePtr->Config.BaseAddress;
		(void)memcpy((void *)&(Hdcp1xConfig),
			(const void *)&CfgPtr->Hdcp1xSubCore.Hdcp1xConfig,
				sizeof(XHdcp1x_Config));

		/* HDCP config initialize */
		Hdcp1xConfig.BaseAddress += InstancePtr->Config.BaseAddress;
		Status = XHdcp1x_CfgInitialize(InstancePtr->Hdcp1xPtr,
				&Hdcp1xConfig, (void *)InstancePtr->DpPtr,
				Hdcp1xConfig.BaseAddress);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:: HDCP "
				"initialization failed\n\r");
			return XST_FAILURE;
		}

		/* Set key selection value for TX */
		XHdcp1x_SetKeySelect(InstancePtr->Hdcp1xPtr, 0x0);
	}

	/* Check for Timer Counter and Hdcp1x availability */
	if (InstancePtr->TmrCtrPtr != NULL && InstancePtr->Hdcp1xPtr != NULL) {
		/* Set Timer Counter instance in HDCP
		 * that will be used in callbacks */
		InstancePtr->Hdcp1xPtr->Hdcp1xRef =
				(void *)InstancePtr->TmrCtrPtr;

		/* Initialize the HDCP timer callback functions */
		XHdcp1x_SetTimerStart(InstancePtr->Hdcp1xPtr,
					&DpTxSs_HdcpStartTimer);
		XHdcp1x_SetTimerStop(InstancePtr->Hdcp1xPtr,
					&DpTxSs_HdcpStopTimer);
		XHdcp1x_SetTimerDelay(InstancePtr->Hdcp1xPtr,
					&DpTxSs_HdcpBusyDelay);
	}
#endif

#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	// HDCP 2.2
	if (InstancePtr->Hdcp22Ptr  &&
			InstancePtr->Config.Hdcp22Enable) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"SS INFO: Initializing HDCP22 IP \n\r");
		if (XDpTxSs_SubcoreInitHdcp22((void *)InstancePtr) !=
				XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"DPTXSS ERR:: Initializing HDCP22 IP"
					" failed \n\r");
			return(XST_FAILURE);
		}

		XHdcp22Tx_SetHdcp22OverProtocol(InstancePtr->Hdcp22Ptr,
				XHDCP22_TX_DP);
	}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) && (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	/* HDCP is ready when both HDCP cores are instantiated and both keys
	 * are loaded */
	if (InstancePtr->Hdcp1xPtr &&
			InstancePtr->Hdcp22Ptr &&
			InstancePtr->Hdcp22Lc128Ptr &&
			InstancePtr->Hdcp22SrmPtr) {
		InstancePtr->HdcpIsReady = TRUE;
		XDpTxSs_HdcpSetCapability(InstancePtr, XDPTXSS_HDCP_BOTH);
		XDpTxSs_HdcpSetProtocol(InstancePtr, XDPTXSS_HDCP_1X);
	}
#elif (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* HDCP is ready when only the HDCP 1.4 core is instantiated
	 * and the key is loaded */
	if (InstancePtr->Hdcp1xPtr) {
		InstancePtr->HdcpIsReady = TRUE;
		XDpTxSs_HdcpSetCapability(InstancePtr, XDPTXSS_HDCP_1X);
		XDpTxSs_HdcpSetProtocol(InstancePtr, XDPTXSS_HDCP_1X);
	}
#elif (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	/* HDCP is ready when only the HDCP 2.2 core is instantiated
	 * and the key is loaded */
	if (InstancePtr->Hdcp22Ptr &&
			InstancePtr->Hdcp22Lc128Ptr &&
			InstancePtr->Hdcp22SrmPtr) {
		InstancePtr->HdcpIsReady = TRUE;
		XDpTxSs_HdcpSetCapability(InstancePtr, XDPTXSS_HDCP_22);
		XDpTxSs_HdcpSetProtocol(InstancePtr, XDPTXSS_HDCP_22);
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
	/* Setup VTC */
	for (Index = 0; Index < InstancePtr->UsrOpt.NumOfStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			Status = XDpTxSs_VtcSetup(InstancePtr->VtcPtr[Index],
			&InstancePtr->DpPtr->TxInstance.MsaConfig[Index],
			InstancePtr->UsrOpt.VtcAdjustBs);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC%d setup failed!\n\r", Index);
				return Status;
			}
		}
	}

	/* Initialize DP */
	Status = XDp_Initialize(InstancePtr->DpPtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR:: DP TX initialization "
			"failed!\n\r");
		return XST_FAILURE;
	}

	/* Set the flag to indicate the subsystem is ready */
	InstancePtr->IsReady = (u32)XIL_COMPONENT_IS_READY;

	XDpTxSs_SetCallBack(InstancePtr, XDPTXSS_DRV_HANDLER_DP_HPD_EVENT,
			    XDpTxSs_HpdEventProcess, InstancePtr);
	XDpTxSs_SetCallBack(InstancePtr, XDPTXSS_DRV_HANDLER_DP_HPD_PULSE,
			    XDpTxSs_HpdPulseProcess, InstancePtr);

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
			XDP_TX_SOFT_RESET,
			(XDP_TX_SOFT_RESET_VIDEO_STREAM_ALL_MASK |
			 XDP_TX_SOFT_RESET_HDCP_MASK));
	XDpTxSs_WriteReg(InstancePtr->Config.DpSubCore.DpConfig.BaseAddr,
		XDP_TX_SOFT_RESET, 0x0);

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	/* Reset Dual Splitter */
	if (InstancePtr->DsPtr) {
		XDualSplitter_Reset(InstancePtr->DsPtr);
	}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Reset HDCP interface */
	if (InstancePtr->Hdcp1xPtr) {
		XHdcp1x_Reset(InstancePtr->Hdcp1xPtr);
	}

	/* Reset Timer Counter zero */
	if (InstancePtr->TmrCtrPtr) {
		XTmrCtr_Reset(InstancePtr->TmrCtrPtr, 0);
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

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((InstancePtr->UsrOpt.MstSupport == 0) ||
				(InstancePtr->UsrOpt.MstSupport == 1));

	/* Check RX device in MST/SST */
	Status = DpTxSs_CheckRxDeviceMode(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Set physical interface (DisplayPort) down */
	Status = XHdcp1x_SetPhysicalState(InstancePtr->Hdcp1xPtr, 0);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting PHY down "
			"failed.\n\r");
		return Status;
	}
#endif
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

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Set lane count in HDCP */
	Status = XHdcp1x_SetLaneCount(InstancePtr->Hdcp1xPtr,
		InstancePtr->DpPtr->TxInstance.LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting HDCP lane "
			"count failed.\n\r");
		return Status;
	}

	/* Enable HDCP interface */
	Status = XHdcp1x_Enable(InstancePtr->Hdcp1xPtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Enabling HDCP failed."
			"\n\r");
		return Status;
	}

	/* Set physical interface (DisplayPort) up */
	Status = XHdcp1x_SetPhysicalState(InstancePtr->Hdcp1xPtr, 1);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting PHY up failed."
			"\n\r");
		return Status;
	}

	/* Poll the HDCP state machine */
	Status = XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: failed to poll HDCP "
			"state machine.\n\r");
		return Status;
	}
#endif
	/* Align video mode being set in DisplayPort */
	InstancePtr->UsrOpt.VmId =
			InstancePtr->DpPtr->TxInstance.MsaConfig[0].Vtm.VmId;

	/* Setup subsystem sub-cores */
	Status = DpTxSs_SetupSubCores(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function starts the DisplayPort Transmitter Subsystem with custom
* multi-stream attributes (MSA)including all sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	MsaConfigCustom is the structure that will be used to copy the
*		main stream attributes from (into
*		InstancePtr->DpPtr->TxInstance.MsaConfig).
*
* @return
*		- XST_SUCCESS, if DP TX Subsystem and its included sub-cores
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_StartCustomMsa(XDpTxSs *InstancePtr,
				XDpTxSs_MainStreamAttributes *MsaConfigCustom)
{
	u32 Status;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((InstancePtr->UsrOpt.MstSupport == 0) ||
				(InstancePtr->UsrOpt.MstSupport == 1));
	Xil_AssertNonvoid(MsaConfigCustom != NULL);

	/* Check RX device in MST/SST */
	Status = DpTxSs_CheckRxDeviceMode(InstancePtr);
	if (Status != XST_SUCCESS) {
			return Status;
	}

	/* Clear MSA values */
	(void)memset((void *)InstancePtr->DpPtr->TxInstance.MsaConfig, 0,
		InstancePtr->UsrOpt.NumOfStreams *
			sizeof(XDpTxSs_MainStreamAttributes));

	/* Copy user provided MSA values */
	(void)memcpy((void *)InstancePtr->DpPtr->TxInstance.MsaConfig,
		(const void *)MsaConfigCustom,
			InstancePtr->UsrOpt.NumOfStreams *
				sizeof(XDpTxSs_MainStreamAttributes));

	/* Calculate required MSA values from user provided MSA values */
	for (Index = 1; Index <= InstancePtr->UsrOpt.NumOfStreams; Index ++) {
		DpTxSs_CalculateMsa(InstancePtr, Index);
	}

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Set physical interface (DisplayPort) down */
	Status = XHdcp1x_SetPhysicalState(InstancePtr->Hdcp1xPtr, 0);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting PHY down "
			"failed.\n\r");
		return Status;
	}
#endif

	/* Start DisplayPort sub-core configuration */
	Status = XDpTxSs_DpTxStart(InstancePtr->DpPtr,
			InstancePtr->UsrOpt.MstSupport,
				InstancePtr->UsrOpt.Bpc,
					InstancePtr->UsrOpt.VmId);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: DP custom start "
			"failed in %s!\n\r",
				InstancePtr->UsrOpt.MstSupport?"MST":"SST");
		return Status;
	}

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Set lane count in HDCP */
	Status = XHdcp1x_SetLaneCount(InstancePtr->Hdcp1xPtr,
		InstancePtr->DpPtr->TxInstance.LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting HDCP lane "
			"count failed.\n\r");
		return Status;
	}

	/* Set physical interface (DisplayPort) up */
	Status = XHdcp1x_SetPhysicalState(InstancePtr->Hdcp1xPtr, 1);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: Setting PHY up failed."
			"\n\r");
		return Status;
	}
#endif

	/* Setup subsystem sub-cores */
	Status = DpTxSs_SetupSubCores(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
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

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	if (InstancePtr->Hdcp1xPtr) {
		/* Disable HDCP */
		XHdcp1x_Disable(InstancePtr->Hdcp1xPtr);
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
* This function enables special timing mode for BS equal timing.
*
* @param        InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*               - void.
*
* @note         None.
*
******************************************************************************/
void XDpTxSs_VtcAdjustBSTimingEnable(XDpTxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

        /* Enable special timing mode for BS equal timing */
        InstancePtr->UsrOpt.VtcAdjustBs = 1;
}
/*****************************************************************************/
/**
*
* This function disables special timing mode for BS equal timing.
*
* @param        InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*               - void.
*
* @note         None.
*
******************************************************************************/
void XDpTxSs_VtcAdjustBSTimingDisable(XDpTxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

        /* Disable special timing mode for BS equal timing */
        InstancePtr->UsrOpt.VtcAdjustBs = 0;
}

/*****************************************************************************/
/**
*
* This function sets the bits per color value of the video stream.
*
* @param	InstancePtr is a pointer to the XDpTxSs instance.
* @param	Bpc is the new number of bits per color that needs to be set.
*		- 6 = XVIDC_BPC_6,
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
	Xil_AssertNonvoid((Bpc == XVIDC_BPC_6) || (Bpc == XVIDC_BPC_8) ||
			(Bpc == XVIDC_BPC_10) || (Bpc == XVIDC_BPC_12) ||
			(Bpc == XVIDC_BPC_16));

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

	/* Application should set Display Core maximum supported rate.
	Here we should check weather sink device rate is higher than
	display Core maximum rate, and if it is higher we should train sink
	device at Display core's maximum supported rate */
	if (LinkRate > InstancePtr->DpPtr->Config.MaxLinkRate) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"SS info: This link rate is "
			"not supported by Source/Sink.\n\rMax Supported link "
			"rate is 0x%x.\n\rSetting maximum supported link "
			"rate.\n\r", InstancePtr->DpPtr->Config.MaxLinkRate);
		LinkRate = InstancePtr->DpPtr->Config.MaxLinkRate;
	}

	/* Verify arguments. */
	Xil_AssertNonvoid((LinkRate == XDPTXSS_LINK_BW_SET_162GBPS) ||
			(LinkRate == XDPTXSS_LINK_BW_SET_270GBPS) ||
			(LinkRate == XDPTXSS_LINK_BW_SET_540GBPS) ||
			(LinkRate == XDPTXSS_LINK_BW_SET_810GBPS));


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
* This function configures the number of pixels output through the user data
* interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	UserPixelWidth is the user pixel width to be configured.
* @param	StreamId is the stream number.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XDpTxSs_SetUserPixelWidth(XDpTxSs *InstancePtr, u8 UserPixelWidth,
				u8 StreamId)
{
	XDp_TxMainStreamAttributes *MsaConfig;

	/* Verify arguments.*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((UserPixelWidth == 1) || (UserPixelWidth == 2) ||
		       (UserPixelWidth == 4));
	Xil_AssertVoid((StreamId == 1) || (StreamId == 2) ||
                       (StreamId == 3) || (StreamId == 4));


	MsaConfig = &InstancePtr->DpPtr->TxInstance.MsaConfig[StreamId - 1];
	/* Update user pixel width */
	MsaConfig->UserPixelWidth = UserPixelWidth;
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
* This function sets software switch that specify whether or not a redriver
* exits on the DisplayPort output path.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	Set establishes that a redriver exists in the DisplayPort output
*		path.
*		1 = Set redriver in the DisplayPort output path.
*		0 = Unset redriver in the DisplayPort output path.
*
* @return	None.
*
* @note		Set the redriver in the DisplayPort output path before
*		starting the training.
*
******************************************************************************/
void XDpTxSs_SetHasRedriverInPath(XDpTxSs *InstancePtr, u8 Set)
{
	/* Verify arguments.*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Set == 1) || (Set == 0));

	/* Set redriver in the DisplayPort output path */
	XDp_TxSetHasRedriverInPath(InstancePtr->DpPtr, Set);
}

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function enables High-Bandwidth Content Protection (HDCP) interface.
* This function ensures that the HDCP protocols are mutually exclusive such that
* either HDCP 1.4 or HDCP 2.2 is enabled and active at any given time.
* When the protocol is set to None, both HDCP protocols are disabled.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if HDCP i/f enabled successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_HdcpEnable(XDpTxSs *InstancePtr)
{
	u32 Status1 = XST_SUCCESS, Status2 = XST_SUCCESS;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr);
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	Xil_AssertNonvoid(InstancePtr->Config.Hdcp22Enable);
#endif

	switch (InstancePtr->HdcpProtocol) {

		/* Disable HDCP 1.4 and HDCP 2.2 */
		case XDPTXSS_HDCP_NONE :
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
			if (InstancePtr->Hdcp1xPtr) {
				Status1 = XHdcp1x_Disable(
						InstancePtr->Hdcp1xPtr);
				/* This is needed to ensure that the previous
				 * command is executed.*/
				XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
			}
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
			if (InstancePtr->Hdcp22Ptr) {
				XDp_TxHdcp22Disable(InstancePtr->DpPtr);
				Status2 = XHdcp22Tx_Disable(
						InstancePtr->Hdcp22Ptr);
			}
#endif
			break;

			/* Enable HDCP 1.4 and disable HDCP 2.2 */
		case XDPTXSS_HDCP_1X :
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
			if (InstancePtr->Hdcp1xPtr) {
				Status1 = XHdcp1x_Enable(
						InstancePtr->Hdcp1xPtr);
				/* This is needed to ensure that the previous
				 * command is executed */
				XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
			}
			else {
				Status1 = XST_FAILURE;
			}
#else
			Status1 = XST_FAILURE;
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
			if (InstancePtr->Hdcp22Ptr) {
				XDp_TxHdcp22Disable(InstancePtr->DpPtr);

				Status2 = XHdcp22Tx_Disable(
						InstancePtr->Hdcp22Ptr);
			}
#endif
			break;

			/* Enable HDCP 2.2 and disable HDCP 1.4 */
		case XDPTXSS_HDCP_22 :
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
			if (InstancePtr->Hdcp1xPtr) {
				Status1 = XHdcp1x_Disable(
						InstancePtr->Hdcp1xPtr);
				/* This is needed to ensure that the previous
				 * command is executed */
				XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
			}
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
			if (InstancePtr->Hdcp22Ptr) {
				/*Enable HDCP22 in DP TX*/
				XDp_TxHdcp22Enable(InstancePtr->DpPtr);

				Status2 = XHdcp22Tx_Enable(
						InstancePtr->Hdcp22Ptr);
			}
			else
				Status2 = XST_FAILURE;
#else
			Status2 = XST_FAILURE;
#endif
			break;

		default :
			return XST_FAILURE;
	}

	return (Status1 == XST_SUCCESS &&
			Status2 == XST_SUCCESS) ?
		XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function disables High-Bandwidth Content Protection (HDCP) interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if HDCP i/f disabled successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_HdcpDisable(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XDpTxSs_HdcpReset(InstancePtr);

	return Status;
}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
/*****************************************************************************/
/**
*
* This function polls the HDCP interface, process events and sets transmit
* state machine accordingly.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if polling the HDCP interface was successful.
*		- XST_FAILURE, if polling the HDCP interface failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_Poll(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);

	/* Poll the HDCP interface */
	Status = XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function determines whether downstream/remote RX device is HDCP capable.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- TRUE, if remote RX device is HDCP capable.
*		- FALSE, if remote RX device is not HDCP capable.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_IsHdcpCapable(XDpTxSs *InstancePtr)
{
	u32 HdcpCapable;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);

	/* Query a port device HDCP capability */
	HdcpCapable = XHdcp1x_PortIsCapable(InstancePtr->Hdcp1xPtr);

	return HdcpCapable;
}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function initiates authentication process.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if authentication initiated successfully.
*		- XST_FAILURE, if authentication initiated failed.
*
* @note		The transmitter initiates authentication by first sending its
*		An and Aksv for HDCP1x or Ake_Init for HDCP22
*		to the HDCP Receiver.
*
******************************************************************************/
u32 XDpTxSs_Authenticate(XDpTxSs *InstancePtr)
{
	u32 Status = XST_FAILURE;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr);
#if (XPAR_DPTXSS_0_HDCP_ENABLE)
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Xil_AssertNonvoid(InstancePtr->Config.Hdcp22Enable);
#endif
	/* Always disable encryption */
	if (XDpTxSs_DisableEncryption(InstancePtr, 0x01)) {
		XDpTxSs_HdcpSetProtocol(InstancePtr, XDPTXSS_HDCP_NONE);
		return XST_FAILURE;
	}

#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	/* Authenticate HDCP 2.2, takes priority*/
	if ((InstancePtr->Hdcp22Ptr) &&
			(InstancePtr->HdcpCapability == XDPTXSS_HDCP_22 ||
			 InstancePtr->HdcpCapability == XDPTXSS_HDCP_BOTH)) {
		if (XDpTxSs_IsSinkHdcp22Capable(InstancePtr)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"Starting HDCP 2.2 authentication\r\n");
			Status = XDpTxSs_HdcpSetProtocol(InstancePtr,
					XDPTXSS_HDCP_22);
			Status |= XDpTxSs_HdcpEnable(InstancePtr);

			/*
			 * As the timer is same for both hdcp1x and hdcp22,
			 * re-attach and set the callback for hdcp22 timeout
			 */
			XHdcp22Tx_timer_attach(InstancePtr->Hdcp22Ptr,
					InstancePtr->TmrCtrPtr);
			XTmrCtr_SetHandler(InstancePtr->TmrCtrPtr,
					(XTmrCtr_Handler)DpTxSs_TimerHdcp22Callback,
					(void *)InstancePtr);

			/* Set lane count in HDCP */
			XHdcp22_TxSetLaneCount(InstancePtr->Hdcp22Ptr,
					InstancePtr->DpPtr->TxInstance.
					LinkConfig.LaneCount);
			Status |= XHdcp22Tx_Authenticate(
					InstancePtr->Hdcp22Ptr);
		} else {
			Status = XST_FAILURE;
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"Sink is not HDCP 2.2 capable\r\n");
		}
	}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/*Authenticate HDCP1x*/
	if ((InstancePtr->Hdcp1xPtr) && (Status == XST_FAILURE) &&
			(InstancePtr->HdcpCapability == XDPTXSS_HDCP_1X ||
			 InstancePtr->HdcpCapability == XDPTXSS_HDCP_BOTH)) {
		if (XHdcp1x_IsDwnstrmCapable(InstancePtr->Hdcp1xPtr)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"Starting HDCP 1X authentication\r\n");
			Status = XDpTxSs_HdcpSetProtocol(InstancePtr,
					XDPTXSS_HDCP_1X);
			Status |= XDpTxSs_HdcpEnable(InstancePtr);
			Status |= XHdcp1x_Authenticate(InstancePtr->Hdcp1xPtr);
		} else {
			Status = XST_FAILURE;
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"Sink is not HDCP 1x capable\r\n");
		}
	}
#endif
	/* Set protocol to None */
	if (Status == XST_FAILURE) {
		XDpTxSs_HdcpSetProtocol(InstancePtr, XDPTXSS_HDCP_NONE);
	}

	return (Status == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function checks whether HDCP Transmitter authenticated the HDCP
* Receiver.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- TRUE, if HDCP Transmitter authenticated the HDCP Receiver.
*		- FALSE, if HDCP Transmitter not authenticated the HDCP
*		Receiver.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_IsAuthenticated(XDpTxSs *InstancePtr)
{
	u32 Authenticate;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr);
#if (XPAR_DPTXSS_0_HDCP_ENABLE)
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Xil_AssertNonvoid(InstancePtr->Config.Hdcp22Enable);
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE)
	if (InstancePtr->Hdcp1xPtr &&
			(InstancePtr->HdcpProtocol == XDPTXSS_HDCP_1X)) {
		/* Check authentication has completed successfully */
		Authenticate = XHdcp1x_IsAuthenticated(InstancePtr->Hdcp1xPtr);
	}
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	if (InstancePtr->Hdcp22Ptr &&
			(InstancePtr->HdcpProtocol == XDPTXSS_HDCP_22)) {
		/* Check authentication has completed successfully */
		Authenticate = XHdcp22Tx_IsAuthenticated(
				InstancePtr->Hdcp22Ptr);
	}
#endif
	return Authenticate;
}

/*****************************************************************************/
/**
*
* This function enables encryption on series of streams within an HDCP
* interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	StreamMap is the bit map of streams to enable encryption on.
*
* @return
*		- XST_SUCCESS, if encryption enabled successfully.
*		- XST_FAILURE, if encryption enabled failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_EnableEncryption(XDpTxSs *InstancePtr, u64 StreamMap)
{
	u32 Status = XST_SUCCESS;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr);
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	Xil_AssertNonvoid(InstancePtr->Config.Hdcp22Enable);
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	if (InstancePtr->Hdcp1xPtr) {
		/* Enable encryption on stream(s) */
		Status = XHdcp1x_EnableEncryption(InstancePtr->Hdcp1xPtr,
				StreamMap);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#endif

#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	if (InstancePtr->Hdcp22Ptr)
		Status = XHdcp22Tx_EnableEncryption(InstancePtr->Hdcp22Ptr);
#endif


	return Status;
}

/*****************************************************************************/
/**
*
* This function disables encryption on series of streams within an HDCP
* interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	StreamMap is the bit map of streams to disable encryption on.
*
* @return
*		- XST_SUCCESS, if encryption disabled successfully.
*		- XST_FAILURE, if encryption disabled failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_DisableEncryption(XDpTxSs *InstancePtr, u64 StreamMap)
{
	u32 Status = XST_SUCCESS;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr);
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable);
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	Xil_AssertNonvoid(InstancePtr->Config.Hdcp22Enable);
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	if (InstancePtr->Hdcp1xPtr) {
		/* Disable encryption on stream(s) */
		Status = XHdcp1x_DisableEncryption(InstancePtr->Hdcp1xPtr,
				StreamMap);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#endif
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	if (InstancePtr->Hdcp22Ptr)
		Status = XHdcp22Tx_DisableEncryption(InstancePtr->Hdcp22Ptr);
#endif

	return Status;
}
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
/*****************************************************************************/
/**
*
* This function retrieves the current encryption map.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- The current encryption map.
*		- Otherwise zero.
*
* @note		None.
*
******************************************************************************/
u64 XDpTxSs_GetEncryption(XDpTxSs *InstancePtr)
{
	u64 StreamMap;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);

	/* Get stream map of the stream(s) */
	StreamMap = XHdcp1x_GetEncryption(InstancePtr->Hdcp1xPtr);

	return StreamMap;
}

/*****************************************************************************/
/**
*
* This function enables/disables the underlying physical interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	PhyState indicates TRUE/FALSE value to enable/disable the
*		underlying physical interface.
*
* @return
*		- XST_SUCCESS, if the underlying physical interface enabled
*		successfully.
*		- XST_FAILURE, if the underlying physical interface failed to
*		enable.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_SetPhysicalState(XDpTxSs *InstancePtr, u32 PhyState)
{
	u32 Status;

	/* Verify argument.*/
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);
	Xil_AssertNonvoid((PhyState == TRUE) || (PhyState == FALSE));

	/* Enable underlying physical interface */
	Status = XHdcp1x_SetPhysicalState(InstancePtr->Hdcp1xPtr, PhyState);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets lane(s) of the HDCP interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	Lane is the number of lanes to be used.
*		- 1 = XDPTXSS_LANE_COUNT_SET_1
*		- 2 = XDPTXSS_LANE_COUNT_SET_2
*		- 4 = XDPTXSS_LANE_COUNT_SET_4
*
* @return
*		- XST_SUCCESS, if lane(s) into the HDCP i/f set successfully.
*		- XST_FAILURE, if failed to set lane(s) into the HDCP i/f.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_SetLane(XDpTxSs *InstancePtr, u32 Lane)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);
	Xil_AssertNonvoid((Lane == XDPTXSS_LANE_COUNT_SET_1) ||
			(Lane == XDPTXSS_LANE_COUNT_SET_2) ||
			(Lane == XDPTXSS_LANE_COUNT_SET_4));

	/* Set lanes into the HDCP interface */
	Status = XHdcp1x_SetLaneCount(InstancePtr->Hdcp1xPtr, Lane);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the debug printf function.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	PrintfFunc is the printf function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_SetDebugPrintf(XDpTxSs *InstancePtr, XDpTxSs_Printf PrintfFunc)
{
	/* Verify arguments.*/
	Xil_AssertVoid(InstancePtr->Config.HdcpEnable == 0x1);
	Xil_AssertVoid(PrintfFunc != NULL);

	/* Set debug printf function */
	XHdcp1x_SetDebugPrintf(PrintfFunc);
}

/*****************************************************************************/
/**
*
* This function sets the debug log message function.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	LogFunc is the debug logging function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_SetDebugLogMsg(XDpTxSs *InstancePtr, XDpTxSs_LogMsg LogFunc)
{
	/* Verify arguments.*/
	Xil_AssertVoid(InstancePtr->Config.HdcpEnable == 0x1);
	Xil_AssertVoid(LogFunc != NULL);

	/* Set debug log message function */
	XHdcp1x_SetDebugLogMsg(LogFunc);
}

/*****************************************************************************/
/**
*
* This function initiates downstream read of READY bit and consequently the
* second part of repeater authentication.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS, if authentication initiated successfully.
*		- XST_FAILURE, if authentication initiated failed.
*
* @note		The transmitter initiates authentication by first sending its
*		An and Aksv value to the HDCP Receiver.
*
******************************************************************************/
u32 XDpTxSs_ReadDownstream(XDpTxSs *InstancePtr)
{
	u32 Status;

	/* Verify arguments.*/
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.HdcpEnable == 0x1);

	/* Initiate downstream read and authentication process */
	Status = XHdcp1x_ReadDownstream(InstancePtr->Hdcp1xPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function handles a timeout for HDCP.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_HandleTimeout(XDpTxSs *InstancePtr)
{
	/* Verify arguments.*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.HdcpEnable == 0x1);

	/* Handle timeout */
	XHdcp1x_HandleTimeout(InstancePtr->Hdcp1xPtr);
}

/*****************************************************************************/
/**
*
* This function starts a timer on behalf of an HDCP interface.
*
* @param	InstancePtr is a pointer to the XHdcp1x core instance.
* @param	TimeoutInMs the timer duration in milliseconds.
*
* @return
*		- XST_SUCCESS if Timer Counter started successfully.
*
* @note		None.
*
******************************************************************************/
static int DpTxSs_HdcpStartTimer(void *InstancePtr, u16 TimeoutInMs)
{
	XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
	XTmrCtr *TmrCtrPtr;
	u8 TimerChannel;
	u32 TimerOptions;
	u32 NumTicks;

	/* Verify argument. */
	Xil_AssertNonvoid(HdcpPtr != NULL);
	Xil_AssertNonvoid(HdcpPtr->Hdcp1xRef != NULL);

	TmrCtrPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

	/* Determine NumTicks */
	NumTicks = DpTxSs_ConvertUsToTicks((TimeoutInMs * 1000),
				TmrCtrPtr->Config.SysClockFreqHz);

	/* Stop Timer Counter */
	TimerChannel = 0;
	XTmrCtr_Stop(TmrCtrPtr, TimerChannel);

	/* Configure the callback */
	XTmrCtr_SetHandler(TmrCtrPtr, &DpTxSs_TimerCallback, (void *)HdcpPtr);

	/* Configure the timer options */
	TimerOptions = XTmrCtr_GetOptions(TmrCtrPtr, TimerChannel);
	TimerOptions |= XTC_DOWN_COUNT_OPTION;
	TimerOptions |= XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TmrCtrPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TmrCtrPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TmrCtrPtr, TimerChannel);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function stops a timer on behalf of an HDCP interface
*
* @param	InstancePtr is a pointer to the XHdcp1x core instance.
*
* @return
*		- XST_SUCCESS if Timer Counter stopped successfully.
*
* @note		None.
*
******************************************************************************/
static int DpTxSs_HdcpStopTimer(void *InstancePtr)
{
	XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
	XTmrCtr *TmrCtrPtr;
	u8 TimerChannel;

	/* Verify argument. */
	Xil_AssertNonvoid(HdcpPtr != NULL);
	Xil_AssertNonvoid(HdcpPtr->Hdcp1xRef != NULL);

	TmrCtrPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

	/* Stop Timer Counter */
	TimerChannel = 0;
	XTmrCtr_Stop(TmrCtrPtr, TimerChannel);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function busy waits for an interval on behalf of an HDCP interface.
*
* @param	InstancePtr is a pointer to the XHdcp1x core instance.
* @param	DelayInMs the delay duration in milliseconds.
*
* @return
*		- XST_SUCCESS if Timer Counter busy wait successfully.
*
* @note		None.
*
******************************************************************************/
static int DpTxSs_HdcpBusyDelay(void *InstancePtr, u16 DelayInMs)
{
	XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
	XTmrCtr *TmrCtrPtr;
	u8 TimerChannel;
	u32 TimerOptions;
	u32 NumTicks;

	/* Verify argument. */
	Xil_AssertNonvoid(HdcpPtr != NULL);
	Xil_AssertNonvoid(HdcpPtr->Hdcp1xRef != NULL);

	TmrCtrPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

	/* Determine number of timer ticks */
	NumTicks = DpTxSs_ConvertUsToTicks((DelayInMs * 1000),
				TmrCtrPtr->Config.SysClockFreqHz);

	/* Stop it */
	TimerChannel = 0;
	XTmrCtr_Stop(TmrCtrPtr, TimerChannel);

	/* Configure the timer options */
	TimerOptions = XTmrCtr_GetOptions(TmrCtrPtr, TimerChannel);
	TimerOptions |= XTC_DOWN_COUNT_OPTION;
	TimerOptions &= ~XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TmrCtrPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TmrCtrPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TmrCtrPtr, TimerChannel);

	/* Wait until done */
	while (!XTmrCtr_IsExpired(TmrCtrPtr, TimerChannel));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function serves as the timer callback.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param 	TmrCtrNumber is the number of the timer/counter within the
*		device. The device typically contains at least two
*		timer/counters.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DpTxSs_TimerCallback(void *InstancePtr, u8 TmrCtrNumber)
{
	XHdcp1x *Hdcp1xPtr = (XHdcp1x *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(Hdcp1xPtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);

	/* Handle timeout */
	XHdcp1x_HandleTimeout(Hdcp1xPtr);
}

/*****************************************************************************/
/**
*
* This function converts from microseconds to timer ticks.
*
* @param	TimeoutInUs is the timeout value to convert into timer ticks.
* @param 	ClkFreq the clock frequency to use in the conversion.
*
* @return	The number of timer ticks.
*
* @note		None.
*
******************************************************************************/
static u32 DpTxSs_ConvertUsToTicks(u32 TimeoutInUs, u32 ClkFreq)
{
	u32 TimeoutFreq;
	u32 NumSeconds;
	u32 NumTicks = 0;

	/* Check for greater than one second */
	if (TimeoutInUs > 1000000) {
		/* Determine the number of seconds */
		NumSeconds = (TimeoutInUs / 1000000);

		/* Update theNumTicks */
		NumTicks = (NumSeconds * ClkFreq);

		/* Adjust the TimeoutInUs */
		TimeoutInUs -= (NumSeconds * 1000000);
	}

	/* Convert TimeoutFreq to a frequency */
	TimeoutFreq = 1000;
	TimeoutFreq *= 1000;
	TimeoutFreq /= TimeoutInUs;

	/* Update NumTicks */
	NumTicks += ((ClkFreq / TimeoutFreq) + 1);

	return NumTicks;
}
#endif

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
	InstancePtr->DsPtr = ((InstancePtr->Config.DsSubCore.IsPresent) ?
		(&DpTxSsSubCores[InstancePtr->Config.DeviceId].DsInst) : NULL);
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Assign instance of HDCP core */
	InstancePtr->Hdcp1xPtr =
		((InstancePtr->Config.Hdcp1xSubCore.IsPresent) ?
	(&DpTxSsSubCores[InstancePtr->Config.DeviceId].Hdcp1xInst) : NULL);
#endif

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	/* Assign instance of Timer Counter core */
	InstancePtr->TmrCtrPtr =
		((InstancePtr->Config.TmrCtrSubCore.IsPresent) ?
		 (&DpTxSsSubCores[InstancePtr->Config.DeviceId].TmrCtrInst)
		 : NULL);
#endif
#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* Set Timer Counter instance in HDCP that will be used in callbacks */
	InstancePtr->Hdcp1xPtr->Hdcp1xRef = (void *)InstancePtr->TmrCtrPtr;
#endif

	/* Assign instance of DisplayPort core */
	InstancePtr->DpPtr = ((InstancePtr->Config.DpSubCore.IsPresent) ?
		(&DpTxSsSubCores[InstancePtr->Config.DeviceId].DpInst) : NULL);

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {

		/* Assign instances of VTC core */
		InstancePtr->VtcPtr[Index] =
			((InstancePtr->Config.VtcSubCore[Index].IsPresent) ?
			(&DpTxSsSubCores[
			InstancePtr->Config.DeviceId].VtcInst[Index]) : NULL);
	}

#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
	/*Assign Instance of HDCP22 core*/
	InstancePtr->Hdcp22Ptr =
		((InstancePtr->Config.Hdcp22SubCore.IsPresent) ?
		 (&DpTxSsSubCores[InstancePtr->Config.DeviceId].Hdcp22Inst) :
		 NULL);
#endif
}

/*****************************************************************************/
/**
*
* This function computes multi-stream attribute and populates frame rate,
* pixel clock and so on.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	Stream is the stream number for which the MSA values will be
*		computed for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DpTxSs_CalculateMsa(XDpTxSs *InstancePtr, u8 Stream)
{
	XDpTxSs_MainStreamAttributes *MsaConfig;
	XVidC_VideoMode VidMode;
	u32 FrameRate;
	u32 ClkFreq;
	u32 Ival;
	float Fval;
	u8 LinkRate;

	MsaConfig = &InstancePtr->DpPtr->TxInstance.MsaConfig[Stream - 1];
	LinkRate = InstancePtr->DpPtr->TxInstance.LinkConfig.LinkRate;

	/*Calculate pixel clock in HZ */
	ClkFreq = (LinkRate * 27 * MsaConfig->MVid) / MsaConfig->NVid;
	MsaConfig->PixelClockHz = ((u32)ClkFreq) * 1000000;

	/*Calculate frame rate */
	Fval = (ClkFreq * 1000000.0) / (MsaConfig->Vtm.Timing.HTotal *
                                MsaConfig->Vtm.Timing.F0PVTotal);

	Ival = (u32) Fval;

	FrameRate = (u32) (Fval == (float) Ival) ? Ival : Ival + 1;

	/* Round of frame rate */
	if ((FrameRate == 59) || (FrameRate == 61)) {
		FrameRate = 60;
	}
	else if ((FrameRate == 29) || (FrameRate == 31)) {
		FrameRate = 30;
	}
	else if ((FrameRate == 76) || (FrameRate == 74)) {
		FrameRate = 75;
	}
	MsaConfig->Vtm.FrameRate = FrameRate;

	/* Calculate horizontal front porch */
	MsaConfig->Vtm.Timing.HFrontPorch = MsaConfig->Vtm.Timing.HTotal -
			MsaConfig->HStart - MsaConfig->Vtm.Timing.HActive;

	/* Calculate horizontal back porch */
	MsaConfig->Vtm.Timing.HBackPorch = MsaConfig->HStart -
			MsaConfig->Vtm.Timing.HSyncWidth;

	/* Calculate vertical frame zero front porch */
	MsaConfig->Vtm.Timing.F0PVFrontPorch =
		MsaConfig->Vtm.Timing.F0PVTotal - MsaConfig->VStart -
			MsaConfig->Vtm.Timing.VActive;

	/* Calculate vertical frame zero back porch */
	MsaConfig->Vtm.Timing.F0PVBackPorch = MsaConfig->VStart -
			MsaConfig->Vtm.Timing.F0PVSyncWidth;

	/* Set frame 1 parameters */
	MsaConfig->Vtm.Timing.F1VFrontPorch = 0;
	MsaConfig->Vtm.Timing.F1VSyncWidth = 0;
	MsaConfig->Vtm.Timing.F1VBackPorch = 0;
	MsaConfig->Vtm.Timing.F1VTotal = 0;

	/* Check video mode is present in video common library */
	VidMode = XVidC_GetVideoModeId(MsaConfig->Vtm.Timing.HActive,
			MsaConfig->Vtm.Timing.VActive,
				MsaConfig->Vtm.FrameRate,
					XVIDC_VF_PROGRESSIVE);
	if (VidMode == XVIDC_VM_NOT_SUPPORTED) {
		MsaConfig->Vtm.VmId = XVIDC_VM_CUSTOM;
		InstancePtr->UsrOpt.VmId = XVIDC_VM_CUSTOM;
	}
	else {
		MsaConfig->Vtm.VmId = XVIDC_VM_CUSTOM;
		InstancePtr->UsrOpt.VmId = XVIDC_VM_CUSTOM;
	}

	/* Set bits per color */
	InstancePtr->UsrOpt.Bpc = MsaConfig->BitsPerColor;

	/* Use custom MSA */
	XDp_TxCfgMsaUseCustom(InstancePtr->DpPtr, Stream, MsaConfig, TRUE);
}

/*****************************************************************************/
/**
*
* This function checks whether RX device in multi-stream (MST) / Single Stream
* transport mode.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_FAILURE if DisplayPort TX initialization failed or RX
*		device is not connected.
*		- XST_SUCCESS if RX device check is successful.
*
* @note		None.
*
******************************************************************************/
static u32 DpTxSs_CheckRxDeviceMode(XDpTxSs *InstancePtr)
{
	u32 Status;

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

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures DisplayPort TX subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_FAILURE if all sub-cores configuration failed.
*		- XST_SUCCESS if all sub-cores configuration was successful.
*
* @note		None.
*
******************************************************************************/
static u32 DpTxSs_SetupSubCores(XDpTxSs *InstancePtr)
{
	u32 Status;
	u32 Index;
	u8 SinkTotal;
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	u8 VertSplit;
#endif
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
		if (InstancePtr->UsrOpt.MstSupport) {
			if ((InstancePtr->UsrOpt.VmId == XVIDC_VM_UHD2_60_P)) {
				/* Vertical split mode */
				VertSplit = (TRUE);
			}
			else if ((InstancePtr->UsrOpt.VmId ==
				XVIDC_VM_CUSTOM) &&
				(InstancePtr->DpPtr->TxInstance.MsaConfig[
				0].Vtm.Timing.HActive == 1920) &&
				(InstancePtr->DpPtr->TxInstance.MsaConfig[
					0].Vtm.Timing.VActive == 2160)) {
				/* Vertical split mode */
				VertSplit = (TRUE);
			}
			else {
				/* Bypass mode */
				VertSplit = FALSE;
			}
		}
		else {
			/* Bypass mode */
			VertSplit = FALSE;
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
			&InstancePtr->DpPtr->TxInstance.MsaConfig[Index],
			InstancePtr->UsrOpt.VtcAdjustBs);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"SS ERR: "
					"VTC%d setup failed!\n\r", Index);
				return Status;
			}
		}
	}

	return XST_SUCCESS;
}

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0) || (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function resets both HDCP 1.4 and 2.2 protocols. This function
* also disables the both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XDpTxSs_HdcpReset(XDpTxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;

#if (XPAR_DPTXSS_0_HDCP_ENABLE > 0)
	/* HDCP 1.4 */
	/* Resetting HDCP 1.4 causes the state machine to be enabled, therefore
	 * disable must be called immediately after reset is called */
	if (InstancePtr->Hdcp1xPtr) {
		Status = XHdcp1x_Reset(InstancePtr->Hdcp1xPtr);
		/* This is needed to ensure that the previous command
		 * is executed */
		XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		Status = XHdcp1x_Disable(InstancePtr->Hdcp1xPtr);
		/* This is needed to ensure that the previous command
		 * is executed. */
		XHdcp1x_Poll(InstancePtr->Hdcp1xPtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	}
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
	/* HDCP 2.2 */
	if (InstancePtr->Hdcp22Ptr) {
		Status = XHdcp22Tx_Reset(InstancePtr->Hdcp22Ptr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		Status = XHdcp22Tx_Disable(InstancePtr->Hdcp22Ptr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	}
#endif

	/* Set defaults */
	XDpTxSs_DisableEncryption(InstancePtr, 1);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the HDCP protocol capability used during authentication.
* The protocol capability can be set to either HDCP 1.4, 2.2, Both, or None.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
* @param Protocol is the desired content protection scheme of type
*        XDpTxSs_HdcpProtocol.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XDpTxSs_HdcpSetCapability(XDpTxSs *InstancePtr,
		XDpTxSs_HdcpProtocol Protocol)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Protocol <= XDPTXSS_HDCP_BOTH);

	/* Set protocol */
	InstancePtr->HdcpCapability = Protocol;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the active HDCP protocol and enables it.
* The protocol can be set to either HDCP 2.2, or None.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
* @param Protocol is the requested content protection scheme of type
*        XDpTxSs_HdcpProtocol.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XDpTxSs_HdcpSetProtocol(XDpTxSs *InstancePtr,
		XDpTxSs_HdcpProtocol Protocol)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Protocol == XDPTXSS_HDCP_NONE)   ||
			(Protocol == XDPTXSS_HDCP_1X) ||
			(Protocol == XDPTXSS_HDCP_22));

	int Status;

	/* Set protocol */
	InstancePtr->HdcpProtocol = Protocol;

	/* Reset Hdcp protocol */
	Status = XDpTxSs_HdcpReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		InstancePtr->HdcpProtocol = XDPTXSS_HDCP_NONE;
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function is the callback called when the Timer Counter reset done with
* specified reset value, assigned during initialization.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
* @param	TmrCtrNumber is the number of the timer/counter within the
*		Timer Counter core.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DpTxSs_TimerHdcp22Callback(void *InstancePtr, u8 TmrCtrNumber)
{
	XDpTxSs *XDpTxSsPtr = (XDpTxSs *)InstancePtr;

	/* Verify arguments.*/
	Xil_AssertVoid(XDpTxSsPtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);

	/*Call HDCP22 Timer handler*/
	XHdcp22Tx_TimerHandler((void *)XDpTxSsPtr->Hdcp22Ptr, TmrCtrNumber);
}

/*****************************************************************************/
/**
 *
 * This function sets pointers to the HDCP 2.2 keys.
 *
 * @param InstancePtr is a pointer to the XDpTxSs instance.
 * @param KeyType is the type of the key that is being set.
 * @param KeyPtr is the pointer to the key buffer
 *
 * @return None.
 *
 * @note   None.
 *
 ******************************************************************************/
void XDpTxSs_Hdcp22SetKey(XDpTxSs *InstancePtr,
		XDpTxSs_Hdcp22KeyType KeyType, u8 *KeyPtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid((KeyType == XDPTXSS_KEY_HDCP22_LC128) ||
			(KeyType == XDPTXSS_KEY_HDCP22_SRM))

	switch (KeyType) {
		/* HDCP 2.2 LC128 */
		case XDPTXSS_KEY_HDCP22_LC128:
			InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
			break;
		/* HDCP 2.2 Private key */
		case XDPTXSS_KEY_HDCP22_SRM:
			InstancePtr->Hdcp22SrmPtr = KeyPtr;
			break;
		default :
			break;
	}
}
#endif

#endif
/** @} */
