/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_proc.h
*
* This file contains declarations for PROC C file in PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
*       sn   07/04/2019 Added support for enabling GIC proxy for sysmon
*       kc   07/16/2019 Added PERF macro to print task times
*       kc   07/16/2019 Added logic to determine the IRO frequency
*       kc   08/01/2019 Added PLM and ROM boot times
* 1.02  kc   02/10/2020 Updated scheduler to add/remove tasks
*       ma   02/28/2020 Added support for new error actions
*       kc   03/20/2020 Scheduler frequency is increased to 100ms for QEMU
*       bsv  04/04/2020 Code clean up
*       kc   04/23/2020 Added interrupt support for SEU event
* 1.03  bm   10/14/2020 Code clean up
* 		td   10/19/2020 MISRA C Fixes
* 1.04  td   11/23/2020 MISRA C Rule 10.4 Fixes
*       ma   03/24/2021 Reduced minimum digits of time stamp decimals to 3
* 1.05  bm   07/12/2021 Updated IRO freqency defines
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Removed unnecessary structure
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
* 1.07  bm   01/03/2023 Remove usage of double data type
* 1.08  bm   04/28/2023 Update Trim related macros
* 1.09  ng   07/06/2023 Added support for SDT flow
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_PROC_H
#define XPLMI_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xiomodule.h"
#include "xil_exception.h"
#include "xplmi_gic_interrupts.h"
#include "xplmi_util.h"
#include "xplmi_status.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK	(0x2U)

/* PMC IRO Frequency related macros */
#define XPLMI_PMC_IRO_FREQ_400_MHZ	(400000000U)
#define XPLMI_PMC_IRO_FREQ_130_MHZ	(130000000U)

/** PIT related macros */
#define XPLMI_PIT1_RESET_VALUE		(0xFFFFFFFDU)
#define XPLMI_PIT2_RESET_VALUE		(0xFFFFFFFEU)
#define XPLMI_PIT1_CYCLE_VALUE		((u64)XPLMI_PIT1_RESET_VALUE + 1U)
#define XPLMI_PIT2_CYCLE_VALUE		(XPLMI_PIT2_RESET_VALUE + 1U)
#define XPLMI_PIT1			(0U)
#define XPLMI_PIT2			(1U)
#define XPLMI_PIT3			(2U)
#define XPLMI_IOMODULE_PMC_PIT3_IRQ	(0x5U)
#define XPLMI_PIT_FREQ_DIVISOR_QEMU	(10U)
#define XPLMI_PIT_FREQ_DIVISOR		(100U)
#define XPLMI_MEGA			(1000000U)
#define XPLMI_KILO			(1000U)

#define XPLMI_EFUSE_IRO_TRIM_SLOW	(0U)
#define XPLMI_EFUSE_IRO_TRIM_FAST	(1U)

/**************************** Type Definitions *******************************/
/**
 * @}
 * @endcond
 */

/*
 * Performance measurement structure
 */
typedef struct {
	u64 TPerfMs;	/**< Whole part of time in milliseconds */
	u64 TPerfMsFrac; /**< Fractional part of time in milliseconds */
} XPlmi_PerfTime;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_StartTimer(void);
u64 XPlmi_GetTimerValue(void);
int XPlmi_SetUpInterruptSystem(void);
void XPlmi_MeasurePerfTime(u64 TCur, XPlmi_PerfTime *PerfTime);
void XPlmi_PlmIntrEnable(u32 IntrId);
int XPlmi_PlmIntrDisable(u32 IntrId);
int XPlmi_PlmIntrClear(u32 IntrId);
int XPlmi_RegisterHandler(u32 IntrId, GicIntHandler_t Handler, void *Data);
void XPlmi_PrintRomTime(void);
void XPlmi_PrintPlmTimeStamp(void);
u32 *XPlmi_GetPmcIroFreq(void);
XIOModule *XPlmi_GetIOModuleInst(void);
void XPlmi_IntrHandler(void *CallbackRef);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PROC_H */
