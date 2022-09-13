/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
/**@cond xnvm_internal
 * @{
 */
/* Enable printfs by setting XNVM_DEBUG to 1 */
#define XNVM_DEBUG	(0U)

#if (XNVM_DEBUG)
#define XNVM_DEBUG_GENERAL (1U)
#else
#define XNVM_DEBUG_GENERAL (0U)
#endif

/*Macros for eFUSE CTRL WRITE LOCKED and UNLOCKED */
#define XNVM_EFUSE_CTRL_WR_LOCKED	(0x01U)
#define XNVM_EFUSE_CTRL_WR_UNLOCKED	(0x00U)

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

typedef enum {
	XNVM_EFUSE_PAGE_0 = 0,
	XNVM_EFUSE_PAGE_1,
	XNVM_EFUSE_PAGE_2
} XNvm_EfuseType;

/**
* @}
* @endcond
*/


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

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_COMMON_H */

/* @} */
