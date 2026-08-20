/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_xfer.c
* @addtogroup amd_xspi Overview
* @{
*
* This file contains the data movement functions of the AXI XSPI driver.
* These functions push data into the transmit FIFO and pull data out of the
* receive FIFO on behalf of the higher level transaction functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "amd_xspi.h"
#include "amd_xspi_control.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**
 * Bound on how long either direction waits for the FIFO to become workable:
 * for a read, to report data; for a write, to report room.
 *
 * The first AMD_XSPI_FIFO_SPIN_COUNT polls run back to back with no delay.
 * Being slow costs no data in either direction, since the core stops the
 * clock when the RX FIFO fills and waits when the TX FIFO runs dry, but it
 * does spend the only budget that matters: chip select cannot stay low past
 * tCSM, 4 us on this part, and the core raises a transaction error when it
 * does. What the FIFO is waiting on arrives within a clock or two, so the
 * wait must not put a sleep in front of it.
 *
 * Once those polls are spent the transaction has outlived its tCSM budget
 * and the core has already failed it, so nothing is gained by continuing to
 * spin and the remaining polls sleep. That costs the working case nothing
 * and makes the timeout a wall clock bound of about
 * AMD_XSPI_FIFO_SLEEP_COUNT microseconds, rather than a spin count whose
 * real duration depends on the CPU and on AXI latency.
 */
#define AMD_XSPI_FIFO_SPIN_COUNT	256U
#define AMD_XSPI_FIFO_SLEEP_COUNT	1000U

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Encode a byte count of command/address bytes into the 2-bit instruction
* field used by AmdXspi_SendInstruction.
*
* Callers must screen the count first with AMD_XSPI_IS_VALID_CMD_ADDR_BYTES,
* which rejects both the counts this field cannot represent and the ones the
* core's mode does not permit. A count outside 1, 2, 4, 5 and 6 that reaches
* here unchecked widens to the 6 byte encoding. The check is left to callers
* because they know the mode and can fail the transfer, neither of which this
* function can do while returning a field value.
*
* @param[in] CmdAddrBytes is the number of command/address bytes (1-6).
*
* @return	The encoded 2-bit field value.
*
******************************************************************************/
u8 AmdXspi_EncodeCmdAddrBytes(u32 CmdAddrBytes)
{
	u8 Encoded;

	if (CmdAddrBytes <= 2U) {
		Encoded = AMD_XSPI_CMD_ADDR_1_OR_2_BYTES;
	} else if (CmdAddrBytes == 4U) {
		Encoded = AMD_XSPI_CMD_ADDR_4_BYTES;
	} else if (CmdAddrBytes == 5U) {
		Encoded = AMD_XSPI_CMD_ADDR_5_BYTES;
	} else {
		Encoded = AMD_XSPI_CMD_ADDR_6_BYTES;
	}

	return Encoded;
}

/*****************************************************************************/
/**
* Pack an instruction descriptor into the 32-bit instruction word.
*
* @param[in] InstrPtr is a pointer to the AmdXspi_Instruction structure.
*
* @return	The assembled instruction word.
*
******************************************************************************/
static u32 AmdXspi_PackInstruction(const AmdXspi_Instruction *InstrPtr)
{
	u32 Instruction = 0U;

	Instruction |= ((u32)InstrPtr->CmdAddrBytes << AMD_XSPI_INSTR_CMD_ADDR_BYTES_SHIFT) &
		       AMD_XSPI_INSTR_CMD_ADDR_BYTES_MASK;

	Instruction |= ((u32)InstrPtr->DataBytes << AMD_XSPI_INSTR_DATA_BYTES_SHIFT) &
		       AMD_XSPI_INSTR_DATA_BYTES_MASK;

	if (InstrPtr->IsRead != 0U) {
		Instruction |= AMD_XSPI_INSTR_RW_MASK;
	}

	Instruction |= ((u32)InstrPtr->DummyCycles << AMD_XSPI_INSTR_DUMMY_CYCLES_SHIFT) &
		       AMD_XSPI_INSTR_DUMMY_CYCLES_MASK;

	Instruction |= ((u32)InstrPtr->IoFormat << AMD_XSPI_INSTR_IO_FORMAT_SHIFT) &
		       AMD_XSPI_INSTR_IO_FORMAT_MASK;

	Instruction |= ((u32)InstrPtr->FrameFormat << AMD_XSPI_INSTR_FRAME_FORMAT_SHIFT) &
		       AMD_XSPI_INSTR_FRAME_FORMAT_MASK;

	return Instruction;
}

/*****************************************************************************/
/**
* Assemble an instruction and send it to the Transmit Register.
*
* In Lite mode, instruction is sent in 2 AXI-Lite transactions:
* - First write: {01, LSB 16-bit instruction}
* - Second write: {10, MSB instruction bits}
*
* The instance and the ready state are checked by the public entry points,
* and the descriptor is a caller local whose command/address field comes from
* AmdXspi_EncodeCmdAddrBytes and so is in range by construction. The data
* byte and dummy cycle counts are checked here because AmdXspi_LiteRead and
* AmdXspi_LiteWrite pass them straight through from the message without
* bounding them, making this the only place the polled path screens them.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] InstrPtr is a pointer to the instruction descriptor.
*
******************************************************************************/
void AmdXspi_SendInstruction(AmdXspi *InstancePtr,
			   const AmdXspi_Instruction *InstrPtr)
{
	u32 Instruction;
	u32 TxData;

	Xil_AssertVoid(InstrPtr->DataBytes <= AMD_XSPI_MAX_DATA_BYTES);
	Xil_AssertVoid(InstrPtr->DummyCycles <= AMD_XSPI_MAX_DUMMY_CYCLES);

	Instruction = AmdXspi_PackInstruction(InstrPtr);

	/*
	 * Per XSpi Design Doc section 2.6.1:
	 * The instruction is split into 2 AXI-Lite transactions:
	 * - 1st: {01, LSB 16-bit instruction} -> bits [15:0]
	 * - 2nd: {10, MSB instruction bits}   -> bits [29:16] (14 bits)
	 *
	 * The TX FIFO is 18 bits: [17:16] = flag, [15:0] = data
	 */

	/* Write LSB 16 bits with flag 01 */
	TxData = ((u32)AMD_XSPI_TX_FLAG_INSTR_LSB << 16) | (Instruction & 0xFFFFU);
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
		       AMD_XSPI_TRANSMIT_OFFSET, TxData);

	/* Write MSB bits [29:16] with flag 10 */
	TxData = ((u32)AMD_XSPI_TX_FLAG_INSTR_MSB << 16) | ((Instruction >> 16) & 0x3FFFU);
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
		       AMD_XSPI_TRANSMIT_OFFSET, TxData);
}

/*****************************************************************************/
/**
* Send command and address bytes to the Transmit Register.
*
* Every caller checks the instance, the ready state and the buffer before
* reaching here, so nothing is rechecked.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] CmdAddrBuf is the buffer containing command and address bytes.
* @param[in] NumBytes is the number of bytes to send.
*
******************************************************************************/
void AmdXspi_SendCmdAddr(AmdXspi *InstancePtr, u8 *CmdAddrBuf, u32 NumBytes)
{
	u32 Index;
	u32 TxData;

	/* Send bytes with flag 11 (data) */
	for (Index = 0U; Index < NumBytes; Index += 2U) {
		if ((Index + 1U) < NumBytes) {
			TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
				 ((u32)CmdAddrBuf[Index] << 8) |
				 (u32)CmdAddrBuf[Index + 1U];
		} else {
			TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
				 ((u32)CmdAddrBuf[Index] << 8);
		}

		AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			       AMD_XSPI_TRANSMIT_OFFSET, TxData);
	}
}

/*****************************************************************************/
/**
* Send data bytes to the Transmit Register.
*
* Every caller checks the instance and the ready state, and calls this only
* when the transmit buffer is non-NULL, so nothing is rechecked.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] DataBuf is the buffer containing data bytes to transmit.
* @param[in] NumBytes is the number of bytes to send.
*
* @return
*		- XST_SUCCESS if all bytes were written to the FIFO.
*		- XST_FAILURE if the FIFO stayed full, which means the core
*		  stopped draining it and no error bit was raised to say so.
*
******************************************************************************/
s32 AmdXspi_SendData(AmdXspi *InstancePtr, u8 *DataBuf, u32 NumBytes)
{
	u32 Index = 0U;
	u32 TxData;
	u32 Space;
	u32 Spins;
	u32 Timeout;

	while (Index < NumBytes) {
		/*
		 * Wait for room, then fill as much of it as the payload still
		 * needs. A write larger than the FIFO simply comes back here
		 * once the core has clocked some of it out, so the payload is
		 * not limited to what the FIFO can hold at one time.
		 *
		 * The core reports what the FIFO holds, so the room left is
		 * the configured depth less that.
		 */
		Spins = AMD_XSPI_FIFO_SPIN_COUNT;
		Timeout = AMD_XSPI_FIFO_SLEEP_COUNT;

		do {
			Space = InstancePtr->Config.FifoDepth -
				(AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
						  AMD_XSPI_TX_OCY_OFFSET) &
				 AMD_XSPI_FIFO_OCY_MASK);
			if (Space > 0U) {
				break;
			}
			if (Spins > 0U) {
				Spins--;
			} else if (Timeout > 0U) {
				usleep(1U);
				Timeout--;
			} else {
				return XST_FAILURE;
			}
		} while (TRUE);

		while ((Space > 0U) && (Index < NumBytes)) {
			if ((Index + 1U) < NumBytes) {
				TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
					 ((u32)DataBuf[Index] << 8) |
					 (u32)DataBuf[Index + 1U];
			} else {
				TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
					 ((u32)DataBuf[Index] << 8);
			}
			AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
				       AMD_XSPI_TRANSMIT_OFFSET, TxData);

			Index += 2U;
			Space--;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Receive data bytes from the Receive Register.
*
* The payload can be larger than the FIFO, since a Lite read may ask for up
* to AMD_XSPI_MAX_DATA_BYTES while C_FIFO_DEPTH goes as low as 16 entries, so
* the data is collected over as many passes as it takes. Each pass waits on
* RX_EMPTY for data to appear and then drains the number of entries the core
* reports, which keeps the wait to one register read per poll and still lets
* a whole burst be taken without re-testing the FIFO per entry.
*
* A failed transaction is not detected here and is left to run the wait out,
* so a read that the core has faulted costs the full timeout before it is
* reported.
*
* AmdXspi_LiteRead(), the only caller, checks the instance, the ready state
* and the receive buffer, so nothing is rechecked.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] DataBuf is the buffer to store received data.
* @param[in] NumBytes is the number of bytes to receive.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
******************************************************************************/
s32 AmdXspi_RecvData(AmdXspi *InstancePtr, u8 *DataBuf, u32 NumBytes)
{
	u32 Index = 0U;
	u32 RxData;
	u32 Available;
	u32 Spins;
	u32 Timeout;

	while (Index < NumBytes) {
		/*
		 * Wait on RX_EMPTY rather than on the occupancy count, so the
		 * wait costs a single register read per poll.
		 */
		Spins = AMD_XSPI_FIFO_SPIN_COUNT;
		Timeout = AMD_XSPI_FIFO_SLEEP_COUNT;

		while ((AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					 AMD_XSPI_ISR_OFFSET) &
			AMD_XSPI_INTR_RX_EMPTY_MASK) != 0U) {
			if (Spins > 0U) {
				Spins--;
				continue;
			}
			if (Timeout == 0U) {
				return XST_FAILURE;
			}
			usleep(1U);
			Timeout--;
		}

		/*
		 * At least one entry is present, so take everything the core
		 * reports in one go. How much that is depends on how the bus
		 * clock compares to the time an AXI read takes: a fast bus
		 * outruns the CPU and the whole burst is already resident,
		 * a slow one trickles in a few entries at a time. Draining by
		 * the reported count covers both without polling per entry.
		 * Only this loop empties the FIFO, so the count cannot fall
		 * between the two reads.
		 */
		Available = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					       AMD_XSPI_RX_OCY_OFFSET) &
			    AMD_XSPI_FIFO_OCY_MASK;

		while ((Available > 0U) && (Index < NumBytes)) {
			RxData = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					       AMD_XSPI_RECEIVE_OFFSET);

			DataBuf[Index] = (u8)((RxData >> 8) & 0xFFU);
			if ((Index + 1U) < NumBytes) {
				DataBuf[Index + 1U] = (u8)(RxData & 0xFFU);
			}

			Index += 2U;
			Available--;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Push as much of the message's transmit data into the TX FIFO as there is
* currently room for, without waiting for the core to make more.
*
* Unlike AmdXspi_SendData, this never blocks: it stops as soon as the FIFO is
* full or the message has been fully written. That makes it safe to call from
* interrupt context, and it lets a write larger than the FIFO be delivered
* across successive fill-level interrupts rather than having to be pushed in
* one blocking pass when the transaction is launched.
*
* Progress is accumulated in InstancePtr->MsgTxCnt.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] Msg is the message whose transmit buffer is being sent.
*
******************************************************************************/
void AmdXspi_FillTxFifo(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg)
{
	u32 Space;
	u32 TxData;

	Space = InstancePtr->Config.FifoDepth -
		(AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				  AMD_XSPI_TX_OCY_OFFSET) &
		 AMD_XSPI_FIFO_OCY_MASK);

	while ((Space > 0U) && (InstancePtr->MsgTxCnt < Msg->ByteCount)) {
		if ((InstancePtr->MsgTxCnt + 1U) < Msg->ByteCount) {
			TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
				 ((u32)Msg->TxBfrPtr[InstancePtr->MsgTxCnt] << 8) |
				 (u32)Msg->TxBfrPtr[InstancePtr->MsgTxCnt + 1U];
		} else {
			TxData = ((u32)AMD_XSPI_TX_FLAG_DATA << 16) |
				 ((u32)Msg->TxBfrPtr[InstancePtr->MsgTxCnt] << 8);
		}

		AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			       AMD_XSPI_TRANSMIT_OFFSET, TxData);

		InstancePtr->MsgTxCnt += 2U;
		Space--;
	}
}

/*****************************************************************************/
/**
* Move receive data from the RX FIFO into the message buffer.
*
* Progress is accumulated in InstancePtr->MsgRxCnt, so a read larger than the
* FIFO can be assembled across successive fill-level interrupts rather than
* having to be resident all at once when the transaction completes.
*
* WaitForAll says what an empty FIFO means. Part way through a transaction it
* means the core has not produced the next entry yet, and returning is right:
* more fill-level interrupts are coming, and blocking in the handler for data
* the bus has not carried would spend the tCSM budget of the transaction that
* is still running. At completion there are no further interrupts to wait for,
* so the same reading has to be retried: the core signals the transaction
* complete as the last entries reach the FIFO, and the occupancy count crosses
* into this clock domain a little behind them, so a drain that starts the
* moment the interrupt arrives can find the FIFO reporting empty with data
* still landing. Giving up on that first reading loses the tail of the burst
* and is reported to the caller as a receive overrun.
*
* The retry only ever spins. It runs in interrupt context, and by the time it
* is reached the bus has already delivered the data, so the wait is the
* handful of cycles the count takes to catch up rather than anything bounded
* by the bus clock.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] Msg is the message whose receive buffer is being filled.
* @param[in] WaitForAll is 0 to take only what is present and return, or 1 to
*		wait out a late occupancy count until the message is satisfied.
*
******************************************************************************/
void AmdXspi_DrainRxFifo(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg,
			 u8 WaitForAll)
{
	u32 Available;
	u32 RxData;
	u32 Spins = AMD_XSPI_FIFO_SPIN_COUNT;

	Available = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					       AMD_XSPI_RX_OCY_OFFSET) &
			    AMD_XSPI_FIFO_OCY_MASK;

	while (InstancePtr->MsgRxCnt < Msg->ByteCount) {
		if (Available == 0U) {
			Available = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					       AMD_XSPI_RX_OCY_OFFSET) &
			    AMD_XSPI_FIFO_OCY_MASK;
		}

		if (Available == 0U) {
			if ((WaitForAll == 0U) || (Spins == 0U)) {
				break;
			}
			Spins--;
			continue;
		}

		RxData = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				       AMD_XSPI_RECEIVE_OFFSET);

		Msg->RxBfrPtr[InstancePtr->MsgRxCnt] =
			(u8)((RxData >> 8) & 0xFFU);
		InstancePtr->MsgRxCnt++;

		if (InstancePtr->MsgRxCnt < Msg->ByteCount) {
			Msg->RxBfrPtr[InstancePtr->MsgRxCnt] =
				(u8)(RxData & 0xFFU);
			InstancePtr->MsgRxCnt++;
		}

		Available--;
	}
}

/** @} */
