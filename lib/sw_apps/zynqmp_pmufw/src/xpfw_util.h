/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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

#ifndef XPFW_UTIL_H_
#define XPFW_UTIL_H_


#include "xil_types.h"
#include "xstatus.h"

#define BIT(n)		(1U << (n))


/**
 * Read Modify Write a register
 */
void XPfw_UtilRMW(u32 RegAddress, u32 Mask, u32 Value);

/**
 * Poll for a set of bits to be set (represented by Mask)
 * or until we TimeOut
 * @param RegAddress is the Address of the Register to be polled
 * @param Mask is the bit mask to poll for in the register value
 * @param TimeOutCount is the value to count down before return failure
 */
XStatus XPfw_UtilPollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount);

/**
 * Poll for a set of bits to be cleared (represented by Mask)
 * or until we TimeOut
 *
 * @param RegAddress is the Address of the Register to be polled
 * @param Mask is the bit mask to poll for in the register value
 * @param TimeOutCount is the value to count down before return failure
 */
XStatus XPfw_UtilPollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount);


/**
 * Wait for a period represented by TimeOut
 * FIXME: Make it more meaningful. Clock Cycles or MilliSeconds
 *
 * @param Timeout is the value to count before we return this function
 */
void XPfw_UtilWait(u32 TimeOutCount);


#endif /* XPFW_UTIL_H_ */
