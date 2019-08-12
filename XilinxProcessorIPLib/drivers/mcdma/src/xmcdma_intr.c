/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmcdma_intr.c
* @addtogroup mcdma_v1_3
* @{
*
* This file contains the implementation of the interrupt handling functions for
* MCDMA driver.
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0    adk    18/07/17 Initial version.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xmcdma.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This function is the Per Channel interrupt handler for the MCDMA core.
*
* This handler reads the pending interrupt from Status register, determines the
* source of the interrupts and calls the respective callbacks for the
* interrupts that are enabled in the per channel control register,
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XMcdma_ChanSetCallBack() during initialization phase.
*
* @param	Instance is a pointer to the Channel instance to be worked on.
*
* @return	None.
*
* @note		To generate interrupt required interrupts should be enabled.
*
******************************************************************************/
void XMcdma_ChanIntrHandler(void *Instance)
{
	u32 IrqStatus;
	XMcdma_ChanCtrl *Chan = NULL;
	Chan = (XMcdma_ChanCtrl *)((void *)Instance);

        /* Get pending interrupts */
	IrqStatus = XMcdma_ChanGetIrq(Chan);

	/* Acknowledge pending interrupts */
	XMcdma_ChanAckIrq(Chan, IrqStatus);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XMCDMA_IRQ_ALL_MASK)) {
		return;
	}

	if ((IrqStatus & (XMCDMA_IRQ_DELAY_MASK | XMCDMA_IRQ_IOC_MASK))) {
                Chan->ChanState = XMCDMA_CHAN_IDLE;
                Chan->DoneHandler(Chan->DoneRef);
	}

	if ((IrqStatus & XMCDMA_IRQ_ERROR_MASK)) {
		Chan->ChanState = XMCDMA_CHAN_PAUSE;
		Chan->ErrorHandler(Chan->ErrorRef, IrqStatus);
	}
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XMCDMA_HANDLER_DONE      Channel Done handler
* XMCDMA_HANDLER_ERROR     Channel Error handler
*
* </pre>
*
* @param	Chan is the Channel instance to be worked on
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
s32 XMcdma_ChanSetCallBack(XMcdma_ChanCtrl *Chan, XMcdma_ChanHandler HandlerType,
			   void *CallBackFunc, void *CallBackRef)
{
	s32 Status;

	/* Verify arguments. */
        Xil_AssertNonvoid(CallBackFunc != NULL);
        Xil_AssertNonvoid(CallBackRef != NULL);
        Xil_AssertNonvoid((HandlerType == XMCDMA_CHAN_HANDLER_DONE) ||
			  (HandlerType == XMCDMA_CHAN_HANDLER_ERROR));

	/*
         * Calls the respective callback function corresponding to
         * the handler type
         */
        switch (HandlerType) {
	case XMCDMA_CHAN_HANDLER_DONE:
		Chan->DoneHandler =
			(XMcdma_ChanDoneHandler)((void *)CallBackFunc);
                Chan->DoneRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	case XMCDMA_CHAN_HANDLER_ERROR:
		Chan->ErrorHandler =
			(XMcdma_ChanErrorHandler)((void *)CallBackFunc);
		Chan->ErrorRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function is the S2MM(RX) interrupt handler for the MCDMA core.
*
* This handler reads the pending interrupt from Status register, determines the
* source of the interrupts and calls the respective callbacks for the
* interrupts that are enabled in global control register, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XMcdma_SetCallBack() during initialization phase.
*
* @param	Instance is a pointer to the XMcdma instance to be worked on.
*
* @return	None.
*
* @note		To generate interrupt required interrupts should be enabled.
*
******************************************************************************/
void XMcdma_IntrHandler(void *Instance)
{
	XMcdma *InstancePtr = (XMcdma *)((void *)Instance);
	u32 IrqStatus;
	u16 Chan_id = 1;
	XMcdma_ChanCtrl *Chan = NULL;
	u32 i;
	u32 Chan_SerMask;

	/* Serviced Channel Numbers */
	while (1) {
		Chan_SerMask = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
					      XMCDMA_RX_OFFSET + XMCDMA_RXINT_SER_OFFSET);

		if (!Chan_SerMask)
			goto out;

		for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask;
			i <<= 1, Chan_id++) {
			if (Chan_SerMask & i) {
				Chan = XMcdma_GetMcdmaRxChan(InstancePtr,
							     Chan_id);
				IrqStatus = XMcdma_ChanGetIrq(Chan);

				/* Acknowledge pending interrupts */
				XMcdma_ChanAckIrq(Chan, IrqStatus);

				/* If no interrupt is asserted, we do not do anything */
				 if (!(IrqStatus & XMCDMA_IRQ_ALL_MASK)) {
					 return;
				 }

				 if ((IrqStatus & (XMCDMA_IRQ_DELAY_MASK | XMCDMA_IRQ_IOC_MASK))) {
					 Chan->ChanState = XMCDMA_CHAN_IDLE;
					 InstancePtr->DoneHandler(InstancePtr->DoneRef, Chan_id);
				 }

				 if ((IrqStatus & XMCDMA_IRQ_PKTDROP_MASK)) {
					 Chan->ChanState = XMCDMA_CHAN_IDLE;
					 InstancePtr->PktDropHandler(InstancePtr->PktDropRef, Chan_id);
				}

				 /* In Case of errors Channel Service Register
				  * will provide the Channel ID that caused error
				  */
				 if ((IrqStatus & XMCDMA_IRQ_ERROR_MASK)) {
					Chan->ChanState = XMCDMA_CHAN_PAUSE;
					InstancePtr->ErrorHandler(InstancePtr->ErrorRef, Chan_id, IrqStatus);
				 }
			}
		}
	}

out:
	return;

}

/*****************************************************************************/
/**
*
* This function is the MM2S interrupt handler for the MCDMA core.
*
* This handler reads the pending interrupt from Status register, determines the
* source of the interrupts and calls the respective callbacks for the
* interrupts that are enabled in global control register, and finally clears the
* interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XMcdma_SetCallBack() during initialization phase.
*
* @param	Instance is a pointer to the XMcdma instance to be worked on.
*
* @return	None.
*
* @note		To generate interrupt required interrupts should be enabled.
*
******************************************************************************/
void XMcdma_TxIntrHandler(void *Instance)
{
	XMcdma *InstancePtr = (XMcdma *)((void *)Instance);
	u32 IrqStatus;
	u16 Chan_id = 1;
	XMcdma_ChanCtrl *Chan = NULL;
	u32 i;
	u32 Chan_SerMask;

	/* Serviced Channel Numbers */
	while(1) {
		Chan_SerMask = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
						XMCDMA_TXINT_SER_OFFSET);

		if (!Chan_SerMask)
			goto out;

		for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask;
			i <<= 1, Chan_id++) {
			if (Chan_SerMask & i) {
				Chan = XMcdma_GetMcdmaTxChan(InstancePtr, Chan_id);

				IrqStatus = XMcdma_ChanGetIrq(Chan);

				/* Acknowledge pending interrupts */
				XMcdma_ChanAckIrq(Chan, IrqStatus);

				 /*
				  * If no interrupt is asserted, we do not do anything
				  */
				 if (!(IrqStatus & XMCDMA_IRQ_ALL_MASK)) {
					 return;
				 }

				 if ((IrqStatus & (XMCDMA_IRQ_DELAY_MASK | XMCDMA_IRQ_IOC_MASK))) {
					 Chan->ChanState = XMCDMA_CHAN_IDLE;
					 InstancePtr->TxDoneHandler(InstancePtr->TxDoneRef, Chan_id);
				 }

				 /* In Case of errors Channel Service Register
				  * will provide the Channel ID that caused error
				  */
				 if ((IrqStatus & XMCDMA_IRQ_ERROR_MASK)) {
					Chan->ChanState = XMCDMA_CHAN_PAUSE;
					InstancePtr->TxErrorHandler(InstancePtr->TxErrorRef, Chan_id, IrqStatus);
				 }
			}
		}
	}

out:
	return;
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType.
*
* <pre>
* HandlerType              Callback Function Type
* -----------------------  --------------------------------------------------
* XMCDMA_TX_HANDLER_DONE   MM2S(TX) Done handler
* XMCDMA_TX_HANDLER_ERROR  MM2S(TX) Error handler
* XMCDMA_HANDLER_DONE      S2MM(RX) Done handler
* XMCDMA_HANDLER_ERROR     S2MM(RX) Error handler
* XMCDMA_HANDLER_PKTDROP   S2MM(RX) Packet drop handler
*
* </pre>
*
* @param	InstancePtr is a pointer to the XMcdma instance to be worked on.
* @param	HandlerType specifies which callback is to be attached.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
s32 XMcdma_SetCallBack(XMcdma *InstancePtr, XMcdma_Handler HandlerType,
		       void *CallBackFunc, void *CallBackRef)
{
	s32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(CallBackFunc != NULL);
	Xil_AssertNonvoid(CallBackRef != NULL);
	Xil_AssertNonvoid((HandlerType == XMCDMA_TX_HANDLER_DONE) ||
			  (HandlerType == XMCDMA_TX_HANDLER_ERROR) ||
			  (HandlerType == XMCDMA_HANDLER_DONE) ||
			  (HandlerType == XMCDMA_HANDLER_ERROR) ||
			  (HandlerType == XMCDMA_HANDLER_PKTDROP));

	/*
	 * Calls the respective callback function corresponding to
	 * the handler type
	 */
	switch (HandlerType) {
	case XMCDMA_TX_HANDLER_DONE:
		InstancePtr->TxDoneHandler = (XMcdma_TxDoneHandler)((void *)CallBackFunc);
		InstancePtr->TxDoneRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	case XMCDMA_TX_HANDLER_ERROR:
		InstancePtr->TxErrorHandler = (XMcdma_TxErrorHandler)((void *)CallBackFunc);
		InstancePtr->TxErrorRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	case XMCDMA_HANDLER_DONE:
		InstancePtr->DoneHandler = (XMcdma_DoneHandler)((void *)CallBackFunc);
		InstancePtr->DoneRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	case XMCDMA_HANDLER_ERROR:
		InstancePtr->ErrorHandler = (XMcdma_ErrorHandler)((void *)CallBackFunc);
		InstancePtr->ErrorRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	case XMCDMA_HANDLER_PKTDROP:
		InstancePtr->PktDropHandler = (XMcdma_PktDropHandler)((void *)CallBackFunc);
		InstancePtr->PktDropRef = CallBackRef;
		Status = (XST_SUCCESS);
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/** @} */
