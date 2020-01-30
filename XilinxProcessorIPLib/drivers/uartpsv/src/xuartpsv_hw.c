/******************************************************************************
*
* Copyright (C) 2017-2020 Xilinx, Inc.  All rights reserved.
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
* @file xuartpsv_hw.c
* @addtogroup uartpsv_v1_2
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
* This function resets UART. (To be implemented if needed)
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
}
/** @} */
