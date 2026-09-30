/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_hw.c
* @brief Implements the register-level functions of the AXI XSPI driver.
*
* This file contains the register level functions of the AXI XSPI driver.
* These functions operate directly on the controller registers to prepare
* the FIFOs for a transfer and to manage the interrupt registers.
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
#include "sleep.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Brings the core to a known state before a transfer is launched.
*
* Any event latched by a previous transfer would otherwise be mistaken for an
* event belonging to this one. Residue left in either FIFO would be prepended to
* the payload. Every event is discarded rather than only completion and error. A
* stale threshold event is just as capable of steering the interrupt handler as
* a stale completion.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
******************************************************************************/
void AmdXspi_PrepareTransfer(AmdXspi *InstancePtr)
{
	u32 ControlReg;

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_INT_EVT_OFFSET, AMD_XSPI_INTR_ALL_MASK);

	ControlReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_CONTROL_OFFSET);

	/*
	 * Neither FIFO reset bit is self clearing, so both are asserted
	 * together and then released. The assert and the release are separate
	 * AXI transactions, which holds the bits for many S_AXI_ACLK cycles on
	 * its own, so the core needs no added delay to flush the FIFOs.
	 */
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET,
			  ControlReg | AMD_XSPI_CR_TXFIFO_RESET_MASK |
			  AMD_XSPI_CR_RXFIFO_RESET_MASK);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET,
			  ControlReg & ~(AMD_XSPI_CR_TXFIFO_RESET_MASK |
					 AMD_XSPI_CR_RXFIFO_RESET_MASK));
}

/*****************************************************************************/
/**
*
* @brief Enables specific interrupts.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
* @param[in]   IntrMask Mask of interrupts to enable.
*
******************************************************************************/
void AmdXspi_IntrEnable(AmdXspi *InstancePtr, u32 IntrMask)
{
	u32 IntrEnable;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrEnable = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_IER_OFFSET);
	IntrEnable |= (IntrMask & AMD_XSPI_INTR_ALL_MASK);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_IER_OFFSET, IntrEnable);
}

/*****************************************************************************/
/**
*
* @brief Disables specific interrupts.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
* @param[in]   IntrMask Mask of interrupts to disable.
*
******************************************************************************/
void AmdXspi_IntrDisable(AmdXspi *InstancePtr, u32 IntrMask)
{
	u32 IntrEnable;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	IntrEnable = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_IER_OFFSET);
	IntrEnable &= ~(IntrMask & AMD_XSPI_INTR_ALL_MASK);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_IER_OFFSET, IntrEnable);
}

/*****************************************************************************/
/**
*
* @brief Resets the AXI XSPI controller.
*
* The core is left disabled, since writing SOFT_RESET returns every control
* bit to its default. Enabling it is the caller's to do, which for the driver
* means AmdXspi_CfgInitialize(). Anything the application configured through
* the option functions is returned to its default and has to be set again.
*
* This can reach past the controller. The core passes the soft reset through to
* its optional resetn output. So on a design built with that output wired, the
* attached memory is hardware reset too.
*
* For a HyperRAM, that returns its configuration registers, including the
* initial latency, to their defaults. It also costs the contents of the array,
* which the datasheet says to treat as lost. Anything recovering an error
* through AmdXspi_Abort() inherits that.
*
* Both callers check the instance beforehand. The ready state deliberately is
* not checked, since AmdXspi_CfgInitialize() resets the core before marking
* the instance ready.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
******************************************************************************/
void AmdXspi_Reset(AmdXspi *InstancePtr)
{
	/*
	 * SOFT_RESET is a read/write bit and is not self clearing, so it has
	 * to be set and then cleared to reset the FIFOs and the internal
	 * registers. Writing the mask on its own also returns the remaining
	 * control bits to their default state.
	 */
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET,
			  AMD_XSPI_CR_SOFT_RESET_MASK);

	/*
	 * Hold the reset asserted rather than releasing it on the next AXI
	 * transaction. The reset has to reach the SPI clock domain, and
	 * ext_spi_clk can be far slower than S_AXI_ACLK, so the handful of
	 * AXI cycles between two back to back writes is not a useful measure
	 * of how long the core has actually seen it.
	 */
	usleep(10U);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET, 0U);

	/*
	 * Let the attached device come out of reset before anything selects
	 * it. The core passes the soft reset through to its optional resetn
	 * output, so on a design that wires it the memory has just been
	 * hardware reset and needs a moment before chip select may go low.
	 * A HyperRAM asks 200 ns from RESET# high, 400 ns from RESET# low;
	 * a microsecond covers that for any part and is too small to be
	 * worth measuring. This is not the power-up wait, which is far
	 * longer, applies only to a cold start, and belongs to whoever knows
	 * what is attached.
	 */
	usleep(1U);

	/* Disable all interrupts */
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_IER_OFFSET, 0U);

	/* Clear all pending interrupts */
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_INT_EVT_OFFSET, AMD_XSPI_INTR_ALL_MASK);
}

/*****************************************************************************/
/**
*
* @brief Performs a polled SPI transfer (AXI-Lite mode).
*
* @param[in,out] InstancePtr Pointer to the AmdXspi instance.
* @param[in]     SendBufPtr Buffer to transmit (can be NULL for read-only).
* @param[out]    RecvBufPtr Buffer for received data (can be NULL for
*                write-only).
* @param[in]     ByteCount Number of bytes to transfer.
*
* @return        - XST_SUCCESS if transfer was successful.
*                - XST_FAILURE if the core reported an error, stalled, or
*                  did not signal completion.
*                - XST_DEVICE_BUSY if a transfer is in progress.
*
******************************************************************************/
s32 AmdXspi_Transfer(AmdXspi *InstancePtr, u8 *SendBufPtr, u8 *RecvBufPtr,
		      u32 ByteCount)
{
	u32 StatusReg;
	u32 TxData;
	u32 RxData;
	u32 TxCount = 0U;
	u32 RxCount = 0U;
	u32 StallCount = 10000U;
	u8 Progress;
	u32 Timeout;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ByteCount > 0U);

	if (InstancePtr->IsBusy == TRUE) {
		return XST_DEVICE_BUSY;
	}

	InstancePtr->IsBusy = TRUE;

	/* Start from a clean slate: discard anything a previous run left. */
	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_INT_EVT_OFFSET, AMD_XSPI_INTR_ALL_MASK);

	while ((TxCount < ByteCount) || (RxCount < ByteCount)) {
		Progress = 0U;

		/*
		 * Error, completion and FIFO levels all come from the live
		 * status register, so this path enables no interrupt and
		 * leaves AMD_XSPI_IER as the application set it.
		 */
		StatusReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
					     AMD_XSPI_ISR_OFFSET);

		if ((StatusReg & AMD_XSPI_INTR_TXN_ERROR_MASK) != 0U) {
			InstancePtr->IsBusy = FALSE;
			return XST_FAILURE;
		}

		if ((TxCount < ByteCount) &&
		    ((StatusReg & AMD_XSPI_INTR_TX_FULL_MASK) == 0U)) {
			if (SendBufPtr != NULL) {
				TxData = (u32)SendBufPtr[TxCount];
			} else {
				TxData = 0xFFU;
			}

			AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
					  AMD_XSPI_TRANSMIT_OFFSET, TxData);
			TxCount++;
			Progress = 1U;
		}

		if ((RxCount < ByteCount) &&
		    ((StatusReg & AMD_XSPI_INTR_RX_EMPTY_MASK) == 0U)) {
			RxData = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
						  AMD_XSPI_RECEIVE_OFFSET);

			if (RecvBufPtr != NULL) {
				RecvBufPtr[RxCount] = (u8)(RxData & 0xFFU);
			}
			RxCount++;
			Progress = 1U;
		}

		/*
		 * Bound the stall rather than the whole transfer, so a large
		 * ByteCount is not penalised. A core that stops draining TX
		 * and never fills RX raises no error bit, so without this the
		 * loop has no way out.
		 */
		if (Progress == 1U) {
			StallCount = 10000U;
		} else {
			if (StallCount == 0U) {
				InstancePtr->IsBusy = FALSE;
				return XST_FAILURE;
			}
			usleep(1U);
			StallCount--;
		}
	}

	Timeout = 10000U;
	while (Timeout > 0U) {
		if ((AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_ISR_OFFSET) &
		     AMD_XSPI_INTR_TXN_COMPLETE_MASK) != 0U) {
			break;
		}
		usleep(1U);
		Timeout--;
	}

	InstancePtr->IsBusy = FALSE;

	if (Timeout == 0U) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/** @} */
