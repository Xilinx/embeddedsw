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
 *       rmv  08/01/25 Added defines related to EFUSE CACHE registers
 *       rmf  09/12/25 Added defines related to ASU IPI registers
 *
 * </pre>
 *
 *************************************************************************************************/
#ifndef XASUFW_HW_H_
#define XASUFW_HW_H_

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
 #define PMC_TAP_VERSION_PLATFORM_COSIM				(0x7U) /**< QEMU+COSIM platform */

/*
 * Definitions required from pmc_global.h
 */
#define PMC_GLOBAL_HW_PCR_0_ADDR			(0xF1115200U)	/**< HW PCR 0 address */

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

/** ASU_GLOBAL ASU_FATAL_ERROR_MASK register address */
#define ASU_GLOBAL_ASU_FATAL_ERROR_MASK				(ASU_GLOBAL_BASEADDR + 0x00000078U)
/** ASU_GLOBAL ASU_FATAL_ERROR_ENABLE register address */
#define ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE						(ASU_GLOBAL_BASEADDR + 0x0000007CU)
/** ASU_GLOBAL ASU_FATAL_ERROR_ENABLE asu_sw_error mask */
#define ASU_GLOBAL_ASU_FATAL_ERROR_ENABLE_ASU_SW_ERROR_MASK		(0x00000020U)

/** ASU_GLOBAL ASU_FATAL_ERROR_TRIGGER register address */
#define ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER						(ASU_GLOBAL_BASEADDR + 0x00000084U)
/** ASU_GLOBAL ASU_FATAL_ERROR_TRIGGER asu_sw_error mask */
#define ASU_GLOBAL_ASU_FATAL_ERROR_TRIGGER_ASU_SW_ERROR_MASK	(0x00000020U)

/** ASU_GLOBAL ASU_NON_FATAL_ERROR_MASK register address */
#define ASU_GLOBAL_ASU_NON_FATAL_ERROR_MASK			(ASU_GLOBAL_BASEADDR + 0x0000008CU)
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
/** ASU_IO_BUS IRQ_STATUS DMA0 done interrupt mask */
#define ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_MASK	(0x00080000U)
/** ASU_IO_BUS IRQ_STATUS DMA0 done interrupt number */
#define ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_NUM	(19U)
/** ASU_IO_BUS IRQ_STATUS DMA1 done interrupt mask */
#define ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_MASK	(0x00100000U)
/** ASU_IO_BUS IRQ_STATUS DMA0 done interrupt number */
#define ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_NUM	(20U)

/** ASU_IO_BUS IRQ_ACK register address */
#define ASU_IO_BUS_IRQ_ACK							(ASU_IO_BUS_BASEADDR + 0x0000003CU)

/*
 * Definitions required from efuse_cache.h
 */
/** EFUSE_CACHE base address */
#define EFUSE_CACHE_BASEADDR				(0xF1250000U)

/** EFUSE_CACHE_DNA_0 register address */
#define EFUSE_CACHE_DNA_0				(EFUSE_CACHE_BASEADDR + 0x00000020U)

/** EFUSE_CACHE Device DNA Size in bytes */
#define EFUSE_CACHE_DNA_SIZE				(16U)

/**< EFUSE_CACHE DME FIPS address */
#define EFUSE_CACHE_DME_FIPS_ADDRESS			(EFUSE_CACHE_BASEADDR + 0x0000234U)

/**< EFUSE_CACHE DME Revoke 0 Mask */
#define EFUSE_CACHE_DME_REVOKE_0_MASK			(0x30U)

/**< EFUSE_CACHE DME Revoke 1 Mask */
#define EFUSE_CACHE_DME_REVOKE_1_MASK			(0xC0U)

/**< EFUSE_CACHE DME Revoke 2 Mask */
#define EFUSE_CACHE_DME_REVOKE_2_MASK			(0x300U)

/**< EFUSE_CACHE DME User Key 0 Address */
#define EFUSE_CACHE_USERKEY_0_ADDR			(EFUSE_CACHE_BASEADDR + 0x00000240U)

/**< EFUSE_CACHE DME User Key 1 Address */
#define EFUSE_CACHE_USERKEY_1_ADDR			(EFUSE_CACHE_BASEADDR + 0x00000270U)

/**< EFUSE_CACHE DME User Key 2 Address */
#define EFUSE_CACHE_USERKEY_2_ADDR			(EFUSE_CACHE_BASEADDR + 0x000002A0U)

/*
 * Definitions required from lpd_xppu.h
 */
/** LPD_XPPU base address */
#define LPD_XPPU_BASEADDR						(0xEB990000U)

/** LPD_XPPU APERPERM 49 Address */
#define LPD_XPPU_APERPERM_49					(LPD_XPPU_BASEADDR + 0x000010C4U)
/** LPD_XPPU_APERPERM_49_TRUSTZONE mask value */
#define LPD_XPPU_APERPERM_49_TRUSTZONE_MASK	(0x08000000U)
/** LPD_XPPU_APERPERM_49_TRUSTZONE shift value */
#define LPD_XPPU_APERPERM_49_TRUSTZONE_SHIFT	(27U)

/*
 * Definitions required from ipi.h
 */
/** ASU IPI base address */
#define IPI_ASU_BASEADDR	XPAR_XIPIPSU_0_BASEADDR

/**< ASU IPI Interrupt Trigger Register */
#define IPI_ASU_TRIG		(IPI_ASU_BASEADDR + 0x00000000U)
/** ASU IPI Interrupt Status Register */
#define IPI_ASU_ISR		(IPI_ASU_BASEADDR + 0x00000010U)
/**< PMC IPI channel mask */
#define IPI_ASU_ISR_PMC_MASK	(0x00000002U)
/**< IPI6 NoBuf channel mask */
#define IPI_ASU_NOBUF_6_MASK	(0x00008000U)

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

#endif  /* XASUFW_HW_H_ */
