/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file rpu.h
*
* This file contains RPU register definitions used by PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  rv   07/17/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_RPU_H_
#define XPSMFW_RPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup psx_rpu_module Realtime Processing Unit registers
 * @{
 */
/**
 * RPU Base Address
 */
#define RPU_BASEADDR      0XEB580000U

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL    ( ( RPU_BASEADDR ) + 0X00000000U )

/**
 * @name RPU Global Control register shifts
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU Global Control register shift
 */
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_SHIFT   10U
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_SHIFT   8U
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_SHIFT   7U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_SHIFT   6U
#define RPU_RPU_GLBL_CNTL_TEINIT_SHIFT   5U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_SHIFT   4U
#define RPU_RPU_GLBL_CNTL_SLSPLIT_SHIFT   3U
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_SHIFT   2U
#define RPU_RPU_GLBL_CNTL_CFGIE_SHIFT   1U
#define RPU_RPU_GLBL_CNTL_CFGEE_SHIFT   0U
/** @} */

/**
 * @name RPU Global Control register masks
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU Global Control register mask
 */
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_MASK    0X00000400U
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_MASK    0X00000100U
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_MASK    0X00000080U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_MASK    0X00000040U
#define RPU_RPU_GLBL_CNTL_TEINIT_MASK    0X00000020U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_MASK    0X00000010U
#define RPU_RPU_GLBL_CNTL_SLSPLIT_MASK    0X00000008U
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_MASK    0X00000004U
#define RPU_RPU_GLBL_CNTL_CFGIE_MASK    0X00000002U
#define RPU_RPU_GLBL_CNTL_CFGEE_MASK    0X00000001U
/** @} */

/**
 * @name RPU Global Control register widths
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU Global Control register width
 */
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TEINIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_SLSPLIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_CFGIE_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_CFGEE_WIDTH   1U
/** @} */

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL    ( ( RPU_BASEADDR ) + 0X00000000U )

/**
 * @name RPU Global status register operations
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU Global status register operation
 */
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_SHIFT   0U
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_WIDTH   1U
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_MASK    0X00000001U
/** @} */

/**
 * Register: RPU_RPU_0_PWRDWN
 */
#define RPU_RPU_0_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000108U )

/**
 * Register: RPU_RPU_1_PWRDWN
 */
#define RPU_RPU_1_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000208U )

/**
 * @name RPU power-down register operations
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU power-down register operation
 */
#define RPU_RPU_0_PWRDWN_EN_SHIFT   0U
#define RPU_RPU_0_PWRDWN_EN_WIDTH   1U
#define RPU_RPU_0_PWRDWN_EN_MASK    0X00000001U

#define RPU_RPU_1_PWRDWN_EN_SHIFT   0U
#define RPU_RPU_1_PWRDWN_EN_WIDTH   1U
#define RPU_RPU_1_PWRDWN_EN_MASK    0X00000001U
/** @} */

/**
 * Register: RPU_ERR_INJ
 */
#define RPU_RPU_ERR_INJ     ( ( RPU_BASEADDR ) + 0X00000020U )

/**
 * @name RPU reserved register operations
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU module reserved register operation
 */
#define RPU_RPU_ERR_INJ_DCCMINP_SHIFT   0U
#define RPU_RPU_ERR_INJ_DCCMINP_WIDTH   8U
#define RPU_RPU_ERR_INJ_DCCMINP_MASK    0X000000FFU

#define RPU_RPU_ERR_INJ_DCCMINP2_SHIFT   8U
#define RPU_RPU_ERR_INJ_DCCMINP2_WIDTH   8U
#define RPU_RPU_ERR_INJ_DCCMINP2_MASK    0X0000FF00U
/** @} */

/**
 * @name RPU configuration registers
 * @ingroup psx_rpu_module
 * @{
 */
/**
 * RPU config register
 */
#define RPU_RPU_0_CFG		( ( RPU_BASEADDR ) + 0x00000100U )
#define RPU_RPU_1_CFG		( ( RPU_BASEADDR ) + 0x00000200U )

#define RPU_HIVEC_ADDR		(0xFFFC0000U)
#define RPU_VINITHI_MASK	(0x4U)

#define RPU_RPU0_CORE0_CFG0		( ( RPU_BASEADDR ) + 0x00008000U )
#define RPU_RPU0_CORE1_CFG0		( ( RPU_BASEADDR ) + 0x0000C000U )
#define RPU_RPU1_CORE0_CFG0		( ( RPU_BASEADDR ) + 0x00018000U )
#define RPU_RPU1_CORE1_CFG0		( ( RPU_BASEADDR ) + 0x0001C000U )

#define RPU_CORE_CPUHALT_MASK (0x00000001U)

#define RPU_TCMBOOT_MASK (0x00000010U)

#define PSX_RPU_CLUSTER_A0_CORE_0_VECTABLE    ( ( RPU_BASEADDR ) + 0x00008010U )
#define PSX_RPU_CLUSTER_A1_CORE_1_VECTABLE    ( ( RPU_BASEADDR ) + 0x0000C010U )
#define PSX_RPU_CLUSTER_B0_CORE_0_VECTABLE    ( ( RPU_BASEADDR ) + 0x00018010U )
#define PSX_RPU_CLUSTER_B1_CORE_1_VECTABLE    ( ( RPU_BASEADDR ) + 0x0001C010U )

#define RPU_A_BASEADDR				(0xEB580000U)
#define RPU_B_BASEADDR				(0xEB590000U)
#define RPU_A_CLUSTER_CFG			(RPU_A_BASEADDR)
#define RPU_B_CLUSTER_CFG			(RPU_B_BASEADDR)
#define RPU_CLUSTER_CFG_SLSPLIT_MASK		(0x00000001U)
/** @} */
/** @} */

/**
 * @defgroup lpd_slcr_module_vn Global System Level Control Registers for LPD
 * @{
 */
/**
 * LPD SLCR base address
 */
#define LPD_SLCR_BASEADDR    0xEB410000U

/**
 * @name LPD SLCR RPU PCIL registers
 * @ingroup lpd_slcr_module_vn
 * @{
 */
/**
 *  LPD SLCR RPU PCIL register address
 */
#define LPD_SLCR_RPU_PCIL_A0_ISR    ( (LPD_SLCR_BASEADDR) + 0x00010000U )
#define LPD_SLCR_RPU_PCIL_A0_PS    ( (LPD_SLCR_BASEADDR) + 0x00010084U )
#define LPD_SLCR_RPU_PCIL_A0_PR    ( (LPD_SLCR_BASEADDR) + 0x00010080U )
#define LPD_SLCR_RPU_PCIL_A0_PA    ( (LPD_SLCR_BASEADDR) + 0x00010088U )

#define LPD_SLCR_RPU_PCIL_A1_ISR    ( (LPD_SLCR_BASEADDR) + 0x00010100U )
#define LPD_SLCR_RPU_PCIL_A1_PS    ( (LPD_SLCR_BASEADDR) + 0x00010184U )
#define LPD_SLCR_RPU_PCIL_A1_PR    ( (LPD_SLCR_BASEADDR) + 0x00010180U )
#define LPD_SLCR_RPU_PCIL_A1_PA    ( (LPD_SLCR_BASEADDR) + 0x00010188U )

#define LPD_SLCR_RPU_PCIL_B0_ISR    ( (LPD_SLCR_BASEADDR) + 0x00011000U )
#define LPD_SLCR_RPU_PCIL_B0_PS    ( (LPD_SLCR_BASEADDR) + 0x00011084U )
#define LPD_SLCR_RPU_PCIL_B0_PR    ( (LPD_SLCR_BASEADDR) + 0x00011080U )
#define LPD_SLCR_RPU_PCIL_B0_PA    ( (LPD_SLCR_BASEADDR) + 0x00011088U )

#define LPD_SLCR_RPU_PCIL_B1_ISR    ( (LPD_SLCR_BASEADDR) + 0x00011100U )
#define LPD_SLCR_RPU_PCIL_B1_PS    ( (LPD_SLCR_BASEADDR) + 0x00011184U )
#define LPD_SLCR_RPU_PCIL_B1_PR    ( (LPD_SLCR_BASEADDR) + 0x00011180U )
#define LPD_SLCR_RPU_PCIL_B1_PA    ( (LPD_SLCR_BASEADDR) + 0x00011188U )
/** @} */

/**
 * @name LPD SLCR RPU PCIL register masks
 * @ingroup lpd_slcr_module_vn
 * @{
 */
/**
 *  LPD SLCR RPU PCIL register mask
 */
#define LPD_SLCR_RPU_PCIL_A0_ISR_PACTIVE1_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A0_PS_PSTATE_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A0_PR_PREQ_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A0_PA_PACCEPT_MASK  0x00000100U

#define LPD_SLCR_RPU_PCIL_A1_ISR_PACTIVE1_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A1_PS_PSTATE_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A1_PR_PREQ_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_A1_PA_PACCEPT_MASK  0x00000100U

#define LPD_SLCR_RPU_PCIL_B0_ISR_PACTIVE1_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B0_PS_PSTATE_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B0_PR_PREQ_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B0_PA_PACCEPT_MASK  0x00000100U

#define LPD_SLCR_RPU_PCIL_B1_ISR_PACTIVE1_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B1_PS_PSTATE_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B1_PR_PREQ_MASK  0x00000001U
#define LPD_SLCR_RPU_PCIL_B1_PA_PACCEPT_MASK  0x00000100U

#define LPD_SLCR_RPU_PCIL_A0_PA_PACTIVE_MASK (0x00000003U)
#define LPD_SLCR_RPU_PCIL_A1_PA_PACTIVE_MASK (0x00000003U)
#define LPD_SLCR_RPU_PCIL_B0_PA_PACTIVE_MASK (0x00000003U)
#define LPD_SLCR_RPU_PCIL_B1_PA_PACTIVE_MASK (0x00000003U)
/** @} */

/**
 * @name LPD SLCR RPU PCIL register offsets
 * @ingroup lpd_slcr_module_vn
 * @{
 */
/**
 *  LPD SLCR RPU PCIL register offset
 */
#define LPX_SLCR_RPU_PCIL_CORE_IEN_OFFSET (0x8U)
#define LPX_SLCR_RPU_PCIL_CORE_IDS_OFFSET (0xCU)
#define LPX_SLCR_RPU_PCIL_PWRDWN_OFFSET (0x38U)
/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_RPU_H_ */
