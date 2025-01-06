/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_hw.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   07/31/24 Initial release
 * 1.01  ng   11/05/24 Add boot time measurements
 *       ng   12/04/24 Fix secondary boot control
 *       ng   11/26/24 Add support for new devices
 * </pre>
 *
 ******************************************************************************/
#ifndef XPLM_HW_H
#define XPLM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_util.h"

/************************** Constant Definitions *****************************/
#define XPLM_SBI_BUF_ADDR	(0x0U)

/* Interrupt ID for SBI */
#define	XPLM_SBI_INTERRUPT_ID	(17U)

/* SBI interface types */
#define XPLM_SBI_IF_JTAG        (0x1U)
#define XPLM_SBI_IF_AXI_SLAVE   (0x2U)
#define XPLM_SBI_IF_MCAP        (0x4U)

/* Addresses for PLM RunTime Configuration Registers */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	#define XPLM_RTCFG_OFFSET	(0xB200U)
#else
	#define XPLM_RTCFG_OFFSET	(0x13200U)
#endif
#define XPLM_RTCFG_BASEADDR	(XPAR_PMC_RAM_0_BASEADDRESS + XPLM_RTCFG_OFFSET)

#define XPLM_RTCFG_LENGTH_BYTES		(0x200U)
#define XPLM_RTCFG_LENGTH_WORDS		(XPLM_BYTES_TO_WORDS(XPLM_RTCFG_LENGTH_BYTES))

#define XPLM_RTCFG_ID_STRING_ADDR	(XPLM_RTCFG_BASEADDR + 0x0U)
#define XPLM_RTCFG_VERSION_ADDR		(XPLM_RTCFG_BASEADDR + 0x4U)
#define XPLM_RTCFG_SIZE_ADDR		(XPLM_RTCFG_BASEADDR + 0x8U)
#define XPLM_RTCFG_DBG_LOG_BUF_ADDR	(XPLM_RTCFG_BASEADDR + 0x10U)
#define XPLM_RTCFG_SECURE_CTRL_ADDR	(XPLM_RTCFG_BASEADDR + 0x20U)
#define XPLM_RTCFG_OSPI_CLK_CFG		(XPLM_RTCFG_BASEADDR + 0x24U)
#define XPLM_RTCFG_DBG_CTRL		(XPLM_RTCFG_BASEADDR + 0x28U)
#define XPLM_RTCFG_EAM_ERR1_STATUS	(XPLM_RTCFG_BASEADDR + 0x30U)
#define XPLM_RTCFG_SEC_BOOT_CTRL	(XPLM_RTCFG_BASEADDR + 0x34U)
#define XPLM_RTCFG_USER_DEF_REV		(XPLM_RTCFG_BASEADDR + 0x38U)

/* Default Values of PLM RunTime Configuration Registers */
#define XPLM_RTCFG_VER			(0x1U)
#define XPLM_RTCFG_SIZE			(XPLM_RTCFG_LENGTH_WORDS)
#define XPLM_RTCFG_IDENTIFICATION	(0x41435452U)

/* Masks for PLM RunTime Configuration Registers */
#define XPLM_RTCFG_OSPI_REF_CLK_DIV_MASK	(0x000000FFU)
#define XPLM_RTCFG_OSPI_REF_CLK_DIV_UNUSED_BITS_MASK	(0x000000C0U)
#define XPLM_RTCFG_OSPI_XDR_MODE_MASK		(0x00000100U)
#define XPLM_OQSPI_DDR_MODE	(XPLM_RTCFG_OSPI_XDR_MODE_MASK)
#define XPLM_RTCFG_OSPI_PHY_MODE_MASK		(0x00000200U)
#define XPLM_RTCFG_OSPI_ECO_MASK		(0x00000400U)
#define XPLM_RTCFG_OSPI_EMCCLK_MASK		(0x00000800U)
#define XPLM_RTCFG_SEC_BOOT_CTRL_ENABLE_MASK	(0x00000001U)
#define XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_MASK	(0x0000001CU)
#define XPLM_RTCG_SEC_CTRL_RED_KEY_CLEAR_MASK	(0x00000003U)

/* Shifts for PLM RunTime Configuration Registers */
#define XPLM_RTCFG_OSPI_XDR_MODE_SHIFT          (0x8U)
#define XPLM_RTCFG_OSPI_PHY_MODE_SHIFT          (0x9U)
#define XPLM_RTCFG_SEC_BOOT_CTRL_BOOT_IF_SHIFT	(0x2U)

/* Addresses for RV_IOMODULE */
#define RV_IOMODULE_BASEADDR                    (0x04040000U)
#define RV_IOMODULE_GPO1                        (RV_IOMODULE_BASEADDR + 0x10U)
#define RV_IOMODULE_PIT1_COUNTER                (RV_IOMODULE_BASEADDR + 0x44U)
#define RV_IOMODULE_PIT2_COUNTER                (RV_IOMODULE_BASEADDR + 0x54U)

/* Masks for RV_IOMODULE */
#define RV_IOMODULE_GPO1_PIT1_PRESCALE_SRC_MASK (0x2U)

/* Addresses for PMC GLOBAL */
#define PMC_GLOBAL_BASEADDR                     (0x040A0000U)
#define PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL       (PMC_GLOBAL_BASEADDR + 0x30U)
#define PMC_GLOBAL_OSPI_CLK_CTRL                (PMC_GLOBAL_BASEADDR + 0x38U)
#define PMC_GLOBAL_RST_PMCL                     (PMC_GLOBAL_BASEADDR + 0x50U)
#define PMC_GLOBAL_RST_OSPI                     (PMC_GLOBAL_BASEADDR + 0x58U)
#define PMC_GLOBAL_RST_SBI                      (PMC_GLOBAL_BASEADDR + 0x5CU)
#define PMC_GLOBAL_RST_CCU                      (PMC_GLOBAL_BASEADDR + 0x60U)
#define PMC_GLOBAL_BOOT_MODE_USER               (PMC_GLOBAL_BASEADDR + 0x7CU)
#define PMC_GLOBAL_SSS_CFG                      (PMC_GLOBAL_BASEADDR + 0x90U)
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE0		(PMC_GLOBAL_BASEADDR + 0xA0U)
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE1		(PMC_GLOBAL_BASEADDR + 0xA4U)
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE2		(PMC_GLOBAL_BASEADDR + 0xA8U)
#define PMC_GLOBAL_GLOBAL_GEN_STORAGE3		(PMC_GLOBAL_BASEADDR + 0xACU)
#define PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0       (PMC_GLOBAL_BASEADDR + 0xC0U)
#define PMC_GLOBAL_MULTI_BOOT                   (PMC_GLOBAL_BASEADDR + 0x130U)
#define PMC_GLOBAL_AUTH_STATUS                  (PMC_GLOBAL_BASEADDR + 0x144U)
#define PMC_GLOBAL_ENC_STATUS                   (PMC_GLOBAL_BASEADDR + 0x148U)
#define PMC_GLOBAL_PMC_BOOT_ERR                 (PMC_GLOBAL_BASEADDR + 0x14CU)
#define PMC_GLOBAL_PMC_FW_ERR                   (PMC_GLOBAL_BASEADDR + 0x154U)
#define PMC_GLOBAL_PMC_FW_STATUS                (PMC_GLOBAL_BASEADDR + 0x158U)
#define PMC_GLOBAL_PMC_FW_DATA                  (PMC_GLOBAL_BASEADDR + 0x15CU)
#define PMC_GLOBAL_PMCL_EAM_ERR1_STATUS         (PMC_GLOBAL_BASEADDR + 0x10000U)
#define PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN         (PMC_GLOBAL_BASEADDR + 0x10024U)
#define PMC_GLOBAL_PMCL_EAM_IRQ1_MASK           (PMC_GLOBAL_BASEADDR + 0x10060U)
#define PMC_GLOBAL_PMCL_EAM_IRQ1_EN             (PMC_GLOBAL_BASEADDR + 0x10064U)
#define PMC_GLOBAL_PMCL_EAM_IRQ1_DIS            (PMC_GLOBAL_BASEADDR + 0x10068U)
#define PMC_GLOBAL_PUF_DISABLE                  (PMC_GLOBAL_BASEADDR + 0x1F358U)
#define PMC_GLOBAL_PUF_SYN_0			(PMC_GLOBAL_BASEADDR + 0x1F368U)

/* Masks for PMC GLOBAL */
#define PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL_DIVISOR_MASK (0x00000F00U)
#define PMC_GLOBAL_OSPI_CLK_CTRL_CLKACT_MASK    (0x02000000U)
#define PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_MASK   (0x00003F00U)
#define PMC_GLOBAL_OSPI_CLK_CTRL_SRCSEL_MASK    (0x00000001U)
#define PMC_GLOBAL_RST_OSPI_RESET_MASK          (0x00000001U)
#define PMC_GLOBAL_RST_SBI_FULLMASK             (0x00000001U)
#define PMC_GLOBAL_BOOT_MODE_USER_MASK          (0x0000000FU)
#define PMC_GLOBAL_PMC_FW_ERR_CR_MASK           (0x40000000U)
#define PMC_GLOBAL_PMC_FW_ERR_NCR_MASK          (0x80000000U)
#define PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_NCR_ERR_MASK     (0x04000000U)
#define PMC_GLOBAL_PMCL_EAM_ERR_OUT1_EN_PMC_FW_CR_ERR_MASK      (0x02000000U)
#define PMC_GLOBAL_PMCL_EAM_ERR1_STATUS_FULLMASK (0x7FFFFFFFU)
#define PMC_GLOBAL_PUF_DISABLE_PUF_MASK         (0x00000001U)

/* Default values for PMC GLOBAL */
#define PMC_GLOBAL_RST_CCU_RESET_DEFVAL	(XPLM_ZERO)

/* Shifts for PMC GLOBAL */
#define PMC_GLOBAL_PMCL_MAIN_IRO_CLK_CTRL_DIVISOR_SHIFT (8U)
#define PMC_GLOBAL_OSPI_CLK_CTRL_DIVISOR_SHIFT  (8U)
#define PMC_GLOBAL_PMCL_EAM_IRQ1_EN_CCU_TAMPER_FABRIC_SHIFT	(0x7U)

/* Addresses for PMC TAP */
#define PMC_TAP_BASEADDR			(0x040C0000U)
#define PMC_TAP_VERSION			(PMC_TAP_BASEADDR + 0x4U)
#define PMC_TAP_INST_MASK_0		(PMC_TAP_BASEADDR + 0x10000U)
#define PMC_TAP_INST_MASK_1		(PMC_TAP_BASEADDR + 0x10004U)

/* Masks for PMC TAP */
#define PMC_TAP_INST_MASK_0_MONITOR_DRP_MASK (0x10000000U)
#define PMC_TAP_INST_MASK_0_JRDBK_MASK	(0x00000010U)
#define PMC_TAP_INST_MASK_0_USER1_MASK	(0x00000004U)
#define PMC_TAP_INST_MASK_0_USER2_MASK	(0x00000008U)
#define PMC_TAP_INST_MASK_1_USER3_MASK	(0x00000004U)
#define PMC_TAP_INST_MASK_1_USER4_MASK	(0x00000008U)
#define PMC_TAP_VERSION_PLATFORM_MASK	(0x0F000000U)

/* Shifts for PMC TAP */
#define PMC_TAP_VERSION_PLATFORM_SHIFT	(24U)

/* Default values for PMC TAP */
#define PMC_TAP_VERSION_SILICON		(0x0U)

/* Addresses for SLAVE BOOT */
#define SLAVE_BOOT_BASEADDR		(0x04110000U)
#define SLAVE_BOOT_SBI_MODE		(SLAVE_BOOT_BASEADDR + 0x0U)
#define SLAVE_BOOT_SBI_CTRL		(SLAVE_BOOT_BASEADDR + 0x4U)
#define SLAVE_BOOT_SBI_STATUS		(SLAVE_BOOT_BASEADDR + 0xCU)
#define SLAVE_BOOT_SBI_STATUS2		(SLAVE_BOOT_BASEADDR + 0x10U)
#define SLAVE_BOOT_SBI_RDBK		(SLAVE_BOOT_BASEADDR + 0x100U)
#define SLAVE_BOOT_SBI_IRQ_STATUS		(SLAVE_BOOT_BASEADDR + 0x300U)
#define SLAVE_BOOT_SBI_IRQ_ENABLE		(SLAVE_BOOT_BASEADDR + 0x308U)
#define SLAVE_BOOT_SBI_IRQ_DISABLE		(SLAVE_BOOT_BASEADDR + 0x30CU)

/* Masks for SLAVE BOOT */
#define SLAVE_BOOT_SBI_MODE_SELECT_MASK		(0x00000001U)
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK		(0x0000001CU)
#define SLAVE_BOOT_SBI_CTRL_ENABLE_MASK		(0x00000001U)
#define SLAVE_BOOT_SBI_IRQ_DISABLE_DATA_RDY_MASK	(0x00000004U)
#define SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK	(0x00000004U)
#define SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK	(0x00000004U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK	(0x00003C00U)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK	(0x003c0000U)
#define SLAVE_BOOT_SBI_RDBK_DIS_MASK    (0x00000001U)
#define SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_MASK (0x00000FFFU)

/* Shifts for SLAVE BOOT */
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_SHIFT	(2U)

/* Default values for SLAVE BOOT */
#define SLAVE_BOOT_SBI_MODE_SELECT_DEFVAL		(XPLM_ZERO)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_DEFVAL  (0x8U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_SHIFT	(10U)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_DEFVAL	(0x8U)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_SHIFT	(18U)
#define SLAVE_BOOT_SBI_STATUS2_WRITE_BUF_SPACE_DEFVAL   (0x800U)

/**
 *  Efuse Registers
 */
#define EFUSE_BASEADDR (0x04160000U)

#define EFUSE_XILINX_CTRL       (EFUSE_BASEADDR + 0x00001000U)
#define EFUSE_CONTROLS		(EFUSE_BASEADDR + 0x00001004U)
#define EFUSE_PPK2_0		(EFUSE_BASEADDR + 0x000010C0U)

#define EFUSE_XILINX_CTRL_PUFHD_INVLD_MASK      (0x00006000U)
#define EFUSE_CONTROLS_HASH_PUF_OR_KEY_MASK	(0x00000400U)

/**
 * AES Registers
 */
#define AES_BASEADDR			(0x040F0000)

/* Addresses for AES registers */
#define AES_SOFT_RST_ADDR		(AES_BASEADDR + 0x10U)
#define AES_KEY_CLEAR_ADDR		(AES_BASEADDR + 0x14U)
#define AES_KEY_ZEROED_STATUS_ADDR	(AES_BASEADDR + 0x64U)

/* Masks for AES registers */
#define AES_SOFT_RST_VAL_MASK		(0x00000001U)

/**
 * SHA Registers
 */
#define SHA_BASEADDR			(0x04100000U)

/* Addresses for SHA registers */
#define SHA_RESET_ADDR		(SHA_BASEADDR + 0x4U)

/* Masks for SHA registers */
#define SHA_RESET_VALUE_MASK		(0x00000001U)

/**
 * Log buffer Base address
 */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	/** Start address of the log buffer in PMC RAM */
	#define XPLM_DEBUG_LOG_BUFFER_OFFSET	(0xAE00U)
#else
	/** Start address of the log buffer in PMC RAM */
	#define XPLM_DEBUG_LOG_BUFFER_OFFSET	(0x12E00U)
#endif
#define XPLM_DEBUG_LOG_BUFFER_ADDR	(XPAR_PMC_RAM_0_BASEADDRESS + XPLM_DEBUG_LOG_BUFFER_OFFSET)

/** Log buffer length in PMC RAM */
#define XPLM_DEBUG_LOG_BUFFER_LEN	(0x400U)

/** Log buffer high address */
#define XPLM_DEBUG_LOG_BUFFER_HIGH_ADDR	(XPLM_DEBUG_LOG_BUFFER_ADDR + XPLM_DEBUG_LOG_BUFFER_LEN - 1U)

/**
 * Secure chunk buffer base address
 */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	#define XPLM_SECURE_CHUNK_BUFFER_OFFSET		(0xC000U)
#else
	#define XPLM_SECURE_CHUNK_BUFFER_OFFSET		(0x14000U)
#endif
#define XPLM_SECURE_CHUNK_BUFFER_ADDR	(XPAR_PMC_RAM_0_BASEADDRESS + XPLM_SECURE_CHUNK_BUFFER_OFFSET)

#define XPLM_CHUNK_SIZE			(0x2000U)

/**
 * Hooks table Base address
 */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	/** Hooks table address in PMC RAM. */
	#define XROM_HOOKS_TBL_OFFSET	(0xF9C0U)
#else
	/** Hooks table address in PMC RAM. */
	#define XROM_HOOKS_TBL_OFFSET	(0x179C0U)
#endif
#define XROM_HOOKS_TBL_BASE_ADDR	(XPAR_PMC_RAM_0_BASEADDRESS + XROM_HOOKS_TBL_OFFSET)

/**
 * Hash block Base address
 */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	#define XPLM_HASH_BLOCK_OFFSET	(0xFBA0U)
#else
	#define XPLM_HASH_BLOCK_OFFSET	(0x17B70)
#endif
#define XPLM_HASH_BLOCK_ADDR_IN_RAM	(XPAR_PMC_RAM_0_BASEADDRESS + XPLM_HASH_BLOCK_OFFSET)

/* Boot Header Base Address */
#if defined(XPS_BOARD_SU10P) || defined(XPS_BOARD_SU25P) || defined(XPS_BOARD_SU35P)
	#define XPLM_BOOT_HEADER_OFFSET		(0xFC40U)
#else
	#define XPLM_BOOT_HEADER_OFFSET		(0x17C40U)
#endif
#define XPLM_BOOT_HEADER_START_ADDR	(XPAR_PMC_RAM_0_BASEADDRESS + XPLM_BOOT_HEADER_OFFSET)

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
/************************** Variable Definitions *****************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLM_HW_H */
