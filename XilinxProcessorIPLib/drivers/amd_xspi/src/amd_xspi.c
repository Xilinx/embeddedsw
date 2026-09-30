/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi.c
* @brief Implements the application-facing APIs of the AXI XSPI driver.
*
* This file contains the implementation of the application facing APIs of the
* AXI XSPI driver, including the interrupt handler.
* This driver supports AXI-Lite mode only. Supplying it a design built in
* any other mode is a configuration error the driver does not check for.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* 1.0   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup amd_xspi Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "amd_xspi.h"
#include "amd_xspi_control.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

static void StubHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Initializes the AXI XSPI driver instance.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     ConfigPtr Pointer to the configuration structure.
* @param[in]     EffectiveAddr Base address of the device.
*
* @return        - XST_SUCCESS if initialization was successful.
*                - XST_DEVICE_IS_STARTED if the instance is already
*                  initialized.
*
******************************************************************************/
s32 AmdXspi_CfgInitialize(AmdXspi *InstancePtr, AmdXspi_Config *ConfigPtr,
			   UINTPTR EffectiveAddr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.Name = ConfigPtr->Name;
	InstancePtr->Config.IntrId = ConfigPtr->IntrId;
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
	InstancePtr->Config.SpiMode = ConfigPtr->SpiMode;
	InstancePtr->Config.FifoDepth = ConfigPtr->FifoDepth;
	InstancePtr->Config.NumSlaves = ConfigPtr->NumSlaves;
	InstancePtr->Config.SpiClkFreq = ConfigPtr->SpiClkFreq;
	InstancePtr->Config.SckRatio = ConfigPtr->SckRatio;

	AmdXspi_Reset(InstancePtr);

	InstancePtr->Msg = NULL;
	InstancePtr->NumMsg = 0U;
	InstancePtr->MsgCnt = 0U;
	InstancePtr->IsBusy = FALSE;
	InstancePtr->StatusHandler = StubHandler;
	InstancePtr->StatusRef = NULL;

	/*
	 * The reset left every control bit at its default, so the core is
	 * enabled here rather than inside the reset, and stays enabled for as
	 * long as the instance is ready.
	 */
	AmdXspi_Enable(InstancePtr);

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Aborts the current transfer and returns the core to an idle state.
*
* The core has no abort control, so a software reset is the only way to tear
* down a transaction that is already running. It is also what the core offers
* for recovering from a transaction error. The reset also disables every
* interrupt and discards every latched event, so nothing the abandoned transfer
* raised is left behind.
*
* The core is enabled again on return, and the clock mode and selected target
* are put back. So the caller can start a new transfer without any further
* setup.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
*
******************************************************************************/
void AmdXspi_Abort(AmdXspi *InstancePtr)
{
	u32 ClkMode;
	u32 TargetSelect;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * A reset returns every register to its default, which would drop the
	 * clock mode and leave no target selected, so both are read back here
	 * and written again once the reset is done.
	 */
	ClkMode = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				   AMD_XSPI_CONTROL_OFFSET) &
		  (AMD_XSPI_CR_CPOL_MASK | AMD_XSPI_CR_CPHA_MASK);
	TargetSelect = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					AMD_XSPI_TARGET_SELECT_OFFSET);

	AmdXspi_Reset(InstancePtr);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_TARGET_SELECT_OFFSET, TargetSelect);
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET, ClkMode);

	AmdXspi_Enable(InstancePtr);

	/*
	 * Tear down any active message queue. The caller asked for the
	 * transfer to end, so the status handler is not invoked, but the
	 * queue state has to go or a late interrupt would run the handler
	 * against a transfer that no longer exists, and the driver would
	 * stay busy and reject every later transfer.
	 */
	InstancePtr->Msg = NULL;
	InstancePtr->IsBusy = FALSE;
}

/*****************************************************************************/
/**
*
* @brief Performs one or more transfers in polled (blocking) mode.
*
* Each message describes a Lite-mode transaction (command/address plus one
* data phase). Messages are dispatched to the existing polled engine
* (AmdXspi_LiteRead / AmdXspi_LiteWrite) in order and the function returns when
* all messages complete or on the first failure.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     Msg Array of messages to transfer.
* @param[in]     NumMsg Number of messages in the array.
*
* @return        - XST_SUCCESS if all messages transferred.
*                - XST_DEVICE_BUSY if a transfer is already in progress.
*                - XST_FAILURE if a transfer failed.
*
******************************************************************************/
s32 AmdXspi_PolledTransfer(AmdXspi *InstancePtr, AmdXspi_Msg *Msg, u32 NumMsg)
{
	s32 Status = XST_SUCCESS;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(NumMsg > 0U);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->IsBusy == TRUE) {
		return XST_DEVICE_BUSY;
	}

	InstancePtr->IsBusy = TRUE;

	/*
	 * The transactions below watch AMD_XSPI_ISR, which reports each
	 * condition live and independently of AMD_XSPI_IER, so a polled
	 * transfer needs no interrupt enabled and leaves the enables alone.
	 * AmdXspi_PrepareTransfer() discards every latched event, so nothing
	 * a previous transfer left behind is read as belonging to this one.
	 */
	AmdXspi_PrepareTransfer(InstancePtr);

	for (Index = 0U; Index < NumMsg; Index++) {
		const AmdXspi_Msg *CurMsg = &Msg[Index];

		if ((CurMsg->Flags & AMD_XSPI_MSG_FLAG_RX) != 0U) {
			Status = AmdXspi_LiteRead(InstancePtr, CurMsg);
		} else {
			Status = AmdXspi_LiteWrite(InstancePtr, CurMsg);
		}

		if (Status != XST_SUCCESS) {
			break;
		}
	}

	InstancePtr->IsBusy = FALSE;

	return Status;
}

/*****************************************************************************/
/**
*
* @brief Initiates an interrupt-driven transfer of one or more messages.
*
* This function is non-blocking: it launches the first message and returns
* immediately. The interrupt handler (AmdXspi_InterruptHandler) drains receive
* data and advances through the message queue. When all messages complete, it
* invokes the status handler registered with AmdXspi_SetStatusHandler with
* XST_SPI_TRANSFER_DONE (or a non-zero error event on failure).
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     Msg Array of messages to transfer.
* @param[in]     NumMsg Number of messages in the array.
*
* @return        - XST_SUCCESS if the transfer was started.
*                - XST_DEVICE_BUSY if a transfer is already in progress.
*
******************************************************************************/
s32 AmdXspi_InterruptTransfer(AmdXspi *InstancePtr, AmdXspi_Msg *Msg, u32 NumMsg)
{
	u32 IntrMask;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(NumMsg > 0U);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->IsBusy == TRUE) {
		return XST_DEVICE_BUSY;
	}

	InstancePtr->IsBusy = TRUE;
	InstancePtr->Msg = Msg;
	InstancePtr->NumMsg = NumMsg;
	InstancePtr->MsgCnt = 0U;
	InstancePtr->MsgRxCnt = 0U;
	InstancePtr->MsgTxCnt = 0U;

	AmdXspi_PrepareTransfer(InstancePtr);

	IntrMask = AMD_XSPI_INTR_TXN_COMPLETE_MASK | AMD_XSPI_INTR_TXN_ERROR_MASK;

	/*
	 * Any read in the queue also needs the RX fill-level events, so the
	 * handler can drain while the transaction is still running instead of
	 * relying on the whole payload being resident at completion.
	 */
	for (Index = 0U; Index < NumMsg; Index++) {
		if (((Msg[Index].Flags & AMD_XSPI_MSG_FLAG_RX) != 0U) &&
		    (Msg[Index].RxBfrPtr != NULL) &&
		    (Msg[Index].ByteCount > 0U)) {
			IntrMask |= AMD_XSPI_INTR_RX_THRESHOLD_MASK;
			break;
		}
	}

	/*
	 * The TX fill-level events are not enabled here. They are true of an
	 * idle FIFO, so they are switched on by AmdXspi_StartMessage() only
	 * when a message leaves data still owed, and off again as soon as the
	 * last of it has been written.
	 */
	AmdXspi_IntrEnable(InstancePtr, IntrMask);

	AmdXspi_StartMessage(InstancePtr, &Msg[0]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Provides a stub handler for uninitialized callbacks.
*
* @param[in]   CallBackRef Callback reference (unused).
* @param[in]   StatusEvent Status event (unused).
* @param[in]   ByteCount Byte count (unused).
*
******************************************************************************/
static void StubHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount)
{
	(void)CallBackRef;
	(void)StatusEvent;
	(void)ByteCount;

	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* @brief Serves as the main interrupt handler for the AXI XSPI controller.
*
* This function reads the interrupt status, clears the pending interrupts,
* and advances the active message queue, reporting the outcome through the
* handler registered with AmdXspi_SetStatusHandler().
*
* Receive data is drained both on RX fill-level events and at transaction
* completion. So a read is not limited to what the RX FIFO holds at the end of
* the transaction.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance (cast to void*).
*
******************************************************************************/
void AmdXspi_InterruptHandler(void *InstancePtr)
{
	AmdXspi *AmdXspiPtr = (AmdXspi *)InstancePtr;
	AmdXspi_Msg *CurMsg;
	u32 PendingIntr;
	u8 IsRead;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AmdXspiPtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * INT_EVT captures an event only if its IER bit was set when the
	 * condition fired, so what it reports is already masked by the enables
	 * and needs no further filtering. Acting on the register as it stands
	 * also means an event latched before its source was disabled still
	 * gets acknowledged rather than being left set for good.
	 */
	PendingIntr = AmdXspi_ReadReg(AmdXspiPtr->Config.BaseAddress,
				       AMD_XSPI_INT_EVT_OFFSET);

	if (PendingIntr == 0U) {
		return;
	}

	AmdXspi_WriteReg(AmdXspiPtr->Config.BaseAddress,
			  AMD_XSPI_INT_EVT_OFFSET,
			  PendingIntr & AMD_XSPI_INTR_ALL_MASK);

	/*
	 * Interrupts are enabled only for the duration of a message queue, so
	 * an interrupt arriving without one is unsolicited. It has already
	 * been acknowledged above, which is all it needs.
	 */
	if (AmdXspiPtr->Msg == NULL) {
		return;
	}

	CurMsg = &AmdXspiPtr->Msg[AmdXspiPtr->MsgCnt];

	/*
	 * Direction alone, as in AmdXspi_StartMessage(). The buffer is
	 * checked at each point it would be dereferenced, and the byte count
	 * needs no check at all: both the drain and the fill run against
	 * Msg->ByteCount, so a message with no data phase leaves them doing
	 * nothing.
	 */
	IsRead = ((CurMsg->Flags & AMD_XSPI_MSG_FLAG_RX) != 0U) ? 1U : 0U;

	if ((PendingIntr & AMD_XSPI_INTR_ERROR_MASK) != 0U) {
		AmdXspi_FinishMessageQueue(AmdXspiPtr,
					 PendingIntr &
					 AMD_XSPI_INTR_ERROR_MASK, 0U);
		return;
	}

	/*
	 * Drain on fill-level events so the transaction is not limited to a
	 * single FIFO load. Note this does not extend how long a transaction
	 * may run: HyperRAM still requires CS# to rise before tCSM, so the
	 * payload of any one message must stay within that budget regardless
	 * of FIFO servicing.
	 */
	if ((IsRead == 1U) && (CurMsg->RxBfrPtr != NULL) &&
	    ((PendingIntr & AMD_XSPI_INTR_RX_THRESHOLD_MASK) != 0U)) {
		AmdXspi_DrainRxFifo(AmdXspiPtr, CurMsg, 0U);
	}

	/*
	 * Top the transmit FIFO up as the core drains it, so a write larger
	 * than the FIFO is delivered without anyone having to block. Once the
	 * last byte is in, stop asking: the empty-side events stay true of an
	 * idle FIFO and would otherwise keep calling back with nothing to do.
	 */
	if ((IsRead == 0U) && (CurMsg->TxBfrPtr != NULL) &&
	    ((PendingIntr & AMD_XSPI_INTR_TX_THRESHOLD_MASK) != 0U)) {
		AmdXspi_FillTxFifo(AmdXspiPtr, CurMsg);

		if (AmdXspiPtr->MsgTxCnt >= CurMsg->ByteCount) {
			AmdXspi_IntrDisable(AmdXspiPtr,
					  AMD_XSPI_INTR_TX_THRESHOLD_MASK);
		}
	}

	if ((PendingIntr & AMD_XSPI_INTR_TXN_COMPLETE_MASK) != 0U) {
		if ((IsRead == 1U) && (CurMsg->RxBfrPtr != NULL)) {
			AmdXspi_DrainRxFifo(AmdXspiPtr, CurMsg, 1U);

			/*
			 * A short count means receive data was lost before it
			 * could be drained; report it rather than handing back
			 * a partly filled buffer.
			 */
			if (AmdXspiPtr->MsgRxCnt < CurMsg->ByteCount) {
				AmdXspi_FinishMessageQueue(AmdXspiPtr,
					(u32)XST_SPI_RECEIVE_OVERRUN,
					AmdXspiPtr->MsgRxCnt);
				return;
			}
		}

		AmdXspiPtr->MsgCnt++;

		if (AmdXspiPtr->MsgCnt < AmdXspiPtr->NumMsg) {
			AmdXspi_StartMessage(AmdXspiPtr,
				&AmdXspiPtr->Msg[AmdXspiPtr->MsgCnt]);
		} else {
			AmdXspi_FinishMessageQueue(AmdXspiPtr,
					(u32)XST_SPI_TRANSFER_DONE,
					CurMsg->ByteCount);
		}
	}
}

/*****************************************************************************/
/**
*
* @brief Sets the status callback of the message-based
* AmdXspi_InterruptTransfer().
*
* The handler is invoked from interrupt context with XST_SPI_TRANSFER_DONE when
* the whole message queue completes. It is invoked with a non-zero error event
* mask if the transfer fails.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     CallBackRef Reference passed back to the handler.
* @param[in]     FuncPtr Status callback function.
*
******************************************************************************/
void AmdXspi_SetStatusHandler(AmdXspi *InstancePtr, void *CallBackRef,
			    AmdXspi_StatusHandler FuncPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(FuncPtr != NULL);

	InstancePtr->StatusHandler = FuncPtr;
	InstancePtr->StatusRef = CallBackRef;
}

/** @} */
