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
* @file xuartpsv.c
* @addtogroup uartpsv_v1_0
* @{
*
* This file contains the implementation of the interface functions for
* XUartPsv driver. Refer to the header file xuartpsv.h for more detailed
* information.
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

#include "xstatus.h"
#include "xuartpsv.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/*
 * The following constant defines the amount of error that is allowed for
 * a specified baud rate. This error is the difference between the actual
 * baud rate that will be generated using the specified clock and the
 * desired baud rate.
 */
#define XUARTPSV_MAX_BAUD_ERROR_RATE	3U	/* max % error allowed */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XUartPsv_StubHandler(void *CallBackRef, u32 Event, u32 ByteCount);

u32  XUartPsv_SendBuffer(XUartPsv *InstancePtr);

u32  XUartPsv_ReceiveBuffer(XUartPsv *InstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a specific XUartPsv instance such that it is ready to be used.
* The data format of the device is setup for 8 data bits, 1 stop bit, and no
* parity by default. The baud rate is set to a default value specified by
* Config->DefaultBaudRate if set, otherwise it is set to 19.2K baud. The
* receive FIFO threshold is set for 8 bytes. The default operating mode of
* the driver is polled mode.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	Config is a reference to a structure containing information
*		about a specific XUartPsv driver.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used, pass
*		in the physical address instead.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_UART_BAUD_ERROR if the baud rate is not possible because
*		  the inputclock frequency is not divisible with an acceptable
*		  amount of error
*
* @note
* The default configuration for the UART after initialization is:
*
* - 19,200 bps or XPAR_DFT_BAUDRATE if defined
* - 8 data bits
* - 1 stop bit
* - no parity
* - FIFO's are enabled with a threshold of 16 ytes
*
*   All interrupts are disabled.
*
******************************************************************************/
s32 XUartPsv_CfgInitialize(XUartPsv *InstancePtr,
			XUartPsv_Config * Config, u32 EffectiveAddr)
{
	s32 Status;
	u32 LineCtrlRegister;
	u32 BaudRate;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Config != NULL);

	/* Setup the driver instance using passed in parameters */
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.InputClockHz = Config->InputClockHz;
	InstancePtr->Config.ModemPinsConnected = Config->ModemPinsConnected;

	/* Initialize other instance data to default values */
	InstancePtr->Handler = XUartPsv_StubHandler;

	InstancePtr->SendBuffer.NextBytePtr = NULL;
	InstancePtr->SendBuffer.RemainingBytes = 0U;
	InstancePtr->SendBuffer.RequestedBytes = 0U;

	InstancePtr->ReceiveBuffer.NextBytePtr = NULL;
	InstancePtr->ReceiveBuffer.RemainingBytes = 0U;
	InstancePtr->ReceiveBuffer.RequestedBytes = 0U;

	/* Flag that the driver instance is ready to use */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Set the default baud rate here, can be changed prior to
	 * starting the device
	 */
	BaudRate = (u32)XUARTPSV_DFT_BAUDRATE;

	Status = XUartPsv_SetBaudRate(InstancePtr, BaudRate);
	if (Status != (s32)XST_SUCCESS) {
		InstancePtr->IsReady = 0U;
	} else {
		/* Set the FIFO trigger level to 1/2 full of Fifo's size */
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTIFLS_OFFSET,
				(XUARTPSV_UARTIFLS_RXIFLSEL_1_2 |
				XUARTPSV_UARTIFLS_TXIFLSEL_1_2));
#if 0
		/* Enable transmit and receive DMA*/
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTDMACR_OFFSET,
				(XUARTPSV_UARTDMACR_TXDMAE |
				XUARTPSV_UARTDMACR_RXDMAE));
#endif
		/*
		 * Set up the default data format: 8 bit data, 1 stop bit,
		 * no parity
		 */
		LineCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.
						BaseAddress,
						XUARTPSV_UARTLCR_OFFSET);

		/* Mask off what's already there */
		LineCtrlRegister &= (~((u32)XUARTPSV_UARTLCR_WLEN_MASK |
					(u32)XUARTPSV_UARTLCR_STP_MASK |
					(u32)XUARTPSV_UARTLCR_PARITY_MASK));

		/* Set the register value to the desired data format */
		LineCtrlRegister |= ((u32)XUARTPSV_UARTLCR_WLEN_8_BIT |
					(u32)XUARTPSV_UARTLCR_STP_1_BIT |
					(u32)XUARTPSV_UARTLCR_PARITY_NONE);

		/* Enable fifo's */
		LineCtrlRegister |= XUARTPSV_UARTLCR_FEN;

		/* Write the line controller register out */
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);

		/*
		 * Disable all interrupts and cleared
		 * Polled mode is the default
		 */
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTIMSC_OFFSET, 0x0);
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTICR_OFFSET,
				XUARTPSV_UARTIMSC_MASK);

		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This functions sends the specified buffer using the device in either
* polled or interrupt driven mode. This function is non-blocking, if the
* device is busy sending data, it will return and indicate zero bytes were
* sent. Otherwise, it fills the TX FIFO as much as it can, and return the
* number of bytes sent.
*
* In a polled mode, this function will only send as much data as TX FIFO can
* buffer. The application may need to call it repeatedly to send the entire
* buffer.
*
* In interrupt mode, this function will start sending the specified buffer,
* then the interrupt handler will continue sending data until the entire
* buffer has been sent. A callback function, as specified by the application,
* will be called to indicate the completion of sending.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	BufferPtr is pointer to a buffer of data to be sent.
* @param	NumBytes contains the number of bytes to be sent. A value of
*		zero will stop a previous send operation that is in progress
*		in interrupt mode. Any data that was already put into the
*		transmit FIFO will be sent.
*
* @return	The number of bytes actually sent.
*
* @note
* The number of bytes is not asserted so that this function may be called with
* a value of zero to stop an operation that is already in progress.
* <br><br>
*
******************************************************************************/
u32 XUartPsv_Send(XUartPsv *InstancePtr, u8 *BufferPtr, u32 NumBytes)
{
	u32 BytesSent;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Setup the buffer parameters */
	InstancePtr->SendBuffer.RequestedBytes = NumBytes;
	InstancePtr->SendBuffer.RemainingBytes = NumBytes;
	InstancePtr->SendBuffer.NextBytePtr = BufferPtr;

	/* Send data to the device */
	BytesSent = XUartPsv_SendBuffer(InstancePtr);

	return BytesSent;
}

/*****************************************************************************/
/**
*
* This function attempts to receive a specified number of bytes of data
* from the device and store it into the specified buffer. This function works
* for both polled or interrupt driven modes. It is non-blocking.
*
* In a polled mode, this function will only receive the data already in the
* RX FIFO. The application may need to call it repeatedly to receive the
* entire buffer. Polled mode is the default mode of operation for the device.
*
* In interrupt mode, this function will start the receiving, if not the entire
* buffer has been received, the interrupt handler will continue receiving data
* until the entire buffer has been received. A callback function, as specified
* by the application, will be called to indicate the completion of the
* receiving or error conditions.
*
* @param	InstancePtr is a pointer to the XUartPsv instance
* @param	BufferPtr is pointer to buffer for data to be received into
* @param	NumBytes is the number of bytes to be received. A value of
*		zero will stop a previous receive operation that is in
*		progress in interrupt mode.
*
* @return	The number of bytes received.
*
* @note
* The number of bytes is not asserted so that this function may be called
* with a value of zero to stop an operation that is already in progress.
*
******************************************************************************/
u32 XUartPsv_Recv(XUartPsv *InstancePtr, u8 *BufferPtr, u32 NumBytes)
{
	u32 ReceivedCount;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Setup the buffer parameters */
	InstancePtr->ReceiveBuffer.RequestedBytes = NumBytes;
	InstancePtr->ReceiveBuffer.RemainingBytes = NumBytes;
	InstancePtr->ReceiveBuffer.NextBytePtr = BufferPtr;

	/* Receive the data from the device */
	ReceivedCount = XUartPsv_ReceiveBuffer(InstancePtr);

	return ReceivedCount;
}

/*****************************************************************************/
/**
*
* This function sends a buffer that has been previously specified by setting
* up the instance variables of the instance. This function is an internal
* function for the XUartPsv driver such that it may be called from a shell
* function that sets up the buffer or from an interrupt handler.
*
* This function sends the specified buffer in either polled or interrupt
* driven modes. This function is non-blocking.
*
* In a polled mode, this function only sends as much data as the TX FIFO
* can buffer. The application may need to call it repeatedly to send the
* entire buffer.
*
* In interrupt mode, this function starts the sending of the buffer, if not
* the entire buffer has been sent, then the interrupt handler continues the
* sending until the entire buffer has been sent. A callback function, as
* specified by the application, will be called to indicate the completion of
* sending.
*
* @param	InstancePtr is a pointer to the XUartPsv instance
*
* @return	The number of bytes actually sent
*
* @note 	None.
*
******************************************************************************/
u32 XUartPsv_SendBuffer(XUartPsv *InstancePtr)
{
	u32 SentCount = 0U;

	/*
	 * Check is TX busy
	 */
	while (XUartPsv_IsTransmitbusy(InstancePtr->Config.BaseAddress));

	/*
	 * If the TX FIFO is full, send nothing.
	 * Otherwise put bytes into the TX FIFO unil it is full, or all of
	 * the data has been put into the FIFO.
	 */
	while ((!XUartPsv_IsTransmitFull(InstancePtr->Config.BaseAddress)) &&
		   (InstancePtr->SendBuffer.RemainingBytes > SentCount)) {

		/* Fill the FIFO from the buffer */
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTDR_OFFSET, ((u32)InstancePtr->
				SendBuffer.NextBytePtr[SentCount]));

		/* Increment the send count. */
		SentCount++;
	}

	/* Update the buffer to reflect the bytes that were sent from it */
	InstancePtr->SendBuffer.NextBytePtr += SentCount;
	InstancePtr->SendBuffer.RemainingBytes -= SentCount;

	return SentCount;
}

/*****************************************************************************/
/**
*
* This function receives a buffer that has been previously specified by
* setting up the instance variables of the instance. This function is an
* internal function, and it may be called from a shell function that sets up
* the buffer or from an interrupt handler.
*
* This function attempts to receive a specified number of bytes from the
* device and store it into the specified buffer. This function works for
* either polled or interrupt driven modes. It is non-blocking.
*
* In polled mode, this function only receives as much data as in the RX FIFO.
* The application may need to call it repeatedly to receive the entire
* buffer. Polled mode is the default mode for the driver.
*
* In interrupt mode, this function starts the receiving, if not the entire
* buffer has been received, the interrupt handler will continue until the
* entire buffer has been received. A callback function, as specified by the
* application, will be called to indicate the completion of the receiving or
* error conditions.
*
* @param	InstancePtr is a pointer to the XUartPsv instance
*
* @return	The number of bytes received.
*
* @note 	None.
*
******************************************************************************/
u32 XUartPsv_ReceiveBuffer(XUartPsv *InstancePtr)
{
	u32 FlagRegister;
	u32 ReceivedCount = 0U;

	/*
	 * Read the Flag Register to determine if there is any data in
	 * the RX FIFO
	 */
	FlagRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTFR_OFFSET);

	/*
	 * Loop until there is no more data in RX FIFO or the specified
	 * number of bytes has been received
	 */
	while ((ReceivedCount <= InstancePtr->ReceiveBuffer.RemainingBytes) &&
		(((FlagRegister & XUARTPSV_UARTFR_RXFE) == (u32)0))) {

		InstancePtr->ReceiveBuffer.NextBytePtr[ReceivedCount] =
				(u8)XUartPsv_ReadReg(InstancePtr->Config.
					BaseAddress, XUARTPSV_UARTDR_OFFSET);

		ReceivedCount++;

		FlagRegister = XUartPsv_ReadReg(InstancePtr->Config.
						BaseAddress,
						XUARTPSV_UARTFR_OFFSET);
	}

	/*
	 * Update the receive buffer to reflect the number of bytes just
	 * received
	 */
	if (InstancePtr->ReceiveBuffer.NextBytePtr != NULL) {
		InstancePtr->ReceiveBuffer.NextBytePtr += ReceivedCount;
	}
	InstancePtr->ReceiveBuffer.RemainingBytes -= ReceivedCount;

	return ReceivedCount;
}

/*****************************************************************************/
/**
*
* Sets the baud rate for the device. Checks the input value for
* validity and also verifies that the requested rate can be configured to
* within the maximum error range specified by XUARTPSV_MAX_BAUD_ERROR_RATE.
* If the provided rate is not possible, the current setting is unchanged.
*
* @param	InstancePtr is a pointer to the XUartPsv instance
* @param	BaudRate to be set
*
* @return
*		- XST_SUCCESS if everything configured as expected
*		- XST_UART_BAUD_ERROR if the requested rate is not available
*	  	  because there was too much error
*
* @note 	None.
*
******************************************************************************/
s32 XUartPsv_SetBaudRate(XUartPsv *InstancePtr, u32 BaudRate)
{
	u32 BAUDIDIV_Value;	/* Value for integer baud rate divisor */
	u16 BAUDFDIV_Value;	/* Value for fractional baud rate divisor */
	u32 BAUDDIV_Value;
	u32 CalcBaudRate;	/* Calculated baud rate */
	u32 BaudError;		/* Diff between calculated and requested
						 * baud rate
						 */
	u16 Best_BAUDIDIV = 0U;	/* Best value for integer baud rate
				 * divisor */
	u8 Best_BAUDFDIV = 0U;	/* Best value for fractional baud rate
				 * divisor
				 */
	u32 Best_Error = 0xFFFFFFFFU;
	u32 PercentError;
	u32 InputClk;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(BaudRate <= (u32)XUARTPSV_MAX_RATE);
	Xil_AssertNonvoid(BaudRate >= (u32)XUARTPSV_MIN_RATE);

	/*
	 * Make sure the baud rate is not impossible by large.
	 * Fastest possible baud rate is Input Clock / 16.
	 */
	if ((BaudRate * 16) > InstancePtr->Config.InputClockHz) {
		return XST_UART_BAUD_ERROR;
	}

	InputClk = InstancePtr->Config.InputClockHz;

	/*
	 * Baud rate divisor BAUDDIV = (FUARTCLK/(16xBaud rate))
	 * where FUARTCLK is the UART reference clock frequency
	 * The BAUDDIV is comprised of the integer value (BAUD DIVINT)
	 * and the fractional value (BAUD DIVFRAC)
	 */

	/* Calculate the baud divisor integer value */
	BAUDIDIV_Value = InputClk / (BaudRate * 16);
	BAUDFDIV_Value = InputClk % (BaudRate * 16);

	Best_BAUDIDIV = BAUDIDIV_Value;
	Best_BAUDFDIV = BAUDFDIV_Value;

	if (BAUDFDIV_Value != 0) {
		/*
		 * Determine the fractional Baud rate divider.
		 * It can be 0 to 63.
		 * Loop through all possible combinations
		 */
		for (BAUDFDIV_Value = 1; BAUDFDIV_Value < 64;
				BAUDFDIV_Value++) {

			/*
			 * Multiply BAUDDIV_Value with 64 to avoid
			 * fractional values
			 */
			BAUDDIV_Value = 64 * BAUDIDIV_Value+BAUDFDIV_Value;

			/*
			 * Calculate the baud rate with BAUDDIV_Value divided
			 * by 64
			 */
			CalcBaudRate = (InputClk / (16 * BAUDDIV_Value)) * 64;

			/* Avoid unsigned integer underflow */
			if (BaudRate > CalcBaudRate) {
				BaudError = BaudRate - CalcBaudRate;
			}
			else {
				BaudError = CalcBaudRate - BaudRate;
			}

			/*
			 * Find the calculated baud rate closest to requested
			 *  baud rate.
			 */
			if (Best_Error > BaudError) {
				Best_BAUDIDIV = BAUDIDIV_Value;
				Best_BAUDFDIV = BAUDFDIV_Value;
				Best_Error = BaudError;
			}
		}

		/* Make sure the best error is not too large. */
		PercentError = (Best_Error * 100) / BaudRate;
		if (XUARTPSV_MAX_BAUD_ERROR_RATE < PercentError) {
			return XST_UART_BAUD_ERROR;
		}
	}

	/* Disable the UART */
	XUartPsv_DisableUart(InstancePtr);

	/*
	 * Wait for the end of transmission or reception of the current
	 * character
	 */

	/* Program baud divisor registers */
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTIBRD_OFFSET, Best_BAUDIDIV);
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTFBRD_OFFSET, Best_BAUDFDIV);

	/* Enable TX and RX */

	/* Enable the UART */
	XUartPsv_EnableUart(InstancePtr);
	InstancePtr->BaudRate = BaudRate;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is a stub handler that is the default handler such that if
* the application has not set the handler when interrupts are enabled, this
* function will be called.
*
* @param	CallBackRef is unused by this function.
* @param	Event is unused by this function.
* @param	ByteCount is unused by this function.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
static void XUartPsv_StubHandler(void *CallBackRef, u32 Event,
				 u32 ByteCount)
{
	(void) CallBackRef;
	(void) Event;
	(void) ByteCount;
	/*
	 * Assert occurs always since this is a stub and should never be
	 * called
	 */
	Xil_AssertVoidAlways();
}
/** @} */
