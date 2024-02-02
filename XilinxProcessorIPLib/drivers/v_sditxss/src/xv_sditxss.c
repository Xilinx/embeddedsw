/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_sditxss.c
 *
 * This is the main file for Xilinx SDI TX core. Please see xv_sditxss.h for
 * more details of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- -------------------------------------------------------
 * 1.00  jsr    07/17/17 Initial release.
 * 2.00  kar    01/25/18 Second  release.
 *       jsr    02/23/2018 Added YUV420 color format support
 *       jsr	03/02/2018 Added core settings API
 * 2.1   jsr    07/03/2018 Corrected 720x480_60_I to be 720x486_60_I for SD mode
 * 2.2   jsr    10/01/2018 Programming the Field register for 720x480_60_I SD mode
 * 3.0   vve    10/03/18 Add support for ST352 in C Stream
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xenv.h"
#include "xv_sditxss.h"
#include "xparameters.h"
#include "xv_sditxss_coreinit.h"
#include <string.h>
#include "xv_sditx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XSDITXSS_LINE_RATE_3G	0
#define XSDITXSS_LINE_RATE_6G	1
#define XSDITXSS_LINE_RATE_12G8DS	2
#define AXI4_STREAM	0
#define NATIVE_VIDEO 1
#define NATIVE_SDI 2
#define XSDITXSS_XVTC_ASIZE_VERT_SHIFT 16
#define XSDITXSS_XVTC_ASIZE_VERT_MASK	0x1FFF0000
#define XSDITXSS_XVTC_GASIZE_F1_OFFSET	0x094
#define XSDITXSS_SD_NTSC_F1_V_ACTIVE	244

#define XST352_PAYLOAD_BYTE_MASK	0xFF
#define XST352_BYTE2_TS_TYPE_MASK	(1 << 15)
#define XST352_BYTE2_PIC_TYPE_MASK	(1 << 14)
#define XST352_BYTE1_ST372_DL_3GB	0x8A
#define XST352_BYTE1_ST425_2008_1125L_3GA	0x89

/**************************** Type Definitions *******************************/
/**
* This typedef declares the driver instances of all the cores in the subsystem
*/
typedef struct {
	XV_SdiTx SdiTx;
	XVtc Vtc;
} XV_SdiTxSs_SubCores;

/**************************** Local Global ***********************************/
XV_SdiTxSs_SubCores XV_SdiTxSs_SubCoreRepo[XPAR_XV_SDITXSS_NUM_INSTANCES];
/**< Define Driver instance of all sub-core
									included in the design */

/************************** Function Prototypes ******************************/
static int XV_SdiTxSs_VtcSetup(XVtc *XVtcPtr, XV_SdiTx *SdiTxPtr);
static void XV_SdiTxSs_ReportCoreInfo(XV_SdiTxSs *InstancePtr);
static void XV_SdiTxSs_ReportTiming(XV_SdiTxSs *InstancePtr);
static void XV_SdiTxSs_GtReadyCallback(void *CallbackRef);
static void XV_SdiTxSs_OverFlowCallback(void *CallbackRef);
static void XV_SdiTxSs_UnderFlowCallback(void *CallbackRef);
static void XV_SdiTxSs_CeAlignErrCallback(void *CallbackRef);
static void XV_SdiTxSs_Axi4sVidLockCallback(void *CallbackRef);
static int XV_SdiTxSs_RegisterSubsysCallbacks(XV_SdiTxSs *InstancePtr);
static u8 XV_SdiTxSs_Is3GBDLor3GA1125L(XV_SdiTxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function calls the interrupt handler for SDI TX
*
* @param  InstancePtr is a pointer to the SDI TX Subsystem
*
*****************************************************************************/
void XV_SdiTxSS_SdiTxIntrHandler(XV_SdiTxSs *InstancePtr)
{
	XV_SdiTx_IntrHandler(InstancePtr->SdiTxPtr);
}

/*****************************************************************************/
/**
* This function reports list of cores included in Video Processing Subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
static void XV_SdiTxSs_ReportCoreInfo(XV_SdiTxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n  ->SDI TX Subsystem Cores\r\n");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->SdiTxPtr) {
		xil_printf("    : SDI TX \r\n");
	}

	if (InstancePtr->VtcPtr) {
		xil_printf("    : VTC Core \r\n");
	}
}

/*****************************************************************************/
/**
* This function register's all sub-core ISR's with interrupt controller and
* any subsystem level call back function with requisite sub-core
*
* @param  InstancePtr is a pointer to the Subsystem instance to be
*       worked on.
*
*****************************************************************************/
static int XV_SdiTxSs_RegisterSubsysCallbacks(XV_SdiTxSs *InstancePtr)
{
	XV_SdiTxSs *SdiTxSsPtr = InstancePtr;

	/* Register SDI callbacks */
	if (SdiTxSsPtr->SdiTxPtr) {
		/*
	* Register call back for Tx Core Interrupts.
	*/
		XV_SdiTx_SetCallback(SdiTxSsPtr->SdiTxPtr,
		XV_SDITX_HANDLER_GTRESET_DONE,
		XV_SdiTxSs_GtReadyCallback,
		InstancePtr);

		XV_SdiTx_SetCallback(SdiTxSsPtr->SdiTxPtr,
		XV_SDITX_HANDLER_OVERFLOW,
		XV_SdiTxSs_OverFlowCallback,
		InstancePtr);

		XV_SdiTx_SetCallback(SdiTxSsPtr->SdiTxPtr,
		XV_SDITX_HANDLER_UNDERFLOW,
		XV_SdiTxSs_UnderFlowCallback,
		InstancePtr);

		XV_SdiTx_SetCallback(SdiTxSsPtr->SdiTxPtr,
		XV_SDITX_HANDLER_CEALIGN,
		XV_SdiTxSs_CeAlignErrCallback,
		InstancePtr);

		XV_SdiTx_SetCallback(SdiTxSsPtr->SdiTxPtr,
		XV_SDITX_HANDLER_AXI4SVIDLOCK,
		XV_SdiTxSs_Axi4sVidLockCallback,
		InstancePtr);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  SdiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
#ifndef SDT
static void XV_SdiTxSs_GetIncludedSubcores(XV_SdiTxSs *SdiTxSsPtr, u16 DevId)
{
	SdiTxSsPtr->SdiTxPtr = ((SdiTxSsPtr->Config.SdiTx.IsPresent) ?
		(&XV_SdiTxSs_SubCoreRepo[DevId].SdiTx) : NULL);
	SdiTxSsPtr->VtcPtr = ((SdiTxSsPtr->Config.Vtc.IsPresent) ?
		(&XV_SdiTxSs_SubCoreRepo[DevId].Vtc) : NULL);
}
#else
static void XV_SdiTxSs_GetIncludedSubcores(XV_SdiTxSs *SdiTxSsPtr, UINTPTR BaseAddress)
{
	u32 Index = 0;

	Index = XV_SdiTxSs_GetDrvIndex(SdiTxSsPtr, BaseAddress);
	SdiTxSsPtr->SdiTxPtr = ((SdiTxSsPtr->Config.SdiTx.IsPresent) ?
		(&XV_SdiTxSs_SubCoreRepo[Index].SdiTx) : NULL);
	SdiTxSsPtr->VtcPtr = ((SdiTxSsPtr->Config.Vtc.IsPresent) ?
		(&XV_SdiTxSs_SubCoreRepo[Index].Vtc) : NULL);
}
#endif

/*****************************************************************************/
/**
* This function queries the subsystem instance if the current mode is 3GB
* DL or not.
*
* @param  SdiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return TRUE - If current mode is 3GB DL
* 		  FALSE - If current mode is not 3GB DL
*
******************************************************************************/
static u8 XV_SdiTxSs_Is3GBDLor3GA1125L(XV_SdiTxSs *InstancePtr)
{
	u8 byte1;
	u32 tscan, pscan, *payloadId;

	if ((InstancePtr->SdiTxPtr->Transport.TMode == XSDIVID_MODE_3GA) ||
			(InstancePtr->SdiTxPtr->Transport.TMode == XSDIVID_MODE_3GB)) {
		payloadId = XV_SdiTxSs_GetPayloadId(InstancePtr, 0);
		byte1 = (*payloadId & XST352_PAYLOAD_BYTE_MASK);
		tscan = (*payloadId & XST352_BYTE2_TS_TYPE_MASK);
		pscan = (*payloadId & XST352_BYTE2_PIC_TYPE_MASK);
		if ((byte1 == XST352_BYTE1_ST372_DL_3GB) ||
				((byte1 == XST352_BYTE1_ST425_2008_1125L_3GA) &&
				 (!tscan) && pscan))
			return TRUE;
	}

	return FALSE;
}

/*****************************************************************************/
/**
* This function initializes the video subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CfgPtr points to the configuration structure associated with the
*         subsystem instance.
* @param  EffectiveAddr is the base address of the device. If address
*         translation is being used, then this parameter must reflect the
*         virtual base address. Otherwise, the physical address should be
*         used.
*
* @return XST_SUCCESS if initialization is successful else XST_FAILURE
*
******************************************************************************/
int XV_SdiTxSs_CfgInitialize(XV_SdiTxSs *InstancePtr, XV_SdiTxSs_Config *CfgPtr,
UINTPTR EffectiveAddr)
{
	XV_SdiTxSs *SdiTxSsPtr = InstancePtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_SdiTxSs));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
	sizeof(XV_SdiTxSs_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Determine sub-cores included in the provided instance of subsystem */
#ifndef SDT
	XV_SdiTxSs_GetIncludedSubcores(SdiTxSsPtr, CfgPtr->DeviceId);
#else
	XV_SdiTxSs_GetIncludedSubcores(SdiTxSsPtr, CfgPtr->BaseAddress);
#endif

	/* Initialize all included sub_cores */
	if (SdiTxSsPtr->SdiTxPtr) {
		if (XV_SdiTxSs_SubcoreInitSdiTx(SdiTxSsPtr) != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (SdiTxSsPtr->VtcPtr) {
		if (XV_SdiTxSs_SubcoreInitVtc(SdiTxSsPtr) != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Register Callbacks */
	XV_SdiTxSs_RegisterSubsysCallbacks(SdiTxSsPtr);

	/* Set the flag to indicate the subsystem is ready */
	SdiTxSsPtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is called when the GT is ready for TX configuration.
*
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_GtReadyCallback(void *CallbackRef)
{
	XV_SdiTxSs *SdiTxSsPtr = (XV_SdiTxSs *)CallbackRef;

	XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_STREAMUP, 0);

	/* Check if user callback has been registered */
	if (SdiTxSsPtr->SdiTxPtr->State == XV_SDITX_STATE_GTRESETDONE_NORMAL &&
			SdiTxSsPtr->GtReadyCallback) {
		SdiTxSsPtr->GtReadyCallback(SdiTxSsPtr->GtReadyRef);
	}

}

/*****************************************************************************/
/**
*
* This function is called when the Tx over flow happens.
*
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_OverFlowCallback(void *CallbackRef)
{
	XV_SdiTxSs *SdiTxSsPtr = (XV_SdiTxSs *)CallbackRef;

	XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_OVERFLOW, 0);

	/* Check if user callback has been registered */
	if (SdiTxSsPtr->OverFlowCallback)
		SdiTxSsPtr->OverFlowCallback(SdiTxSsPtr->OverFlowRef);

}

/*****************************************************************************/
/**
*
* This function is called when the Tx under flow happens.
*
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_UnderFlowCallback(void *CallbackRef)
{
	XV_SdiTxSs *SdiTxSsPtr = (XV_SdiTxSs *)CallbackRef;

	XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_UNDERFLOW, 0);

	/* Check if user callback has been registered */
	if (SdiTxSsPtr->UnderFlowCallback)
		SdiTxSsPtr->UnderFlowCallback(SdiTxSsPtr->UnderFlowRef);

}

/*****************************************************************************/
/**
*
* This function is called when the Tx CE align errors happens.
*
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_CeAlignErrCallback(void *CallbackRef)
{
	XV_SdiTxSs *SdiTxSsPtr = (XV_SdiTxSs *)CallbackRef;

	XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_CEALIGN, 0);

	/* Check if user callback has been registered */
	if (SdiTxSsPtr->CeAlignErrCallback)
		SdiTxSsPtr->CeAlignErrCallback(SdiTxSsPtr->CeAlignErrRef);

}

/*****************************************************************************/
/**
*
* This function is called when the Tx Axi4s video lock bridge happens.
*
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_Axi4sVidLockCallback(void *CallbackRef)
{
	XV_SdiTxSs *SdiTxSsPtr = (XV_SdiTxSs *)CallbackRef;

	XV_SdiTxSs_LogWrite(SdiTxSsPtr, XV_SDITXSS_LOG_EVT_AXI4SVIDLOCK, 0);

	/* Check if user callback has been registered */
	if (SdiTxSsPtr->Axi4sVidLockCallback)
		SdiTxSsPtr->Axi4sVidLockCallback(SdiTxSsPtr->Axi4sVidLockRef);

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                     Callback Function Type
* -----------------------         ---------------------------------------------
* (XV_SDITXSS_HANDLER_GTREADY)			GtReadyCallback
* (XV_SDITXSS_HANDLER_OVERFLOW)			OverFlowCallback
* (XV_SDITXSS_HANDLER_UNDERFLOW)		UnderFlowCallback
* (XV_SDITXSS_HANDLER_CEALIGN)			CeAlignErrCallback
* (XV_SDITXSS_HANDLER_AXI4SVIDLOCK)		Axi4sVidLockCallback
* </pre>
*
* @param    InstancePtr is a pointer to the SDI TX Subsystem instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*			callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_SdiTxSs_SetCallback(XV_SdiTxSs *InstancePtr, u32 HandlerType,
void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_SDITXSS_HANDLER_GTREADY));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	Status = (XST_SUCCESS);

	/* Check for handler type */
	switch (HandlerType) {
		/* Stream down */
	case (XV_SDITXSS_HANDLER_GTREADY):
		InstancePtr->GtReadyCallback = (XV_SdiTxSs_Callback)CallbackFunc;
		InstancePtr->GtReadyRef = CallbackRef;
		break;

		/* Overflow */
	case (XV_SDITXSS_HANDLER_OVERFLOW):
		InstancePtr->OverFlowCallback = (XV_SdiTxSs_Callback)CallbackFunc;
		InstancePtr->OverFlowRef = CallbackRef;
		break;

		/* Underflow */
	case (XV_SDITXSS_HANDLER_UNDERFLOW):
		InstancePtr->UnderFlowCallback = (XV_SdiTxSs_Callback)CallbackFunc;
		InstancePtr->UnderFlowRef = CallbackRef;
		break;

		/* CE align error */
	case (XV_SDITXSS_HANDLER_CEALIGN):
		InstancePtr->CeAlignErrCallback = (XV_SdiTxSs_Callback)CallbackFunc;
		InstancePtr->CeAlignErrRef = CallbackRef;
		break;

		/* Axi4s video lock */
	case (XV_SDITXSS_HANDLER_AXI4SVIDLOCK):
		InstancePtr->Axi4sVidLockCallback = (XV_SdiTxSs_Callback)CallbackFunc;
		InstancePtr->Axi4sVidLockRef = CallbackRef;
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This macro disables the SDI TX peripheral.
*
* @param	InstancePtr is a pointer to the XV_SdiTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTxSs_Stop(XV_SdiTxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiTx_StopSdi(InstancePtr->SdiTxPtr);

	XV_SdiTx_Axi4sBridgeVtcDisable(InstancePtr->SdiTxPtr);
	XV_SdiTx_VidBridgeDisable(InstancePtr->SdiTxPtr);

	/* Set stream up flag */
	InstancePtr->IsStreamUp = (FALSE);

	XV_SdiTxSs_LogWrite(InstancePtr, XV_SDITXSS_LOG_EVT_STOP, 0);

	/* VTC should not be accessed here as its clock may not be stable which
	may cause axilite access to stall due to a known issue of VTC. */
	/* if (InstancePtr->VtcPtr) { */
	/* Disable VTC */
	/* XVtc_DisableGenerator(InstancePtr->VtcPtr); */
	/* } */
}

/*****************************************************************************/
/**
*
* This function enables the ST352 value to be used from DS2 instead of DS3 register
* in C stream of the SDI TX Ss core.
*
* @param	InstancePtr is a pointer to the XV_SdiTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTxSs_ST352CSwitch3GA(XV_SdiTxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.InsertCSTRST352);
	XV_SdiTx_ST352CSwitch3GA(InstancePtr->SdiTxPtr);
}

/*****************************************************************************/
/**
*
* This function enables the insertion of ST352 in C stream of the SDI TX Ss core.
*
* @param	InstancePtr is a pointer to the XV_SdiTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTxSs_ST352CStreamEnable(XV_SdiTxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.InsertCSTRST352);
	XV_SdiTx_ST352CStreamEnable(InstancePtr->SdiTxPtr);
}

/*****************************************************************************/
/**
*
* This function Set the video format of the SDI TX Ss core.
*
* @param	InstancePtr is a pointer to the XV_SdiTxSs core instance.
* @param	ColorFormat is a variable of type XVidC_ColorFormat.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int XV_SdiTxSs_SetColorFormat(XV_SdiTxSs *InstancePtr, XVidC_ColorFormat ColorFormat)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	return XV_SdiTx_SetColorFormat(InstancePtr->SdiTxPtr, ColorFormat);
}
/*****************************************************************************/
/**
*
* This function configures Video Timing Controller (VTC).
*
* @param	XVtcPtr is a pointer to the XVtc core instance.
* @param	SdiTxPtr is a pointer to the XV_SdiTx core instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static int XV_SdiTxSs_VtcSetup(XVtc *XVtcPtr, XV_SdiTx *SdiTxPtr)
{
	/* Polarity configuration */
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;
	u32 SdiTx_Hblank;
	u32 Vtc_Hblank;
	u32 RegValue;

	/* Disable Generator */
	XVtc_Reset(XVtcPtr);
	XVtc_DisableGenerator(XVtcPtr);
	XVtc_Disable(XVtcPtr);

	/* Set up source select */
	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

	/* 1 = Generator registers, 0 = Detector registers */
	SourceSelect.VChromaSrc = 1;
	SourceSelect.VActiveSrc = 1;
	SourceSelect.VBackPorchSrc = 1;
	SourceSelect.VSyncSrc = 1;
	SourceSelect.VFrontPorchSrc = 1;
	SourceSelect.VTotalSrc = 1;
	SourceSelect.HActiveSrc = 1;
	SourceSelect.HBackPorchSrc = 1;
	SourceSelect.HSyncSrc = 1;
	SourceSelect.HFrontPorchSrc = 1;
	SourceSelect.HTotalSrc = 1;

	XVtc_SetSource(XVtcPtr, &SourceSelect);
	VideoTiming.HActiveVideo = SdiTxPtr->Stream[0].Video.Timing.HActive;
	VideoTiming.HFrontPorch = SdiTxPtr->Stream[0].Video.Timing.HFrontPorch;
	VideoTiming.HSyncWidth = SdiTxPtr->Stream[0].Video.Timing.HSyncWidth;
	VideoTiming.HBackPorch = SdiTxPtr->Stream[0].Video.Timing.HBackPorch;
	VideoTiming.HSyncPolarity = SdiTxPtr->Stream[0].Video.Timing.HSyncPolarity;

	/* Vertical Timing */
	VideoTiming.VActiveVideo = SdiTxPtr->Stream[0].Video.Timing.VActive;

	VideoTiming.V0FrontPorch = SdiTxPtr->Stream[0].Video.Timing.F0PVFrontPorch;
	VideoTiming.V0BackPorch = SdiTxPtr->Stream[0].Video.Timing.F0PVBackPorch;
	VideoTiming.V0SyncWidth = SdiTxPtr->Stream[0].Video.Timing.F0PVSyncWidth;

	VideoTiming.V1FrontPorch = SdiTxPtr->Stream[0].Video.Timing.F1VFrontPorch;
	VideoTiming.V1SyncWidth = SdiTxPtr->Stream[0].Video.Timing.F1VSyncWidth;
	VideoTiming.V1BackPorch = SdiTxPtr->Stream[0].Video.Timing.F1VBackPorch;

	VideoTiming.VSyncPolarity = SdiTxPtr->Stream[0].Video.Timing.VSyncPolarity;

	VideoTiming.Interlaced = SdiTxPtr->Stream[0].Video.IsInterlaced;

	/* 4 pixels per clock */
	if (SdiTxPtr->Stream[0].Video.PixPerClk == XVIDC_PPC_4) {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/4;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/4;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/4;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/4;
	}

	/* 2 pixels per clock */
	else if (SdiTxPtr->Stream[0].Video.PixPerClk == XVIDC_PPC_2) {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
	}

	/* 1 pixels per clock */
	else {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth;
	}

	/* For YUV420 the line width is double there for double the blanking */
	if (SdiTxPtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
	}

/** When compensating the vtc horizontal timing parameters for the pixel mode
* (quad or dual) rounding errors might be introduced (due to the divide)
* If this happens, the vtc total horizontal blanking is less than the sdi tx
* horizontal blanking.
* As a result the sdi tx vid out bridge is not able to lock to
* the incoming video stream.
* This process will check the horizontal blank timing and compensate
* for this condition.
* Calculate sdi tx horizontal blanking */

	SdiTx_Hblank = SdiTxPtr->Stream[0].Video.Timing.HFrontPorch +
	SdiTxPtr->Stream[0].Video.Timing.HSyncWidth +
	SdiTxPtr->Stream[0].Video.Timing.HBackPorch;

	do {
		/* Calculate vtc horizontal blanking */
		Vtc_Hblank = VideoTiming.HFrontPorch +
		VideoTiming.HBackPorch +
		VideoTiming.HSyncWidth;

		/* Quad pixel mode */
		if (SdiTxPtr->Stream[0].Video.PixPerClk == XVIDC_PPC_4) {
			Vtc_Hblank *= 4;
		}

		/* Dual pixel mode */
		else if (SdiTxPtr->Stream[0].Video.PixPerClk == XVIDC_PPC_2) {
			Vtc_Hblank *= 2;
		}

		/* Single pixel mode */
		else {
			/* Vtc_Hblank *= 1; */
		}

		/* For YUV420 the line width is double there for double the blanking */
		if (SdiTxPtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
			Vtc_Hblank *= 2;
		}

		/* If the horizontal total blanking differs, */
		/* then increment the Vtc horizontal front porch. */
		if (Vtc_Hblank != SdiTx_Hblank) {
			VideoTiming.HFrontPorch++;
		}

	} while (Vtc_Hblank < SdiTx_Hblank);

	if (Vtc_Hblank != SdiTx_Hblank) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"Error! Current format with total Hblank (%d) cannot \r\n",
		SdiTx_Hblank);
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"       be transmitted with pixels per clock = %d\r\n",
		SdiTxPtr->Stream.Video.PixPerClk);
		return XST_FAILURE;
	}

	XVtc_SetGeneratorTiming(XVtcPtr, &VideoTiming);

	/* Only for XVIDC_VM_720x486_60_I (SDI NTSC), the FIELD1 vactive
	 * size is different from FIELD0. As there is no vactive FIELD1
	 * entry in the video common library, program it separately as below */
	if (SdiTxPtr->Stream[0].Video.VmId == XVIDC_VM_720x486_60_I)
	{
		RegValue = (XSDITXSS_SD_NTSC_F1_V_ACTIVE << XSDITXSS_XVTC_ASIZE_VERT_SHIFT) &
				XSDITXSS_XVTC_ASIZE_VERT_MASK;
		XVtc_WriteReg(XVtcPtr->Config.BaseAddress,
				XSDITXSS_XVTC_GASIZE_F1_OFFSET, RegValue);
	}
	/* Set up Polarity of all outputs */
	memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol = 1;

	/* Polarity.FieldIdPol = 0; */
	if (VideoTiming.Interlaced) {
		Polarity.FieldIdPol = 1;
	} else {
		Polarity.FieldIdPol = 0;
	}

	/* SDI requires blanking polarity to be 1, which differs from what
* is in the Video Common Library for SD SDI modes. As a workaround
* they're manually set to 1.
*/
	if (SdiTxPtr->Stream[0].Video.VmId == XVIDC_VM_720x486_60_I ||
			SdiTxPtr->Stream[0].Video.VmId == XVIDC_VM_720x576_50_I) {
		Polarity.VBlankPol = 1;
		Polarity.VSyncPol = 1;
		Polarity.HBlankPol = 1;
		Polarity.HSyncPol = 1;
	} else {
		Polarity.VBlankPol = VideoTiming.VSyncPolarity;
		Polarity.VSyncPol = VideoTiming.VSyncPolarity;
		Polarity.HBlankPol = VideoTiming.HSyncPolarity;
		Polarity.HSyncPol = VideoTiming.HSyncPolarity;
	}

	XVtc_SetPolarity(XVtcPtr, &Polarity);

	/* VTC driver does not take care of the setting of the VTC in
* interlaced operation. As a work around the register
* is set manually */
	if (VideoTiming.Interlaced) {
		/* Interlaced mode */
		XVtc_WriteReg(XVtcPtr->Config.BaseAddress, 0x68, 0x42);
	} else {
		/* Progressive mode */
		XVtc_WriteReg(XVtcPtr->Config.BaseAddress, 0x68, 0x2);
	}

	/* Enable generator module */
	XVtc_EnableGenerator(XVtcPtr);
	XVtc_RegUpdateEnable(XVtcPtr);

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function starts the SDI TX stream
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_StreamStart(XV_SdiTxSs *InstancePtr)
{
	XSdiVid_TransMode SdiMode;
	SdiMode = InstancePtr->SdiTxPtr->Transport.TMode;

	/* Following assertions make sure the subssytem is configured with in the
	 * subcore GUI paramter limit
	 */
	Xil_AssertVoid(((InstancePtr->Config.MaxRateSupported == XSDITXSS_LINE_RATE_3G) &&
			(SdiMode <= XSDIVID_MODE_3GB)) ||
			((InstancePtr->Config.MaxRateSupported == XSDITXSS_LINE_RATE_6G) &&
			(SdiMode <= XSDIVID_MODE_6G)) ||
			((InstancePtr->Config.MaxRateSupported == XSDITXSS_LINE_RATE_12G8DS) &&
			(SdiMode <= XSDIVID_MODE_12G)));

	XV_SdiTx_SetVidBridgeMode(InstancePtr->SdiTxPtr,
	InstancePtr->SdiTxPtr->Transport.TMode);

	XV_SdiTx_VidBridgeEnable(InstancePtr->SdiTxPtr);

	/* Configure VTC */
	if (InstancePtr->VtcPtr) {
		/* Setup VTC */
		XV_SdiTxSs_VtcSetup(InstancePtr->VtcPtr, InstancePtr->SdiTxPtr);
	}

	XV_SdiTx_Axi4sBridgeVtcEnable(InstancePtr->SdiTxPtr);

	XV_SdiTxSs_LogWrite(InstancePtr, XV_SDITXSS_LOG_EVT_STREAMSTART, 0);
}

/*****************************************************************************/
/**
*
* This function configures SDI TX stream
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_StreamConfig(XV_SdiTxSs *InstancePtr)
{
	u32 PayloadLineNum1;
	u32 PayloadLineNum2;

	switch (InstancePtr->SdiTxPtr->Transport.TMode) {
	case XSDIVID_MODE_SD:
		if (InstancePtr->SdiTxPtr->Stream[0].Video.VmId
				== XVIDC_VM_720x486_60_I) {
			/* NTSC */
			PayloadLineNum1 = XV_SDITX_PAYLOADLN1_SDNTSC;
			PayloadLineNum2 = XV_SDITX_PAYLOADLN2_SDNTSC;
		} else {
			/* PAL */
			PayloadLineNum1 = XV_SDITX_PAYLOADLN1_SDPAL;
			PayloadLineNum2 = XV_SDITX_PAYLOADLN2_SDPAL;
		}
		break;

	case XSDIVID_MODE_3GA:
	case XSDIVID_MODE_3GB:
		/* Only for 3GB DL case we need to change the IsInterlaced
		 * to true as it should be true for 3GB DL. For remaining
		 * cases it should be as it is configured from the upper layer*/
		if (XV_SdiTxSs_Is3GBDLor3GA1125L(InstancePtr)) {
			InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced =
			XVidC_IsInterlaced(InstancePtr->SdiTxPtr->Stream[0].Video.VmId);
			if (InstancePtr->SdiTxPtr->Stream[0].Video.ColorDepth ==
			    XVIDC_BPC_10 &&
			    InstancePtr->SdiTxPtr->Stream[0].Video.ColorFormatId ==
			    XVIDC_CSF_YCRCB_422 ||
			    InstancePtr->SdiTxPtr->Stream[0].Video.ColorFormatId ==
			    XVIDC_CSF_YCRCB_420) {
				InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced = 1;
			}
		}
	case XSDIVID_MODE_HD:
	case XSDIVID_MODE_12G:
		PayloadLineNum1 = XV_SDITX_PAYLOADLN1_HD_3G_6G_12G;
		PayloadLineNum2 = XV_SDITX_PAYLOADLN2_HD_3G_6G_12G;
		break;
	case XSDIVID_MODE_6G:
		if (InstancePtr->SdiTxPtr->Stream[0].Video.ColorFormatId ==
				XVIDC_CSF_YCRCB_444) {
			InstancePtr->MaxDataStreams = 4;
		} else if (InstancePtr->SdiTxPtr->Stream[0].Video.ColorFormatId ==
				XVIDC_CSF_YCRCB_422){
			InstancePtr->MaxDataStreams =
					(InstancePtr->Config.bitdepth == 10) ? 8 : 4;
		} else {
			InstancePtr->MaxDataStreams = 8;
		}
		PayloadLineNum1 = XV_SDITX_PAYLOADLN1_HD_3G_6G_12G;
		PayloadLineNum2 = XV_SDITX_PAYLOADLN2_HD_3G_6G_12G;
		break;
	default:
		PayloadLineNum1 = 0;
		PayloadLineNum2 = 0;
		break;
	}

	XV_SdiTx_SetPayloadLineNum(InstancePtr->SdiTxPtr,
	PayloadLineNum1,
	PayloadLineNum2,
	InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced);

	XV_SdiTx_StreamStart(InstancePtr->SdiTxPtr);

	XV_SdiTxSs_LogWrite(InstancePtr, XV_SDITXSS_LOG_EVT_STREAMCFG, 0);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to SDI TX SS video stream payload ID
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
* @param  StreamId specifies which of the pointer of PayloadId to be returned
*
* @return u32 pointer
*
* @note   None.
*
******************************************************************************/
u32 *XV_SdiTxSs_GetPayloadId(XV_SdiTxSs *InstancePtr, u8 StreamId)
{
	return &InstancePtr->SdiTxPtr->Stream[StreamId].PayloadId;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to SDI TX SS video transport
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return XSdiVid_Transport pointer
*
* @note   None.
*
******************************************************************************/
XSdiVid_Transport *XV_SdiTxSs_GetTransport(XV_SdiTxSs *InstancePtr)
{
	return &InstancePtr->SdiTxPtr->Transport;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to SDI TX SS video stream
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
* @param  StreamId specifies which video stream's pointer to be returned
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_SdiTxSs_GetVideoStream(XV_SdiTxSs *InstancePtr,
	u8 StreamId)
{
	return &InstancePtr->SdiTxPtr->Stream[StreamId].Video;
}

/*****************************************************************************/
/**
*
* This function Sets the SDI TX SS video stream
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
* @param  VidStream specifies the settings for the video stream
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_SetVideoStream(XV_SdiTxSs *InstancePtr,
XVidC_VideoStream VidStream)
{
	InstancePtr->SdiTxPtr->Stream[0].Video = VidStream;
}

/*****************************************************************************/
/**
*
* This function reports stream errors detected.
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_ReportDetectedError(XV_SdiTxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiTx_ReportDetectedError(InstancePtr->SdiTxPtr);
}

/*****************************************************************************/
/**
*
* This function prints the SDI TX SS information.
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_ReportInfo(XV_SdiTxSs *InstancePtr)
{
	/* u32 Data; */
	u8 flag = (FALSE);

	xil_printf("------------\n\r");
	xil_printf("SDI TX SubSystem\n\r");
	xil_printf("------------\n\r");
	XV_SdiTxSs_ReportCoreInfo(InstancePtr);
	xil_printf("\n\r");
	xil_printf("SDI stream info\n\r");
	xil_printf("------------\n\r");
	/* Just for the info printing purpose we make interlaced is false
	 * before printing as for 3GB DL stream info is progressive and
	 * transmission is interlaced.*/
	if (XV_SdiTxSs_Is3GBDLor3GA1125L(InstancePtr) &&
			(InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced == 1)) {
		InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced = 0;
	    flag = (TRUE);
	}
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_STRMINFO);
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_SDIINFO);
	if (flag == (TRUE))
		InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced = 1;

	XV_SdiTx_ReportDetectedError(InstancePtr->SdiTxPtr);
	xil_printf("\n\r");
	xil_printf("SDI TX timing\n\r");
	xil_printf("------------\n\r");
	XV_SdiTxSs_ReportTiming(InstancePtr);
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the SDI TX SS debug information
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
* @param  VtcFlag specifies if VTC block is currently stable or not.
*
* @return None.
*
* @note   A known issue on the VTC causes it to require a stable clock before
*		  the registers may be access. VtcFlag is used to decide whether VTC
*		  can be read or not.
*
******************************************************************************/
void XV_SdiTxSs_ReportDebugInfo(XV_SdiTxSs *InstancePtr, u32 VtcFlag)
{
	u32 Data;

	xil_printf("------------\n\r");
	xil_printf("SDI TX SubSystem\n\r");
	xil_printf("------------\n\r");
	xil_printf("\n\r");
	xil_printf("Debug info\n\r");
	xil_printf("------------\n\r");
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_SDIDBGINFO);
	xil_printf("\n\r");
	xil_printf("SDI Registers Dump\n\r");
	xil_printf("------------\n\r");
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_REGDUMP);
	xil_printf("\n\r");
	if (VtcFlag == 1) {
		xil_printf("VTC Registers Dump\n\r");
		xil_printf("------------\n\r");
		for (int i = 0; i <= 81; i++) {
			Data =	XVtc_ReadReg(InstancePtr->VtcPtr->Config.BaseAddress, (i*4));
			xil_printf("Address: 0x%X Data: 0x%X\r\n",
			(InstancePtr->VtcPtr->Config.BaseAddress + (i*4)),
			Data);
		}
		xil_printf("\n\r");
	} else {
		xil_printf("VTC Registers Dump stopped due to unstable clock.\n\r");
	}
}

/*****************************************************************************/
/**
*
* This function prints the SDI TX SS stream information.
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiTxSs_ReportStreamInfo(XV_SdiTxSs *InstancePtr)
{
	u8 flag = (FALSE);

	xil_printf("------------\n\r");
	xil_printf("SDI TX SubSystem\n\r");
	xil_printf("------------\n\r");
	xil_printf("SDI stream info\n\r");
	xil_printf("------------\n\r");
	/* Just for the info printing purpose we make interlaced is false
	 * before printing, as the 3GB DL stream info is progressive and
	 * transmission is interlaced.*/
	if (XV_SdiTxSs_Is3GBDLor3GA1125L(InstancePtr) &&
			(InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced == 1)) {
		InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced = 0;
	    flag = (TRUE);
	}
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_STRMINFO);
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_SDIINFO);
	if (flag == (TRUE))
		InstancePtr->SdiTxPtr->Stream[0].Video.IsInterlaced = 1;

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the SDI TX SS timing information
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_SdiTxSs_ReportTiming(XV_SdiTxSs *InstancePtr)
{
	XV_SdiTx_DebugInfo(InstancePtr->SdiTxPtr, XV_SDITX_DBGSELID_TIMINGINFO);
	print("\r\n");
}

/*****************************************************************************/
/**
*
* This function checks if the video stream is up.
*
* @param  InstancePtr pointer to XV_SdiTxSs instance
*
* @return
*   - TRUE if stream is up.
*   - FALSE if stream is down.
*
* @note   None.
*
******************************************************************************/
int XV_SdiTxSs_IsStreamUp(XV_SdiTxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->IsStreamUp;
}

/*****************************************************************************/
/**
* This function is used to configure the SDI TX interrupts that are to
* be handled by the application. Refer to xv_sditxss_hw.h for interrupt masks.
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
* @param	IntrMask Indicates Mask for enable interrupts.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTxSs_IntrEnable(XV_SdiTxSs *InstancePtr, u32 IntrMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiTxPtr != NULL);

	XV_SdiTx *SdiTxPtr;

	SdiTxPtr = InstancePtr->SdiTxPtr;

	IntrMask &= XV_SDITXSS_IER_ALLINTR_MASK;
	Xil_AssertVoid(IntrMask != 0);
	XV_SdiTx_IntrEnable(SdiTxPtr, IntrMask);
}

/*****************************************************************************/
/**
* This function is used to configure the SDI TX interrupts that are to
* be handled by the application. Refer to xv_sditxss_hw.h for interrupt masks.
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
* @param	IntrMask Indicates Mask for disabling interrupts.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTxSs_IntrDisable(XV_SdiTxSs *InstancePtr, u32 IntrMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiTxPtr != NULL);

	XV_SdiTx *SdiTxPtr;

	SdiTxPtr = InstancePtr->SdiTxPtr;

	IntrMask &= XV_SDITXSS_IER_ALLINTR_MASK;
	Xil_AssertVoid(IntrMask != 0);
	XV_SdiTx_IntrDisable(SdiTxPtr, IntrMask);
}

/*****************************************************************************/
/**
* This function is used to update the Eotf and colorimetry fields of stream
* video structure. If SDI TX is already streaming, then this function updates
* the payload registers.
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
* @param	Eotf is a variable of type XVidC_Eotf
* @param	Colorimetry is a variable of type XVidC_ColorStd
*
* @return
*		None.
*
* @note	None.
*
******************************************************************************/
void XV_SdiTxSs_SetEotf(XV_SdiTxSs *InstancePtr, XVidC_Eotf Eotf,
		XVidC_ColorStd Colorimetry)
{
	XV_SdiTx *SdiTxPtr;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiTxPtr != NULL);

	SdiTxPtr = InstancePtr->SdiTxPtr;
	for (u8 Index = 0; Index < XV_SDITX_MAX_DATASTREAM; Index++) {
		SdiTxPtr->Stream[Index].Video.Eotf = Eotf;
		SdiTxPtr->Stream[Index].Video.ColorStd = Colorimetry;
	}

	/* Update the Eotf into st352 payload registers, only if stream is Up */
	if (InstancePtr->IsStreamUp) {
		u32 payload = XV_SdiTx_GetPayloadEotf(SdiTxPtr, Eotf,
						      Colorimetry);
		for (u8 StreamId = 0; StreamId < XV_SDITX_MAX_DATASTREAM;
		     StreamId++) {
			XV_SdiTx_SetPayloadId(SdiTxPtr, StreamId, payload);
		}
	}
}

/*****************************************************************************/
/**
* This function calculates the final ST352 payload value for all SDI modes
* with given video mode and SDI data stream number
*
* @param	InstancePtr is a pointer to the XV_SdiTxSs core instance.
* @param	VideoMode is a variable of type XVidC_VideoMode.
* @param	SdiMode is a variable of type XSdiVid_TransMode.
* @param	DataStream is the stream number for which payload is calculated.
*
* @return
*		XST_SUCCESS / XST_FAILURE.
*
* @note		None.
*
******************************************************************************/

u32 XV_SdiTxSs_GetPayload(XV_SdiTxSs *InstancePtr, XVidC_VideoMode VideoMode, XSdiVid_TransMode SdiMode, u8 DataStream)
{
	u32 Status;
	XV_SdiTx *SdiTxPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->SdiTxPtr != NULL);

	SdiTxPtr = InstancePtr->SdiTxPtr;

	Status = XV_SdiTx_GetPayload(SdiTxPtr, VideoMode, SdiMode, DataStream);

	return Status;

}

/*****************************************************************************/
/**
*
* This function sets the SDI TXSs stream parameters.
*
* @param    InstancePtr is a pointer to the XV_SdiTxSs core instance.
* @param    SelId specifies which parameter of the stream to be set.
*       - 0 = XV_SDITX_STREAMSELID_VMID
*       - 1 = XV_SDITX_STREAMSELID_COLORFORMAT
*       - 2 = XV_SDITX_STREAMSELID_BPC
*       - 3 = XV_SDITX_STREAMSELID_PPC
*       - 4 = XV_SDITX_STREAMSELID_ASPECTRATIO
*       - 5 = XV_SDITX_STREAMSELID_STANDARD
*       - 6 = XV_SDITX_STREAMSELID_STREAMINTERLACE
*       - 7 = XV_SDITX_STREAMSELID_CHANNEL
* @param    Data specifies what data to be set for the selected parameter.
* @param    StreamId specifies which of the streams to be set.
*
* @return
*	- XST_SUCCESS on successful Set stream
*	- XST_FAILURE if TimingPtr is not derived
*
*
* @note     None.
*
******************************************************************************/
u32 XV_SdiTxSs_SetStream(XV_SdiTxSs *InstancePtr, XV_SdiTx_StreamSelId SelId,
				u32 Data, u8 StreamId)
{
	XV_SdiTx *SdiTxPtr;
	int Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(StreamId < 8);
	SdiTxPtr = InstancePtr->SdiTxPtr;

	Status = XV_SdiTx_SetStream(SdiTxPtr, SelId, Data, StreamId);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the SDI TXSS core settings.
*
* @param    InstancePtr is a pointer to the XV_SdiTxSs core instance.
* @param    SelId specifies which parameter of the stream to be set.
*       - 0 = XV_SDITXSS_CORESELID_INSERTCRC
*       - 1 = XV_SDITXSS_CORESELID_INSERTST352
*       - 2 = XV_SDITXSS_CORESELID_ST352OVERWRITE
*       - 3 = XV_SDITXSS_CORESELID_INSERTSYNCBIT
*       - 4 = XV_SDITXSS_CORESELID_SDBITREPBYPASS
*       - 5 = XV_SDITXSS_CORESELID_USEANCIN
*       - 6 = XV_SDITXSS_CORESELID_INSERTLN
*       - 7 = XV_SDITXSS_CORESELID_INSERTEDH
* @param    Data specifies what data to be set for the selected parameter.
*
* @return
*
*
* @note     None.
*
******************************************************************************/
void XV_SdiTxSs_SetCoreSettings(XV_SdiTxSs *InstancePtr, XV_SdiTxSs_CoreSelId SelId,
				u8 Data)
{
	XV_SdiTx *SdiTxPtr;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiTxPtr != NULL);

	SdiTxPtr = InstancePtr->SdiTxPtr;

	switch (SelId) {
	case XV_SDITXSS_CORESELID_INSERTCRC:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_INSERTCRC, Data);
		break;
	case XV_SDITXSS_CORESELID_INSERTST352:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_INSERTST352, Data);
		break;
	case XV_SDITXSS_CORESELID_ST352OVERWRITE:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_ST352OVERWRITE, Data);
		break;
	case XV_SDITXSS_CORESELID_INSERTSYNCBIT:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_INSERTSYNCBIT, Data);
		break;
	case XV_SDITXSS_CORESELID_SDBITREPBYPASS:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_SDBITREPBYPASS, Data);
		break;
	case XV_SDITXSS_CORESELID_USEANCIN:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITX_CORESELID_USEANCIN, Data);
		break;
	case XV_SDITXSS_CORESELID_INSERTLN:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITXSS_CORESELID_INSERTLN, Data);
		break;
	case XV_SDITXSS_CORESELID_INSERTEDH:
		XV_SdiTx_SetCoreSettings(SdiTxPtr, XV_SDITXSS_CORESELID_INSERTEDH, Data);
		break;
	};
}

/*****************************************************************************/
/**
*
* This function enable the YUV444/RGB 10bit support SDI TX SS video stream
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
*
* @return	None
*
******************************************************************************/
void XV_SdiTxSs_SetYCbCr444_RGB_10bit(XV_SdiTxSs *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* Print detected error information */
        XV_SdiTx_SetYCbCr444_RGB_10bit(InstancePtr->SdiTxPtr);
}


/*****************************************************************************/
/**
*
* This function disable the YUV444/RGB 10bit support SDI TX SS video stream
*
* @param	InstancePtr pointer to XV_SdiTxSs instance
*
* @return	None
*
******************************************************************************/
void XV_SdiTxSs_ClearYCbCr444_RGB_10bit(XV_SdiTxSs *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* Print detected error information */
        XV_SdiTx_ClearYCbCr444_RGB_10bit(InstancePtr->SdiTxPtr);
}
