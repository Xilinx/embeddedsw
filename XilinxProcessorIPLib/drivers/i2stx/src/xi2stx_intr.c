/******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2stx_intr.c
 * @addtogroup i2stx_v1_1
 * @{
 *
 * This file contains functions related to i2s_transmitter interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date       Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0   kar   11/16/17   Initial release.
 * 1.1   kar   04/02/18   Changed log write API's argument to i2stx instance.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2stx.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**************************** Type Definitions *******************************/

/*************************** Function Prototypes *****************************/

/************************** Variable Definitions *****************************/

/*************************** Function Definitions ****************************/

/*****************************************************************************/
/**
 *
 * This function is the interrupt handler for the I2S Transmitter driver.
 *
 * This handler reads the pending interrupt from the I2S Transmitter peripheral,
 * determines the source of the interrupts, clears the interrupts and calls
 * callbacks accordingly.
 *
 *
 * @param InstancePtr is a pointer to the XI2s_Tx instance.
 *
 * @return None.
 *
 * @note None.
 *
 *****************************************************************************/
void XI2s_Tx_IntrHandler(void *InstancePtr)
{
	u32 Data;
	u32 EnableMask;

	/* Convert the non-typed pointer to a XI2S_Tx instance pointer */
	XI2s_Tx *TxPtr = (XI2s_Tx *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(TxPtr != NULL);
	Xil_AssertVoid(TxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the interrupt status register */
	Data = XI2s_Tx_ReadReg(TxPtr->Config.BaseAddress,
			XI2S_TX_IRQSTS_OFFSET);

	/* Read the interrupt control register */
	EnableMask = XI2s_Tx_ReadReg(TxPtr->Config.BaseAddress,
			XI2S_TX_IRQCTRL_OFFSET);
	/* AES Block Complete Detected */
	if (Data & EnableMask & XI2S_TX_INTR_AES_BLKCMPLT_MASK) {
		/* Clear AES Block Complete event */
		XI2s_Tx_IntrClear(TxPtr, XI2S_TX_INTR_AES_BLKCMPLT_MASK);
		XI2s_Tx_LogWrite(TxPtr, XI2S_TX_AES_BLKCMPLT_EVT, 0);

		/* Call the AES Block Complete Handler */
		if (TxPtr->AesBlkCmpltHandler)
			TxPtr->AesBlkCmpltHandler(TxPtr->AesBlkCmpltRef);
	}
	/* AES Block Synchronization Error */
	if (Data & EnableMask & XI2S_TX_INTR_AES_BLKSYNCERR_MASK) {
		/* Clear AES Block Synchronization Error event */
		XI2s_Tx_IntrClear(TxPtr, XI2S_TX_INTR_AES_BLKSYNCERR_MASK);
		XI2s_Tx_LogWrite(TxPtr, XI2S_TX_AES_BLKSYNCERR_EVT, 0);

		/* Call the AES Block Synchronization Error Handler */
		if (TxPtr->AesBlkSyncErrHandler)
			TxPtr->AesBlkSyncErrHandler(TxPtr->AesBlkSyncErrRef);
	}
	/* AES Channel Status Updated */
	if (Data & EnableMask & XI2S_TX_INTR_AES_CHSTSUPD_MASK) {
		/* Clear the AES Channel Status Updated event */
		XI2s_Tx_IntrClear(TxPtr, XI2S_TX_INTR_AES_CHSTSUPD_MASK);
		XI2s_Tx_LogWrite(TxPtr, XI2S_TX_AES_CHSTSUPD_EVT, 0);

		/* Call the AES Channel Status Updated Handler */
		if (TxPtr->AesChStsUpdHandler)
			TxPtr->AesChStsUpdHandler(TxPtr->AesChStsUpdRef);
	}
	/* Audio Underflow Detected */
	if (Data & EnableMask & XI2S_TX_INTR_AUDUNDRFLW_MASK) {
		/* Clear the Audio Underflow Detected event */
		XI2s_Tx_IntrClear(TxPtr, XI2S_TX_INTR_AUDUNDRFLW_MASK);
		XI2s_Tx_LogWrite(TxPtr, XI2S_TX_AUD_UNDRFLW_EVT, 0);

		/* Call the Audio Underflow Detected Handler */
		if (TxPtr->AudUndrflwHandler)
			TxPtr->AudUndrflwHandler(TxPtr->AudUndrflwRef);
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
 * --------------------------------         -------------------------
 * (XI2S_TX_HANDLER_AES_BLKCMPLT)            AesBlkCmpltHandler
 * (XI2S_TX_HANDLER_AES_BLKSYNCERR)          AesBlkSyncErrHandler
 * (XI2S_TX_HANDLER_AES_CHSTSUPD)            AesChStsUpdHandler
 * (XI2S_TX_HANDLER_AUD_UNDRFLW)             AudUndrflwHandler
 * </pre>
 *
 * @param InstancePtr is a pointer to the XI2s_Tx core instance.
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
int XI2s_Tx_SetHandler(XI2s_Tx *InstancePtr,
		XI2s_Tx_HandlerType HandlerType,
		XI2s_Tx_Callback FuncPtr,
		void *CallbackRef)
{
	int Status = XST_SUCCESS;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((HandlerType >= XI2S_TX_HANDLER_AES_BLKCMPLT) &&
			(HandlerType <  XI2S_TX_NUM_HANDLERS));
	Xil_AssertNonvoid(FuncPtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		case (XI2S_TX_HANDLER_AES_BLKCMPLT):
			InstancePtr->AesBlkCmpltHandler = FuncPtr;
			InstancePtr->AesBlkCmpltRef = CallbackRef;
			break;

		case (XI2S_TX_HANDLER_AES_BLKSYNCERR):
			InstancePtr->AesBlkSyncErrHandler = FuncPtr;
			InstancePtr->AesBlkSyncErrRef = CallbackRef;
			break;

		case (XI2S_TX_HANDLER_AES_CHSTSUPD):
			InstancePtr->AesChStsUpdHandler = FuncPtr;
			InstancePtr->AesChStsUpdRef = CallbackRef;
			break;

		case (XI2S_TX_HANDLER_AUD_UNDRFLW):
			InstancePtr->AudUndrflwHandler = FuncPtr;
			InstancePtr->AudUndrflwRef = CallbackRef;
			break;

		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	return Status;
}
/** @} */
