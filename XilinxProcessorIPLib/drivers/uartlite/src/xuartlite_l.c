/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartlite_l.c
*
* @brief Low-level driver functions for the UART Lite device.
*
* Refer to the hardware device specification for more
* details of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm  04/25/02 First release
* 1.12a rpm  07/16/07 Fixed arg type for RecvByte
* 2.00a ktn  10/20/09 The macros have been renamed to remove _m from the name.
* 3.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XUartLite_SendByte,
*                     XUartLite_RecvByte APIs.
* 3.11  adk  12/09/24 Since PLM has custom implementation of outbyte() API
*       	      don't pull it for PLM template app case.
*       ss   07/11/25 Add define for ASUFW as outbyte() specific implementation
*                     for ASUFW is added
* 3.14  vn   08/26/26 Fix Doxygen warnings
* 3.14  vn   09/09/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartlite Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xuartlite_l.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/


/*****************************************************************************/
/**
*
* @brief Sends a single byte using the UART.
*
* It is blocking in that it waits for the transmitter to become non-full
* before it writes the byte to the transmit register.
*
* @param[in]   BaseAddress Base address of the device.
* @param[in]   Data Byte of data to send.
*
******************************************************************************/
void XUartLite_SendByte(UINTPTR BaseAddress, u8 Data)
{
	while (XUartLite_IsTransmitFull(BaseAddress));

	XUartLite_WriteReg(BaseAddress, XUL_TX_FIFO_OFFSET, Data);
}


/*****************************************************************************/
/**
*
* @brief Receives a single byte using the UART.
*
* It is blocking in that it waits for the receiver to become non-empty
* before it reads from the receive register.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      The byte of data received.
*
******************************************************************************/
u8 XUartLite_RecvByte(UINTPTR BaseAddress)
{
	while (XUartLite_IsReceiveEmpty(BaseAddress));

	return (u8)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}

#if defined (SDT) && defined (XPAR_STDIN_IS_UARTLITE)
#if !defined(VERSAL_PLM) && !defined(ASUFW)
/*****************************************************************************/
/**
*
* @brief Sends a single character to the standard output.
*
* @param[in]   c Character to send.
*
******************************************************************************/
void outbyte(char c) {
         XUartLite_SendByte(STDOUT_BASEADDRESS, c);
}
#endif

/*****************************************************************************/
/**
*
* @brief Receives a single character from the standard input.
*
* @return      The received character.
*
******************************************************************************/
char inbyte(void) {
         return XUartLite_RecvByte(STDIN_BASEADDRESS);
}
#endif

/** @} */
