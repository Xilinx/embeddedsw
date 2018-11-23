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
* @file xuartpsv_hw.c
* @addtogroup uartpsv_v1_0
* @{
*
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
#include "xuartpsv_hw.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function sends one byte using the device. This function operates in
* polled mode and blocks until the data has been put into the TX FIFO
* register.
*
* @param	BaseAddress contains the base address of the device.
* @param	Data contains the byte to be sent.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_SendByte(u32 BaseAddress, u8 Data)
{
	/* Wait until there is space in TX FIFO */
	while (XUartPsv_IsTransmitFull(BaseAddress)) {
		;
	}

	/* Write the byte into the TX FIFO */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_UARTDR_OFFSET, (u32)Data);
}

/*****************************************************************************/
/**
*
* This function receives a byte from the device. It operates in polled mode
* and blocks until a byte has received.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The data byte received.
*
* @note 	None.
*
******************************************************************************/
u8 XUartPsv_RecvByte(u32 BaseAddress)
{
	u32 RecievedByte;
	/* Wait until there is data */
	while (!XUartPsv_IsReceiveData(BaseAddress)) {
		;
	}
	RecievedByte = XUartPsv_ReadReg(BaseAddress, XUARTPSV_UARTDR_OFFSET);
	/* Return the byte received */
	return (u8)RecievedByte;
}

/*****************************************************************************/
/**
*
* This function resets UART
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_ResetHw(u32 BaseAddress)
{
	(void) BaseAddress;
#if 0
	TBD
	/* Disable interrupts */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_IDR_OFFSET,
			XUARTPSV_IXR_MASK);

	/* Disable receive and transmit */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_CR_OFFSET,
			((u32)XUARTPSV_CR_RX_DIS |
			(u32)XUARTPSV_CR_TX_DIS));

	/*
	 * Software reset of receive and transmit
	 * This clears the FIFO.
	 */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_CR_OFFSET,
			((u32)XUARTPSV_CR_TXRST | (u32)XUARTPSV_CR_RXRST));

	/* Clear status flags - SW reset wont clear sticky flags. */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_ISR_OFFSET,
			XUARTPSV_IXR_MASK);

	/*
	 * Mode register reset value : All zeroes
	 * Normal mode, even parity, 1 stop bit
	 */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_MR_OFFSET,
			XUARTPSV_MR_CHMODE_NORM);

	/* Rx and TX trigger register reset values */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_RXWM_OFFSET,
			XUARTPSV_RXWM_RESET_VAL);
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_TXWM_OFFSET,
			XUARTPSV_TXWM_RESET_VAL);

	/* Rx timeout disabled by default */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_RXTOUT_OFFSET,
			XUARTPSV_RXTOUT_DISABLE);

	/* Baud rate generator and dividor reset values */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_BAUDGEN_OFFSET,
			XUARTPSV_BAUDGEN_RESET_VAL);
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_BAUDDIV_OFFSET,
			XUARTPSV_BAUDDIV_RESET_VAL);

	/*
	 * Control register reset value -
	 * RX and TX are disable by default
	 */
	XUartPsv_WriteReg(BaseAddress, XUARTPSV_CR_OFFSET,
			((u32)XUARTPSV_CR_RX_DIS |
			(u32)XUARTPSV_CR_TX_DIS |
			(u32)XUARTPSV_CR_STOPBRK));
#endif

}
/** @} */
