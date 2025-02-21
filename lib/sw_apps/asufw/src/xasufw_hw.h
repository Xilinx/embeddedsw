/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   ma   12/12/24 Added defines related to ASU IO MODULE
 *       ma   02/06/25 Removed unused defines
 *       ma   02/21/25 Added defines related to FATAL and NON-FAL registers
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
#define PMC_TAP_VERSION_PLATFORM_PROTIUM			(0x1U) /**< Protium platform */
#define PMC_TAP_VERSION_PLATFORM_QEMU				(0x3U) /**< QEMU platform */

/*
 * Definitions required from asu_global.h
 */
/** ASU_GLOBAL base address */
#define ASU_GLOBAL_BASEADDR							(0xEBF80000U)

/** ASU_GLOBAL GLOBAL_CNTRL register address */
#define ASU_GLOBAL_GLOBAL_CNTRL						(ASU_GLOBAL_BASEADDR + 0x00000000U)
/** ASU_GLOBAL GLOBAL_CNTRL FW_Is_Present mask */
#define ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)

/** ASU_GLOBAL ASU_SW_ERROR register address */
#define ASU_GLOBAL_ASU_SW_ERROR						(ASU_GLOBAL_BASEADDR + 0x0000005CU)

/** ASU_GLOBAL ASU_FATAL_ERROR_ENABLE register address */
#define ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE						(ASU_GLOBAL_BASEADDR + 0x0000007CU)
/** ASU_GLOBAL ASU_FATAL_ERROR_ENABLE asu_sw_error mask */
#define ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK		(0x00000020U)

/** ASU_GLOBAL ASU_FATAL_ERROR_TRIGGER register address */
#define ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER						(ASU_GLOBAL_BASEADDR + 0x00000084U)
/** ASU_GLOBAL ASU_FATAL_ERROR_TRIGGER asu_sw_error mask */
#define ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER_ASU_SW_ERROR_MASK	(0x00000020U)

/** ASU_GLOBAL ASU_NON_FATAL_ERROR_ENABLE register address */
#define ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE					(ASU_GLOBAL_BASEADDR + 0x00000090U)
/** ASU_GLOBAL ASU_NON_FATAL_ERROR_ENABLE asu_sw_error mask */
#define ASU_GLOBAL_ASU_NON_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK		(0x00000100U)

/** ASU_GLOBAL ASU_NON_FATAL_ERROR_TRIGGER register address */
#define ASU_GLOBAL_ASU_NON_FATAL_ERROR_TRIGGER					(ASU_GLOBAL_BASEADDR + 0x00000098U)
/** ASU_GLOBAL ASU_NON_FATAL_ERROR_TRIGGER asu_sw_error mask */
#define ASU_GLOBAL_ASU_NON_FATAL_ERROR_TRIGGER_ASU_SW_ERROR_MASK	(0x00000100U)

/*
 * Definitions required from asu_io_bus.h
 */
/** ASU_IO_BUS base address */
#define ASU_IO_BUS_BASEADDR							(0xEBE80000U)

/** ASU_IO_BUS IRQ_STATUS register address */
#define ASU_IO_BUS_IRQ_STATUS						(ASU_IO_BUS_BASEADDR + 0x00000030U)
#define ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_MASK	(0x00080000U)
#define ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_NUM	(19U)
#define ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_MASK	(0x00100000U)
#define ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_NUM	(20U)

/** ASU_IO_BUS IRQ_ACK register address */
#define ASU_IO_BUS_IRQ_ACK							(ASU_IO_BUS_BASEADDR + 0x0000003CU)


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