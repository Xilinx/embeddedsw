/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse_common.h
* @addtogroup xnvm_efuse_apis XilNvm eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains function declarations of eFUSE APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 3.0   kal  07/16/2022 Initial release
* 3.1   skg  10/25/2022 Added comments for macros and enums
* 3.2   kum  04/11/2023 Moved Voltage and Temp macros for LP,MP,HP ranges from versal xnvm_efuse.h
*	vss  09/19/2023	Fixed MISRA-C Rule 8.7 violation
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_COMMON_H
#define XNVM_EFUSE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xsysmonpsv.h"
#include "xnvm_defs.h"
#include "xnvm_efuse_error.h"

/*************************** Constant Definitions *****************************/
/**
 *  @name Enable printfs
 */
/**< Enable printfs by setting XNVM_DEBUG to 1 */
#define XNVM_DEBUG	(0U)

#if (XNVM_DEBUG)
#define XNVM_DEBUG_GENERAL (1U)
#else
#define XNVM_DEBUG_GENERAL (0U)
#endif
/** @} */

/**
 *  @name eFuse ctrls constants
 */
/**< Macros for eFUSE CTRL WRITE LOCKED and UNLOCKED */
#define XNVM_EFUSE_CTRL_WR_LOCKED	(0x01U)
#define XNVM_EFUSE_CTRL_WR_UNLOCKED	(0x00U)
/** @} */

/**
 *  @{ Temaperature limits defination for versal Efuses
 */
#define XNVM_EFUSE_FULL_RANGE_TEMP_MIN	(-55.0f)
#define XNVM_EFUSE_FULL_RANGE_TEMP_MAX	(125.0f)

#define XNVM_EFUSE_TEMP_LP_MIN		(0.0f)
#define XNVM_EFUSE_TEMP_LP_MAX		(100.0f)
#define XNVM_EFUSE_TEMP_MP_MIN		(-40.0f)
#define XNVM_EFUSE_TEMP_MP_MAX		(110.0f)
#define XNVM_EFUSE_TEMP_HP_MIN		(-55.0f)
#define XNVM_EFUSE_TEMP_HP_MAX		(125.0f)

/**< eFuse Range check  definations*/
#define XNVM_EFUSE_FULL_RANGE_CHECK		(0U)
#define XNVM_EFUSE_LP_RANGE_CHECK		(1U)
#define XNVM_EFUSE_MP_RANGE_CHECK		(2U)
#define XNVM_EFUSE_HP_RANGE_CHECK		(3U)

/**< eFuse volatage limits definations*/
#define XNVM_EFUSE_VCC_PMC_LP_MIN		(0.676f)
#define XNVM_EFUSE_VCC_PMC_LP_MAX		(0.724f)
#define XNVM_EFUSE_VCC_PMC_MP_MIN		(0.775f)
#define XNVM_EFUSE_VCC_PMC_MP_MAX		(0.825f)
#define XNVM_EFUSE_VCC_PMC_HP_MIN		(0.854f)
#define XNVM_EFUSE_VCC_PMC_HP_MAX		(0.906f)

#define XNVM_EFUSE_SYSMON_LOCK_CODE	(0xF9E8D7C6U)

/***************************** Type Definitions *******************************/
/**
 * @name  Operation mode
 */
typedef enum {
	XNVM_EFUSE_MODE_RD, /**< eFuse read mode */
	XNVM_EFUSE_MODE_PGM /**< eFuse program mode */
} XNvm_EfuseOpMode;
/** @} */

/**
 * @name  Read mode
 */
typedef enum {
	XNVM_EFUSE_NORMAL_RD, /**< eFuse normal read */
	XNVM_EFUSE_MARGIN_RD /**< eFuse margin read */
} XNvm_EfuseRdMode;
/** @} */

/**
 * @name  eFuse Page
 */
typedef enum {
	XNVM_EFUSE_PAGE_0 = 0, /**< Efuse page 0*/
	XNVM_EFUSE_PAGE_1, /**< Efuse page 1*/
	XNVM_EFUSE_PAGE_2 /**< Efuse page 2*/
} XNvm_EfuseType;
/** @} */



/*************************** Function Prototypes ******************************/
int XNvm_EfuseCacheReload(void);
void XNvm_EfuseDisablePowerDown(void);
int  XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
void XNvm_EfuseSetRefClk(void);
void XNvm_EfuseEnableProgramming(void);
int XNvm_EfuseDisableProgramming(void);
int XNvm_EfuseResetReadMode(void);
void XNvm_EfuseInitTimers(void);
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op, XNvm_EfuseRdMode RdMode);
int XNvm_EfuseCheckForTBits(void);
u32 XNvm_GetSysmonSupplyRegId(UINTPTR SysmonpsvSatBaseAddr);
int XNvm_EfuseTempAndVoltChecks(const XSysMonPsv *SysMonInstPtr);


#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_COMMON_H */

/* @} */
