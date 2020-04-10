/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_l.c
* @addtogroup uartlite_v3_4
* @{
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.

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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xuartlite_l.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/


/****************************************************************************/
/**
*
* This functions sends a single byte using the UART. It is blocking in that it
* waits for the transmitter to become non-full before it writes the byte to
* the transmit register.
*
* @param	BaseAddress is the base address of the device
* @param	Data is the byte of data to send
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUartLite_SendByte(UINTPTR BaseAddress, u8 Data)
{
	while (XUartLite_IsTransmitFull(BaseAddress));

	XUartLite_WriteReg(BaseAddress, XUL_TX_FIFO_OFFSET, Data);
}


/****************************************************************************/
/**
*
* This functions receives a single byte using the UART. It is blocking in that
* it waits for the receiver to become non-empty before it reads from the
* receive register.
*
* @param	BaseAddress is the base address of the device
*
* @return	The byte of data received.
*
* @note		None.
*
******************************************************************************/
u8 XUartLite_RecvByte(UINTPTR BaseAddress)
{
	while (XUartLite_IsReceiveEmpty(BaseAddress));

	return (u8)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}

/** @} */
