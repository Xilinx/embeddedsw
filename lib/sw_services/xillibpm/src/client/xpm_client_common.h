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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_CLIENT_COMMON_H_
#define XPM_CLIENT_COMMON_H_

#include <xil_types.h>
#include <xstatus.h>
#include <xil_exception.h>
#include <xil_io.h>
#include <xipipsu.h>
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_MODE

#ifndef bool
	#define bool	u8
	#define true	1U
	#define false	0U
#endif

/**
 * XPm_Proc - Processor structure
 */
struct XPm_Proc {
	const u32 DevId;                /**< Device ID */
	const u32 PwrCtrl;              /**< Power Control Register Address */
	const u32 PwrDwnMask;           /**< Power Down Mask */
	XIpiPsu *Ipi;			/**< IPI Instance */
};

extern struct XPm_Proc *PrimaryProc;

#define XPm_Read(addr)			Xil_In32(addr)
#define XPm_Write(addr, value)		Xil_Out32(addr, value)
#define XpmEnableInterrupts()		Xil_ExceptionEnable()
#define XpmDisableInterrupts()		Xil_ExceptionDisable()

#if defined (__aarch64__)
#define XPm_Print(MSG, ...)		xil_printf("APU: "MSG, ##__VA_ARGS__)
#elif defined (__arm__)
extern char ProcName[5];
#define XPm_Print(MSG, ...)		xil_printf("%s: "MSG, ProcName, ##__VA_ARGS__)
#endif

/* Conditional debugging prints */
#ifdef DEBUG_MODE
	#define XPm_Dbg(MSG, ...) 	XPm_Print(MSG, ##__VA_ARGS__);
#else
	#define XPm_Dbg(MSG, ...)	{}
#endif

void XPm_SetPrimaryProc(void);
struct XPm_Proc *XPm_GetProcByDeviceId(u32 DeviceId);
void XPm_ClientSuspend(const struct XPm_Proc *const Proc);
void XPm_ClientWakeUp(const struct XPm_Proc *const Proc);
void XPm_ClientSuspendFinalize(void);
void XPm_ClientAbortSuspend(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_CLIENT_COMMON_H_ */
