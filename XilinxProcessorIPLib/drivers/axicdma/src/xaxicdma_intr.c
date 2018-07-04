/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 *  @file xaxicdma_intr.c
* @addtogroup axicdma_v4_5
* @{
 *
 * The implementation of the interrupt related API. The interrupt handler is
 * also implemented here.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/08/10 First release
 * 2.01a rkv  01/25/11 Replaced with "\r\n" in place on "\n\r" in printf
 *		       statements
 * 2.03a srt  04/13/13 Removed Warnings (CR 705006).
 * </pre>
 *
 *****************************************************************************/
#include "xaxicdma.h"
#include "xaxicdma_i.h"

/*****************************************************************************/
/**
 * This function enables interrupts specified by the Mask. Interrupts that are
 * not in the mask are not affected.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Mask is the mask for the interrupts to be enabled
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_IntrEnable(XAxiCdma *InstancePtr, u32 Mask)
{
	u32 RegValue;

	RegValue = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET);

	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
		RegValue | (Mask & XAXICDMA_XR_IRQ_ALL_MASK));
}

/*****************************************************************************/
/**
 * This function gets the mask for the interrupts that are currently enabled
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The bit mask for the interrupts that are currently enabled
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_IntrGetEnabled(XAxiCdma *InstancePtr)
{
	return (XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET) &
		XAXICDMA_XR_IRQ_ALL_MASK);
}

/*****************************************************************************/
/**
 * This function disables interrupts specified by the Mask. Interrupts that
 * are not in the mask are not affected.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Mask is the mask for the interrupts to be disabled
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_IntrDisable(XAxiCdma *InstancePtr, u32 Mask)
{
	u32 RegValue;

	RegValue = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET);

	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
		RegValue & ~(Mask & XAXICDMA_XR_IRQ_ALL_MASK));
}

/*****************************************************************************/
/**
 * This function is the interrupt handler for the driver, it handles all the
 * interrupts. For the completion of a transfer that has a callback function,
 * the callback function is called.
 *
 * @param	HandlerRef is a reference pointer passed to the interrupt
 *			registration function. It will be a pointer to the driver
 *			instance we are working on
 *
 * @return	None
 *
 * @note	If one transfer does not have all its submitted BDs completed
 *		successfully,then a reset is needed to clean up the mess left
 *		by that transfer.Otherwise, the wrong interrupt callback maybe
 *		called for the following transfers. However, if you always use
 *		the same interrupt callback for all the transfers, and you are
 *		the only user of the DMA engine, then you do not have to worry
 *		about this.
 *****************************************************************************/
void XAxiCdma_IntrHandler(void *HandlerRef)
{
	XAxiCdma *InstancePtr;
	u32 Status;
	u32 Irq;
	u32 Error = 0x0;

	InstancePtr = (XAxiCdma *)HandlerRef;
	Status = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET);

	/* Check what interrupts have fired
	 */
	Irq = Status & XAXICDMA_XR_IRQ_ALL_MASK;

	if (Irq == 0x0) {
		xdbg_printf(XDBG_DEBUG_ERROR, "No interrupt for intr handler\r\n");
		return;
	}

	/* Acknowledge the interrupt
	 */
	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET, Irq);

	/* Pass the interrupt and error status to the callback function
	 * if the transfer has one
	 */

	/* If SimpleNotDone flag is set, then it is a simple transfer
	 */
	if (InstancePtr->SimpleNotDone) {
		if (InstancePtr->SimpleCallBackFn) {
			(InstancePtr->SimpleCallBackFn)(
			    InstancePtr->SimpleCallBackRef, Irq, NULL);

			InstancePtr->SimpleCallBackFn = NULL;
		}

		InstancePtr->SimpleNotDone = 0;

		if (InstancePtr->SGWaiting) {
			XAxiCdma_BdRingStartTransfer(InstancePtr);
		}
	}
	else {	/* SG transfer */
		if (InstancePtr->SgHandlerHead != InstancePtr->SgHandlerTail) {
			int Tmp;

			XAxiCdma_IntrHandlerList Handler =
			    InstancePtr->Handlers[InstancePtr->SgHandlerHead];

			Tmp = Handler.NumBds;

			/* Caution: may have race condition here
			 *
			 * If an interrupt for another transfer happens
			 * while in callback function, then the wrong callback
			 * function may be called.
			 */
			Handler.CallBackFn(Handler.CallBackRef, Irq, &(Tmp));

			InstancePtr->Handlers[InstancePtr->SgHandlerHead].NumBds = Tmp;

			/* Update the handler head if this transfer is done
			 */
			if (Tmp == 0) {
				Tmp = InstancePtr->SgHandlerHead + 1;

				if (Tmp == XAXICDMA_MAXIMUM_MAX_HANDLER) {
					Tmp	= 0;
				}

				InstancePtr->SgHandlerHead = Tmp;
			}
		}
		else {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "ERROR: SG transfer intr without handler\r\n");
		}
	}

	/* If has error interrupt, hardware needs to be reset
	 */
	Error = Status & XAXICDMA_SR_ERR_ALL_MASK;
	if ((Irq & XAXICDMA_XR_IRQ_ERROR_MASK) && Error) {
		int TimeOut;


		TimeOut = XAXICDMA_RESET_LOOP_LIMIT;

		/* Need to reset the hardware to clear the errors
		 */
		XAxiCdma_Reset(InstancePtr);

		while (TimeOut) {

			if (XAxiCdma_ResetIsDone(InstancePtr)) {
				break;
			}

			TimeOut -= 1;
		}

		/* Reset failed
		 */
		if (!TimeOut) {
			/* Mark the driver/engine is not in working state
			 */
			InstancePtr->Initialized = 0;
		}

		/* In case of error, no further handling is needed
		 *
		 * User should check send/receive buffers to see what happened
		 * as well as check the DMA engine registers
		 */
		return;
 	}

	return;
}
/** @} */
