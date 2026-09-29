/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_intr.c
*
* @brief Interrupt-related functions for the UART Lite component (XUartLite).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 1.02a rpm  02/14/07 Added check for outstanding transmission before
*			calling the send callback (avoids extraneous
*			callback invocations)
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs. The macros have been
*		      renamed to remove _m from the name.
* 3.8	gm   09/25/22 Use XUartLite_GetSR instead of accessing status register
*                     directly.
* 3.14 vn  08/26/26 Fix Doxygen warnings
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartlite Overview
 * @{
 */

/***************************** Include Files ********************************/

#include "xuartlite.h"
#include "xuartlite_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static void ReceiveDataHandler(XUartLite *InstancePtr);
static void SendDataHandler(XUartLite *InstancePtr);

/************************** Variable Definitions ****************************/

/** @cond INTERNAL */
typedef void (*Handler)(XUartLite *InstancePtr);
/** @endcond */

/*****************************************************************************/
/**
*
* @brief Sets the receive callback handler.
*
* This function sets the handler called when an event
* (interrupt) occurs in the driver. The handler allows
* application-specific processing.
*
* @param[in,out] InstancePtr Pointer to the XUartLite instance.
* @param[in]     FuncPtr Pointer to the callback function.
* @param[in]     CallBackRef Upper layer callback reference passed back
*                when the callback function is invoked.
*
* @note          There is no assert on the CallBackRef since the driver does not
*                know what it is (nor should it)
*
******************************************************************************/
void XUartLite_SetRecvHandler(XUartLite *InstancePtr,
				XUartLite_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->RecvHandler = FuncPtr;
	InstancePtr->RecvCallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
*
* @brief Sets the send callback handler.
*
* This function sets the handler called when an event
* (interrupt) occurs in the driver. The handler allows
* application-specific processing.
*
* @param[in,out] InstancePtr Pointer to the XUartLite instance.
* @param[in]     FuncPtr Pointer to the callback function.
* @param[in]     CallBackRef Upper layer callback reference passed back
*                when the callback function is invoked.
*
* @note          There is no assert on the CallBackRef since the driver does not
*                know what it is (nor should it)
*
******************************************************************************/
void XUartLite_SetSendHandler(XUartLite *InstancePtr,
				XUartLite_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->SendHandler = FuncPtr;
	InstancePtr->SendCallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
*
* @brief Handles interrupts for the UART Lite driver.
*
* It must be connected to an interrupt system such that it is
* called when an interrupt for any UART lite occurs. This function
* does not save or restore the processor context such that the caller must
* ensure this occurs.
*
* @param[in]   InstancePtr Pointer to the instance of the UART that
*              the interrupt is for.
*
******************************************************************************/
void XUartLite_InterruptHandler(XUartLite *InstancePtr)
{
	u32 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Read the status register to determine which, coulb be both
	 * interrupt is active
	 */
	IsrStatus = XUartLite_GetSR(InstancePtr);

	if ((IsrStatus & (XUL_SR_RX_FIFO_FULL |
		XUL_SR_RX_FIFO_VALID_DATA)) != 0) {
		ReceiveDataHandler(InstancePtr);
	}

	if (((IsrStatus & XUL_SR_TX_FIFO_EMPTY) != 0) &&
		(InstancePtr->SendBuffer.RequestedBytes > 0)) {
		SendDataHandler(InstancePtr);
	}
}

/*****************************************************************************/
/**
*
* @brief Handles the interrupt when data is received.
*
* Handles either a single byte when FIFOs are not enabled, or multiple
* bytes with the FIFO.
*
* @param[in]   InstancePtr Pointer to the XUartLite instance.
*
******************************************************************************/
static void ReceiveDataHandler(XUartLite *InstancePtr)
{
	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartLite_ReceiveBuffer(InstancePtr);
	}

	/*
	 * If the last byte of a message was received then call the application
	 * handler, this code should not use an else from the previous check of
	 * the number of bytes to receive because the call to receive the buffer
	 * updates the bytes to receive
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes == 0) {
		InstancePtr->RecvHandler(InstancePtr->RecvCallBackRef,
		InstancePtr->ReceiveBuffer.RequestedBytes -
		InstancePtr->ReceiveBuffer.RemainingBytes);
	}

	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.ReceiveInterrupts++;
}

/*****************************************************************************/
/**
*
* @brief Handles the interrupt when data has been sent.
*
* The transmit FIFO is empty (transmitter holding register).
*
* @param[in,out] InstancePtr Pointer to the XUartLite instance.
*
******************************************************************************/
static void SendDataHandler(XUartLite *InstancePtr)
{
	/*
	 * If there are not bytes to be sent from the specified buffer,
	 * call the callback function
	 */
	if (InstancePtr->SendBuffer.RemainingBytes == 0) {
		int SaveReq;

		/*
		 * Save and zero the requested bytes since transmission
		 * is complete
		 */
		SaveReq = InstancePtr->SendBuffer.RequestedBytes;
		InstancePtr->SendBuffer.RequestedBytes = 0;

		/*
		 * Call the application handler to indicate
		 * the data has been sent
		 */
		InstancePtr->SendHandler(InstancePtr->SendCallBackRef, SaveReq);
	}
	/*
	 * Otherwise there is still more data to send in the specified buffer
	 * so go ahead and send it
	 */
	else {
		XUartLite_SendBuffer(InstancePtr);
	}

	/*
	 * Update the transmit stats to reflect the transmit interrupt
	 */
	InstancePtr->Stats.TransmitInterrupts++;
}


/*****************************************************************************/
/**
*
* @brief Disables the UART interrupt.
*
* After calling this function, data may still be received by the UART but
* no interrupt is generated since the hardware device has no way to
* disable the receiver.
*
* @param[in]   InstancePtr Pointer to the XUartLite instance.
*
******************************************************************************/
void XUartLite_DisableInterrupt(XUartLite *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to disable the interrupts, the only
	 * other bits in this register are the FIFO reset bits such that
	 * writing them to zero will not affect them.
	 */
	XUartLite_WriteReg(InstancePtr->RegBaseAddress,
				XUL_CONTROL_REG_OFFSET, 0);
}

/*****************************************************************************/
/**
*
* @brief Enables the UART interrupt.
*
* An interrupt occurs when data is received or data has been transmitted.
* The device contains 16 byte receive and transmit FIFOs such that an
* interrupt is generated anytime there is data in the receive FIFO and
* when the transmit FIFO transitions from not empty to empty.
*
* @param[in]   InstancePtr Pointer to the XUartLite instance.
*
******************************************************************************/
void XUartLite_EnableInterrupt(XUartLite *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the control register to enable the interrupts, the only
	 * other bits in this register are the FIFO reset bits such that
	 * writing them to zero will not affect them.
	 */
	XUartLite_WriteReg(InstancePtr->RegBaseAddress,
				XUL_CONTROL_REG_OFFSET, XUL_CR_ENABLE_INTR);
}

/** @} */
