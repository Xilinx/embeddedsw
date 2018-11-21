/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xuartsbsa_intr.c
* @addtogroup uartsbsa_v1_0
* @{
*
* This file contains the functions for interrupt handling
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xuartsbsa.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XUartSbsa_ReceiveDataHandler(XUartSbsa *InstancePtr);
static void XUartSbsa_SendDataHandler(XUartSbsa *InstancePtr);
static void XUartSbsa_ModemHandler(XUartSbsa *InstancePtr);


/* Internal function prototypes implemented in xuartsbsa.c */
extern u32 XUartSbsa_ReceiveBuffer(XUartSbsa *InstancePtr);
extern u32 XUartSbsa_SendBuffer(XUartSbsa *InstancePtr);

/************************** Variable Definitions *****************************/

typedef void (*Handler)(XUartSbsa *InstancePtr);

/*****************************************************************************/
/**
*
* This function gets the interrupt mask
*
* @param	InstancePtr is a pointer to the XUartSbsa instance.
*
* @return
*		The current interrupt mask. The mask indicates which
*		interrupts are enabled.
*
* @note 	None.
*
******************************************************************************/
u32 XUartSbsa_GetInterruptMask(XUartSbsa *InstancePtr)
{
	/* Assert validates the input argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read the Interrupt Mask register */
	return (XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTIMSC_OFFSET));
}

/*****************************************************************************/
/**
*
* This function sets the interrupt mask.
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
* @param	Mask contains the interrupts to be enabled or disabled.
*		A '1' enables an interrupt, and a '0' disables.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartSbsa_SetInterruptMask(XUartSbsa *InstancePtr, u32 Mask)
{
	u32 TempMask;
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	TempMask = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTIMSC_OFFSET);

	TempMask &= (u32)XUARTSBSA_UARTIMSC_MASK;

	TempMask |= Mask;

	/* Write the mask to the mask set/clear Register */
	XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTSBSA_UARTIMSC_OFFSET, TempMask);

}

/*****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs that needs application's attention.
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return	None.
*
* @note
* There is no assert on the CallBackRef since the driver doesn't know what it
* is (nor should it)
*
******************************************************************************/
void XUartSbsa_SetHandler(XUartSbsa *InstancePtr, XUartSbsa_Handler FuncPtr,
					void *CallBackRef)
{
	/*
	 * Asserts validate the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the driver.
* It must be connected to an interrupt system by the application such that it
* can be called when an interrupt occurs.
*
* @param	InstancePtr contains a pointer to the driver instance
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartSbsa_InterruptHandler(XUartSbsa *InstancePtr)
{
	u32 Imsc;
	u32 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the raw interrupt status register to determine which
	 * interrupt is active
	 */
	Imsc = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTIMSC_OFFSET);

	IsrStatus = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTRIS_OFFSET);

	IsrStatus &= Imsc;

	if (IsrStatus) {
		do {
			/* Clear the interrupt status. */
			XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
					XUARTSBSA_UARTICR_OFFSET,
					IsrStatus & ~(XUARTSBSA_UARTRIS_TXRIS |
					XUARTSBSA_UARTRIS_RXRIS |
					XUARTSBSA_UARTRIS_RTRIS));


			/* Dispatch an appropriate handler. */
			if ((IsrStatus & ((u32)XUARTSBSA_UARTRIS_RXRIS |
					XUARTSBSA_UARTRIS_RTRIS)) != (u32)0) {
				/* Received data interrupt */
				XUartSbsa_ReceiveDataHandler(InstancePtr);
			}

			if ((IsrStatus & ((u32)XUARTSBSA_UARTRIS_TXRIS)) != (u32)0) {
				/* Transmit data interrupt */
				XUartSbsa_SendDataHandler(InstancePtr);
			}
			if ((IsrStatus & ((u32)XUARTSBSA_UARTRIS_DSRRMIS |
					(u32)XUARTSBSA_UARTRIS_DCDRMIS	|
					(u32)XUARTSBSA_UARTRIS_CTSRMIS |
					(u32)XUARTSBSA_UARTRIS_RIRMIS)) !=
					(u32)0) {
				/* Modem status interrupt */
				XUartSbsa_ModemHandler(InstancePtr);
			}

			IsrStatus = XUartSbsa_ReadReg(InstancePtr->Config.
						BaseAddress,
						XUARTSBSA_UARTRIS_OFFSET);
			IsrStatus &= Imsc;

		} while (IsrStatus != (u32)0);
	}
}

/*****************************************************************************/
/**
*
* This function handles the interrupt when data is in RX FIFO.
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartSbsa_ReceiveDataHandler(XUartSbsa *InstancePtr)
{
	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them. Removing bytes from the RX FIFO will
	 * clear the interrupt.
	 */
	 if (InstancePtr->ReceiveBuffer.RemainingBytes != (u32)0) {
		(void)XUartSbsa_ReceiveBuffer(InstancePtr);
	}

	 /*
	  * If the last byte of a message was received then call the
	  * application handler, this code should not use an else from
	  * the previous check of the number of bytes to receive because
	  * the call to receive the buffer updates the bytes ramained
	  */
	if (InstancePtr->ReceiveBuffer.RemainingBytes == (u32)0) {
		InstancePtr->Handler(InstancePtr->CallBackRef,
				XUARTSBSA_EVENT_RECV_DATA,
				(InstancePtr->ReceiveBuffer.RequestedBytes -
				InstancePtr->ReceiveBuffer.RemainingBytes));
	}

}

/*****************************************************************************/
/**
*
* This function handles the interrupt when data has been sent, the transmit
* FIFO is empty (transmitter holding register).
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
static void XUartSbsa_SendDataHandler(XUartSbsa *InstancePtr)
{
	u32 IntrMaskRegister;

	/*
	 * If there are no bytes to be sent from the specified buffer then
	 * disable the transmit interrupt so it will stop interrupting as it
	 * interrupts any time the FIFO is empty
	 */
	if (InstancePtr->SendBuffer.RemainingBytes == (u32)0) {
		/*
		 * Clear TX interrupt
		 */
		XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTICR_OFFSET,
				XUARTSBSA_UARTIMSC_TXIM);

		/*
		 * Disable TX interrupt
		 */
		IntrMaskRegister = XUartSbsa_ReadReg(InstancePtr->Config.
						BaseAddress,
						XUARTSBSA_UARTIMSC_OFFSET);
		IntrMaskRegister &= ~XUARTSBSA_UARTIMSC_TXIM;
		XUartSbsa_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTIMSC_OFFSET, IntrMaskRegister);

		/*
		 * Call the application handler to indicate the sending
		 * is done
		 */
		InstancePtr->Handler(InstancePtr->CallBackRef,
				XUARTSBSA_EVENT_SENT_DATA,
				InstancePtr->SendBuffer.RequestedBytes -
				InstancePtr->SendBuffer.RemainingBytes);
	}

	/* If TX FIFO is empty, send more. */
	else if ((XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTFR_OFFSET) &
				((u32)XUARTSBSA_UARTFR_TXFE)) != (u32)0) {
		(void)XUartSbsa_SendBuffer(InstancePtr);
	}
	else {
		/* Else with dummy entry for MISRA-C Compliance.*/
		;
	}
}

/*****************************************************************************/
/**
*
* This function handles modem interrupts.  It does not do any processing
* except to call the application handler to indicate a modem event.
*
* @param	InstancePtr is a pointer to the XUartSbsa instance
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
static void XUartSbsa_ModemHandler(XUartSbsa *InstancePtr)
{
	u32 MsrRegister;

	/*
	 * Read the modem status register so that the interrupt is
	 * acknowledged and it can be passed to the callback handler with
	 * the event
	 */
	MsrRegister = XUartSbsa_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTSBSA_UARTRIS_OFFSET);

	/*
	 * Call the application handler to indicate the modem status changed,
	 * passing the modem status and the event data in the call
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, XUARTSBSA_EVENT_MODEM,
			MsrRegister);

}
/** @} */
