/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       kp   03/16/26 Added defines related to ASU RAM ECC controllers
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

/** ASU_GLOBAL GLOBAL_GEN_STORAGE7 register address */
#define ASU_GLOBAL_GLOBAL_GEN_STORAGE7		(ASU_GLOBAL_BASEADDR + 0x0000003CU)

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

/** EFUSE_CACHE IP_DISABLE_0 Address */
#define EFUSE_CACHE_IP_DISABLE_0_ADDR		(EFUSE_CACHE_BASEADDR + 0x00000018U)

/** EFUSE_CACHE IP_DISABLE_0 PS_CRYPTO_DIS mask */
#define EFUSE_CACHE_IP_DISABLE_0_PS_CRYPTO_DIS_MASK	(0x10000000U)

/** EFUSE_CACHE UDE FIPS address */
#define EFUSE_CACHE_UDE_FIPS_ADDRESS			(EFUSE_CACHE_BASEADDR + 0x0000234U)

/** EFUSE_CACHE UDE Revoke 0 Mask */
#define EFUSE_CACHE_UDE_REVOKE_0_MASK			(0x30U)

/** EFUSE_CACHE UDE Revoke 1 Mask */
#define EFUSE_CACHE_UDE_REVOKE_1_MASK			(0xC0U)

/** EFUSE_CACHE UDE Revoke 2 Mask */
#define EFUSE_CACHE_UDE_REVOKE_2_MASK			(0x300U)

/** EFUSE_CACHE UDE Revoke 0, 1, 2 Mask */
#define EFUSE_CACHE_UDE_REVOKE_ALL_MASK		(EFUSE_CACHE_UDE_REVOKE_0_MASK | \
						 EFUSE_CACHE_UDE_REVOKE_1_MASK | \
						 EFUSE_CACHE_UDE_REVOKE_2_MASK)

/** EFUSE_CACHE UDE User Key 0 Address */
#define EFUSE_CACHE_USERKEY_0_ADDR			(EFUSE_CACHE_BASEADDR + 0x00000240U)

/** EFUSE_CACHE UDE User Key 1 Address */
#define EFUSE_CACHE_USERKEY_1_ADDR			(EFUSE_CACHE_BASEADDR + 0x00000270U)

/** EFUSE_CACHE UDE User Key 2 Address */
#define EFUSE_CACHE_USERKEY_2_ADDR			(EFUSE_CACHE_BASEADDR + 0x000002A0U)

/** EFUSE_CACHE OFFCHIP Revoke 0 Address */
#define EFUSE_CACHE_OFFCHIP_REVOKE_0_ADDR		(EFUSE_CACHE_BASEADDR + 0x00000160U)

/*
 * Definitions required from ipi aperture registers
 */
/** IPI_APER_TZ_008 register address - contains TZ settings for all IPI agents */
#define IPI_APER_TZ_008							(0xEB3000DCU)
/** IPI_APER_TZ_008 single bit mask for extracting agent TZ status */
#define IPI_APER_TZ_008_AGENT_MASK				(0x00000001U)

/*
 * Definitions required from ipi.h
 */
/** ASU IPI base address */
#define IPI_ASU_BASEADDR	XPAR_XIPIPSU_0_BASEADDR

/** ASU IPI Interrupt Trigger Register */
#define IPI_ASU_TRIG		(IPI_ASU_BASEADDR + 0x00000000U)
/** ASU IPI Interrupt Status Register */
#define IPI_ASU_ISR		(IPI_ASU_BASEADDR + 0x00000010U)
/** PMC IPI channel mask */
#define IPI_ASU_ISR_PMC_MASK	(0x00000002U)
/** IPI6 NoBuf channel mask */
#define IPI_ASU_NOBUF_6_MASK	(0x00008000U)

/*
 * Definitions required from asu_ram_ecc_ctrl
 */
/** ASU RAM instruction ECC controller base address */
#define ASU_RAM_INSTR_ECC_CTRL_BASEADDR		(0xEBEA0000U)
/** ASU RAM data ECC controller base address */
#define ASU_RAM_DATA_ECC_CTRL_BASEADDR		(0xEBEB0000U)

/** ASU RAM ECC STATUS register offset */
#define ASU_RAM_ECC_CTRL_STATUS_OFFSET		(0x000U)
/** ASU RAM ECC STATUS uncorrectable error mask */
#define ASU_RAM_ECC_CTRL_STATUS_UE_MASK		(0x00000001U)
/** ASU RAM ECC STATUS correctable error mask */
#define ASU_RAM_ECC_CTRL_STATUS_CE_MASK		(0x00000002U)

/** ASU RAM ECC EN_IRQ register offset */
#define ASU_RAM_ECC_CTRL_EN_IRQ_OFFSET		(0x004U)
/** ASU RAM ECC EN_IRQ uncorrectable error interrupt enable mask */
#define ASU_RAM_ECC_CTRL_EN_IRQ_UE_MASK		(0x00000001U)
/** ASU RAM ECC EN_IRQ correctable error interrupt enable mask */
#define ASU_RAM_ECC_CTRL_EN_IRQ_CE_MASK		(0x00000002U)

/** ASU RAM ECC ONOFF register offset */
#define ASU_RAM_ECC_CTRL_ONOFF_OFFSET		(0x008U)
/** ASU RAM ECC ONOFF enable mask */
#define ASU_RAM_ECC_CTRL_ONOFF_EN_MASK		(0x00000001U)

/** ASU RAM ECC correctable error count register offset */
#define ASU_RAM_ECC_CTRL_CE_CNT_OFFSET		(0x00CU)
/** ASU RAM ECC CE first failing data register offset */
#define ASU_RAM_ECC_CTRL_CE_FFD_OFFSET		(0x100U)
/** ASU RAM ECC CE first failing ECC syndrome register offset */
#define ASU_RAM_ECC_CTRL_CE_FFE_OFFSET		(0x180U)
/** ASU RAM ECC CE first failing address register offset */
#define ASU_RAM_ECC_CTRL_CE_FFA_OFFSET		(0x1C0U)
/** ASU RAM ECC UE first failing data register offset */
#define ASU_RAM_ECC_CTRL_UE_FFD_OFFSET		(0x200U)
/** ASU RAM ECC UE first failing ECC syndrome register offset */
#define ASU_RAM_ECC_CTRL_UE_FFE_OFFSET		(0x280U)
/** ASU RAM ECC UE first failing address register offset */
#define ASU_RAM_ECC_CTRL_UE_FFA_OFFSET		(0x2C0U)

/*
 * Map canonical xparameters instances to instruction/data ECC controllers by base address.
 * XPAR_XASU_RAM_ECC_0/1 numbering may not match the HW controller identity, so compare
 * base addresses to determine which controller each instance corresponds to.
 */
#if defined(XPAR_XASU_RAM_ECC_0_BASEADDR) && defined(XPAR_XASU_RAM_ECC_0_IS_ECC)
#if (XPAR_XASU_RAM_ECC_0_BASEADDR == ASU_RAM_INSTR_ECC_CTRL_BASEADDR) && \
    (XPAR_XASU_RAM_ECC_0_IS_ECC == 1U)
#define XASUFW_RAM_INSTR_ECC_ENABLE
#elif (XPAR_XASU_RAM_ECC_0_BASEADDR == ASU_RAM_DATA_ECC_CTRL_BASEADDR) && \
      (XPAR_XASU_RAM_ECC_0_IS_ECC == 1U)
#define XASUFW_RAM_DATA_ECC_ENABLE
#endif
#endif

#if defined(XPAR_XASU_RAM_ECC_1_BASEADDR) && defined(XPAR_XASU_RAM_ECC_1_IS_ECC)
#if (XPAR_XASU_RAM_ECC_1_BASEADDR == ASU_RAM_INSTR_ECC_CTRL_BASEADDR) && \
    (XPAR_XASU_RAM_ECC_1_IS_ECC == 1U)
#ifndef XASUFW_RAM_INSTR_ECC_ENABLE
#define XASUFW_RAM_INSTR_ECC_ENABLE
#endif
#elif (XPAR_XASU_RAM_ECC_1_BASEADDR == ASU_RAM_DATA_ECC_CTRL_BASEADDR) && \
      (XPAR_XASU_RAM_ECC_1_IS_ECC == 1U)
#ifndef XASUFW_RAM_DATA_ECC_ENABLE
#define XASUFW_RAM_DATA_ECC_ENABLE
#endif
#endif
#endif

#if defined(XASUFW_RAM_INSTR_ECC_ENABLE) && defined(XASUFW_RAM_DATA_ECC_ENABLE)
#define XASUFW_RAM_ECC_ENABLE
#endif

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
