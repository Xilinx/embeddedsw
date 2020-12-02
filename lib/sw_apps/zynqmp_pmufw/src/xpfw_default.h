/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



#ifndef XPFW_DEFAULT_H_
#define XPFW_DEFAULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_config.h"
#include "xpfw_util.h"
#include "xpfw_debug.h"

/* BSP Headers */
#include "xil_io.h"
#include "xil_types.h"
#include "mb_interface.h"
#include "xstatus.h"
/* REGDB Headers */
#include "pmu_local.h"
#include "pmu_iomodule.h"
#include "pmu_global.h"
#include "ipi.h"
#include "uart0.h"
#include "uart1.h"
#include "crl_apb.h"
#include "lpd_slcr.h"
#include "rtc.h"

/* Base address of the IOU_SLCR module */
#define IOU_SLCR_BASE			0xFF180000U
#define IOU_SLCR_MIO_PIN_34_OFFSET	0x00000088U
#define IOU_SLCR_MIO_PIN_35_OFFSET	0x0000008CU
#define IOU_SLCR_MIO_PIN_36_OFFSET	0x00000090U
#define IOU_SLCR_MIO_PIN_37_OFFSET	0x00000094U

#define IOU_SLCR_CTRL		( IOU_SLCR_BASE + (u32)(0x600U) )

#define SLVERR_MASK				(u32)(0x1U)

/* XPPU SINK Registers */
#define XPPU_SINK_BASE_ADDR		0xFF9CFF00U
#define XPPU_SINK_ERR_CTRL		(XPPU_SINK_BASE_ADDR + 0xECU)

/* BBRAM registers */
#define BBRAM_BASE_ADDR			0xFFCD0000U
#define BBRAM_SLVERR_REG		(BBRAM_BASE_ADDR + 0x34U)

/* RAM address used for scrubbing */
#define PARAM_RAM_LOW_ADDRESS		0Xffdc0000U
#define PARAM_RAM_HIGH_ADDRESS		0Xffdcff00U

/* RAM base address for general usage */
#define PMU_RAM_BASE_ADDR		0Xffdc0000U

/* Register Access Macros */

#define XPfw_Write32(Addr, Value)  Xil_Out32((Addr), (Value))

#define XPfw_Read32(Addr)  Xil_In32((Addr))

#define XPfw_RMW32  XPfw_UtilRMW

#define ARRAYSIZE(x)	(u32)(sizeof(x)/sizeof(x[0]))
/* Custom Flags */

#define MASK_ALL 	0XffffffffU
#define ENABLE_ALL	0XffffffffU
#define ALL_HIGH	0XffffffffU
#define FLAG_ALL	0XffffffffU

#define MASK32_ALL_HIGH	((u32)0xFFFFFFFFU)
#define MASK32_ALL_LOW	((u32)0x0U)


#define YES 0x01U
#define NO 0x00U


#define XPFW_ACCESS_ALLOWED 0x01U
#define XPFW_ACCESS_DENIED	0x00U

/*
 * time in ms for checking psu init completion by FSBL
 */
#define CHECK_FSBL_COMPLETION	100U

#define FSBL_COMPLETION			1U

#define IPI_CRC_ERROR_OCCURRED	0x1U
#define HW_EXCEPTION_RECEIVED	0x2U

/* Error code to be written for RPU_run mode error */
#define RPU_RUN_MODE_ERROR 		0x04U

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);
struct HandlerTable{
	u32 Mask;
	VoidFunction_t Handler;
};

#ifdef __cplusplus
}
#endif

#endif /* XPFW_DEFAULT_H_ */
