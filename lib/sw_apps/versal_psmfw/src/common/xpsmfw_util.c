/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_util.c
*
* This file contains read/write utilities for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xil_io.h"
#include "xpsmfw_util.h"

void XPsmFw_UtilRMW(u32 RegAddress, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = Xil_In32(RegAddress);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	Xil_Out32(RegAddress, l_Val);
}

XStatus XPsmFw_UtilPollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;
	/**
	 * Read the Register value
	 */
	l_RegValue = Xil_In32(RegAddress);
	/**
	 * Loop while the MAsk is not set or we timeout
	 */
	while(((l_RegValue & Mask) != Mask) && (TimeOut > 0U)){
		/**
		 * Latch up the Register value again
		 */
		l_RegValue = Xil_In32(RegAddress);
		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);

}

XStatus XPsmFw_UtilPollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;

	l_RegValue = Xil_In32(RegAddress);
	/**
	 * Loop until all bits defined by mask are cleared
	 * or we time out
	 */
	while (((l_RegValue & Mask) != 0U) && (TimeOut > 0U)) {
		/**
		 * Latch up the reg value again
		 */
		l_RegValue = Xil_In32(RegAddress);
		/**
		 * Decrement the timeout count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);

}

/****************************************************************************/
/**
 * @brief	Waits for a specified timeout count
 *
 * @param TimeOutCount	The number of iterations to wait
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_UtilWait(u32 TimeOutCount)
{
	u32 TimeOut = TimeOutCount;
	while (TimeOut > 0U) {
		TimeOut--;
	}
}

/****************************************************************************/
/**
 * @brief	Polls a register until a specified value is read or a timeout occurs
 *
 * @param RegAddress	The address of the register to poll
 * @param Mask		The mask to apply to the register value
 * @param Value		The expected value after masking
 * @param TimeOutUs	The timeout period in microseconds
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *		or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_UtilPollForValue(u32 RegAddress, u32 Mask, u32 Value, u32 TimeOutUs)
{
	XStatus Status = XST_FAILURE;
	u32 TimeOut = TimeOutUs;

	if (TimeOut == 0U){
		Status = ((Xil_In32(RegAddress) & Mask) == Value) ? XST_SUCCESS : XST_FAILURE;
	}else {
		while (((Xil_In32(RegAddress) & Mask) != Value) && (TimeOut > 0U)){
			TimeOut--;
		}
		Status = (0U < TimeOut) ? XST_SUCCESS: XST_TIMEOUT;
	}
	return Status;
}
