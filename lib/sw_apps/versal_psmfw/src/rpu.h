/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
 * RPU Base Address
 */
#define RPU_BASEADDR      0XFF9A0000U

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL    ( ( RPU_BASEADDR ) + 0X00000000U )

#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_SHIFT   10U
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_MASK    0X00000400U

#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_SHIFT   8U
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_MASK    0X00000100U

#define RPU_RPU_GLBL_CNTL_TCM_WAIT_SHIFT   7U
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_MASK    0X00000080U

#define RPU_RPU_GLBL_CNTL_TCM_COMB_SHIFT   6U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_MASK    0X00000040U

#define RPU_RPU_GLBL_CNTL_TEINIT_SHIFT   5U
#define RPU_RPU_GLBL_CNTL_TEINIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_TEINIT_MASK    0X00000020U

#define RPU_RPU_GLBL_CNTL_SLCLAMP_SHIFT   4U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_MASK    0X00000010U

#define RPU_RPU_GLBL_CNTL_SLSPLIT_SHIFT   3U
#define RPU_RPU_GLBL_CNTL_SLSPLIT_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_SLSPLIT_MASK    0X00000008U

#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_SHIFT   2U
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_MASK    0X00000004U

#define RPU_RPU_GLBL_CNTL_CFGIE_SHIFT   1U
#define RPU_RPU_GLBL_CNTL_CFGIE_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_CFGIE_MASK    0X00000002U

#define RPU_RPU_GLBL_CNTL_CFGEE_SHIFT   0U
#define RPU_RPU_GLBL_CNTL_CFGEE_WIDTH   1U
#define RPU_RPU_GLBL_CNTL_CFGEE_MASK    0X00000001U

/**
 * Register: RPU_RPU_GLBL_STATUS
 */
#define RPU_RPU_GLBL_STATUS    ( ( RPU_BASEADDR ) + 0X00000004U )

#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_SHIFT   0U
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_WIDTH   1U
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_MASK    0X00000001U

/**
 * Register: RPU_RPU_0_PWRDWN
 */
#define RPU_RPU_0_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000108U )

#define RPU_RPU_0_PWRDWN_EN_SHIFT   0U
#define RPU_RPU_0_PWRDWN_EN_WIDTH   1U
#define RPU_RPU_0_PWRDWN_EN_MASK    0X00000001U

/**
 * Register: RPU_RPU_1_PWRDWN
 */
#define RPU_RPU_1_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000208U )

#define RPU_RPU_1_PWRDWN_EN_SHIFT   0U
#define RPU_RPU_1_PWRDWN_EN_WIDTH   1U
#define RPU_RPU_1_PWRDWN_EN_MASK    0X00000001U

/**
 * Register: RPU_ERR_INJ
 */
#define RPU_RPU_ERR_INJ     ( ( RPU_BASEADDR ) + 0X00000020U )

#define RPU_RPU_ERR_INJ_DCCMINP_SHIFT   0U
#define RPU_RPU_ERR_INJ_DCCMINP_WIDTH   8U
#define RPU_RPU_ERR_INJ_DCCMINP_MASK    0X000000FFU

#define RPU_RPU_ERR_INJ_DCCMINP2_SHIFT   8U
#define RPU_RPU_ERR_INJ_DCCMINP2_WIDTH   8U
#define RPU_RPU_ERR_INJ_DCCMINP2_MASK    0X0000FF00U

/* RPU config register */
#define RPU_RPU_0_CFG		( ( RPU_BASEADDR ) + 0x00000100U )
#define RPU_RPU_1_CFG		( ( RPU_BASEADDR ) + 0x00000200U )

#define RPU_HIVEC_ADDR		(0xFFFC0000U)
#define RPU_VINITHI_MASK	(0x4U)

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_RPU_H_ */
