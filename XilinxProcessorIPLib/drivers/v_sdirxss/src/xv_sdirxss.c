/******************************************************************************
* Copyright (C) 2017 - 2020  Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_sdirxss.c
 *
 * This is the main file for Xilinx SDI RX core. Please see xv_ddirxss.h for
 * more details of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- -------------------------------------------------------
 * 1.00  jsr    07/17/17  Initial release.
 * 2.00  kar    01/25/18  Second release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_sdirxss.h"
#include "xv_sdirxss_coreinit.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XSDIRXSS_LINE_RATE_3G	0
#define XSDIRXSS_LINE_RATE_6G	1
#define XSDIRXSS_LINE_RATE_12G8DS 2
#define XSDIRXSS_AXI4_STREAM 0
#define XSDIRXSS_NATIVE_VIDEO 1
#define XSDIRXSS_NATIVE_SDI 2
/**************************** Type Definitions *******************************/
/**
* This typedef declares the driver instances of all the cores in the subsystem
*/
typedef struct {
	XV_SdiRx SdiRx;
} XV_SdiRxSs_SubCores;

/**************************** Local Global ***********************************/
#ifndef SDT
XV_SdiRxSs_SubCores XV_SdiRxSs_SubCoreRepo[XPAR_XV_SDIRXSS_NUM_INSTANCES];
#else
XV_SdiRxSs_SubCores XV_SdiRxSs_SubCoreRepo[];
#endif
/**< Define Driver instance of all sub-core
									included in the design */

/************************** Function Prototypes ******************************/
static void XV_SdiRxSs_StreamDownCallback(void *CallbackRef);
static void XV_SdiRxSs_StreamUpCallback(void *CallbackRef);
static void XV_SdiRxSs_OverFlowCallback(void *CallbackRef);
static void XV_SdiRxSs_UnderFlowCallback(void *CallbackRef);
static void XV_SdiRxSs_VsyncCallback(void *CallbackRef);
static void XV_SdiRxSs_ReportTiming(XV_SdiRxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function calls the interrupt handler for SDI RX
*
* @param InstancePtr is a pointer to the SDI RX Subsystem
*
*****************************************************************************/
void XV_SdiRxSS_SdiRxIntrHandler(XV_SdiRxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiRx_IntrHandler(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
* This function reports list of cores included in Video Processing Subsystem
*
* @param InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XV_SdiRxSs_ReportCoreInfo(XV_SdiRxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n ->SDI RX Subsystem Cores\r\n");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->SdiRxPtr) {
		xil_printf(" : SDI RX \r\n");
	}
}

/*****************************************************************************/
/**
* This function register's all sub-core ISR's with interrupt controller and
* any subsystem level call back function with requisite sub-core
*
* @param InstancePtr is a pointer to the Subsystem instance to be worked on.
*
*****************************************************************************/
static int XV_SdiRxSs_RegisterSubsysCallbacks(XV_SdiRxSs *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	XV_SdiRxSs *SdiRxSsPtr = InstancePtr;

	/* Register SDI callbacks */
	if (SdiRxSsPtr->SdiRxPtr) {
		/*
	* Register call back for Rx Core Interrupts.
	*/
		XV_SdiRx_SetCallback(SdiRxSsPtr->SdiRxPtr,
					XV_SDIRX_HANDLER_STREAM_DOWN,
					XV_SdiRxSs_StreamDownCallback,
					InstancePtr);

		XV_SdiRx_SetCallback(SdiRxSsPtr->SdiRxPtr,
					XV_SDIRX_HANDLER_STREAM_UP,
					XV_SdiRxSs_StreamUpCallback,
					InstancePtr);

		XV_SdiRx_SetCallback(SdiRxSsPtr->SdiRxPtr,
					XV_SDIRX_HANDLER_OVERFLOW,
					XV_SdiRxSs_OverFlowCallback,
					InstancePtr);

		XV_SdiRx_SetCallback(SdiRxSsPtr->SdiRxPtr,
					XV_SDIRX_HANDLER_UNDERFLOW,
					XV_SdiRxSs_UnderFlowCallback,
					InstancePtr);

		XV_SdiRx_SetCallback(SdiRxSsPtr->SdiRxPtr,
					XV_SDIRX_HANDLER_VSYNC,
					XV_SdiRxSs_VsyncCallback,
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
* @param SdiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
#ifndef SDT
static void XV_SdiRxSs_GetIncludedSubcores(XV_SdiRxSs *SdiRxSsPtr, u16 DevId)
{
	SdiRxSsPtr->SdiRxPtr = ((SdiRxSsPtr->Config.SdiRx.IsPresent) ?
	(&XV_SdiRxSs_SubCoreRepo[DevId].SdiRx) : NULL);
}
#else
static void XV_SdiRxSs_GetIncludedSubcores(XV_SdiRxSs *SdiRxSsPtr, UINTPTR BaseAddress)
{
	u32 Index = 0;

	Index = XV_SdiRxSs_GetDrvIndex(SdiRxSsPtr, BaseAddress);
	SdiRxSsPtr->SdiRxPtr = ((SdiRxSsPtr->Config.SdiRx.IsPresent) ?
	(&XV_SdiRxSs_SubCoreRepo[Index].SdiRx) : NULL);
}
#endif

/*****************************************************************************/
/**
* This function initializes the video subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param	CfgPtr points to the configuration structure associated with the
*       	subsystem instance.
* @param	EffectiveAddr is the base address of the device. If address
*        	translation is being used, then this parameter must reflect the
*        	virtual base address. Otherwise, the physical address should be
*        	used.
*
* @return	XST_SUCCESS if initialization is successful else XST_FAILURE
*
******************************************************************************/
int XV_SdiRxSs_CfgInitialize(XV_SdiRxSs *InstancePtr,
XV_SdiRxSs_Config *CfgPtr, UINTPTR EffectiveAddr)
{
	XV_SdiRxSs *SdiRxSsPtr = InstancePtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_SdiRxSs));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
	sizeof(XV_SdiRxSs_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Determine sub-cores included in the provided instance of subsystem */
#ifndef SDT
	XV_SdiRxSs_GetIncludedSubcores(SdiRxSsPtr, CfgPtr->DeviceId);
#else
	XV_SdiRxSs_GetIncludedSubcores(SdiRxSsPtr, CfgPtr->BaseAddress);
#endif

	/* Initialize all included sub_cores */
	if (SdiRxSsPtr->SdiRxPtr) {
		if (XV_SdiRxSs_SubcoreInitSdiRx(SdiRxSsPtr) != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Register Callbacks */
	XV_SdiRxSs_RegisterSubsysCallbacks(SdiRxSsPtr);

	/* Set the flag to indicate the subsystem is ready */
	SdiRxSsPtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is down.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_StreamDownCallback(void *CallbackRef)
{
	XV_SdiRxSs *SdiRxSsPtr = (XV_SdiRxSs *)CallbackRef;

	/* Set stream up flag */
	SdiRxSsPtr->IsStreamUp = (FALSE);

	XV_SdiRxSs_Stop(SdiRxSsPtr);
	XV_SdiRxSs_StreamFlowDisable(SdiRxSsPtr);

	/* TODO: Should only start RX based on earlier SEARCHMODE selection. */
	XV_SdiRxSs_Start(SdiRxSsPtr, XV_SDIRX_MULTISEARCHMODE);
	XV_SdiRxSs_LogWrite(SdiRxSsPtr, XV_SDIRXSS_LOG_EVT_STREAMDOWN, 0);

	/* Check if user callback has been registered */
	if (SdiRxSsPtr->StreamDownCallback) {
		SdiRxSsPtr->StreamDownCallback(SdiRxSsPtr->StreamDownRef);
	}

}

/*****************************************************************************/
/**
*
* This function is called when the Rx Over Flow occurs.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_OverFlowCallback(void *CallbackRef)
{
	XV_SdiRxSs *SdiRxSsPtr = (XV_SdiRxSs *)CallbackRef;

	/* Check if user callback has been registered */
	if (SdiRxSsPtr->OverFlowCallback) {
		SdiRxSsPtr->OverFlowCallback(SdiRxSsPtr->OverFlowRef);
	}

}

/*****************************************************************************/
/**
*
* This function is called when the Rx Under Flow occurs.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_UnderFlowCallback(void *CallbackRef)
{
	XV_SdiRxSs *SdiRxSsPtr = (XV_SdiRxSs *)CallbackRef;

	/* Check if user callback has been registered */
	if (SdiRxSsPtr->UnderFlowCallback) {
		SdiRxSsPtr->UnderFlowCallback(SdiRxSsPtr->UnderFlowRef);
	}

}

/*****************************************************************************/
/**
*
* This function is called when the Rx Vsync occurs.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_VsyncCallback(void *CallbackRef)
{
	XV_SdiRxSs *SdiRxSsPtr = (XV_SdiRxSs *)CallbackRef;

	/* Check if user callback has been registered */
	if (SdiRxSsPtr->VsyncCallback) {
		SdiRxSsPtr->VsyncCallback(SdiRxSsPtr->VsyncRef);
	}

}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is up.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_StreamUpCallback(void *CallbackRef)
{
	XV_SdiRxSs *SdiRxSsPtr = (XV_SdiRxSs *)CallbackRef;

	/* Set stream up flag */
	SdiRxSsPtr->IsStreamUp = (TRUE);

	XV_SdiRxSs_LogWrite(SdiRxSsPtr, XV_SDIRXSS_LOG_EVT_STREAMUP, 0);

	/* Check if user callback has been registered */
	if (SdiRxSsPtr->StreamUpCallback) {
		SdiRxSsPtr->StreamUpCallback(SdiRxSsPtr->StreamUpRef);
	}
}

/*****************************************************************************/
/**
*
* This function enables the AXIS and video bridges.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_StreamFlowEnable(XV_SdiRxSs *InstancePtr)
{
	XV_SdiRx_Axi4sBridgeEnable(InstancePtr->SdiRxPtr);
	XV_SdiRx_VidBridgeEnable(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
*
* This function disables the AXIS and video bridges.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_StreamFlowDisable(XV_SdiRxSs *InstancePtr)
{
	XV_SdiRx_Axi4sBridgeDisable(InstancePtr->SdiRxPtr);
	XV_SdiRx_VidBridgeDisable(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
*
* This function starts the SDI RX stream detection.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
* @param	Mode specifies the mode of SDI modes searching operation.
*		- 0 = XV_SDIRX_SINGLESEARCHMODE_HD
*		- 1 = XV_SDIRX_SINGLESEARCHMODE_SD
*		- 2 = XV_SDIRX_SINGLESEARCHMODE_3G
*		- 4 = XV_SDIRX_SINGLESEARCHMODE_6G
*		- 5 = XV_SDIRX_SINGLESEARCHMODE_12GI
*		- 6 = XV_SDIRX_SINGLESEARCHMODE_12GF
*		- 10 = XV_SDIRX_MULTISEARCHMODE where the supported modes will be
*				enabled by XV_SdiRx_EnableMode function
*
* @return	None.
*
* @note   None.
*
******************************************************************************/
void XV_SdiRxSs_Start(XV_SdiRxSs *InstancePtr, XV_SdiRx_SearchMode Mode)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* TODO: Should only start RX based on earlier SEARCHMODE selection. */
	XV_SdiRx_Start(InstancePtr->SdiRxPtr, Mode);

	/* Stat_reset */
	XV_SdiRxSs_LogWrite(InstancePtr, XV_SDIRXSS_LOG_EVT_START, 0);
}

/*****************************************************************************/
/**
* This function stops the SDI RX stream detection.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XV_SdiRxSs_Stop(XV_SdiRxSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiRx_Stop(InstancePtr->SdiRxPtr);

	XV_SdiRxSs_LogWrite(InstancePtr, XV_SDIRXSS_LOG_EVT_STOP, 0);
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
* (XV_SDIRXSS_HANDLER_STREAM_DOWN)         StreamDownCallback
* (XV_SDIRXSS_HANDLER_STREAM_UP)           StreamUpCallback
* (XV_SDIRXSS_HANDLER_OVERFLOW)		OverFlowCallback
* (XV_SDIRXSS_HANDLER_UNDERFLOW)           UnderFlowCallback
* (XV_SDIRXSS_HANDLER_VSYNC)		   VsyncCallback
* </pre>
*
* @param    InstancePtr is a pointer to the SDI RX Subsystem instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_SdiRxSs_SetCallback(XV_SdiRxSs *InstancePtr, u32 HandlerType,
void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_SDIRXSS_HANDLER_STREAM_DOWN));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		/* Stream down */
	case (XV_SDIRXSS_HANDLER_STREAM_DOWN):
		InstancePtr->StreamDownCallback =
		(XV_SdiRxSs_Callback)CallbackFunc;
		InstancePtr->StreamDownRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

		/* Stream up */
	case (XV_SDIRXSS_HANDLER_STREAM_UP):
		InstancePtr->StreamUpCallback =
		(XV_SdiRxSs_Callback)CallbackFunc;
		InstancePtr->StreamUpRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

		/* Over Flow */
	case (XV_SDIRXSS_HANDLER_OVERFLOW):
		InstancePtr->OverFlowCallback =
		(XV_SdiRxSs_Callback)CallbackFunc;
		InstancePtr->OverFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

		/* Under Flow */
	case (XV_SDIRXSS_HANDLER_UNDERFLOW):
		InstancePtr->UnderFlowCallback =
		(XV_SdiRxSs_Callback)CallbackFunc;
		InstancePtr->UnderFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

		/* Vsync */
	case (XV_SDIRXSS_HANDLER_VSYNC):
		InstancePtr->VsyncCallback =
		(XV_SdiRxSs_Callback)CallbackFunc;
		InstancePtr->VsyncRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

u32 *XV_SdiRxSs_GetPayloadId(XV_SdiRxSs *InstancePtr, u8 StreamId)
{
	return &InstancePtr->SdiRxPtr->Stream[StreamId].PayloadId;
}

XSdiVid_Transport *XV_SdiRxSs_GetTransport(XV_SdiRxSs *InstancePtr)
{
	return &InstancePtr->SdiRxPtr->Transport;
}

XSdiVid_TransMode XV_SdiRxSs_GetTransportMode(XV_SdiRxSs *InstancePtr)
{
	XSdiVid_TransMode TMode;
	TMode =  InstancePtr->SdiRxPtr->Transport.TMode;

	/* Following assertions make sure the IPSS is configured with in the
	 * subsystem GUI paramter limit
	 */
	Xil_AssertNonvoid(((InstancePtr->Config.MaxRateSupported == XSDIRXSS_LINE_RATE_3G) &&
			(TMode <= XSDIVID_MODE_3GB)) ||
			((InstancePtr->Config.MaxRateSupported == XSDIRXSS_LINE_RATE_6G) &&
			(TMode <= XSDIVID_MODE_6G)) ||
			((InstancePtr->Config.MaxRateSupported == XSDIRXSS_LINE_RATE_12G8DS) &&
			(TMode <= XSDIVID_MODE_12G)));

	return InstancePtr->SdiRxPtr->Transport.TMode;
}

u8 XV_SdiRxSs_GetTransportBitRate(XV_SdiRxSs *InstancePtr)
{
	return InstancePtr->SdiRxPtr->Transport.IsFractional;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to SDI RX SS video stream
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
* @param	StreamId specifies which video stream's pointer to be returned
*
* @return	XVidC_VideoStream pointer
*
* @note		None.
*
******************************************************************************/
XVidC_VideoStream *XV_SdiRxSs_GetVideoStream(XV_SdiRxSs *InstancePtr,
	u8 StreamId)
{
	return &InstancePtr->SdiRxPtr->Stream[StreamId].Video;
}

void XV_SdiRxSs_ReportDetectedError(XV_SdiRxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Print detected error information */
	XV_SdiRx_ReportDetectedError(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
*
* This function enable the YUV444/RGB 10bit support SDI RX SS video stream
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None
*
******************************************************************************/
void XV_SdiRxSs_SetYCbCr444_RGB_10bit(XV_SdiRxSs *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* Print detected error information */
        XV_SdiRx_SetYCbCr444_RGB_10bit(InstancePtr->SdiRxPtr);
}


/*****************************************************************************/
/**
*
* This function disable the YUV444/RGB 10bit support SDI RX SS video stream
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None
*
******************************************************************************/
void XV_SdiRxSs_ClearYCbCr444_RGB_10bit(XV_SdiRxSs *InstancePtr)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);

        /* Print detected error information */
        XV_SdiRx_ClearYCbCr444_RGB_10bit(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
*
* This function prints the SDI RX SS information.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_ReportInfo(XV_SdiRxSs *InstancePtr)
{
	xil_printf("------------\n\r");
	xil_printf("SDI Rx SubSystem\n\r");
	xil_printf("------------\n\r");
	XV_SdiRxSs_ReportCoreInfo(InstancePtr);
	xil_printf("\n\r");
	xil_printf("SDI stream info\n\r");
	xil_printf("------------\n\r");
	XV_SdiRx_DebugInfo(InstancePtr->SdiRxPtr, XV_SDIRX_DBGSELID_STRMINFO);
	XV_SdiRx_DebugInfo(InstancePtr->SdiRxPtr, XV_SDIRX_DBGSELID_SDIINFO);
	XV_SdiRx_ReportDetectedError(InstancePtr->SdiRxPtr);
	xil_printf("\n\r");
	xil_printf("SDI RX timing\n\r");
	xil_printf("------------\n\r");
	XV_SdiRxSs_ReportTiming(InstancePtr);
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the SDI RX SS debug information.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_ReportDebugInfo(XV_SdiRxSs *InstancePtr)
{
	xil_printf("------------\n\r");
	xil_printf("SDI Rx SubSystem\n\r");
	xil_printf("------------\n\r");
	xil_printf("\n\r");
	xil_printf("Debug info\n\r");
	xil_printf("------------\n\r");
	XV_SdiRx_DebugInfo(InstancePtr->SdiRxPtr, XV_SDIRX_DBGSELID_SDIDBGINFO);
	xil_printf("\n\r");
	xil_printf("SDI Registers Dump\n\r");
	xil_printf("------------\n\r");
	XV_SdiRx_DebugInfo(InstancePtr->SdiRxPtr, XV_SDIRX_DBGSELID_REGDUMP);
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the SDI RX SS timing information
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XV_SdiRxSs_ReportTiming(XV_SdiRxSs *InstancePtr)
{
	XV_SdiRx_DebugInfo(InstancePtr->SdiRxPtr, XV_SDIRX_DBGSELID_TIMINGINFO);
	print("\n\r");
}

/*****************************************************************************/
/**
*
* This function checks if the video stream is up.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return
*		- TRUE if stream is up.
*		- FALSE if stream is down.
*
* @note		None.
*
******************************************************************************/
int XV_SdiRxSs_IsStreamUp(XV_SdiRxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->IsStreamUp;
}

/*****************************************************************************/
/**
* This function is used to configure the SDI RX interrupts that are to
* be handled by the application. Refer to xv_sdirxss_hw.h for interrupt masks.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
* @param	IntrMask Indicates Mask for enable interrupts.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_IntrEnable(XV_SdiRxSs *InstancePtr, u32 IntrMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiRxPtr != NULL);

	XV_SdiRx *SdiRxPtr;

	SdiRxPtr = InstancePtr->SdiRxPtr;

	IntrMask &= XV_SDIRXSS_IER_ALLINTR_MASK;
	Xil_AssertVoid(IntrMask != 0);
	XV_SdiRx_IntrEnable(SdiRxPtr, IntrMask);
}

/*****************************************************************************/
/**
* This function is used to configure the SDI RX interrupts that are to
* be handled by the application. Refer to xv_sdirxss_hw.h for interrupt masks.
*
* @param	InstancePtr pointer to XV_SdiRxSs instance
* @param	IntrMask Indicates Mask for disabling interrupts.
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRxSs_IntrDisable(XV_SdiRxSs *InstancePtr, u32 IntrMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->SdiRxPtr != NULL);

	XV_SdiRx *SdiRxPtr;

	SdiRxPtr = InstancePtr->SdiRxPtr;

	IntrMask &= XV_SDIRXSS_IER_ALLINTR_MASK;
	Xil_AssertVoid(IntrMask != 0);
	XV_SdiRx_IntrDisable(SdiRxPtr, IntrMask);
}


/*****************************************************************************/
/**
* This function is used to wait for the payload valid bit to be set.
* This has to be called from application based on the callback indication of
* the video lock interrupt handler.

* @param	InstancePtr pointer to XV_SdiRxSs instance
*
* @return
*		- XST_FAILURE if the payload valid bits are not set.
*		- XST_SUCCESS if ST352 registers are read and loaded to
*		  Rx structures
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRxSs_WaitforPayLoad(XV_SdiRxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->SdiRxPtr != NULL);

	return XV_SdiRx_WaitforPayLoad(InstancePtr->SdiRxPtr);
}

/*****************************************************************************/
/**
*
* This function disables handling for no payload case
*
* @param	InstancePtr is a pointer to the XV_SdiRxSs core instance.
* @param	enable is to enable/disable the Handling of nopayload case
*
* @return	None
*
******************************************************************************/
void XV_SdiRxSs_HandleNoPayload(XV_SdiRxSs *InstancePtr, u8 enable)
{
	XV_SdiRx_HandleNoPayload(InstancePtr->SdiRxPtr, enable);
}
