/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/****************************************************************************/
/**
*
* @file xiomodule_uart_intr.c
* @addtogroup iomodule_v2_12
* @{
*
* Contains required functions for the XIOModule UART driver interrupt mode.
* See the xiomodule.h header file for more details on this driver.
*
* This file also contains interrupt-related functions for the UART.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.03a sa   10/16/12 First release
* 2.2	nsk  08/06/15 Updated XIOModule_Uart_InterruptHandler function
*		      to read Status register instead of reading Interrupt
*		      Pending register.
* 2.12	sk   06/08/21 Update XIOModule_Send and XIOModule_Recv API's argument
		      (NumBytes) datatype to fix the coverity warnings.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_assert.h"
#include "xiomodule.h"
#include "xiomodule_i.h"
#include "xiomodule_l.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

static void ReceiveDataHandler(XIOModule *InstancePtr);
static void SendDataHandler(XIOModule *InstancePtr);

/************************** Variable Definitions ****************************/

typedef void (*Handler)(XIOModule *InstancePtr);


/****************************************************************************/
/**
*
* This functions sends the specified buffer of data using the UART in either
* polled or interrupt driven modes. This function is non-blocking such that it
* will return before the data has been sent by the UART. If the UART is busy
* sending data, it will return and indicate zero bytes were sent.
*
* In a polled mode, this function will only send as much data as the UART can
* buffer in the transmitter. The application may need to call it repeatedly to
* send a buffer.
*
* In interrupt mode, this function will start sending the specified buffer and
* then the interrupt handler of the driver will continue sending data until the
* buffer has been sent. A callback function, as specified by the application,
* will be called to indicate the completion of sending the buffer.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	DataBufferPtr is pointer to a buffer of data to be sent.
* @param	NumBytes contains the number of bytes to be sent. A value of
*		zero will stop a previous send operation that is in progress
*		in interrupt mode. Any data that was already put into the
*		transmit FIFO will be sent.
*
* @return	The number of bytes actually sent.
*
* @note		The number of bytes is not asserted so that this function may
*		be called with a value of zero to stop an operation that is
*		already in progress.
*
******************************************************************************/
u32 XIOModule_Send(XIOModule *InstancePtr, u8 *DataBufferPtr,
				u32 NumBytes)
{
	unsigned int BytesSent;
	u32 StatusRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataBufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(((signed)NumBytes) >= 0);

	/*
	 * Enter a critical region by disabling the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven.
	 */
	StatusRegister = InstancePtr->CurrentIER;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
			StatusRegister & 0xFFFFFFF8);

	/*
	 * Setup the specified buffer to be sent by setting the instance
	 * variables so it can be sent with polled or interrupt mode
	 */
	InstancePtr->SendBuffer.RequestedBytes = NumBytes;
	InstancePtr->SendBuffer.RemainingBytes = NumBytes;
	InstancePtr->SendBuffer.NextBytePtr = DataBufferPtr;

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited.
	 * This is done here to minimize the amount of time the interrupt is
	 * disabled since there is only one interrupt and the receive could
	 * be filling up while interrupts are blocked.
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
	    (InstancePtr->CurrentIER & 0xFFFFFFF8) | (StatusRegister & 0x7));

	/*
	 * Send the buffer using the UART and return the number of bytes sent
	 */
	BytesSent = XIOModule_SendBuffer(InstancePtr);

	return BytesSent;
}


/****************************************************************************/
/**
*
* This function will attempt to receive a specified number of bytes of data
* from the UART and store it into the specified buffer. This function is
* designed for either polled or interrupt driven modes. It is non-blocking
* such that it will return if no data has already received by the UART.
*
* In a polled mode, this function will only receive as much data as the UART
* can buffer in the receiver. The application may need to call it repeatedly to
* receive a buffer. Polled mode is the default mode of operation for the driver.
*
* In interrupt mode, this function will start receiving and then the interrupt
* handler of the driver will continue receiving data until the buffer has been
* received. A callback function, as specified by the application, will be called
* to indicate the completion of receiving the buffer or when any receive errors
* or timeouts occur.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	DataBufferPtr is pointer to buffer for data to be received into.
* @param	NumBytes is the number of bytes to be received. A value of zero
*		will stop a previous receive operation that is in progress in
*		interrupt mode.
*
* @return	The number of bytes received.
*
* @note 	The number of bytes is not asserted so that this function
*		may be called with a value of zero to stop an operation
*		that is already in progress.
*
*****************************************************************************/
u32 XIOModule_Recv(XIOModule *InstancePtr, u8 *DataBufferPtr,
				u32 NumBytes)
{
	unsigned int ReceivedCount;
	u32 StatusRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataBufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(((signed)NumBytes) >= 0);

	/*
	 * Enter a critical region by disabling all the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven
	 */
	StatusRegister = InstancePtr->CurrentIER;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
			StatusRegister & 0xFFFFFFF8);

	/*
	 * Setup the specified buffer to be received by setting the instance
	 * variables so it can be received with polled or interrupt mode
	 */
	InstancePtr->ReceiveBuffer.RequestedBytes = NumBytes;
	InstancePtr->ReceiveBuffer.RemainingBytes = NumBytes;
	InstancePtr->ReceiveBuffer.NextBytePtr = DataBufferPtr;

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
	    (InstancePtr->CurrentIER & 0xFFFFFFF8) | (StatusRegister & 0x7));

	/*
	 * Receive the data from the UART and return the number of bytes
	 * received. This is done here to minimize the amount of time the
	 * interrupt is disabled.
	 */
	ReceivedCount = XIOModule_ReceiveBuffer(InstancePtr);

	return ReceivedCount;
}

/****************************************************************************/
/**
*
* This function does nothing, since the UART doesn't have any FIFOs. It is
* included for compatibility with the UART Lite driver.
*
* @param	InstancePtr is a pointer to the XIOModule instance .
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XIOModule_ResetFifos(XIOModule *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
}

/****************************************************************************/
/**
*
* This function determines if the specified UART is sending data. If the
* transmitter register is not empty, it is sending data.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	A value of TRUE if the UART is sending data, otherwise FALSE.
*
* @note		None.
*
*****************************************************************************/
int XIOModule_IsSending(XIOModule *InstancePtr)
{
	u32 StatusRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Read the status register to determine if the transmitter is empty
	 */
	StatusRegister = XIOModule_ReadReg(InstancePtr->BaseAddress,
						XUL_STATUS_REG_OFFSET);

	/*
	 * If the transmitter is not empty then indicate that the UART is still
	 * sending some data
	 */
	return ((StatusRegister & XUL_SR_TX_FIFO_FULL) == XUL_SR_TX_FIFO_FULL);
}

/****************************************************************************/
/**
*
* This function sends a buffer that has been previously specified by setting
* up the instance variables of the instance. This function is designed to be
* an internal function for the XIOModule component such that it may be called
* from a shell function that sets up the buffer or from an interrupt handler.
*
* This function sends the specified buffer of data to the UART in either
* polled or interrupt driven modes. This function is non-blocking such that
* it will return before the data has been sent by the UART.
*
* In a polled mode, this function will only send as much data as the UART can
* buffer in the transmitter. The application may need to call it repeatedly to
* send a buffer.
*
* In interrupt mode, this function will start sending the specified buffer and
* then the interrupt handler of the driver will continue until the buffer
* has been sent. A callback function, as specified by the application, will
* be called to indicate the completion of sending the buffer.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	NumBytes is the number of bytes actually sent (put into the
*		UART transmitter and/or FIFO).
*
* @note		None.
*
*****************************************************************************/
unsigned int XIOModule_SendBuffer(XIOModule *InstancePtr)
{
	unsigned int SentCount = 0;
	u8 StatusRegister;
	u8 IntrEnableStatus;

	/*
	 * Read the status register to determine if the transmitter is full
	 */
	StatusRegister = XIOModule_GetStatusReg(InstancePtr->BaseAddress);

	/*
	 * Enter a critical region by disabling all the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
			StatusRegister & 0xFFFFFFF8);

	/*
	 * Save the status register contents to restore the interrupt enable
	 * register to it's previous value when that the critical region is
	 * exited
	 */
	IntrEnableStatus = StatusRegister;

	/*
	 * Fill the FIFO from the the buffer that was specified
	 */

	while (((StatusRegister & XUL_SR_TX_FIFO_FULL) == 0) &&
		(SentCount < InstancePtr->SendBuffer.RemainingBytes)) {
		XIOModule_WriteReg(InstancePtr->BaseAddress,
					XUL_TX_OFFSET,
					InstancePtr->SendBuffer.NextBytePtr[
					SentCount]);

		SentCount++;

		StatusRegister =
			XIOModule_GetStatusReg(InstancePtr->BaseAddress);
	}

	/*
	 * Update the buffer to reflect the bytes that were sent from it
	 */
	InstancePtr->SendBuffer.NextBytePtr += SentCount;
	InstancePtr->SendBuffer.RemainingBytes -= SentCount;

	/*
	 * Increment associated counters
	 */
	 InstancePtr->Uart_Stats.CharactersTransmitted += SentCount;

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
	    (InstancePtr->CurrentIER & 0xFFFFFFF8) | (IntrEnableStatus & 0x7));

	/*
	 * Return the number of bytes that were sent, although they really were
	 * only put into the FIFO, not completely sent yet
	 */
	return SentCount;
}

/****************************************************************************/
/**
*
* This function receives a buffer that has been previously specified by setting
* up the instance variables of the instance. This function is designed to be
* an internal function for the XIOModule component such that it may be called
* from a shell function that sets up the buffer or from an interrupt handler.
*
* This function will attempt to receive a specified number of bytes of data
* from the UART and store it into the specified buffer. This function is
* designed for either polled or interrupt driven modes. It is non-blocking
* such that it will return if there is no data has already received by the
* UART.
*
* In a polled mode, this function will only receive as much data as the UART
* can buffer, either in the receiver or in the FIFO if present and enabled.
* The application may need to call it repeatedly to receive a buffer. Polled
* mode is the default mode of operation for the driver.
*
* In interrupt mode, this function will start receiving and then the interrupt
* handler of the driver will continue until the buffer has been received. A
* callback function, as specified by the application, will be called to indicate
* the completion of receiving the buffer or when any receive errors or timeouts
* occur.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	The number of bytes received.
*
* @note		None.
*
*****************************************************************************/
unsigned int XIOModule_ReceiveBuffer(XIOModule *InstancePtr)
{
	u8 StatusRegister;
	unsigned int ReceivedCount = 0;

	/*
	 * Loop until there is not more data buffered by the UART or the
	 * specified number of bytes is received
	 */

	while (ReceivedCount < InstancePtr->ReceiveBuffer.RemainingBytes) {
		/*
		 * Read the Status Register to determine if there is any data in
		 * the receiver
		 */
		StatusRegister =
			XIOModule_GetStatusReg(InstancePtr->BaseAddress);

		/*
		 * If there is data ready to be removed, then put the next byte
		 * received into the specified buffer and update the stats to
		 * reflect any receive errors for the byte
		 */
		if (StatusRegister & XUL_SR_RX_FIFO_VALID_DATA) {
			InstancePtr->ReceiveBuffer.NextBytePtr[ReceivedCount++]=
				XIOModule_ReadReg(InstancePtr->BaseAddress,
							XUL_RX_OFFSET);

			XIOModule_UpdateStats(InstancePtr, StatusRegister);
		}

		/*
		 * There's no more data buffered, so exit such that this
		 * function does not block waiting for data
		 */
		else {
			break;
		}
	}

	/*
	 * Enter a critical region by disabling all the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven
	 */
	StatusRegister = InstancePtr->CurrentIER;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
			StatusRegister & 0xFFFFFFF8);

	/*
	 * Update the receive buffer to reflect the number of bytes that was
	 * received
	 */
	InstancePtr->ReceiveBuffer.NextBytePtr += ReceivedCount;
	InstancePtr->ReceiveBuffer.RemainingBytes -= ReceivedCount;

	/*
	 * Increment associated counters in the statistics
	 */
	InstancePtr->Uart_Stats.CharactersReceived += ReceivedCount;

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
	    (InstancePtr->CurrentIER & 0xFFFFFFF8) | (StatusRegister & 0x7));

	return ReceivedCount;
}



/****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver for the UART. The purpose of the handler is to allow
* application specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return	None.
*
* @note		There is no assert on the CallBackRef since the driver doesn't
*		know what it is (nor should it)
*
*****************************************************************************/
void XIOModule_SetRecvHandler(XIOModule *InstancePtr,
				XIOModule_Handler FuncPtr, void *CallBackRef)
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

/****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver for the UART. The purpose of the handler is to allow
* application specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XIOModule instance .
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return 	None.
*
* @note		There is no assert on the CallBackRef since the driver doesn't
*		know what it is (nor should it)
*
*****************************************************************************/
void XIOModule_SetSendHandler(XIOModule *InstancePtr,
				XIOModule_Handler FuncPtr, void *CallBackRef)
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

/****************************************************************************/
/**
*
* This function is the interrupt handler for the UART.
* It must be connected to an interrupt system by the user such that it is
* called when an interrupt for any UART lite occurs. This function
* does not save or restore the processor context such that the user must
* ensure this occurs.
*
* @param	InstancePtr contains a pointer to the instance of the IOModule
*		that the interrupt is for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Uart_InterruptHandler(XIOModule *InstancePtr)
{
	u32 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Read the status register to determine which, could be both,
	 * interrupt is active
	 */
	IsrStatus = XIOModule_GetStatusReg(InstancePtr->BaseAddress);

	if ((IsrStatus & XUL_SR_RX_FIFO_VALID_DATA) != 0) {
		ReceiveDataHandler(InstancePtr);
	}

	if (((IsrStatus & XUL_SR_TX_FIFO_FULL) == XUL_SR_TX_FIFO_FULL) &&
		(InstancePtr->SendBuffer.RequestedBytes > 0)) {
		SendDataHandler(InstancePtr);
	}
}

/****************************************************************************/
/**
*
* This function handles the interrupt when data is received, either a single
* byte when FIFOs are not enabled, or multiple bytes with the FIFO.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ReceiveDataHandler(XIOModule *InstancePtr)
{
	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XIOModule_ReceiveBuffer(InstancePtr);
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
	InstancePtr->Uart_Stats.ReceiveInterrupts++;
}

/****************************************************************************/
/**
*
* This function handles the interrupt when data has been sent, the transmit
* FIFO is empty (transmitter holding register).
*
* @param	InstancePtr is a pointer to the XIOModule instance .
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void SendDataHandler(XIOModule *InstancePtr)
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
		XIOModule_SendBuffer(InstancePtr);
	}

	/*
	 * Update the transmit stats to reflect the transmit interrupt
	 */
	InstancePtr->Uart_Stats.TransmitInterrupts++;
}


/*****************************************************************************/
/**
*
* This function disables the UART interrupt. After calling this function,
* data may still be received by the UART but no interrupt will be generated
* since the hardware device has no way to disable the receiver.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XIOModule_Uart_DisableInterrupt(XIOModule *InstancePtr)
{
	u32 NewIER;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the interrupt enable register to disable the UART
	 * interrupts.
	 */
	NewIER = InstancePtr->CurrentIER & 0xFFFFFFF8;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;
}

/*****************************************************************************/
/**
*
* This function enables the UART interrupts such that an interrupt will occur
* when data is received or data has been transmitted.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XIOModule_Uart_EnableInterrupt(XIOModule *InstancePtr)
{
	u32 NewIER;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the interrupt enable register to enable the interrupts.
	 */
	NewIER = InstancePtr->CurrentIER | 0x7;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;
}
/** @} */
