/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XPM_COMMON_H_
#define XPM_COMMON_H_

#include "xstatus.h"
#include "xil_io.h"

#include "xplmi_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hack: These will increase code size.  Define them as needed. */
#define xSELF_TEST
#define xSELF_TEST_DEVICE_REQUEST
#define xSELF_TEST_PIN_API
#define xSELF_TEST_RESET_API
#define xSELF_TEST_CLOCK_API
#define xDEBUG_REG_IO

/* Debug logs */
#define PmAlert(...)	XPlmi_Printf(DEBUG_INFO, "%s: ", __func__); XPlmi_Printf(DEBUG_INFO, __VA_ARGS__)
#define PmErr(...)	XPlmi_Printf(DEBUG_INFO, "%s: ", __func__); XPlmi_Printf(DEBUG_INFO, __VA_ARGS__)
#define PmWarn(...)	XPlmi_Printf(DEBUG_INFO, "%s: ", __func__); XPlmi_Printf(DEBUG_INFO, __VA_ARGS__)
#define PmInfo(...)	XPlmi_Printf(DEBUG_INFO, "%s: ", __func__); XPlmi_Printf(DEBUG_INFO, __VA_ARGS__)
#define PmDbg(...)	//XPlmi_Printf(DEBUG_INFO, "%s: ", __func__); XPlmi_Printf(DEBUG_INFO, __VA_ARGS__)

#ifdef __MICROBLAZE__
#define VERIFY(X) \
	if (!(X)) { \
		PmErr("Assert failed: %s\n\r", #X); \
		while (1); \
	}
#else
#define VERIFY(X)	assert(X)
#endif

#ifdef DEBUG_REG_IO

#define PmIn32(ADDR, VAL) \
	(VAL) = XPm_In32(ADDR); \
	PmInfo("Read from 0x%08X: 0x%08X\n\r", ADDR, VAL); \

#define PmOut32(ADDR, VAL) \
	PmInfo("Write to 0x%08X: 0x%08X\n\r", ADDR, VAL); \
	XPm_Out32(ADDR, VAL);

#define PmRmw32(ADDR, MASK, VAL) \
	XPm_RMW32(ADDR, MASK, VAL); \
	PmInfo("RMW: Addr=0x%08X, Mask=0x%08X, Val=0x%08X, Reg=0x%08X\n\r", \
		ADDR, MASK, VAL, XPm_In32(ADDR));

#else

#define PmIn32(ADDR, VAL) \
	(VAL) = XPm_In32(ADDR);

#define PmOut32(ADDR, VAL) \
	XPm_Out32(ADDR, VAL);

#define PmRmw32(ADDR, MASK, VAL) \
	XPm_RMW32(ADDR, MASK, VAL);

#endif

#define BIT(n)		(1U << (n))
// set the first n bits to 1, rest to 0
#define BITMASK(n) ((1ULL << (n)) - 1ULL)
// set width specified bits at offset to 1, rest to 0
#define BITNMASK(offset, width) (BITMASK(width) << offset)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define XPm_Read32			XPm_In32
#define XPm_Write32			XPm_Out32

void *XPm_AllocBytes(u32 Size);

void XPm_Out32(u32 RegAddress, u32 l_Val);

u32 XPm_In32(u32 RegAddress);

/**
 * Read Modify Write a register
 */
void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value);

/**
 * Wait for a period represented by TimeOut
 *
 */
void XPm_Wait(u32 TimeOutCount);

/**
 * Poll for mask for a period represented by TimeOut
 */
XStatus XPm_PollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount);

#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_H_ */
