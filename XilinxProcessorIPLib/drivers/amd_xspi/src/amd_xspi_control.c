/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_control.c
* @brief Implements the low-level transaction functions of the AXI XSPI driver.
*
* This file contains the low-level transaction functions of the AXI XSPI driver.
* These functions assemble a Lite mode instruction and sequence the data
* movement functions to carry out a complete read or write. They also maintain
* the message queue used by the interrupt driven API.
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

/**
 * Poll bound for a transaction to report complete. Sized well past the longest
 * Lite mode transfer the core runs so that it only trips on a wedged
 * transaction, never on a slow one.
 */
#define AMD_XSPI_TXN_POLL_COUNT		1000000U

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Polls until the core reports the transaction finished.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
* @return      - XST_SUCCESS once TXN_COMPLETE is seen.
*              - XST_FAILURE if the core raises TXN_ERROR or the poll expires.
*
******************************************************************************/
static s32 AmdXspi_WaitTxnComplete(AmdXspi *InstancePtr)
{
	u32 StatusReg;
	u32 Timeout = AMD_XSPI_TXN_POLL_COUNT;

	do {
		StatusReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					     AMD_XSPI_ISR_OFFSET);
		if ((StatusReg & AMD_XSPI_INTR_TXN_ERROR_MASK) != 0U) {
			return XST_FAILURE;
		}
		if (Timeout == 0U) {
			return XST_FAILURE;
		}
		Timeout--;
	} while ((StatusReg & AMD_XSPI_INTR_TXN_COMPLETE_MASK) == 0U);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Performs a Lite mode read transaction.
*
* This function sends instruction, command/address, and receives data
* following the Lite mode protocol for xSPI Profile 2.0 (HyperBus).
*
* The receive buffer is taken from the message rather than passed separately,
* so a caller cannot hand a read the transmit buffer by mistake.
*
* The instance, the message and the ready state are checked by
* AmdXspi_PolledTransfer(), the only caller. So only the buffers this function
* goes on to dereference are checked here.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
* @param[in]   Msg Describes the transaction: command/address bytes, receive
*              buffer, byte count, dummy cycles and frame/IO format.
*
* @return      - XST_SUCCESS if successful.
*              - XST_FAILURE if failed.
*
******************************************************************************/
s32 AmdXspi_LiteRead(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg)
{
	s32 Status;
	AmdXspi_Instruction Instr;
	u32 StatusReg;

	Xil_AssertNonvoid(Msg->CmdAddrBufPtr != NULL);
	Xil_AssertNonvoid(Msg->RxBfrPtr != NULL);

	if (!AMD_XSPI_IS_VALID_CMD_ADDR_BYTES(InstancePtr->Config.SpiMode,
					       Msg->CmdAddrBytes)) {
		return XST_FAILURE;
	}

	Instr.CmdAddrBytes = AmdXspi_EncodeCmdAddrBytes(Msg->CmdAddrBytes);
	Instr.DataBytes = (u16)Msg->ByteCount;
	Instr.IsRead = 1U;
	Instr.DummyCycles = Msg->DummyCycles;
	Instr.IoFormat = Msg->IoFormat;
	Instr.FrameFormat = Msg->FrameFormat;

	/*
	 * The instruction and the command/address bytes have to reach the FIFO
	 * back to back, or the controller starts the transaction on a partial
	 * instruction.
	 */
	AmdXspi_SendInstruction(InstancePtr, &Instr);

	AmdXspi_SendCmdAddr(InstancePtr, Msg->CmdAddrBufPtr, Msg->CmdAddrBytes);

	/* Check for a transaction error after issuing the transaction */
	StatusReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				     AMD_XSPI_ISR_OFFSET);
	if ((StatusReg & AMD_XSPI_INTR_TXN_ERROR_MASK) != 0U) {
		return XST_FAILURE;
	}

	/*
	 * Drain as the data streams in rather than waiting for the core to
	 * finish first. A Lite read may ask for up to 512 bytes while
	 * C_FIFO_DEPTH goes as low as 16 entries, so the payload frequently
	 * cannot be resident all at once and the FIFO has to be emptied while
	 * the transaction is still running. Draining first costs nothing when
	 * the payload would have fitted anyway.
	 */
	if (Msg->ByteCount > 0U) {
		Status = AmdXspi_RecvData(InstancePtr, Msg->RxBfrPtr,
					Msg->ByteCount);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return AmdXspi_WaitTxnComplete(InstancePtr);
}

/*****************************************************************************/
/**
*
* @brief Performs a Lite mode write transaction.
*
* This function sends instruction, command/address, and data bytes
* following the Lite mode protocol for xSPI Profile 2.0 (HyperBus).
*
* The transmit buffer is taken from the message rather than passed separately,
* so a caller cannot hand a write the receive buffer by mistake.
*
* The instance, the message and the ready state are checked by
* AmdXspi_PolledTransfer(), the only caller. So only the buffer this function
* goes on to dereference is checked here.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
* @param[in]   Msg Describes the transaction: command/address bytes, transmit
*              buffer, byte count, dummy cycles and frame/IO format.
*
* @return      - XST_SUCCESS if successful.
*              - XST_FAILURE if failed.
*
******************************************************************************/
s32 AmdXspi_LiteWrite(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg)
{
	s32 Status;
	AmdXspi_Instruction Instr;

	Xil_AssertNonvoid(Msg->CmdAddrBufPtr != NULL);

	if (!AMD_XSPI_IS_VALID_CMD_ADDR_BYTES(InstancePtr->Config.SpiMode,
					       Msg->CmdAddrBytes)) {
		return XST_FAILURE;
	}

	Instr.CmdAddrBytes = AmdXspi_EncodeCmdAddrBytes(Msg->CmdAddrBytes);
	Instr.DataBytes = (u16)Msg->ByteCount;
	Instr.IsRead = 0U;
	Instr.DummyCycles = Msg->DummyCycles;
	Instr.IoFormat = Msg->IoFormat;
	Instr.FrameFormat = Msg->FrameFormat;

	/* Send instruction */
	AmdXspi_SendInstruction(InstancePtr, &Instr);

	/* Send command and address bytes */
	AmdXspi_SendCmdAddr(InstancePtr, Msg->CmdAddrBufPtr, Msg->CmdAddrBytes);

	/* Send data bytes */
	if ((Msg->ByteCount > 0U) && (Msg->TxBfrPtr != NULL)) {
		Status = AmdXspi_SendData(InstancePtr, Msg->TxBfrPtr,
					Msg->ByteCount);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return AmdXspi_WaitTxnComplete(InstancePtr);
}

/*****************************************************************************/
/**
*
* @brief Starts a single message on the bus.
*
* The message comprises the instruction and command/address, plus transmit data
* for a write.
*
* This reuses the existing low-level building blocks
* (AmdXspi_SendInstruction/AmdXspi_SendCmdAddr/AmdXspi_FillTxFifo). It is used
* both by AmdXspi_InterruptTransfer (to launch the first message) and by
* AmdXspi_InterruptHandler (to launch each subsequent message on completion).
*
* Nothing here blocks, which is what makes the second of those callers safe.
*
* For a read, the interrupt handler drains receive data as the RX FIFO fills and
* again at completion. This function only issues the transaction.
*
* The instance, the message and the ready state are checked by both callers. So
* what is checked here is the message content. Neither of them inspects it, and
* this function is the first to act on it.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     Msg Message to start.
*
******************************************************************************/
void AmdXspi_StartMessage(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg)
{
	AmdXspi_Instruction Instr;
	u8 IsRead;

	Xil_AssertVoid(Msg->CmdAddrBufPtr != NULL);
	Xil_AssertVoid(AMD_XSPI_IS_VALID_CMD_ADDR_BYTES(
				InstancePtr->Config.SpiMode,
				Msg->CmdAddrBytes));
	Xil_AssertVoid(Msg->ByteCount <= AMD_XSPI_MAX_DATA_BYTES);
	Xil_AssertVoid(Msg->DummyCycles <= AMD_XSPI_MAX_DUMMY_CYCLES);

	InstancePtr->MsgRxCnt = 0U;
	InstancePtr->MsgTxCnt = 0U;

	IsRead = ((Msg->Flags & AMD_XSPI_MSG_FLAG_RX) != 0U) ? 1U : 0U;

	Instr.CmdAddrBytes = AmdXspi_EncodeCmdAddrBytes(Msg->CmdAddrBytes);
	Instr.DataBytes = (u16)Msg->ByteCount;
	Instr.IsRead = IsRead;
	Instr.DummyCycles = Msg->DummyCycles;
	Instr.IoFormat = Msg->IoFormat;
	Instr.FrameFormat = Msg->FrameFormat;

	AmdXspi_SendInstruction(InstancePtr, &Instr);
	AmdXspi_SendCmdAddr(InstancePtr, Msg->CmdAddrBufPtr, Msg->CmdAddrBytes);

	/*
	 * Push only what the FIFO has room for. A payload larger than the
	 * FIFO is delivered across the TX fill-level interrupts that are
	 * enabled below, so this function never blocks; it has to be able to
	 * run from the interrupt handler, which launches every message after
	 * the first.
	 *
	 * A stuck TX FIFO cannot be reported from here because this function
	 * only launches the message; the interrupt handler owns the outcome
	 * and will report the resulting transaction error or let the caller's
	 * own timeout expire.
	 */
	if ((IsRead == 0U) && (Msg->TxBfrPtr != NULL) &&
	    (Msg->ByteCount > 0U)) {
		AmdXspi_FillTxFifo(InstancePtr, Msg);

		/*
		 * Ask to be told about room only while data is still owed.
		 * The empty-side events are true of an idle FIFO, so leaving
		 * them enabled past the last byte would invite interrupts
		 * with nothing left to do.
		 */
		if (InstancePtr->MsgTxCnt < Msg->ByteCount) {
			AmdXspi_IntrEnable(InstancePtr,
					 AMD_XSPI_INTR_TX_THRESHOLD_MASK);
		}
	}
}

/*****************************************************************************/
/**
*
* @brief Tears down the active message queue and reports the outcome to the
* caller.
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     StatusEvent Event to pass to the status handler.
* @param[in]     ByteCount Byte count to pass to the status handler.
*
******************************************************************************/
void AmdXspi_FinishMessageQueue(AmdXspi *InstancePtr, u32 StatusEvent,
				     u32 ByteCount)
{
	/*
	 * Disable every interrupt the queue enabled, not just the RX threshold.
	 * Leaving transaction complete and error enabled would let a later
	 * polled transfer raise the interrupt line for a transfer the ISR has
	 * no queue for.
	 */
	AmdXspi_IntrDisable(InstancePtr,
			  AMD_XSPI_INTR_TXN_COMPLETE_MASK |
			  AMD_XSPI_INTR_TXN_ERROR_MASK |
			  AMD_XSPI_INTR_RX_THRESHOLD_MASK |
			  AMD_XSPI_INTR_TX_THRESHOLD_MASK);

	InstancePtr->IsBusy = FALSE;
	InstancePtr->Msg = NULL;

	InstancePtr->StatusHandler(InstancePtr->StatusRef, StatusEvent,
				   ByteCount);
}

/** @} */
