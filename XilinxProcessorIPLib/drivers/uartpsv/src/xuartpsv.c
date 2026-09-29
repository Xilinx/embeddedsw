/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv.c
*
* @brief Implements the interface functions for the XUartPsv driver.
*
* Refer to the header file xuartpsv.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* 1.2  rna  01/20/20  Add function to Program control register following
*		      the sequence mentioned in TRM
* 1.4  rna  03/12/21  Add read,write of LCR in 'XUartPsv_SetBaudRate' from TRM
*           03/15/21  Improve the accuracy of FBRD value
* 1.5  sd   05/04/22  Update the loop XUartPsv_ReceiveBuffer
* 1.8  sd   10/20/22  Add XUartPsv_CleanupRx and XUartPsv_CleanupTx to reset rx
* 		      and Tx buffers.
* 1.14 vlt  01/27/26  Fixed codespell issues
* 1.15 vn   07/30/26  Fix Doxygen warnings
* 1.15 vn   09/10/26  Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartpsv Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xuartpsv.h"
#include "xuartpsv_xfer.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**
 * @brief Maximum baud rate error percentage allowed.
 *
 * The following constant defines the amount of error allowed for
 * a specified baud rate. This error is the difference between the actual
 * baud rate generated using the specified clock and the
 * desired baud rate.
 */
#define XUARTPSV_MAX_BAUD_ERROR_RATE	3U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XUartPsv_StubHandler(void *CallBackRef, u32 Event, u32 ByteCount);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Initializes a specific XUartPsv instance for use.
*
* The data format of the device is setup for 8 data bits, 1 stop bit, and no
* parity by default. The baud rate is set to a default value specified by
* Config->DefaultBaudRate if set, otherwise it is set to 19.2K baud. The
* receive FIFO threshold is set for 8 bytes. The default operating mode of
* the driver is polled mode.
*
* @param[out]  InstancePtr Pointer to the XUartPsv instance.
* @param[in]   Config Reference to a structure containing information
*               about a specific XUartPsv driver.
* @param[in]   EffectiveAddr Device base address in the virtual memory
*               address space. The caller is responsible for keeping the
*               address mapping from EffectiveAddr to the device physical base
*               address unchanged once this function is invoked. Unexpected
*               errors may occur if the address mapping changes after this
*               function is called. If address translation is not used, pass
*               in the physical address instead.
*
* @return
*              - XST_SUCCESS if initialization was successful.
*              - XST_UART_BAUD_ERROR if the baud rate is not possible because
*                the inputclock frequency is not divisible with an acceptable
*                amount of error.
*
* @note
* The default configuration for the UART after initialization is:
*
* - 19,200 bps or XPAR_DFT_BAUDRATE if defined
* - 8 data bits
* - 1 stop bit
* - no parity
* - FIFO's are enabled with a threshold of 16 bytes
*
*   All interrupts are disabled.
*
******************************************************************************/
s32 XUartPsv_CfgInitialize(XUartPsv *InstancePtr,
			XUartPsv_Config * Config, UINTPTR EffectiveAddr)
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
				((u32)XUARTPSV_UARTIFLS_RXIFLSEL_1_2 |
				(u32)XUARTPSV_UARTIFLS_TXIFLSEL_1_2));

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

		Status = (s32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* @brief Sends the specified buffer in polled or interrupt driven mode.
*
* This function is non-blocking. If the device is busy sending data, it
* returns and indicates zero bytes were sent. Otherwise, it fills the TX
* FIFO as much as it can, and returns the number of bytes sent.
*
* In polled mode, this function only sends as much data as the TX FIFO can
* buffer. The application may need to call it repeatedly to send the entire
* buffer.
*
* In interrupt mode, this function starts sending the specified buffer,
* then the interrupt handler continues sending data until the entire
* buffer has been sent. A callback function, as specified by the application,
* is called to indicate the completion of sending.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
* @param[in]     BufferPtr Pointer to a buffer of data to send.
* @param[in]     NumBytes Number of bytes to send. A value of
*                 zero stops a previous send operation that is in progress
*                 in interrupt mode. Any data that was already put into the
*                 transmit FIFO is sent.
*
* @return        The number of bytes actually sent.
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
 * @brief Sends a buffer previously specified by setting up the instance
 * variables of the instance.
 *
 * Implemented for RTOS environment as an internal function for the
 * XUartPsv driver, called from XUartPsv_Send_NoWait which sets up
 * the buffer.
 *
 * Sends the specified buffer in polled mode. Only sends as much data
 * as the TX FIFO can buffer. The application may need to call it
 * repeatedly to send the entire buffer.
 *
 * @param[in,out] InstancePtr Pointer to the XUartPsv instance.
 *
 * @return        The number of bytes actually sent.
 *
 ******************************************************************************/
static u32 XUartPsv_SendBuffer_NoWait(XUartPsv *InstancePtr)
{
	u32 SentCount = 0U;

	/*
	 * If the TX FIFO is full, send nothing.
	 * Otherwise put bytes into the TX FIFO until it is full, or all of
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
 * @brief Sends the specified buffer in polled mode without waiting.
 *
 * This function is non-blocking. If the device is busy sending data, it
 * returns and indicates zero bytes were sent. Otherwise, it fills the TX
 * FIFO as much as it can, and returns the number of bytes sent. The
 * application may need to call it repeatedly to send the entire buffer.
 *
 * Implemented for RTOS environment. It checks status and returns without
 * waiting. The caller of XUartPsv_Send_NoWait can wait based on the
 * return value.
 *
 * @param[in,out] InstancePtr Pointer to the XUartPsv instance.
 * @param[in]     BufferPtr Pointer to a buffer of data to send.
 * @param[in]     NumBytes Number of bytes to send. A value of
 *                 zero stops a previous send operation that is in progress
 *                 in interrupt mode. Any data that was already put into the
 *                 transmit FIFO is sent.
 *
 * @return
 *                - The number of bytes actually sent, if data successfully
 *                  sent.
 *                - 0, if TX is busy. The caller of XUartPsv_Send_NoWait can
 *                  wait if return is 0.
 *
 * @note
 * The number of bytes is not asserted so that this function may be called with
 * a value of zero to stop an operation that is already in progress.
 * <br><br>
 *
 ******************************************************************************/
u32 XUartPsv_Send_NoWait(XUartPsv *InstancePtr, u8 *BufferPtr, u32 NumBytes)
{
        u32 BytesSent;
	u32 IsBusy;

        /* Asserts validate the input arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(BufferPtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check TX busy, return 0 if TX busy.
	 */

	IsBusy = (u32)XUartPsv_IsTransmitbusy(InstancePtr->Config.BaseAddress);
	if (IsBusy == (u32)TRUE) {
		return 0;
	}

        /* Setup the buffer parameters */
        InstancePtr->SendBuffer.RequestedBytes = NumBytes;
        InstancePtr->SendBuffer.RemainingBytes = NumBytes;
        InstancePtr->SendBuffer.NextBytePtr = BufferPtr;

        /* Send data to the device */
        BytesSent = XUartPsv_SendBuffer_NoWait(InstancePtr);

        return BytesSent;
}

/*****************************************************************************/
/**
*
* @brief Attempts to receive a specified number of bytes of data from the
* device and store it into the specified buffer.
*
* Works for both polled and interrupt driven modes. It is non-blocking.
*
* In polled mode, this function only receives the data already in the
* RX FIFO. The application may need to call it repeatedly to receive the
* entire buffer. Polled mode is the default mode of operation for the device.
*
* In interrupt mode, this function starts the receiving. If not the entire
* buffer has been received, the interrupt handler continues receiving data
* until the entire buffer has been received. A callback function, as specified
* by the application, is called to indicate completion of the
* receiving or error conditions.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
* @param[out]    BufferPtr Pointer to buffer to receive data into.
* @param[in]     NumBytes Number of bytes to receive. A value of
*                 zero stops a previous receive operation that is in
*                 progress in interrupt mode.
*
* @return        The number of bytes received.
*
* @note
* The number of bytes is not asserted so that this function may be called
* with a value of zero to stop an operation that is already in progress.
*
******************************************************************************/
u32 XUartPsv_Recv(XUartPsv *InstancePtr, u8 *BufferPtr, u32 NumBytes)
{
	u32 ReceivedCount = 0U;
	u32 IntrMask;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Setup the buffer parameters */
	InstancePtr->ReceiveBuffer.RequestedBytes = NumBytes;
	InstancePtr->ReceiveBuffer.RemainingBytes = NumBytes;
	InstancePtr->ReceiveBuffer.NextBytePtr = BufferPtr;

	IntrMask = XUartPsv_GetInterruptMask(InstancePtr);
	/* Receive the data from the device */
	if (IntrMask == 0 )
		ReceivedCount = XUartPsv_ReceiveBuffer(InstancePtr);

	return ReceivedCount;
}

/*****************************************************************************/
/**
*
* @brief Sends a buffer previously specified by setting up the instance
* variables of the instance.
*
* This function sends the buffer in either polled or interrupt driven modes.
*
* This is an internal function for the XUartPsv driver. It may be called
* from a shell function that sets up the buffer or from an interrupt handler.
* This function is non-blocking.
*
* In polled mode, this function only sends as much data as the TX FIFO
* can buffer. The application may need to call it repeatedly to send the
* entire buffer.
*
* In interrupt mode, this function starts the sending of the buffer. If not
* the entire buffer has been sent, then the interrupt handler continues the
* sending until the entire buffer has been sent. A callback function, as
* specified by the application, is called to indicate the completion of
* sending.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
*
* @return        The number of bytes actually sent.
*
******************************************************************************/
u32 XUartPsv_SendBuffer(XUartPsv *InstancePtr)
{
	u32 SentCount = 0U;
	s32 Status;

	/*
	 * Check is TX busy
	 */
	Status = (int)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) +
			XUARTPSV_UARTFR_OFFSET), XUARTPSV_UARTFR_BUSY, 0,
			TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * If the TX FIFO is full, send nothing.
	 * Otherwise put bytes into the TX FIFO until it is full, or all of
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

END:
	return SentCount;
}

/*****************************************************************************/
/**
*
* @brief Receives a buffer previously specified by setting up the instance
* variables of the instance.
*
* This is an internal function. It may be called from a shell function that
* sets up the buffer or from an interrupt handler.
*
* Attempts to receive a specified number of bytes from the device and store
* them into the specified buffer. Works for either polled or interrupt
* driven modes. It is non-blocking.
*
* In polled mode, this function only receives as much data as in the RX FIFO.
* The application may need to call it repeatedly to receive the entire
* buffer. Polled mode is the default mode for the driver.
*
* In interrupt mode, this function starts the receiving. If not the entire
* buffer has been received, the interrupt handler continues until the
* entire buffer has been received. A callback function, as specified by the
* application, is called to indicate the completion of the receiving or
* error conditions.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
*
* @return        The number of bytes received.
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
	while ((ReceivedCount < InstancePtr->ReceiveBuffer.RemainingBytes) &&
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
* @brief Sets the baud rate for the device.
*
* Checks the input value for validity and also verifies that the requested
* rate can be configured to within the maximum error range specified by
* XUARTPSV_MAX_BAUD_ERROR_RATE. If the provided rate is not possible, the
* current setting is unchanged.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
* @param[in]     BaudRate Baud rate to set.
*
* @return
*                - XST_SUCCESS if everything configured as expected.
*                - XST_UART_BAUD_ERROR if the requested rate is not available
*                  because there was too much error.
*
******************************************************************************/
s32 XUartPsv_SetBaudRate(XUartPsv *InstancePtr, u32 BaudRate)
{
	u16 BAUDIDIV_Value;	/* Value for integer baud rate divisor */
	u32 BAUDFDIV_Value;	/* Value for fractional baud rate divisor */
	u32 BAUDDIV_Value;
	u32 CalcBaudRate;	/* Calculated baud rate */
	u32 BaudError;		/* Diff between calculated and requested
						 * baud rate
						 */
	u16 Best_BAUDIDIV = 0U;	/* Best value for integer baud rate
				 * divisor */
	u32 Best_BAUDFDIV = 0U;	/* Best value for fractional baud rate
				 * divisor
				 */
	u32 Best_Error = 0xFFFFFFFFU;
	u32 PercentError;
	u32 InputClk;
	u32 Temp;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(BaudRate <= (u32)XUARTPSV_MAX_RATE);
	Xil_AssertNonvoid(BaudRate >= (u32)XUARTPSV_MIN_RATE);

	/*
	 * Make sure the baud rate is not impossible by large.
	 * Fastest possible baud rate is Input Clock / 16.
	 */
	if ((BaudRate * 16U) > InstancePtr->Config.InputClockHz) {
		return (s32)XST_UART_BAUD_ERROR;
	}

	InputClk = InstancePtr->Config.InputClockHz;

	/*
	 * Baud rate divisor BAUDDIV = (FUARTCLK/(16xBaud rate))
	 * where FUARTCLK is the UART reference clock frequency
	 * The BAUDDIV is comprised of the integer value (BAUD DIVINT)
	 * and the fractional value (BAUD DIVFRAC)
	 */

	/* Calculate the baud divisor integer value */
	BAUDIDIV_Value = (u16)(InputClk / (BaudRate * 16U));
	BAUDFDIV_Value = (u32)(InputClk % (BaudRate * 16U));

	Best_BAUDIDIV = BAUDIDIV_Value;
	Best_BAUDFDIV = BAUDFDIV_Value;

	if (BAUDFDIV_Value != 0U) {
		/*
		 * Determine the fractional Baud rate divider.
		 * It can be 0 to 63.
		 * Loop through all possible combinations
		 */
		for (BAUDFDIV_Value = 1; BAUDFDIV_Value < 64U;
				BAUDFDIV_Value++) {

			/*
			 * Multiply BAUDDIV_Value with 64 to avoid
			 * fractional values
			 */
			BAUDDIV_Value = (64U * (u32)BAUDIDIV_Value)+BAUDFDIV_Value;

			/*
			 * Calculate the baud rate with BAUDDIV_Value divided
			 * by 64
			 */
			CalcBaudRate = (InputClk / (16U * BAUDDIV_Value)) * 64U;

			/* Avoid unsigned integer underflow */
			if (BaudRate > CalcBaudRate) {
				BaudError = BaudRate - CalcBaudRate;
			}
			else {
				BaudError = CalcBaudRate - BaudRate;
			}

			/*
			 * Find the calculated baud rate closest to requested
			 * baud rate. For the same 'Best_Error', take the maximum
			 * 'BAUDFDIV_Value'.
			 */
			if (Best_Error >= BaudError) {
				Best_BAUDIDIV = BAUDIDIV_Value;
				Best_BAUDFDIV = BAUDFDIV_Value;
				Best_Error = BaudError;
			}
		}

		/* Make sure the best error is not too large. */
		PercentError = (Best_Error * 100U) / BaudRate;
		if (XUARTPSV_MAX_BAUD_ERROR_RATE < PercentError) {
			return (s32)XST_UART_BAUD_ERROR;
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

	/* As per TRM, do write of LCR after writing to baud rate registers */
	Temp = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress, XUARTPSV_UARTLCR_OFFSET);
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, Temp);

	/* Enable TX and RX */

	/* Enable the UART */
	XUartPsv_EnableUart(InstancePtr);
	InstancePtr->BaudRate = BaudRate;

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Reprograms the control register per the TRM sequence.
 *
 * Sequence to Program Control Register:
 * 	1. Disable UART
 * 	2. Check if Busy
 * 	3. Flush the transmit FIFO
 * 	4. Program the Control Register
 * 	5. Enable the UART
 *
 * @param[in]   InstancePtr Pointer to the XUartPsv instance.
 * @param[in]   CtrlRegister Value to write.
 *
 ******************************************************************************/
void XUartPsv_ProgramCtrlReg(XUartPsv *InstancePtr, u32 CtrlRegister)
{
	u32 LineCtrlRegister;
	u32 TempCtrlRegister;
	u32 IsBusy;

	/*
	 * Check is TX completed. If Uart is disabled in the middle, cannot
	 * recover. So, keep this check before disable.
	 */
	IsBusy = (u32)XUartPsv_IsTransmitbusy(InstancePtr->Config.BaseAddress);
	while (IsBusy == (u32)TRUE) {
		IsBusy = (u32)XUartPsv_IsTransmitbusy(InstancePtr->Config.BaseAddress);
	}

	/* Disable UART */
	TempCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET);

	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, TempCtrlRegister & (~XUARTPSV_UARTCR_UARTEN));

	/*
	 * Flush the transmit FIFO by setting the FEN bit to 0 in the
	 * Line Control Register
	 */
	LineCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTLCR_OFFSET);

	LineCtrlRegister &= ~XUARTPSV_UARTLCR_FEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);


	/* Setup the Control Register with the passed argument.*/
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

	/* By default, driver works in FIFO mode, so set FEN as it is
	 * cleared above
	 */
	LineCtrlRegister |= XUARTPSV_UARTLCR_FEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);

	/* Enable UART */
	TempCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET);
	TempCtrlRegister |= XUARTPSV_UARTCR_UARTEN;
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, TempCtrlRegister);
}

/*****************************************************************************/
/**
*
* @brief Resets the Rx NextBytePtr, RemainingBytes and RequestedBytes.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
*
******************************************************************************/
void XUartPsv_CleanupRx(XUartPsv *InstancePtr)
{
	InstancePtr->ReceiveBuffer.NextBytePtr = NULL;
	InstancePtr->ReceiveBuffer.RemainingBytes = 0U;
	InstancePtr->ReceiveBuffer.RequestedBytes = 0U;
}

/*****************************************************************************/
/**
*
* @brief Resets the Tx NextBytePtr, RemainingBytes and RequestedBytes.
*
* @param[in,out] InstancePtr Pointer to the XUartPsv instance.
*
******************************************************************************/
void XUartPsv_CleanupTx(XUartPsv *InstancePtr)
{
	InstancePtr->SendBuffer.NextBytePtr = NULL;
	InstancePtr->SendBuffer.RemainingBytes = 0U;
	InstancePtr->SendBuffer.RequestedBytes = 0U;

}

/*****************************************************************************/
/**
*
* @brief Provides a default stub handler for interrupt callbacks.
*
* If the application has not set the handler when interrupts are enabled,
* this function is called.
*
* @param[in]   CallBackRef Unused by this function.
* @param[in]   Event Unused by this function.
* @param[in]   ByteCount Unused by this function.
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
