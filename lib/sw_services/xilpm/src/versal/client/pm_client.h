/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef PM_CLIENT_H_
#define PM_CLIENT_H_

#include <xil_types.h>
#include <xstatus.h>
#include <xil_exception.h>
#include <xil_io.h>
#include <xipipsu.h>
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL */

/* User needs to enable this macro to enable prints of client library */
/* #define DEBUG_MODE */

/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			(8U)
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		(8U)
#define PM_IPI_TIMEOUT			(~0U)
#define TARGET_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK

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
	#define XPm_Dbg(MSG, ...) 	do { XPm_Print(MSG, ##__VA_ARGS__); } while (0)
#else
	#define XPm_Dbg(MSG, ...)	do {} while (0)
#endif

/* Define below macro to disable error prints for memory constrained applications */
#ifndef DISABLE_ERROR_PRINTS
	#define XPm_Err(MSG, ...) 	do { XPm_Print("ERROR: "MSG, ##__VA_ARGS__); } while (0)
#else
	#define XPm_Err(MSG, ...)	do {} while (0)
#endif

#define pm_print		XPm_Dbg
#define pm_dbg			XPm_Dbg
#define pm_read			XPm_Read
#define pm_write		XPm_Write

void XPm_SetPrimaryProc(void);
struct XPm_Proc *XPm_GetProcByDeviceId(u32 DeviceId);
void XPm_ClientSuspend(const struct XPm_Proc *const Proc);
void XPm_ClientWakeUp(const struct XPm_Proc *const Proc);
void XPm_ClientSuspendFinalize(void);
void XPm_ClientAbortSuspend(void);

/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* PM_CLIENT_H_ */
