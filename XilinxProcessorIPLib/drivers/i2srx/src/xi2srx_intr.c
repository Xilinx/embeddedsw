/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2srx_intr.c
 * @addtogroup i2srx_v2_1
 * @{
 * ...
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0   kar    01/25/18    Initial release.
 * 1.1   kar    04/02/18    Changed log write API's argument to i2srx instance.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/*************************** Function Prototypes *****************************/

/************************** Variable Definitions *****************************/

/*************************** Function Definitions ****************************/

/*****************************************************************************/
/**
 *
 * This function is the interrupt handler for the XI2s Receiver driver.
 *
 * This handler reads the pending interrupt from the XI2s Receiver peripheral,
 * determines the source of the interrupts, clears the interrupts and calls
 * callbacks accordingly.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx instance.
 *
 * @return None.
 *
 * @note None.
 *
 *****************************************************************************/
void XI2s_Rx_IntrHandler(void *InstancePtr)
{
	u32 Data;
	u32 EnableMask;

	/* Convert the non-typed pointer to a XI2s_Rx instance pointer */
	XI2s_Rx *RxPtr = (XI2s_Rx *)InstancePtr;
	/* Verify arguments */
	Xil_AssertVoid(RxPtr != NULL);
	Xil_AssertVoid(RxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the interrupt status register */
	Data = XI2s_Rx_ReadReg(RxPtr->Config.BaseAddress,
			XI2S_RX_IRQSTS_OFFSET);

	/* Read the interrupt control register */
	EnableMask = XI2s_Rx_ReadReg(RxPtr->Config.BaseAddress,
			XI2S_RX_IRQCTRL_OFFSET);
	/* AES Block Complete Detected */
	if (Data & EnableMask & XI2S_RX_INTR_AES_BLKCMPLT_MASK) {
		/* Clear AES Block Complete event */
		XI2s_Rx_IntrClear(RxPtr, XI2S_RX_INTR_AES_BLKCMPLT_MASK);
		XI2s_Rx_LogWrite(RxPtr, XI2S_RX_AES_BLKCMPLT_EVT, 0);

		/* Call the AES Block Complete Handler */
		if (RxPtr->AesBlkCmpltHandler)
			RxPtr->AesBlkCmpltHandler(RxPtr->AesBlkCmpltRef);
	}
	/* Audio Overflow Detected */
	if (Data & EnableMask & XI2S_RX_INTR_AUDOVRFLW_MASK) {
		/* Clear the Audio Underflow Detected event */
		XI2s_Rx_IntrClear(RxPtr, XI2S_RX_INTR_AUDOVRFLW_MASK);
		XI2s_Rx_LogWrite(RxPtr, XI2S_RX_AUD_OVERFLOW_EVT, 0);

		/* Call the Audio Underflow Detected Handler */
		if (RxPtr->AudOverflowHandler)
			RxPtr->AudOverflowHandler(RxPtr->AudOverflowRef);
	}
}
/*****************************************************************************/
/**
 *
 * This function installs an asynchronous callback function for the given
 * HandlerType:
 *
 * <pre>
 * HandlerType                              Callback Function
 * --------------------------------         ----------------------------------
 * (XI2S_RX_HANDLER_AES_BLKCMPLT)            AesBlkCmpltHandler
 * (XI2S_RX_HANDLER_AUD_OVERFLOW)            AudOverflowHandler
 * </pre>
 *
 * @param InstancePtr is a pointer to the XI2s_Rx core instance.
 * @param HandlerType specifies the type of handler.
 * @param FuncPtr is a pointer to the callback function.
 * @param CallbackRef is a reference pointer passed on actual
 *        calling of the callback function.
 *
 * @return
 *  - XST_SUCCESS if callback function installed successfully.
 *  - XST_INVALID_PARAM when HandlerType is invalid.
 *
 * @note Invoking this function for a handler that already has been
 *       installed replaces it with the new handler.
 *
 *****************************************************************************/
int XI2s_Rx_SetHandler(XI2s_Rx *InstancePtr, XI2s_Rx_HandlerType HandlerType,
		XI2s_Rx_Callback FuncPtr, void *CallbackRef)
{
	int Status = XST_SUCCESS;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((HandlerType >= XI2S_RX_HANDLER_AES_BLKCMPLT) &&
			(HandlerType <  XI2S_RX_NUM_HANDLERS));
	Xil_AssertNonvoid(FuncPtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		case (XI2S_RX_HANDLER_AES_BLKCMPLT):
			InstancePtr->AesBlkCmpltHandler = FuncPtr;
			InstancePtr->AesBlkCmpltRef = CallbackRef;
			break;

		case (XI2S_RX_HANDLER_AUD_OVRFLW):
			InstancePtr->AudOverflowHandler = FuncPtr;
			InstancePtr->AudOverflowRef = CallbackRef;
			break;

		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	return Status;
}
/** @} */
