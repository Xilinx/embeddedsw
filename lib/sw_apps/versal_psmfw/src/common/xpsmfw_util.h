/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_util.h
*
* This file contains definitions for read/write utilities
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

#ifndef XPSMFW_UTIL_H_
#define XPSMFW_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

#define BIT(n)		(1U << (n))


/**
 * Read Modify Write a register
 */
void XPsmFw_UtilRMW(u32 RegAddress, u32 Mask, u32 Value);

/**
 * Poll for a set of bits to be set (represented by Mask)
 * or until we TimeOut
 * @param RegAddress is the Address of the Register to be polled
 * @param Mask is the bit mask to poll for in the register value
 * @param TimeOutCount is the value to count down before return failure
 */
XStatus XPsmFw_UtilPollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount);

/**
 * Poll for a set of bits to be cleared (represented by Mask)
 * or until we TimeOut
 *
 * @param RegAddress is the Address of the Register to be polled
 * @param Mask is the bit mask to poll for in the register value
 * @param TimeOutCount is the value to count down before return failure
 */
XStatus XPsmFw_UtilPollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount);


/**
 * Wait for a period represented by TimeOut
 * FIXME: Make it more meaningful. Clock Cycles or MilliSeconds
 *
 * @param Timeout is the value to count before we return this function
 */
void XPsmFw_UtilWait(u32 TimeOutCount);

/**
 * Poll for certain value to be set under mask
 * or until we TimeOut
 * @param RegAddress is the Address of the Register to be polled
 * @param Mask is the bit mask to poll for in the register value
 * @param Value is value to check agains after mask
 * @param TimeOutus is timeout value in microsecond
 */
XStatus XPsmFw_UtilPollForValue(u32 RegAddress, u32 Mask, u32 Value, u32 TimeOutUs);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_UTIL_H_ */
