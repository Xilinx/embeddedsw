/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartns550_l.c
* @addtogroup uartns550 Overview
* @{
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  04/24/02 First release
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a sdm  09/22/09 Converted all register accesses to 32 bit access.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions.
* 3.3	nsk  04/13/15 Fixed Clock Divisor Enhancement.
*		      (CR 857013)
* 3.4   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XUartNs550_SendByte,
*                     XUartNs550_RecvByte, XUartNs550_SetBaud APIs.
* 3.6   sd   03/02/20 Updated the register macros for DRL and DRM registers.
* 3.12  adk  06/02/25 Since PLM has custom implementation of outbyte() API
*                     don't pull it for PLM template app case.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xuartns550_l.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
*
* This function sends a data byte with the UART. This function operates in the
* polling mode and blocks until the data has been put into the UART transmit
* holding register.
*
* @param	BaseAddress contains the base address of the UART.
* @param	Data contains the data byte to be sent.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUartNs550_SendByte(UINTPTR BaseAddress, u8 Data)
{
	/*
	 * Wait til we know that the byte can be sent, the 550 does not have any
	 * way to tell how much room is in the FIFO such that we must wait for
	 * it to be empty
	 */
	while (!XUartNs550_IsTransmitEmpty(BaseAddress));

	/*
	 * Write the data byte to the UART to be transmitted
	 */
	XUartNs550_WriteReg(BaseAddress, XUN_THR_OFFSET, (u32)Data);
}

/****************************************************************************/
/**
*
* This function receives a byte from the UART. It operates in a polling mode
* and blocks until a byte of data is received.
*
* @param	BaseAddress contains the base address of the UART.
*
* @return	The data byte received by the UART.
*
* @note		None.
*
*****************************************************************************/
u8 XUartNs550_RecvByte(UINTPTR BaseAddress)
{
	/*
	 * Wait for there to be data received
	 */
	while (!XUartNs550_IsReceiveData(BaseAddress));

	/*
	 * Return the next data byte the UART received
	 */
	return (u8) XUartNs550_ReadReg(BaseAddress, XUN_RBR_OFFSET);
}


/****************************************************************************/
/**
*
* Set the baud rate for the UART.
*
* @param	BaseAddress contains the base address of the UART.
* @param	InputClockHz is the frequency of the input clock to the device
*		in Hertz.
* @param	BaudRate is the baud rate to be set.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUartNs550_SetBaud(UINTPTR BaseAddress, u32 InputClockHz, u32 BaudRate)
{

	u32 BaudLSB;
	u32 BaudMSB;
	u32 LcrRegister;
	u32 Divisor;

	/*
	 * Determine what the divisor should be to get the specified baud
	 * rater based upon the input clock frequency and a baud clock prescaler
	 * of 16
	 */
	Divisor = ((InputClockHz +((BaudRate * 16UL)/2)) /
			(BaudRate * 16UL));
	/*
	 * Get the least significant and most significant bytes of the divisor
	 * so they can be written to 2 byte registers
	 */
	BaudLSB = Divisor & XUN_DIVISOR_BYTE_MASK;
	BaudMSB = (Divisor >> 8) & XUN_DIVISOR_BYTE_MASK;

	/*
	 * Get the line control register contents and set the divisor latch
	 * access bit so the baud rate can be set
	 */
	LcrRegister = XUartNs550_GetLineControlReg(BaseAddress);
	XUartNs550_SetLineControlReg(BaseAddress,
					LcrRegister | XUN_LCR_DLAB);

	/*
	 * Set the baud Divisors to set rate, the initial write of 0xFF is to
	 * keep the divisor from being 0 which is not recommended as per the
	 * NS16550D spec sheet
	 */
	XUartNs550_WriteReg(BaseAddress, XUN_DLL_OFFSET, 0xFF);
	XUartNs550_WriteReg(BaseAddress, XUN_DLM_OFFSET, BaudMSB);
	XUartNs550_WriteReg(BaseAddress, XUN_DLL_OFFSET, BaudLSB);

	/*
	 * Clear the Divisor latch access bit, DLAB to allow nornal
	 * operation and write to the line control register
	 */
	XUartNs550_SetLineControlReg(BaseAddress, LcrRegister);
}
#if defined(SDT) && defined(XPAR_STDIN_IS_UARTNS550)
#if !defined(VERSAL_PLM)
void outbyte(char c) {
         XUartNs550_SendByte(STDOUT_BASEADDRESS, c);
}
#endif

char inbyte(void) {
         return XUartNs550_RecvByte(STDIN_BASEADDRESS);
}
#endif
/** @} */
