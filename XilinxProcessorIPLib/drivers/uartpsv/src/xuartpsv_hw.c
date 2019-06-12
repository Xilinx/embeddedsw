/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartpsv_hw.c
* @addtogroup uartpsv_v1_3
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
* This function resets UART. (To be implemented if needed)
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_ResetHw(UINTPTR BaseAddress)
{
	(void) BaseAddress;
}
/** @} */
