/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_hw.h
*
* This is the header file which contains definitions for the hardware
* registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
*       bsv  04/18/2019 Added support for NPI readback and CFI readback
*       kc   04/27/2019 Added SPP check for UART frequency
*       kc   05/21/2019 Updated IPI error code to response buffer
*       kc   07/16/2019 Added logic to determine the IRO frequency
*       bsv  08/29/2019 Added Multiboot and Fallback support in PLM
*       scs  08/31/2019 Added support for Extended IDCODE checks
* 1.02  ma   02/05/2020 Removed SRST error action for PSM errors
*       ma   02/18/2020 Added event logging code
*       ma   02/28/2020 Code related to handling PSM errors from PLM
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/10/2020 Added PMC_IOU_SLCR register related macros
*       kc   07/28/2020 Added PMC PS GPIO related macros
*       kc   08/04/2020 Added CRP NPLL related macros
*       bm   08/19/2020 Added ImageInfo Table related macros
*       bm   09/08/2020 Added PMC RAM Usage for RunTime Configuration registers
*       bsv  09/21/2020 Set clock source to IRO before SRST for ES1 silicon
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP
*                       and PCIE boot modes
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_HW_H
#define XPLMI_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xparameters.h"
#include "xil_io.h"
#include "pmc_global.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief        This function reads a 32-bit register
 *
 * @param        Addr is the address of the register
 *
 * @return       32-bit register value
 *
 ******************************************************************************/
static inline u32 XPlmi_In32(UINTPTR Addr)
{
	return Xil_In32(Addr);
}

/*****************************************************************************/
/**
 * @brief	This function writes 32-bit value to 32-bit register
 *
 * @param	Addr is the address of the register
 * @param	Value is the value to store in register
 *
 * @return	None
 *
******************************************************************************/
static inline void XPlmi_Out32(UINTPTR Addr, u32 Value)
{
	Xil_Out32(Addr, Value);
}

#define XPlmi_In64(Addr)		lwea(Addr)
#define XPlmi_InByte64(Addr)	lbuea(Addr)

/*****************************************************************************/
/**
 * @brief       This function disables waking up of PPU1 processor
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************/
static inline void XPlmi_PpuWakeUpDis(void)
{
	XPlmi_Out32(PMC_GLOBAL_PPU_1_RST_MODE,
		XPlmi_In32(PMC_GLOBAL_PPU_1_RST_MODE) &
		(~PMC_GLOBAL_PPU_1_RST_MODE_WAKEUP_MASK));
}

#define XPlmi_Out64(Addr, Data)		swea(Addr, Data)
#define XPlmi_OutByte64(Addr, Data)	sbea(Addr, Data)

/*
 * Register: PMC Global PLM Error
 */
#define PMC_GLOBAL_PLM_ERR		(PMC_GLOBAL_BASEADDR + 0X00010100U)

/*
 * Definitions required from pmc_tap.h
 */
#define PMC_TAP_BASEADDR		(0XF11A0000U)
#define PMC_TAP_IDCODE		(PMC_TAP_BASEADDR + 0X00000000U)
#define PMC_TAP_IDCODE_SI_REV_MASK	(0xF0000000U)
#define PMC_TAP_IDCODE_SBFMLY_MASK	(0x001C0000U)
#define PMC_TAP_IDCODE_DEV_MASK		(0x0003F000U)
#define PMC_TAP_IDCODE_SIREV_DVCD_MASK	(PMC_TAP_IDCODE_SI_REV_MASK | \
		PMC_TAP_IDCODE_SBFMLY_MASK | PMC_TAP_IDCODE_DEV_MASK)

#define PMC_TAP_IDCODE_SI_REV_1		(0x00000000U)
#define PMC_TAP_IDCODE_SBFMLY_S		(0x00080000U)
#define PMC_TAP_IDCODE_DEV_80		(0x00028000U)
#define PMC_TAP_IDCODE_ES1_VC1902	(PMC_TAP_IDCODE_SI_REV_1 | \
	PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_80)

#define PMC_TAP_VERSION		(PMC_TAP_BASEADDR + 0X00000004U)
#define PMC_TAP_VERSION_PLATFORM_SHIFT		(24U)
#define PMC_TAP_VERSION_PS_VERSION_SHIFT		(8U)
#define PMC_TAP_VERSION_PS_VERSION_MASK		(0X0000FF00U)
#define PMC_TAP_VERSION_PMC_VERSION_SHIFT		(0U)
#define PMC_TAP_VERSION_PMC_VERSION_MASK		(0X000000FFU)

#define PMC_TAP_VERSION_SILICON			(0x0U)
#define PMC_TAP_VERSION_SPP			(0x1U)
#define PMC_TAP_VERSION_QEMU			(0x3U)
#define XPLMI_PLATFORM_MASK			(0x07000000U)
#define XPLMI_PLATFORM		((XPlmi_In32(PMC_TAP_VERSION) & \
					XPLMI_PLATFORM_MASK) >> \
					PMC_TAP_VERSION_PLATFORM_SHIFT)
#define XPLMI_SILICON_ES1_VAL	(0x10U)

/*
 * PMC RAM Memory usage:
 * 0xF2000000U to 0xF2010100U - Used by XilLoader to process CDO
 * 0xF2014000U to 0xF2014FFFU - Used for PLM Runtime Configuration Registers
 * 0xF2019000U to 0xF201D000U - Used by XilPlmi to store PLM prints
 * 0xF201D000U to 0xF201E000U - Used by XilPlmi to store PLM Trace Events
 * 0xF201E000U to 0xF2020000U - Used by XilPdi to get boot Header copied by ROM
 */
#define XPLMI_PMCRAM_BASEADDR			(0xF2000000U)
#define XPLMI_PMCRAM_LEN			(0x20000U)

/* Loader chunk memory */
#define XPLMI_PMCRAM_CHUNK_MEMORY		(XPLMI_PMCRAM_BASEADDR)
#define XPLMI_PMCRAM_CHUNK_MEMORY_1		(XPLMI_PMCRAM_BASEADDR + 0x8100U)

/* Log Buffer default address and length */
#define XPLMI_DEBUG_LOG_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x19000U)
#define XPLMI_DEBUG_LOG_BUFFER_LEN	(0x4000U) /* 16KB */

/* Trace Buffer default address and length */
#define XPLMI_TRACE_LOG_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x1D000U)
#define XPLMI_TRACE_LOG_BUFFER_LEN	(0xD00U)	/* 3.25KB */

/* Image Info Table related macros */
#define XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR	(XPLMI_PMCRAM_BASEADDR + 0x1DD00U)
#define XPLMI_IMAGE_INFO_TBL_BUFFER_LEN		(0x300U)	/* 768B */

/*
 * Definitions required from Efuse
 */
#define EFUSE_CACHE_BASEADDR		(0XF1250000U)
#define EFUSE_CACHE_ANLG_TRIM_5		(EFUSE_CACHE_BASEADDR + 0X000000E0U)
#define EFUSE_CACHE_ANLG_TRIM_7		(EFUSE_CACHE_BASEADDR + 0X000000F8U)
#define EFUSE_TRIM_LP_MASK		(0xFFFFU)

/*
 * Definition for QSPI to be included
 */
#if (!defined(PLM_QSPI_EXCLUDE) && defined(XPAR_XQSPIPSU_0_DEVICE_ID))
#define XLOADER_QSPI
#define XLOADER_QSPI_BASEADDR		XPAR_XQSPIPS_0_BASEADDR
#endif

/*
 * Definition for OSPI to be included
 */
#if (!defined(PLM_OSPI_EXCLUDE) && defined(XPAR_XOSPIPSV_0_DEVICE_ID))
#define XLOADER_OSPI
#define XLOADER_OSPI_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#define XLOADER_OSPI_BASEADDR		XPAR_XOSPIPSV_0_BASEADDR
#endif

/*
 * Definitions for SD to be included
 */
#if (!defined(PLM_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1040000U))
#define XLOADER_SD_0
#endif

#if (!defined(PLM_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1050000U) ||\
			(XPAR_XSDPS_1_BASEADDR == 0xF1050000U))
#define XLOADER_SD_1
#endif

/*
 * Definition for SBI to be included
 */
#if !defined(PLM_SBI_EXCLUDE)
#define XLOADER_SBI
#endif

#if (!defined(PLM_USB_EXCLUDE) && defined(XPAR_XUSBPSU_0_DEVICE_ID) &&\
		(XPAR_XUSBPSU_0_BASEADDR == 0xFE200000U))
#define XLOADER_USB
#endif

/*
 * Definition for SEM to be included
 */
#if !defined(PLM_SEM_EXCLUDE) && (defined(XSEM_CFRSCAN_EN) ||\
		defined(XSEM_NPISCAN_EN))
#define XPLM_SEM
#endif

/*
 * Definitions required from crp.h
 */
#define CRP_BASEADDR		(0XF1260000U)
#define CRP_BOOT_MODE_USER		(CRP_BASEADDR + 0X00000200U)
#define CRP_BOOT_MODE_USER_BOOT_MODE_MASK		(0X0000000FU)
#define CRP_BOOT_MODE_POR		(CRP_BASEADDR + 0X00000204U)
#define CRP_RESET_REASON		(CRP_BASEADDR + 0X00000220U)
#define CRP_RST_SBI				(CRP_BASEADDR + 0X00000324U)
#define CRP_RST_SBI_RESET_MASK			(0X00000001U)
#define CRP_RST_PDMA				(CRP_BASEADDR + 0X00000328U)
#define CRP_RST_PDMA_RESET1_MASK		(0X00000002U)
/*
 * Register: CRP_RST_PS
 */
#define CRP_RST_PS		(CRP_BASEADDR + 0x0000031CU)
#define CRP_RST_PS_PMC_SRST_MASK		(0X00000008U)
#define CRP_RST_PS_PMC_POR_MASK		(0X00000080U)

/*
 * Register: CRP_NOCPLL_CTRL
 */
#define CRP_NOCPLL_CTRL		((CRP_BASEADDR) + 0X00000050U)
#define CRP_NOCPLL_CTRL_BYPASS_MASK	(0X00000008U)
#define CRP_NOCPLL_CTRL_RESET_MASK	(0X00000001U)

/*
 * Register: CRP_NOCPLL_CFG
 */
#define CRP_NOCPLL_CFG		((CRP_BASEADDR) + 0X00000054U)

/*
 * Register: CRP_PLL_STATUS
 */
#define CRP_PLL_STATUS		((CRP_BASEADDR) + 0X00000060U)
#define CRP_PLL_STATUS_NOCPLL_LOCK_MASK		(0X00000002U)

/*
 * Register: CRP_SYSMON_REF_CTRL
 */
#define CRP_SYSMON_REF_CTRL    ((CRP_BASEADDR) + 0X00000138U)
#define CRP_SYSMON_REF_CTRL_SRCSEL_MASK    (0X00000004U)

/*
 * PMC_ANALOG Base Address
 */
#define PMC_ANALOG_BASEADDR		(0XF1160000U)
/*
 * Register: PMC_ANALOG_VGG_CTRL
 */
#define PMC_ANALOG_VGG_CTRL		(PMC_ANALOG_BASEADDR + 0X0000000CU)
#define PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK		(0X00000001U)

/*
 * Definitions required from slave_boot.h
 */
#define SLAVE_BOOT_BASEADDR				(0XF1220000U)
#define SLAVE_BOOT_SBI_MODE		(SLAVE_BOOT_BASEADDR + 0X00000000U)
#define SLAVE_BOOT_SBI_MODE_JTAG_MASK			(0X00000002U)
#define SLAVE_BOOT_SBI_MODE_SELECT_MASK			(0X00000001U)

#define SLAVE_BOOT_SBI_CTRL		(SLAVE_BOOT_BASEADDR + 0X00000004U)
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK		(0X0000001CU)
#define SLAVE_BOOT_SBI_CTRL_ENABLE_MASK		(0X00000001U)

/*
 * Register: SLAVE_BOOT_SBI_STATUS
 */
#define SLAVE_BOOT_SBI_STATUS		(SLAVE_BOOT_BASEADDR + 0X0000000CU)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK		(0XF0000000U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK		(0X00F00000U)
#define SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK		(0X00001FF8U)
#define SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_VAL			(0X1000U)
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_VAL		(0x800000U)
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL		(0x80000000U)

#define SLAVE_BOOT_SBI_IRQ_STATUS	(SLAVE_BOOT_BASEADDR + 0X00000300U)
#define SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK		(0X00000004U)

#define SLAVE_BOOT_SBI_IRQ_ENABLE	(SLAVE_BOOT_BASEADDR + 0X00000308U)
#define SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK		(0X00000004U)

#define XPLMI_SBI_CTRL_INTERFACE_SMAP			(0x0U)
#define XPLMI_SBI_CTRL_INTERFACE_JTAG			(0x4U)
#define XPLMI_SBI_CTRL_INTERFACE_AXI_SLAVE		(0x8U)
#define XPLMI_SBI_CTRL_ENABLE					(0x1U)

/*
 * Definitions required from psm_gloabl_reg
 */
#define PSM_GLOBAL_REG_BASEADDR			(0XFFC90000U)
#define PSM_GLOBAL_REG_PSM_CR_ERR1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001028U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001048U)
#define PSM_GLOBAL_REG_PSM_IRQ1_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001068U)
#define PSM_GLOBAL_REG_PSM_CR_ERR2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001038U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001058U)
#define PSM_GLOBAL_REG_PSM_IRQ2_DIS	(PSM_GLOBAL_REG_BASEADDR + 0X00001078U)
#define PSM_GLOBAL_REG_PSM_IRQ1_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001064U)
#define PSM_GLOBAL_REG_PSM_IRQ2_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001074U)
#define PSM_GLOBAL_REG_PSM_CR_ERR1_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001024U)
#define PSM_GLOBAL_REG_PSM_CR_ERR2_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001034U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001044U)
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_EN	(PSM_GLOBAL_REG_BASEADDR + 0X00001054U)
#define PSM_GLOBAL_REG_PSM_ERR1_STATUS	(PSM_GLOBAL_REG_BASEADDR + 0X00001000U)
#define PSM_GLOBAL_REG_PSM_ERR2_STATUS	(PSM_GLOBAL_REG_BASEADDR + 0X00001004U)

/*
 * Register: EFUSE_CACHE_IP_DISABLE_0
 */
#define EFUSE_CACHE_IP_DISABLE_0	(EFUSE_CACHE_BASEADDR + 0x00000018U)
#define EFUSE_CACHE_IP_DISABLE_0_EID_MASK		(0x07FFC000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID_SEL_MASK		(0x04000000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID1_MASK		(0x000FC000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID1_SHIFT		(14U)
#define EFUSE_CACHE_IP_DISABLE_0_EID2_MASK		(0x03F00000U)
#define EFUSE_CACHE_IP_DISABLE_0_EID2_SHIFT		(20U)

/*
 * Register: PMC_TAP_SLR_TYPE
 */
#define PMC_TAP_SLR_TYPE		(PMC_TAP_BASEADDR + 0X00000024U)
#define PMC_TAP_SLR_TYPE_VAL_MASK		(0X00000007U)

/*
 * Register: PMC_IOU_SLCR
 */
#define PMC_IOU_SLCR_BASEADDR      (0XF1060000U)

/*
 * Register: PMC_IOU_SLCR_SD0_CDN_CTRL
 */
#define PMC_IOU_SLCR_SD0_CDN_CTRL    (PMC_IOU_SLCR_BASEADDR + 0X0000044CU)
#define PMC_IOU_SLCR_SD0_CDN_CTRL_SD0_CDN_CTRL_MASK    (0X00000001U)

/*
 * Register: PMC_IOU_SLCR_SD1_CDN_CTRL
 */
#define PMC_IOU_SLCR_SD1_CDN_CTRL    (PMC_IOU_SLCR_BASEADDR + 0X000004CCU)
#define PMC_IOU_SLCR_SD1_CDN_CTRL_SD1_CDN_CTRL_MASK    (0X00000001U)

/*
 * Definitions required for PMC, PS GPIO
 */
#define PMC_GPIO_DATA_0_OFFSET		(0xF1020040U)
#define PMC_GPIO_DATA_1_OFFSET		(0xF1020044U)
#define PS_GPIO_DATA_0_OFFSET		(0xFF0B0040U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_HW_H */
