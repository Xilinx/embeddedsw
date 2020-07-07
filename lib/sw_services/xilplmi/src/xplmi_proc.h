/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
* 1.01  ma   02/03/2020 Change Performance measurement functions generic to be
*                       used for logging
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

/************************** Constant Definitions *****************************/
#define IOMODULE_DEVICE_ID XPAR_IOMODULE_0_DEVICE_ID
#define MB_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK	(0x2U)

/* PMC IRO Frequency related macros */
#define XPLMI_PMC_IRO_FREQ_320_MHZ	(320000000U)
#define XPLMI_PMC_IRO_FREQ_130_MHZ	(130000000U)

/** PIT related macros */
#define XPLMI_PIT1_RESET_VALUE		(0xFFFFFFFEU)
#define XPLMI_PIT2_RESET_VALUE		(0xFFFFFFFEU)
#define XPLMI_PIT1			(0U)
#define XPLMI_PIT2			(1U)
#define XPLMI_PIT3			(2U)
#define XPLMI_IOMODULE_PMC_PIT3_IRQ			(0x5U)

/**************************** Type Definitions *******************************/
/*
 * PMC IOmodule interrupts
 */
enum {
	XPLMI_IOMODULE_PMC_GIC_IRQ = 16U,
	XPLMI_IOMODULE_PPU1_MB_RAM, /**< 17U */
	XPLMI_IOMODULE_ERR_IRQ, /**< 18U */
	XPLMI_IOMODULE_RESERVED_19, /**< 19U */
	XPLMI_IOMODULE_CFRAME_SEU, /**< 20U */
	XPLMI_IOMODULE_RESERVED_21, /**< 21U */
	XPLMI_IOMODULE_PMC_GPI, /**< 22U */
	XPLMI_IOMODULE_PL_IRQ, /**< 23U */
	XPLMI_IOMODULE_SSIT_IRQ_2, /**< 24U */
	XPLMI_IOMODULE_SSIT_IRQ_1, /**< 25U */
	XPLMI_IOMODULE_SSIT_IRQ_0, /**< 26U */
	XPLMI_IOMODULE_PWRDN_REQ, /**< 27U */
	XPLMI_IOMODULE_PWRUP_REQ, /**< 28U */
	XPLMI_IOMODULE_SRST_REQ, /**< 29U */
	XPLMI_IOMODULE_ISO_REQ, /**< 30U */
	XPLMI_IOMODULE_WAKEUP_REQ, /**< 31U */
	XPLMI_IOMODULE_MASK = 0xFFU,
};

/*
 * External interrupt mapping
 */
enum {
	XPLMI_CFRAME_SEU = 0U,
	XPLMI_IPI_IRQ,	/**< 1U */
	XPLMI_SBI_DATA_RDY, /**< 2U */
};

/*
 * Performance measurement structure
 */
typedef struct XPlmi_PerfTime {
	u64 TPerfMs;
	u64 TPerfMsFrac;
}XPlmi_PerfTime;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_StartTimer(void);
int XPlmi_InitProc(void);
int XPlmi_InitIOModule(void);
u64 XPlmi_GetTimerValue(void);
int XPlmi_SetUpInterruptSystem(void);
void XPlmi_MeasurePerfTime(u64 TCur, XPlmi_PerfTime * PerfTime);
void XPlmi_PlmIntrEnable(u32 IntrId);
void XPlmi_PlmIntrDisable(u32 IntrId);
void XPlmi_PlmIntrClear(u32 IntrId);
void XPlmi_RegisterHandler(u32 IntrId, Function_t Handler, void * Data);
void XPlmi_PrintRomTime(void);
void XPlmi_PrintPlmTimeStamp(void);
void XPlmi_GetPerfTime(u64 TCur, u64 TStart, XPlmi_PerfTime * PerfTime);

/* Handler Table Structure */
struct HandlerTable {
	XInterruptHandler Handler;
};

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PROC_H */
