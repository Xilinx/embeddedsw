/******************************************************************************
*
* Copyright (C) 2007 - 2016 Xilinx, Inc.  All rights reserved.
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
*
* @file xhwicap_intr.c
* @addtogroup hwicap_v11_1
* @{
*
* This file contains interrupt handling API functions of the HwIcap device.
*
* Refer to xhwicap.h header file and device specification for more information.
*
* @note
*
* Calling the interrupt functions without including the interrupt component will
* result in asserts if asserts are enabled, and will result in a unpredictable
* behavior if the asserts are not enabled.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 2.00a  sv    09/22/07 First release
* 4.00a  hvm   12/1/09  Updated with HAL phase 1 changes
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhwicap.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Sets the status callback function, the status handler, which the driver calls
* when it encounters conditions that should be reported to the higher layer
* software. The handler executes in an interrupt context, so it must minimize
* the amount of processing performed such as transferring data to a thread
* context. One of the following status events is passed to the status handler.
* <pre>
*
* </pre>
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	FuncPtr is the pointer to the callback function.
*
* @return	None.
*
* @note
*
* The handler is called within interrupt context, so it should do its work
* quickly and queue potentially time-consuming work to a task-level thread.
*
******************************************************************************/
void XHwIcap_SetInterruptHandler(XHwIcap * InstancePtr, void *CallBackRef,
			   		XHwIcap_StatusHandler FuncPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FuncPtr;
	InstancePtr->StatusRef = CallBackRef;
}

/*****************************************************************************/
/**
*
* The interrupt handler for HwIcap interrupts. This function must be connected
* by the user to an interrupt source.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		The interrupts are being used only while writing data to the
*		ICAP device. The reading of the data from the ICAP device is
*		done in polled mode.
*		In a worst case scenario the interrupt handler can
*		be busy writing large amount of data to the Write FIFO.
*
******************************************************************************/
void XHwIcap_IntrHandler(void *InstancePtr)
{
	XHwIcap *HwIcapPtr = (XHwIcap *) InstancePtr;
	u32 IntrStatus;
	u32 WrFifoVacancy;


	Xil_AssertVoid(InstancePtr != NULL);


	/*
	 * Get the Interrupt status.
	 */
	IntrStatus = XHwIcap_IntrGetStatus(HwIcapPtr);

	if (IntrStatus & XHI_IPIXR_WRP_MASK) {

		/*
		 * A transmit has just completed. Check for more data
		 * to transmit.
		 */
		if (HwIcapPtr->RemainingWords > 0) {

			/*
			 * Fill the FIFO with as many words as it will take (or
			 * as many as we have to write). We can use the Write
			 * FIFO vacancy to know if the device can take more data.
			 */
			WrFifoVacancy = XHwIcap_GetWrFifoVacancy(HwIcapPtr);
			while ((WrFifoVacancy != 0) &&
			       (HwIcapPtr->RemainingWords > 0)) {
				XHwIcap_FifoWrite(HwIcapPtr,
						*HwIcapPtr->SendBufferPtr);

				HwIcapPtr->RemainingWords--;
				WrFifoVacancy--;
				HwIcapPtr->SendBufferPtr++;
			}

			XHwIcap_StartConfig(HwIcapPtr);
		}
		else {

			if (HwIcapPtr->RequestedWords != 0) {

				/*
				 * No more data to send. Disable the interrupts
				 * by disabling the Global Interrupts.
				 * Inform the upper layer software that the
				 * transfer is done.
				 */
				XHwIcap_IntrGlobalDisable(HwIcapPtr);
				HwIcapPtr->IsTransferInProgress = FALSE;
				HwIcapPtr->StatusHandler(HwIcapPtr->StatusRef,
						    XST_HWICAP_WRITE_DONE,
						    HwIcapPtr->RequestedWords);
			}
		}
	}


	/*
	 * Clear the Interrupt status.
	 */
	XHwIcap_IntrClear(HwIcapPtr, IntrStatus);

}

/** @} */
