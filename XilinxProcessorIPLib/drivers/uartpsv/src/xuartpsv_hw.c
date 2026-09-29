/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_hw.c
*
* @brief Implements low-level hardware access functions for the XUartPsv driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
* 1.15 vn   07/30/26  Fix Doxygen warnings
* 1.15 vn   09/10/26  Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup uartpsv Overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xuartpsv_hw.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Sends one byte using the device in polled mode.
*
* Blocks until the data has been put into the TX FIFO register.
*
* @param[in]   BaseAddress Base address of the device.
* @param[in]   Data Byte to send.
*
******************************************************************************/
void XUartPsv_SendByte(UINTPTR BaseAddress, u8 Data)
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
* @brief Receives a byte from the device in polled mode.
*
* Blocks until a byte has been received.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      The data byte received.
*
******************************************************************************/
u8 XUartPsv_RecvByte(UINTPTR BaseAddress)
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
* @brief Resets the UART.
*
* This function is to be implemented if needed.
*
* @param[in]   BaseAddress Base address of the device.
*
* @note        Not implemented.
*
******************************************************************************/
void XUartPsv_ResetHw(UINTPTR BaseAddress)
{
	(void) BaseAddress;
}

#ifdef SDT
#ifdef XPAR_STDIN_IS_UARTPSV
#if !defined(VERSAL_PLM) && !defined(ASUFW)
/*****************************************************************************/
/**
*
* @brief Sends a character to the standard output UART.
*
* @param[in]   c Character to send.
*
******************************************************************************/
void outbyte(char c) {
         XUartPsv_SendByte(STDOUT_BASEADDRESS, c);
}
#endif

/*****************************************************************************/
/**
*
* @brief Receives a character from the standard input UART.
*
* @return      Character received.
*
******************************************************************************/
char inbyte(void) {
         return XUartPsv_RecvByte(STDIN_BASEADDRESS);
}
#endif
#endif
/** @} */
