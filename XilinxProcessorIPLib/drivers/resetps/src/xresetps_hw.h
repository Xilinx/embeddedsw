/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XRESETPS_HW_H
#define XRESETPS_HW_H

/*****************************************************************************/
/**
*
* @file xresetps_hw.h
*
* @brief Defines the hardware interface to the System Reset controller.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.1   Nava   04/20/18 Fixed compilation warnings.
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.7   sd     11/07/23 Remove the unused pss ref frequency
* 1.8   vlt    01/27/26 Fixed codespell issues.
* 1.9   vn     09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup resetps Overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/** @name Register address defines
 *
 * CRF_APB defines.
 * @{
 */
#define XPAR_PSU_CRL_APB_S_AXI_BASEADDR 0xFF5E0000 /**< CRL APB base address. */
/** IOU SLCR base address. */
#define XPAR_PSU_IOUSLCR_0_S_AXI_BASEADDR 0xFF180000
/** CRF APB base address. */
#define XPAR_PSU_CRF_APB_S_AXI_BASEADDR		0xFD1A0000
/** PMU IO module base address. */
#define XPAR_PSU_PMU_IOMODULE_S_AXI_BASEADDR 0xFFD40000
/** PMU global base address. */
#define XPAR_PSU_PMU_GLOBAL_0_S_AXI_BASEADDR 0xFFD80000
/** LPD SLCR base address. */
#define XPAR_PSU_LPD_SLCR_S_AXI_BASEADDR 0xFF410000
/** CRF APB base address. */
#define XRESETPS_CRF_APB_BASE     (XPAR_PSU_CRF_APB_S_AXI_BASEADDR)
/** @} */

/** @name RST_FPD_TOP Address and mask definitions
 * @{
 */
/** FPD TOP reset register address. */
#define XRESETPS_CRF_APB_RST_FPD_TOP \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X00000100U))
/** PCIe CFG reset mask. */
#define PCIE_CFG_RESET_MASK       ((u32)0X00080000U)
/** PCIe bridge reset mask. */
#define PCIE_BRIDGE_RESET_MASK    ((u32)0X00040000U)
/** PCIe CTRL reset mask. */
#define PCIE_CTRL_RESET_MASK      ((u32)0X00020000U)
#define DP_RESET_MASK             ((u32)0X00010000U) /**< DP reset mask. */
#define SWDT_RESET_MASK           ((u32)0X00008000U) /**< SWDT reset mask. */
#define AFI_FM5_RESET_MASK        ((u32)0X00001000U) /**< AFI FM5 reset mask. */
#define AFI_FM4_RESET_MASK        ((u32)0X00000800U) /**< AFI FM4 reset mask. */
#define AFI_FM3_RESET_MASK        ((u32)0X00000400U) /**< AFI FM3 reset mask. */
#define AFI_FM2_RESET_MASK        ((u32)0X00000200U) /**< AFI FM2 reset mask. */
#define AFI_FM1_RESET_MASK        ((u32)0X00000100U) /**< AFI FM1 reset mask. */
#define AFI_FM0_RESET_MASK        ((u32)0X00000080U) /**< AFI FM0 reset mask. */
#define GDMA_RESET_MASK           ((u32)0X00000040U) /**< GDMA reset mask. */
#define GPU_PP1_RESET_MASK        ((u32)0X00000020U) /**< GPU PP1 reset mask. */
#define GPU_PP0_RESET_MASK        ((u32)0X00000010U) /**< GPU PP0 reset mask. */
#define GPU_RESET_MASK            ((u32)0X00000008U) /**< GPU reset mask. */
#define GT_RESET_MASK             ((u32)0X00000004U) /**< GT reset mask. */
#define SATA_RESET_MASK           ((u32)0X00000002U) /**< SATA reset mask. */
/** @} */

/** @name RST_FPD_APU Address and mask definitions
 * @{
 */
/** FPD APU reset register address. */
#define XRESETPS_CRF_APB_RST_FPD_APU \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X00000104U))
/** ACPU3 power-on reset mask. */
#define ACPU3_PWRON_RESET_MASK    ((u32)0X00002000U)
/** ACPU2 power-on reset mask. */
#define ACPU2_PWRON_RESET_MASK    ((u32)0X00001000U)
/** ACPU1 power-on reset mask. */
#define ACPU1_PWRON_RESET_MASK    ((u32)0X00000800U)
/** ACPU0 power-on reset mask. */
#define ACPU0_PWRON_RESET_MASK    ((u32)0X00000400U)
#define APU_L2_RESET_MASK         ((u32)0X00000100U) /**< APU L2 reset mask. */
#define ACPU3_RESET_MASK          ((u32)0X00000008U) /**< ACPU3 reset mask. */
#define ACPU2_RESET_MASK          ((u32)0X00000004U) /**< ACPU2 reset mask. */
#define ACPU1_RESET_MASK          ((u32)0X00000002U) /**< ACPU1 reset mask. */
#define ACPU0_RESET_MASK          ((u32)0X00000001U) /**< ACPU0 reset mask. */
/** @} */

/** @name RST_DDR_SS Address and mask definitions
 * @{
 */
/** DDR SS reset register address. */
#define XRESETPS_CRF_APB_RST_DDR_SS \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X00000108U))
#define DDR_RESET_MASK            ((u32)0X00000008U) /**< DDR reset mask. */
#define DDR_APM_RESET_MASK        ((u32)0X00000004U) /**< DDR APM reset mask. */
/** @} */

/** @name APLL_CTRL Address and mask definitions
 * @{
 */
/** APLL control register address. */
#define XRESETPS_CRF_APB_APLL_CTRL \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X00000020U))
#define APLL_RESET_MASK           ((u32)0X00000001U) /**< APLL reset mask. */
/** @} */

/** @name DPLL_CTRL Address and mask definitions
 * @{
 */
/** DPLL control register address. */
#define XRESETPS_CRF_APB_DPLL_CTRL \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X0000002CU))
#define DPLL_RESET_MASK           ((u32)0X00000001U) /**< DPLL reset mask. */
/** @} */

/** @name VPLL_CTRL Address and mask definitions
 * @{
 */
/** VPLL control register address. */
#define XRESETPS_CRF_APB_VPLL_CTRL \
	((XRESETPS_CRF_APB_BASE) + ((u32)0X00000038U))
#define VPLL_RESET_MASK           ((u32)0X00000001U) /**< VPLL reset mask. */

/** @} */

/* CRL_APB defines */
/** CRL APB base address. */
#define XRESETPS_CRL_APB_BASE     (XPAR_PSU_CRL_APB_S_AXI_BASEADDR)

/** @name RESET_CTRL Address and mask definitions
 * @{
 */
/** CRL APB RESET_CTRL register address. */
#define XRESETPS_CRL_APB_RESET_CTRL \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000218U))
#define SOFT_RESET_MASK           ((u32)0X00000010U) /**< Soft reset mask. */
/** @} */

/** @name RST_LPD_IOU0 Address and mask definitions
 * @{
 */
/** CRL APB RST_LPD_IOU0 register address. */
#define XRESETPS_CRL_APB_RST_LPD_IOU0 \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000230U))
#define GEM0_RESET_MASK           ((u32)0X00000001U) /**< GEM0 reset mask. */
#define GEM1_RESET_MASK           ((u32)0X00000002U) /**< GEM1 reset mask. */
#define GEM2_RESET_MASK           ((u32)0X00000004U) /**< GEM2 reset mask. */
#define GEM3_RESET_MASK           ((u32)0X00000008U) /**< GEM3 reset mask. */
/** @} */

/** @name RST_LPD_IOU2 Address and mask definitions
 * @{
 */
/** CRL APB RST_LPD_IOU2 register address. */
#define XRESETPS_CRL_APB_RST_LPD_IOU2 \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000238U))
#define QSPI_RESET_MASK           ((u32)0X00000001U) /**< QSPI reset mask. */
#define UART0_RESET_MASK          ((u32)0X00000002U) /**< UART0 reset mask. */
#define UART1_RESET_MASK          ((u32)0X00000004U) /**< UART1 reset mask. */
#define SPI0_RESET_MASK           ((u32)0X00000008U) /**< SPI0 reset mask. */
#define SPI1_RESET_MASK           ((u32)0X00000010U) /**< SPI1 reset mask. */
#define SDIO0_RESET_MASK          ((u32)0X00000020U) /**< SDIO0 reset mask. */
#define SDIO1_RESET_MASK          ((u32)0X00000040U) /**< SDIO1 reset mask. */
#define CAN0_RESET_MASK           ((u32)0X00000080U) /**< CAN0 reset mask. */
#define CAN1_RESET_MASK           ((u32)0X00000100U) /**< CAN1 reset mask. */
#define I2C0_RESET_MASK           ((u32)0X00000200U) /**< I2C0 reset mask. */
#define I2C1_RESET_MASK           ((u32)0X00000400U) /**< I2C1 reset mask. */
#define TTC0_RESET_MASK           ((u32)0X00000800U) /**< TTC0 reset mask. */
#define TTC1_RESET_MASK           ((u32)0X00001000U) /**< TTC1 reset mask. */
#define TTC2_RESET_MASK           ((u32)0X00002000U) /**< TTC2 reset mask. */
#define TTC3_RESET_MASK           ((u32)0X00004000U) /**< TTC3 reset mask. */
#define SWDT_RESET_MASK           ((u32)0X00008000U) /**< SWDT reset mask. */
#define NAND_RESET_MASK           ((u32)0X00010000U) /**< NAND reset mask. */
#define ADMA_RESET_MASK           ((u32)0X00020000U) /**< ADMA reset mask. */
#define GPIO_RESET_MASK           ((u32)0X00040000U) /**< GPIO reset mask. */
#define IOU_CC_RESET_MASK         ((u32)0X00080000U) /**< IOU CC reset mask. */
/** Timestamp reset mask. */
#define TIMESTAMP_RESET_MASK      ((u32)0X00100000U)
/** @} */

/** @name RST_LPD_TOP Address and mask definitions
 * @{
 */
/** CRL APB RST_LPD_TOP register address. */
#define XRESETPS_CRL_APB_RST_LPD_TOP \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X0000023CU))
#define RPU_R50_RESET_MASK        ((u32)0X00000001U) /**< RPU R50 reset mask. */
#define RPU_R51_RESET_MASK        ((u32)0X00000002U) /**< RPU R51 reset mask. */
/** RPU AMBA reset mask. */
#define RPU_AMBA_RESET_MASK       ((u32)0X00000004U)
#define OCM_RESET_MASK            ((u32)0X00000008U) /**< OCM reset mask. */
#define RPU_PGE_RESET_MASK        ((u32)0X00000010U) /**< RPU PGE reset mask. */
/** USB0 core reset mask. */
#define USB0_CORERESET_MASK       ((u32)0X00000040U)
/** USB1 core reset mask. */
#define USB1_CORERESET_MASK       ((u32)0X00000080U)
/** USB0 hibernation reset mask. */
#define USB0_HIBERRESET_MASK      ((u32)0X00000100U)
/** USB1 hibernation reset mask. */
#define USB1_HIBERRESET_MASK      ((u32)0X00000200U)
/** USB0 APB reset mask. */
#define USB0_APB_RESET_MASK       ((u32)0X00000400U)
/** USB1 APB reset mask. */
#define USB1_APB_RESET_MASK       ((u32)0X00000800U)
#define IPI_RESET_MASK            ((u32)0X00004000U) /**< IPI reset mask. */
#define APM_RESET_MASK            ((u32)0X00008000U) /**< APM reset mask. */
#define RTC_RESET_MASK            ((u32)0X00010000U) /**< RTC reset mask. */
#define SYSMON_RESET_MASK         ((u32)0X00020000U) /**< SYSMON reset mask. */
#define AFI_FM6_RESET_MASK        ((u32)0X00080000U) /**< AFI FM6 reset mask. */
/** LPD SWDT reset mask. */
#define LPD_SWDT_RESET_MASK       ((u32)0X00100000U)
#define FPD_RESET_MASK            ((u32)0X00800000U) /**< FPD reset mask. */
/** @} */

/** @name RST_LPD_DBG Address and mask definitions
 * @{
 */
/** CRL APB RST_LPD_DBG register address. */
#define XRESETPS_CRL_APB_RST_LPD_DBG \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000240U))
/** RPU DBG1 reset mask. */
#define RPU_DBG1_RESET_MASK       ((u32)0X00000020U)
/** RPU DBG0 reset mask. */
#define RPU_DBG0_RESET_MASK       ((u32)0X00000010U)
#define DBG_LPD_RESET_MASK        ((u32)0X00000002U) /**< DBG LPD reset mask. */
#define DBG_FPD_RESET_MASK        ((u32)0X00000001U) /**< DBG FPD reset mask. */
/** @} */

/** @name IOPLL_CTRL Address and mask definitions
 * @{
 */
/** IOPLL control register address. */
#define XRESETPS_CRL_APB_IOPLL_CTRL \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000020U))
#define IOPLL_RESET_MASK          ((u32)0X00000001U) /**< IOPLL reset mask. */
/** @} */

/** @name RPLL_CTRL Address and mask definitions
 * @{
 */
/** RPLL control register address. */
#define XRESETPS_CRL_APB_RPLL_CTRL \
	((XRESETPS_CRL_APB_BASE) + ((u32)0X00000030U))
#define RPLL_RESET_MASK           ((u32)0X00000001U) /**< RPLL reset mask. */
#define RPLL_BYPASS_MASK          ((u32)0X00000008U) /**< RPLL bypass mask. */

/** @} */

/* PMU_IOM defines */
/** PMU IOM base address. */
#define XRESETPS_PMU_IOM_BASE     (XPAR_PSU_PMU_IOMODULE_S_AXI_BASEADDR)

/** @name PMU_IOM_GPO3 Address and mask definitions
 * @{
 */
/** PMU IOM GPO3 control register address. */
#define XRESETPS_PMU_IOM_GPO3_CTRL \
	((XRESETPS_PMU_IOM_BASE) + ((u32)0X0000001CU))
/** GPO3 PL0 reset mask. */
#define GPO3_PL0_RESET_MASK       ((u32)0X00000001U)
/** GPO3 PL1 reset mask. */
#define GPO3_PL1_RESET_MASK       ((u32)0X00000002U)
/** GPO3 PL2 reset mask. */
#define GPO3_PL2_RESET_MASK       ((u32)0X00000004U)
/** GPO3 PL3 reset mask. */
#define GPO3_PL3_RESET_MASK       ((u32)0X00000008U)
/** GPO3 PL4 reset mask. */
#define GPO3_PL4_RESET_MASK       ((u32)0X00000010U)
/** GPO3 PL5 reset mask. */
#define GPO3_PL5_RESET_MASK       ((u32)0X00000020U)
/** GPO3 PL6 reset mask. */
#define GPO3_PL6_RESET_MASK       ((u32)0X00000040U)
/** GPO3 PL7 reset mask. */
#define GPO3_PL7_RESET_MASK       ((u32)0X00000080U)
/** GPO3 PL8 reset mask. */
#define GPO3_PL8_RESET_MASK       ((u32)0X00000100U)
/** GPO3 PL9 reset mask. */
#define GPO3_PL9_RESET_MASK       ((u32)0X00000200U)
/** GPO3 PL10 reset mask. */
#define GPO3_PL10_RESET_MASK      ((u32)0X00000400U)
/** GPO3 PL11 reset mask. */
#define GPO3_PL11_RESET_MASK      ((u32)0X00000800U)
/** GPO3 PL12 reset mask. */
#define GPO3_PL12_RESET_MASK      ((u32)0X00001000U)
/** GPO3 PL13 reset mask. */
#define GPO3_PL13_RESET_MASK      ((u32)0X00002000U)
/** GPO3 PL14 reset mask. */
#define GPO3_PL14_RESET_MASK      ((u32)0X00004000U)
/** GPO3 PL15 reset mask. */
#define GPO3_PL15_RESET_MASK      ((u32)0X00008000U)
/** GPO3 PL16 reset mask. */
#define GPO3_PL16_RESET_MASK      ((u32)0X00010000U)
/** GPO3 PL17 reset mask. */
#define GPO3_PL17_RESET_MASK      ((u32)0X00020000U)
/** GPO3 PL18 reset mask. */
#define GPO3_PL18_RESET_MASK      ((u32)0X00040000U)
/** GPO3 PL19 reset mask. */
#define GPO3_PL19_RESET_MASK      ((u32)0X00080000U)
/** GPO3 PL20 reset mask. */
#define GPO3_PL20_RESET_MASK      ((u32)0X00100000U)
/** GPO3 PL21 reset mask. */
#define GPO3_PL21_RESET_MASK      ((u32)0X00200000U)
/** GPO3 PL22 reset mask. */
#define GPO3_PL22_RESET_MASK      ((u32)0X00400000U)
/** GPO3 PL23 reset mask. */
#define GPO3_PL23_RESET_MASK      ((u32)0X00800000U)
/** GPO3 PL24 reset mask. */
#define GPO3_PL24_RESET_MASK      ((u32)0X01000000U)
/** GPO3 PL25 reset mask. */
#define GPO3_PL25_RESET_MASK      ((u32)0X02000000U)
/** GPO3 PL26 reset mask. */
#define GPO3_PL26_RESET_MASK      ((u32)0X04000000U)
/** GPO3 PL27 reset mask. */
#define GPO3_PL27_RESET_MASK      ((u32)0X08000000U)
/** GPO3 PL28 reset mask. */
#define GPO3_PL28_RESET_MASK      ((u32)0X10000000U)
/** GPO3 PL29 reset mask. */
#define GPO3_PL29_RESET_MASK      ((u32)0X20000000U)
/** GPO3 PL30 reset mask. */
#define GPO3_PL30_RESET_MASK      ((u32)0X40000000U)
/** GPO3 PL31 reset mask. */
#define GPO3_PL31_RESET_MASK      ((u32)0X80000000U)

/** @} */

/** @name PMU_LCL defines
 * @{
 */
#ifndef XPAR_PSU_PMU_LOCAL_0_S_AXI_BASEADDR
/** PMU local base address. */
#define XPAR_PSU_PMU_LOCAL_0_S_AXI_BASEADDR 0xFFD60000
#endif
/** PMU local base address. */
#define XRESETPS_PMU_LCL_BASE     (XPAR_PSU_PMU_LOCAL_0_S_AXI_BASEADDR)
/** @} */

/** GPO read control address. */
#define XRESETPS_PMU_LCL_READ_CTRL \
	((XRESETPS_PMU_LCL_BASE) + ((u32)0X0000021CU))

/* PMU_GLB defines */
/** PMU global base address. */
#define XRESETPS_PMU_GLB_BASE     (XPAR_PSU_PMU_GLOBAL_0_S_AXI_BASEADDR)

/** @name PMU_GLB_RST Address and mask definitions
 * @{
 */
/** PMU global reset control register address. */
#define XRESETPS_PMU_GLB_RST_CTRL ((XRESETPS_PMU_GLB_BASE) + ((u32)0X00000608U))
/** RPU LS reset mask. */
#define RPU_LS_RESET_MASK         ((u32)0X00000100U)
#define FPD_APU_RESET_MASK        ((u32)0X00000200U) /**< FPD APU reset mask. */
#define PS_ONLY_RESET_MASK        ((u32)0X00000400U) /**< PS-only reset mask. */
/** @} */

/** @name PMU_GLB_PS Address and mask definitions
 * @{
 */
/** PMU global PS control register address. */
#define XRESETPS_PMU_GLB_PS_CTRL  ((XRESETPS_PMU_GLB_BASE) + ((u32)0X00000004U))
#define PROG_ENABLE_MASK          ((u32)0X00000002U) /**< PROG enable mask. */
#define PROG_GATE_MASK            ((u32)0X00000001U) /**< PROG gate mask. */
/** @} */

/** @name PMU_GLB_AIB Address and mask definitions
 * @{
 */
/** PMU global AIB control register address. */
#define XRESETPS_PMU_GLB_AIB_CTRL ((XRESETPS_PMU_GLB_BASE) + ((u32)0X00000600U))
/** LPD AFI FM isolation mask. */
#define LPD_AFI_FM_ISO_MASK       ((u32)0X00000001U)
/** LPD AFI FS isolation mask. */
#define LPD_AFI_FS_ISO_MASK       ((u32)0X00000002U)
/** FPD AFI FM isolation mask. */
#define FPD_AFI_FM_ISO_MASK       ((u32)0X00000004U)
/** FPD AFI FS isolation mask. */
#define FPD_AFI_FS_ISO_MASK       ((u32)0X00000008U)
/** AIB isolation control mask. */
#define AIB_ISO_CTRL_MASK         (LPD_AFI_FM_ISO_MASK | LPD_AFI_FS_ISO_MASK | \
				   FPD_AFI_FM_ISO_MASK | FPD_AFI_FS_ISO_MASK)
/** @} */

/** @name PMU_GLB_AIB status Address and mask definitions
 * @{
 */
/** PMU global AIB status register address. */
#define XRESETPS_PMU_GLB_AIB_STATUS \
	((XRESETPS_PMU_GLB_BASE) + ((u32)0X00000604U))
/** AIB isolation status mask. */
#define AIB_ISO_STATUS_MASK       (AIB_ISO_CTRL_MASK)
/** @} */

/** @name PMU_GLB_PWR status Address and mask definitions
 * @{
 */
/** PMU global power status register address. */
#define XRESETPS_PMU_GLB_PWR_STATUS \
	((XRESETPS_PMU_GLB_BASE) + ((u32)0X00000100U))
/** FPD power state check mask. */
#define FPD_PSCHK_MASK            ((u32)0x00400000U)
/** PCIe CTRL power state check mask. */
#define PCIE_CTRL_PSCHK_MASK      (FPD_PSCHK_MASK)
/** DP power state check mask. */
#define DP_PSCHK_MASK             (FPD_PSCHK_MASK)
/** SATA power state check mask. */
#define SATA_PSCHK_MASK           (FPD_PSCHK_MASK)
/** R50 power state check mask. */
#define R50_PSCHK_MASK            ((u32)0x00000400U)
/** R51 power state check mask. */
#define R51_PSCHK_MASK            ((u32)0x00000800U)
/** GPU PP0 power state check mask. */
#define GPU_PP0_PSCHK_MASK        (((u32)0x00000010U) | (FPD_PSCHK_MASK))
/** GPU PP1 power state check mask. */
#define GPU_PP1_PSCHK_MASK        (((u32)0x00000020U) | (FPD_PSCHK_MASK))
/** GPU power state check mask. */
#define GPU_PSCHK_MASK            ((GPU_PP0_PSCHK_MASK) | \
				   (GPU_PP1_PSCHK_MASK) | (FPD_PSCHK_MASK))

/** @} */

/* LPD_SLCR defines */
/** LPD SLCR base address. */
#define XRESETPS_LPD_SCR_BASE     (XPAR_PSU_LPD_SLCR_S_AXI_BASEADDR)

/** @name LPD_SCR_AXIISO_REQ and LPD_SCR_AXIISO_ACK Address and mask definitions
 * @{
 */
/** LPD SLCR AXI isolation request register address. */
#define XRESETPS_LPD_SCR_AXIISO_REQ_CTRL \
	((XRESETPS_LPD_SCR_BASE) + ((u32)0X00003030U))
/** LPD SLCR AXI isolation acknowledge register address. */
#define XRESETPS_LPD_SCR_AXIISO_ACK_CTRL \
	((XRESETPS_LPD_SCR_BASE) + ((u32)0X00003040U))
/** RPU0 master isolation mask. */
#define RPU0_MASTER_ISO_MASK      ((u32)0X00010000U)
/** RPU1 master isolation mask. */
#define RPU1_MASTER_ISO_MASK      ((u32)0X00020000U)
/** RPU master isolation mask. */
#define RPU_MASTER_ISO_MASK	  (RPU0_MASTER_ISO_MASK | RPU1_MASTER_ISO_MASK)
/** RPU0 slave isolation mask. */
#define RPU0_SLAVE_ISO_MASK       ((u32)0X00040000U)
/** RPU1 slave isolation mask. */
#define RPU1_SLAVE_ISO_MASK       ((u32)0X00080000U)
/** RPU slave isolation mask. */
#define RPU_SLAVE_ISO_MASK	  (RPU0_SLAVE_ISO_MASK | RPU1_SLAVE_ISO_MASK)
/** FPD OCM isolation mask. */
#define FPD_OCM_ISO_MASK          ((u32)0X00000008U)
/** FPD LPDIBS isolation mask. */
#define FPD_LPDIBS_ISO_MASK       ((u32)0X00000004U)
/** AFIFS1 isolation mask. */
#define AFIFS1_ISO_MASK           ((u32)0X00000002U)
/** AFIFS0 isolation mask. */
#define AFIFS0_ISO_MASK           ((u32)0X00000001U)
/** FPD to LPD isolation mask. */
#define FPD_TO_LPD_ISO_MASK       (FPD_LPDIBS_ISO_MASK | FPD_OCM_ISO_MASK | \
				   AFIFS0_ISO_MASK | AFIFS1_ISO_MASK)
/** LPD DDR isolation mask. */
#define LPD_DDR_ISO_MASK          ((u32)0X08000000U)
/** FPD MAIN isolation mask. */
#define FPD_MAIN_ISO_MASK         ((u32)0X01000000U)
/** LPD to FPD isolation mask. */
#define LPD_TO_FPD_ISO_MASK       (LPD_DDR_ISO_MASK | FPD_MAIN_ISO_MASK)
/** @} */

/** @name LPD_SLCR_APBISO_REQ Address and mask definitions
 * @{
 */
/** LPD SLCR APB isolation request register address. */
#define XRESETPS_LPD_SLCR_APBISO_REQ_CTRL \
	((XRESETPS_LPD_SCR_BASE) + ((u32)0X00003048U))
#define GPU_ISO_MASK              ((u32)0X00000001U) /**< GPU isolation mask. */

/** @} */

/** @name CSU defines
 * @{
 */
#define XRESETPS_CSU_BASE         (0xFFCA0000U) /**< CSU base address. */
/** CSU version register address. */
#define XRESETPS_CSU_VERSION_REG  ((XRESETPS_CSU_BASE) + ((u32)0x00000044U))
/** Platform PS version 1 value. */
#define XRESETPS_PLATFORM_PS_VER1 ((u32)0x00000000U)
#define PS_VERSION_MASK           ((u32)0x0000000FU) /**< PS version mask. */

/** @} */

/** @name Timeout delay defines
 * @{
 */
#define XRESETPS_RST_PROP_DELAY   (0xFU) /**< Reset propagation delay. */
#define XRESETPS_AIB_ISO_DELAY	  (0xFU) /**< AIB isolation delay. */
#define XRESETPS_AIB_PSPL_DELAY	  (0xFU) /**< AIB PS-PL delay. */
#define XRESETPS_PULSE_PROP_DELAY (0xFU) /**< Pulse propagation delay. */

/** AIB acknowledgment reconfirmation count. */
#define XRESETPS_AIB_PSPL_RECONFIRM_CNT \
	(0x2U)
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* end of protection macro */
