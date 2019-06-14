/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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

int XPsmFw_UtilPollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount)
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

int XPsmFw_UtilPollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount)
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

void XPsmFw_UtilWait(u32 TimeOutCount)
{
	u32 TimeOut = TimeOutCount;
	while (TimeOut > 0U) {
		TimeOut--;
	}
}
