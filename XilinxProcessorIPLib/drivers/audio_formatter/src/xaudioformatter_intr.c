/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xaudioformatter_intr.c
* @addtogroup audioformatter_v1_1
* @{
*
* This file contains interrupt related functions of Xilinx Audio Formatter core.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaudioformatter.h"

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

void XAudioFormatter_SetS2MMCallback(XAudioFormatter *InstancePtr,
	XAudioFormatter_HandlerType handler_type, void *CallbackFunc,
	void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (handler_type == XAudioFormatter_IOC_Handler) {
		InstancePtr->S2MMIOCCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->S2MMIOCCallbackRef = CallbackRef;
	} else if (handler_type == XAudioFormatter_TIMEOUT_Handler) {
		InstancePtr->S2MMTOCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->S2MMTOCallbackRef = CallbackRef;
	} else if (handler_type == XAudioFormatter_ERROR_Handler) {
		InstancePtr->S2MMERRCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->S2MMERRCallbackRef = CallbackRef;
	}
}

void XAudioFormatter_SetMM2SCallback(XAudioFormatter *InstancePtr,
	XAudioFormatter_HandlerType handler_type, void *CallbackFunc,
	void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (handler_type == XAudioFormatter_IOC_Handler) {
		InstancePtr->MM2SIOCCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->MM2SIOCCallbackRef = CallbackRef;
	} else if (handler_type == XAudioFormatter_TIMEOUT_Handler) {
		InstancePtr->MM2STOCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->MM2STOCallbackRef = CallbackRef;
	} else if (handler_type == XAudioFormatter_ERROR_Handler) {
		InstancePtr->MM2SERRCallback =
			(XAudioFormatter_Callback)CallbackFunc;
		InstancePtr->MM2SERRCallbackRef = CallbackRef;
	}
}

void *XAudioFormatterS2MMIntrHandler(void *InstancePtr)
{
	u32 Data;

	XAudioFormatter *AFPtr = (XAudioFormatter *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(AFPtr != NULL);
	Xil_AssertVoid(AFPtr->IsReady == XIL_COMPONENT_IS_READY);

	AFPtr->ChannelId = XAudioFormatter_S2MM;
	Data = XAudioFormatter_ReadReg(AFPtr->BaseAddress,
		XAUD_FORMATTER_STS + XAUD_FORMATTER_S2MM_OFFSET);
	if (Data & XAUD_STS_IOC_IRQ_MASK) {
		XAudioFormatter_InterruptClear(AFPtr, XAUD_STS_IOC_IRQ_MASK);
		if (AFPtr->S2MMIOCCallback)
			AFPtr->S2MMIOCCallback(AFPtr);
	}
	if (Data & XAUD_STS_TIMEOUT_IRQ_MASK) {
		XAudioFormatter_InterruptClear(AFPtr,
			XAUD_STS_TIMEOUT_IRQ_MASK);
		if (AFPtr->S2MMTOCallback)
			AFPtr->S2MMTOCallback(AFPtr);
	}
	if (Data & XAUD_STS_ERROR_IRQ_MASK) {
		XAudioFormatter_InterruptClear(AFPtr, XAUD_STS_ERROR_IRQ_MASK);
		if (AFPtr->S2MMERRCallback)
			AFPtr->S2MMERRCallback(AFPtr);
	}
}

void *XAudioFormatterMM2SIntrHandler(void *InstancePtr)
{
	u32 Data;

	XAudioFormatter *AFPtr = (XAudioFormatter *) InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(AFPtr != NULL);
	Xil_AssertVoid(AFPtr->IsReady == XIL_COMPONENT_IS_READY);

	AFPtr->ChannelId = XAudioFormatter_MM2S;
	Data = XAudioFormatter_ReadReg(AFPtr->BaseAddress,
		XAUD_FORMATTER_STS + XAUD_FORMATTER_MM2S_OFFSET);
	if (Data & XAUD_STS_IOC_IRQ_MASK) {
		XAudioFormatter_InterruptClear(AFPtr, XAUD_STS_IOC_IRQ_MASK);
		if (AFPtr->MM2SIOCCallback)
			AFPtr->MM2SIOCCallback(AFPtr);
	}
	if (Data & XAUD_STS_ERROR_IRQ_MASK) {
		XAudioFormatter_InterruptClear(AFPtr, XAUD_STS_ERROR_IRQ_MASK);
		if (AFPtr->MM2SERRCallback)
			AFPtr->MM2SERRCallback(AFPtr);
	}
}
/*****************************************************************************/
/** @} */
