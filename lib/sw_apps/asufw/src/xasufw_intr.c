/**************************************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_intr.c
 *
 * This file contains the interrupts handling code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   12/12/24 Initial release
 *       ma   12/23/24 Check and handle both DMA0_DONE and DMA1_DONE interrupts when handling
 *                     pending interrupts
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_util.h"
#include "xasufw_hw.h"
#include "xasufw_dma.h"
#include "xasufw_intr.h"
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function checks if there are any pending interrupts and calls their corresponding
 * handlers.
 *
 *************************************************************************************************/
void XAsufw_HandlePendingInterrupts(void)
{
	u32 IrqStatus = XAsufw_ReadReg(ASU_IO_BUS_IRQ_STATUS);

	/** Call the DMA interrupt handler if the received interrupt is DMA0 Done interrupt. */
	if ((IrqStatus & ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_MASK) ==
			ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_MASK) {
		XAsufw_HandleDmaDoneIntr(ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_NUM);
	}

	/** Call the DMA interrupt handler if the received interrupt is DMA1 Done interrupt. */
	if ((IrqStatus & ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_MASK) ==
			ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_MASK) {
		XAsufw_HandleDmaDoneIntr(ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_NUM);
	}

	/** Acknowledge the interrupts at the ASU IO Module. */
	XAsufw_WriteReg(ASU_IO_BUS_IRQ_ACK, IrqStatus);
}
/** @} */
