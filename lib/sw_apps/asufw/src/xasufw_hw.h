/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_hw.h
 *
 * This file contains definitions for the ASU hardware registers.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   07/26/24 Initial release
 *       ma   07/30/24 Added defines required from ASU_GLOBAL register space
 *
 * </pre>
 *
 *************************************************************************************************/
#ifndef XASUFW_HW_H
#define XASUFW_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasufw_util.h"

/************************************ Constant Definitions ***************************************/
/*
 * Definitions required from pmc_tap.h
 */
/** PMC_TAP base address */
#define PMC_TAP_BASEADDR							(0xF11A0000U)

/** PMC_TAP Version register address */
#define PMC_TAP_VERSION								(PMC_TAP_BASEADDR + 0x00000004U)
/** PMC_TAP Version register platform mask */
#define PMC_TAP_VERSION_PLATFORM_MASK				(0x0F000000U)
/** PMC_TAP Version register platform shift */
#define PMC_TAP_VERSION_PLATFORM_SHIFT				(24U)

/** PMC_TAP Version register platform values */
#define PMC_TAP_VERSION_PLATFORM_SILICON			(0x0U) /**< Silicon platform */
#define PMC_TAP_VERSION_PLATFORM_PROTIUM			(0x1U) /**< Protium platform */
#define PMC_TAP_VERSION_PLATFORM_PALLADIUM			(0x2U) /**< Palladium platform */
#define PMC_TAP_VERSION_PLATFORM_QEMU				(0x3U) /**< QEMU platform */
#define PMC_TAP_VERSION_PLATFORM_FCV				(0x4U) /**< FCV platform */

/*
 * Definitions required from asu_global.h
 */
/** ASU_GLOBAL base address */
#define ASU_GLOBAL_BASEADDR							(0xEBF80000U)

/** ASU_GLOBAL GLOBAL_CNTRL register address */
#define ASU_GLOBAL_GLOBAL_CNTRL						(ASU_GLOBAL_BASEADDR + 0x00000000U)
/** ASU_GLOBAL GLOBAL_CNTRL FW_Is_Present mask */
#define ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/** Macro for reading platform from PMC_TAP version register */
#define XASUFW_PLATFORM			((XAsufw_ReadReg(PMC_TAP_VERSION) & \
	PMC_TAP_VERSION_PLATFORM_MASK) >> \
	PMC_TAP_VERSION_PLATFORM_SHIFT)

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_HW_H */