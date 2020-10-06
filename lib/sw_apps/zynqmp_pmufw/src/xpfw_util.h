/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_UTIL_H_
#define XPFW_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

#define BIT(n)		((u32)1U << (n))


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
 *
 * @param Timeout is the value to count before we return this function
 */
void XPfw_UtilWait(u32 TimeOutCount);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_UTIL_H_ */
