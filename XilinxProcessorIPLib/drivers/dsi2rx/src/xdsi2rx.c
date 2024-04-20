/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rx.c
* @addtogroup dsi2rx Overview
* @{
*
* This file implements the functions to control and get info from the DSI TX
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who   Date    Changes
* --- ---  ------- -------------------------------------------------------
* 1.0 Kunal 18/04/24 Initial Release DSI2RX driver
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi2rx.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/*
* Each of callback functions to be called on different types of interrupts.
* These stub functions are set during XDsi2Rx_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubErrCallback(void *CallbackRef, u32 ErrorMask);

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
* Initialize the XDsi2Rx instance provided by the caller based on the
* given Config structure.
*
* @param	InstancePtr is the XDsi2Rx instance to operate on.
* @param	CfgPtr is the device configuration structure containing
*		information about a specific DSI.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was unsuccessful.
*
* @note		None.
*****************************************************************************/
u32 XDsi2Rx_CfgInitialize(XDsi2Rx *InstancePtr, XDsi2Rx_Config *CfgPtr,
					 UINTPTR EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;

	InstancePtr->Config.BaseAddr = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->UnSupportedDataTypeCallback = StubErrCallback;
	InstancePtr->CRCCallback   = StubErrCallback;
	InstancePtr->ECC1Callback   = StubErrCallback;
	InstancePtr->ECC2Callback   = StubErrCallback;
	InstancePtr->SOTSyncErrLane1Callback   = StubErrCallback;
	InstancePtr->SOTErrLane1Callback   = StubErrCallback;
	InstancePtr->SOTSyncErrLane2Callback   = StubErrCallback;
	InstancePtr->SOTErrLane2Callback   = StubErrCallback;
	InstancePtr->SOTSyncErrLane3Callback   = StubErrCallback;
	InstancePtr->SOTErrLane3Callback   = StubErrCallback;
	InstancePtr->SOTSyncErrLane4Callback   = StubErrCallback;
	InstancePtr->SOTErrLane4Callback   = StubErrCallback;
	InstancePtr->StopStateCallback   = StubErrCallback;
	InstancePtr->LmAsyncFifoFullCallback   = StubErrCallback;
	InstancePtr->StreamAsyncFifoFullCallback   = StubErrCallback;
	InstancePtr->GSPFifoNECallback   = StubErrCallback;
	InstancePtr->GSPFifoFullCallback   = StubErrCallback;
	InstancePtr->FrmStartDetCallback   = StubErrCallback;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will do a reset of the IP. Register ISR gets reset.
* Internal FIFO(command queue) gets flushed. FSM stops processing further
* packets. Controller gracefully ends by waiting for the current sub-block in
* operation to complete its task and mark next byte as LP byte to end the
* transfer. Once soft reset is released, controller start from VSS packet.
*(that is new video frame)
*
* @param	InstancePtr is the XDsi2Rx instance to operate on.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XDsi2Rx_Reset(XDsi2Rx *InstancePtr)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	XDsi2Rx_SetSoftReset(InstancePtr);

	XDsi2Rx_ClearSoftReset(InstancePtr);
}

/*****************************************************************************/
/**
* This function will configure protocol reg with video mode, Blank packet mode,
* Blank packet Type, End of Transmisstion packet
*
* @param	InstancePtr is the XDsi2Rx instance to operate on.
*
* @return
* 		- XST_SUCCESS On enabling the core.
*
* @note		None.
*
****************************************************************************/
u32 XDsi2Rx_DefaultConfigure(XDsi2Rx *InstancePtr)
{

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Set the Pixel Mode */
	XDsi2Rx_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI2RX_PCR_OFFSET, XDSI2RX_PCR_PIXELMODE_MASK, XDSI2RX_PCR_PIXELMODE_SHIFT,
	InstancePtr->PixelMode);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will enable/disable the IP Core to start processing.
*
* @param	InstancePtr is the XDsi2Rx instance to operate on.
*
* @param 	Flag will be used to indicate Enable or Disable action.
*
* @return
* 		- XST_SUCCESS On enabling/disabling the core.
*		- XST_INVALID_PARAM if user passes invalid parameter.
*
* @note		None.
*
****************************************************************************/
u32 XDsi2Rx_Activate(XDsi2Rx *InstancePtr, XDsi2Rx_Selection Flag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Flag == XDSI2RX_DISABLE) {
		XDsi2Rx_ResetGlobalInterrupt(InstancePtr);
		XDsi2Rx_Disable(InstancePtr);
		return XST_SUCCESS;
	}
	else {
		XDsi2Rx_SetGlobalInterrupt(InstancePtr);
		XDsi2Rx_Enable(InstancePtr);
		return XST_SUCCESS;
	}
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* this function will get the information from the gui settings and other
* protocol control register values like pixel mode, pixel format.
*
* @param	instanceptr is the xdsi2rx instance to operate on
* @param	configinfo is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	none.
*
* @note		none.
*
****************************************************************************/
void XDsi2Rx_GetConfigParams(XDsi2Rx *InstancePtr,
		XDsi2Rx_ConfigParameters *ConfigInfo)
{
	/* verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigInfo != NULL);

	/* read xdsi2rx config structure */
	ConfigInfo->Config =  InstancePtr->Config;

	/* get the video mode transmission sequence*/
	ConfigInfo->PixelMode = XDsi2Rx_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI2RX_PCR_OFFSET,
	 XDSI2RX_PCR_PIXELMODE_MASK, XDSI2RX_PCR_PIXELMODE_SHIFT);
}

/*****************************************************************************/
/**
* this routine is a stub for the asynchronous error interrupt callback. the
* stub is here in case the upper layer forgot to set the handler. on
* initialization, error interrupt handler is set to this callback. it is
* considered an error for this handler to be invoked.
*
* @param	callbackref is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param 	errormask is a bit mask indicating the cause of the error. its
*		value equals 'or'ing one or more xdsi2rx_isr_*_mask values defined
*		in xdsi_hw.h.
*
* @return	none.
*
* @note		none.
*
******************************************************************************/
static void StubErrCallback(void *CallbackRef, u32 ErrorMask)
{
	(void) ((void *)CallbackRef);
	(void) ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
