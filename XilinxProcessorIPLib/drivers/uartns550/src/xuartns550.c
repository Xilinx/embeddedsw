/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550.c
* @addtogroup uartns550_v3_7
* @{
*
* This file contains the required functions for the 16450/16550 UART driver.
* Refer to the header file xuartns550.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.00b rmm  05/14/03 Fixed diab compiler warnings relating to asserts.
* 1.01a jvb  12/13/05 I changed Initialize() into CfgInitialize(), and made
*                     CfgInitialize() take a pointer to a config structure
*                     instead of a device id. I moved Initialize() into
*                     xgpio_sinit.c, and had Initialize() call CfgInitialize()
*                     after it retrieved the config structure using the device
*                     id. I removed include of xparameters.h along with any
*                     dependencies on xparameters.h and the _g.c config table.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions. XUartNs550_mClearStats
*		      macro is removed, XUartNs550_ClearStats function should be
*		      used in its place.
* 2.01a bss  01/13/12 Removed unnecessary read of the LCR register in the
*                     XUartNs550_CfgInitialize function. Removed compiler
*		      warnings for unused variables in the
*		      XUartNs550_StubHandler.
* 3.3	nsk  04/13/15 Fixed Clock Divisor Enhancement.
*		      (CR 857013)
* 3.4   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XUartNs550_CfgInitialize API.
* 3.7   sd   03/02/20 Update the macro names.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xuartns550.h"
#include "xuartns550_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/* The following constant defines the amount of error that is allowed for
 * a specified baud rate. This error is the difference between the actual
 * baud rate that will be generated using the specified clock and the
 * desired baud rate.
 */
#define XUN_MAX_BAUD_ERROR_RATE		3	 /* max % error allowed */

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

static void XUartNs550_StubHandler(void *CallBackRef, u32 Event,
					unsigned int ByteCount);

/****************************************************************************/
/**
*
* Initializes a specific XUartNs550 instance such that it is ready to be used.
* The data format of the device is setup for 8 data bits, 1 stop bit, and no
* parity by default. The baud rate is set to a default value specified by
* Config->DefaultBaudRate if set, otherwise it is set to 19.2K baud. If the
* device has FIFOs (16550), they are enabled and the a receive FIFO threshold
* is set for 8 bytes. The default operating mode of the driver is polled mode.
*
* @param 	InstancePtr is a pointer to the XUartNs550 instance.
* @param 	Config is a reference to a structure containing information
*		about a specific UART 16450/16550 device. XUartNs550_Init
*		initializes an InstancePtr object for a specific device
*		specified by the contents f Config. XUartNs550_Init can
*		initialize multiple instance objects with the use of multiple
*		calls giving different Config information on each call.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address	space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used,
*		use Config->BaseAddress for this parameters, passing the
*		physical address instead.
*
* @return
* 		- XST_SUCCESS if initialization was successful.
* 		- XST_UART_BAUD_ERROR if the baud rate is not possible because
*		the input clock frequency is not divisible with an acceptable
*		amount of error.
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_CfgInitialize(XUartNs550 *InstancePtr,
					XUartNs550_Config *Config,
					UINTPTR EffectiveAddr)
{
	int Status;
	u32 BaudRate;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Setup the data that is from the configuration information
	 */
	InstancePtr->BaseAddress = EffectiveAddr;
	InstancePtr->InputClockHz = Config->InputClockHz;

	/*
	 * Initialize the instance data to some default values and setup
	 * a default handler
	 */
	InstancePtr->Handler = XUartNs550_StubHandler;

	InstancePtr->SendBuffer.NextBytePtr = NULL;
	InstancePtr->SendBuffer.RemainingBytes = 0;
	InstancePtr->SendBuffer.RequestedBytes = 0;

	InstancePtr->ReceiveBuffer.NextBytePtr = NULL;
	InstancePtr->ReceiveBuffer.RemainingBytes = 0;
	InstancePtr->ReceiveBuffer.RequestedBytes = 0;

	/*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Set the default Baud rate here, can be changed prior to
	 * starting the device
	 */
	BaudRate = Config->DefaultBaudRate;
	if (! BaudRate) {
		BaudRate = 19200;
	}

	Status = XUartNs550_SetBaudRate(InstancePtr, BaudRate);
	if (Status != XST_SUCCESS) {
		InstancePtr->IsReady = 0;
		return Status;
	}


	/*
	 * Set up the default format for the data, 8 bit data, 1 stop bit,
	 * no parity
	 */
	XUartNs550_SetLineControlReg(InstancePtr->BaseAddress,
						XUN_FORMAT_8_BITS);

	/*
	 * Enable the FIFOs assuming they are present and set the receive FIFO
	 * trigger level for 8 bytes assuming that this will work best with most
	 * baud rates, enabling the FIFOs also clears them, note that this must
	 * be done with 2 writes, 1st enabling the FIFOs then set the trigger
	 * level
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress,
				XUN_FCR_OFFSET, XUN_FIFO_ENABLE);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_FCR_OFFSET,
				XUN_FIFO_ENABLE | XUN_FIFO_RX_TRIG_MSB);
	/*
	 * Clear the statistics for this driver
	 */
	XUartNs550_ClearStats(InstancePtr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This functions sends the specified buffer of data using the UART in either
* polled or interrupt driven modes. This function is non-blocking such that it
* will return before the data has been sent by the UART. If the UART is busy
* sending data, it will return and indicate zero bytes were sent.
*
* In a polled mode, this function will only send as much data as the UART can
* buffer, either in the transmitter or in the FIFO if present and enabled. The
* application may need to call it repeatedly to send a buffer.
*
* In interrupt mode, this function will start sending the specified buffer and
* then the interrupt handler of the driver will continue sending data until the
* buffer has been sent. A callback function, as specified by the application,
* will be called to indicate the completion of sending the buffer.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	BufferPtr is pointer to a buffer of data to be sent.
* @param	NumBytes contains the number of bytes to be sent. A value of
*		zero will stop a previous send operation that is in progress
*		in interrupt mode. Any data that was already put into the
*		transmit FIFO will be sent.
*
* @return	The number of bytes actually sent.
*
* @note
*
* The number of bytes is not asserted so that this function may be called with
* a value of zero to stop an operation that is already in progress.
* <br><br>
* This function and the XUartNs550_SetOptions() function modify shared data
* such that there may be a need for mutual exclusion in a multithreaded
* environment and if XUartNs550_SetOptions() if called from a handler.
*
*****************************************************************************/
unsigned int XUartNs550_Send(XUartNs550 *InstancePtr, u8 *BufferPtr,
					unsigned int NumBytes)
{
	unsigned int BytesSent;
	u32 IerRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(((signed)NumBytes) >= 0);

	/*
	 * Enter a critical region by disabling the UART transmit interrupts to
	 * allow this call to stop a previous operation that may be interrupt
	 * driven, only stop the transmit interrupt since this critical region
	 * is not really exited in the normal manner
	 */
	IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_IER_OFFSET);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				IerRegister & ~XUN_IER_TX_EMPTY);

	/*
	 * Setup the specified buffer to be sent by setting the instance
	 * variables so it can be sent with polled or interrupt mode
	 */
	InstancePtr->SendBuffer.RequestedBytes = NumBytes;
	InstancePtr->SendBuffer.RemainingBytes = NumBytes;
	InstancePtr->SendBuffer.NextBytePtr = BufferPtr;

	/*
	 * Send the buffer using the UART and return the number of bytes sent
	 */

	BytesSent = XUartNs550_SendBuffer(InstancePtr);

	/*
	 * The critical region is not exited in this function because of the way
	 * the transmit interrupts work.  The other function called enables the
	 * tranmit interrupt such that this function can't restore a value to
	 * the interrupt enable register and does not need to exit the critical
	 * region
	 */
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
* can buffer, either in the receiver or in the FIFO if present and enabled.
* The application may need to call it repeatedly to receive a buffer. Polled
* mode is the default mode of operation for the driver.
*
* In interrupt mode, this function will start receiving and then the interrupt
* handler of the driver will continue receiving data until the buffer has been
* received. A callback function, as specified by the application, will be called
* to indicate the completion of receiving the buffer or when any receive errors
* or timeouts occur. Interrupt mode must be enabled using the SetOptions function.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	BufferPtr is pointer to buffer for data to be received into
* @param	NumBytes is the number of bytes to be received. A value of zero
*		will stop a previous receive operation that is in progress in
*		interrupt mode.
*
* @return	The number of bytes received.
*
* @note
*
* The number of bytes is not asserted so that this function may be called with
* a value of zero to stop an operation that is already in progress.
*
*****************************************************************************/
unsigned int XUartNs550_Recv(XUartNs550 *InstancePtr, u8 *BufferPtr,
				unsigned int NumBytes)
{
	unsigned int ReceivedCount;
	u32 IerRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(((signed)NumBytes) >= 0);

	/*
	 * Enter a critical region by disabling all the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven
	 */
	IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_IER_OFFSET);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET, 0);

	/*
	 * Setup the specified buffer to be received by setting the instance
	 * variables so it can be received with polled or interrupt mode
	 */
	InstancePtr->ReceiveBuffer.RequestedBytes = NumBytes;
	InstancePtr->ReceiveBuffer.RemainingBytes = NumBytes;
	InstancePtr->ReceiveBuffer.NextBytePtr = BufferPtr;

	/*
	 * Receive the data from the UART and return the number of bytes
	 * received
	 */
	ReceivedCount = XUartNs550_ReceiveBuffer(InstancePtr);

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				IerRegister);

	return ReceivedCount;
}

/****************************************************************************/
/**
*
* This function sends a buffer that has been previously specified by setting
* up the instance variables of the instance. This function is designed to be
* an internal function for the XUartNs550 component such that it may be called
* from a shell function that sets up the buffer or from an interrupt handler.
*
* This function sends the specified buffer of data to the UART in either
* polled or interrupt driven modes. This function is non-blocking such that
* it will return before the data has been sent by the UART.
*
* In a polled mode, this function will only send as much data as the UART can
* buffer, either in the transmitter or in the FIFO if present and enabled.
* The application may need to call it repeatedly to send a buffer.
*
* In interrupt mode, this function will start sending the specified buffer and
* then the interrupt handler of the driver will continue until the buffer
* has been sent. A callback function, as specified by the application, will
* be called to indicate the completion of sending the buffer.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	NumBytes is the number of bytes actually sent (put into the
*		UART tranmitter and/or FIFO).
*
* @note		None.
*
*****************************************************************************/
unsigned int XUartNs550_SendBuffer(XUartNs550 *InstancePtr)
{
	unsigned int SentCount = 0;
	unsigned int BytesToSend = 0;   /* default to not send anything */
	unsigned int FifoSize;
	u32 LsrRegister;
	u32 IirRegister;
	u32 IerRegister;

	/*
	 * Read the line status register to determine if the transmitter is
	 * empty
	 */
	LsrRegister = XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);

	/*
	 * If the transmitter is not empty then don't send any data, the empty
	 * room in the FIFO is not available
	 */
	if (LsrRegister & XUN_LSR_TX_BUFFER_EMPTY) {
		/*
		 * Read the interrupt ID register to determine if FIFOs
		 * are enabled
		 */
		IirRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
							XUN_IIR_OFFSET);

		/*
		 * When there are FIFOs, send up to the FIFO size. When there
		 * are no FIFOs, only send 1 byte of data
		 */
		if (IirRegister & XUN_INT_ID_FIFOS_ENABLED) {
			/*
			 * Determine how many bytes can be sent depending on if
			 * the transmitter is empty, a FIFO size of N is really
			 * N - 1 plus the transmitter register
			 */
			if (LsrRegister & XUN_LSR_TX_EMPTY) {
				FifoSize = XUN_FIFO_SIZE;
			} else {
				FifoSize = XUN_FIFO_SIZE - 1;
			}

			/*
			 * FIFOs are enabled, if the number of bytes to send
			 * is less than the size of the FIFO, then send all
			 * bytes, otherwise fill the FIFO
			 */
			if (InstancePtr->SendBuffer.RemainingBytes < FifoSize) {
				BytesToSend =
					InstancePtr->SendBuffer.RemainingBytes;
			} else {
				BytesToSend = FifoSize;
			}
		} else if (InstancePtr->SendBuffer.RemainingBytes > 0) {
			/*
			 * Without FIFOs, we can only send 1 byte. We needed to
			 * check for non-zero remaining bytes in case this
			 * routine was called only to kick the transmitter and
			 * enable the UART interrupt
			 */
			BytesToSend = 1;
		}

		/*
		 * Fill the FIFO if it's present or the transmitter only from
		 * the the buffer that was specified
		 */
		for (SentCount = 0; SentCount < BytesToSend; SentCount++) {
			XUartNs550_WriteReg(InstancePtr->BaseAddress,
						XUN_THR_OFFSET,
				InstancePtr->SendBuffer.NextBytePtr[SentCount]);
		}
	}
	/*
	 * Update the buffer to reflect the bytes that were sent from it
	 */
	InstancePtr->SendBuffer.NextBytePtr += SentCount;
	InstancePtr->SendBuffer.RemainingBytes -= SentCount;

	/*
	 * Increment associated counters
	 */
	 InstancePtr->Stats.CharactersTransmitted += SentCount;

	/*
	 * If interrupts are enabled as indicated by the receive interrupt, then
	 * enable the transmit interrupt, it is not enabled continuously because
	 * it causes an interrupt whenever the FIFO is empty
	 */
	IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_IER_OFFSET);
	if (IerRegister & XUN_IER_RX_DATA) {
		XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				 IerRegister | XUN_IER_TX_EMPTY);
	}
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
* an internal function for the XUartNs550 component such that it may be called
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
* occur. Interrupt mode must be enabled using the SetOptions function.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	The number of bytes received.
*
* @note		None.
*
*****************************************************************************/
unsigned int XUartNs550_ReceiveBuffer(XUartNs550 *InstancePtr)
{
	u32 LsrRegister;
	unsigned int ReceivedCount = 0;

	/*
	 * Loop until there is not more data buffered by the UART or the
	 * specified number of bytes is received
	 */
	while (ReceivedCount < InstancePtr->ReceiveBuffer.RemainingBytes) {

		/*
		 * Read the Line Status Register to determine if there is any
		 * data in the receiver/FIFO
		 */
		LsrRegister =
			XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);

		/*
		 * If there is a break condition then a zero data byte was put
		 * into the receiver, just read it and dump it and update the
		 * stats
		 */
		if (LsrRegister & XUN_LSR_BREAK_INT) {
			(void)XUartNs550_ReadReg(InstancePtr->BaseAddress,
							XUN_RBR_OFFSET);
			XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
		}

		/*
		 * If there is data ready to be removed, then put the next byte
		 * received into the specified buffer and update the stats to
		 * reflect any receive errors for the byte
		 */
		else if (LsrRegister & XUN_LSR_DATA_READY) {
			InstancePtr->ReceiveBuffer.NextBytePtr[ReceivedCount++] =
			XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_RBR_OFFSET);

			XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
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
	 * Update the receive buffer to reflect the number of bytes that was
	 * received
	 */
	InstancePtr->ReceiveBuffer.NextBytePtr += ReceivedCount;
	InstancePtr->ReceiveBuffer.RemainingBytes -= ReceivedCount;

	/*
	 * Increment associated counters in the statistics
	 */
	InstancePtr->Stats.CharactersReceived += ReceivedCount;

	return ReceivedCount;
}

/****************************************************************************
*
* Sets the baud rate for the specified UART. Checks the input value for
* validity and also verifies that the requested rate can be configured to
* within the 3 percent error range for RS-232 communications. If the provided
* rate is not valid, the current setting is unchanged.
*
* This function is designed to be an internal function only used within the
* XUartNs550 component. It is necessary for initialization and for the user
* available function that sets the data format.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	BaudRate to be set in the hardware.
*
* @return
*		- XST_SUCCESS if everything configures as expected
* 		- XST_UART_BAUD_ERROR if the requested rate is not available
*		because there was too much error due to the input clock
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_SetBaudRate(XUartNs550 *InstancePtr, u32 BaudRate)
{

	u32 BaudLSB;
	u32 BaudMSB;
	u32 LcrRegister;
	u32 Divisor;
	u32 TargetRate;
	u32 Error;
	u32 PercentError;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Determine what the divisor should be to get the specified baud
	 * rater based upon the input clock frequency and a baud clock prescaler
	 * of 16
	 */
	Divisor = ((InstancePtr->InputClockHz +((BaudRate * 16UL)/2)) /
			(BaudRate * 16UL));
	/*
	 * check for too much error between the baud rate that will be generated
	 * using the divisor and the expected baud rate, integer division also
	 * truncates always positive
	 */
	TargetRate = Divisor * BaudRate * 16UL;
	if (TargetRate < InstancePtr->InputClockHz) {
		Error = InstancePtr->InputClockHz - TargetRate;
	} else {
		Error = TargetRate - InstancePtr->InputClockHz;
	}
	/*
	 * Error has total error now compute the percentage multiplied by 100 to
	 * avoid floating point calculations, should be less than 3% as per
	 * RS-232 spec
	 */
	PercentError = (Error * 100UL) / InstancePtr->InputClockHz;
	if (PercentError > XUN_MAX_BAUD_ERROR_RATE) {
		return XST_UART_BAUD_ERROR;

	}

	/*
	 * Get the least significant and most significant bytes of the divisor
	 * so they can be written to 2 byte registers
	 */
	BaudLSB = Divisor & XUN_DIVISOR_BYTE_MASK;
	BaudMSB = (Divisor >> 8) & XUN_DIVISOR_BYTE_MASK;

	/*
	 * Save the baud rate in the instance so that the get baud rate function
	 * won't have to calculate it from the divisor
	 */
	InstancePtr->BaudRate = BaudRate;

	/*
	 * Get the line control register contents and set the divisor latch
	 * access bit so the baud rate can be set
	 */
	LcrRegister = XUartNs550_GetLineControlReg(InstancePtr->BaseAddress);
	XUartNs550_SetLineControlReg(InstancePtr->BaseAddress ,
					LcrRegister | XUN_LCR_DLAB);

	/*
	 * Set the baud Divisors to set rate, the initial write of 0xFF is
	 * to keep the divisor from being 0 which is not recommended as per
	 * the NS16550D spec sheet
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_DLL_OFFSET, 0xFF);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_DLM_OFFSET,
				BaudMSB);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_DLL_OFFSET,
				BaudLSB);

	/*
	 * Clear the Divisor latch access bit, DLAB to allow nornal
	 * operation and write to the line control register
	 */
	XUartNs550_SetLineControlReg(InstancePtr->BaseAddress, LcrRegister);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is a stub handler that is the default handler such that if the
* application has not set the handler when interrupts are enabled, this
* function will be called. The function interface has to match the interface
* specified for a handler even though none of the arguments are used.
*
* @param	CallBackRef is unused by this function.
* @param	Event is unused by this function.
* @param	ByteCount is unused by this function.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XUartNs550_StubHandler(void *CallBackRef, u32 Event,
					unsigned int ByteCount)
{

	(void) CallBackRef;
	(void) Event;
	(void) ByteCount;

	/*
	 * Assert occurs always since this is a stub and should never be called
	 */
	Xil_AssertVoidAlways();
}
/** @} */
