/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XSYSMONPSU_HW_H__
#define XSYSMONPSU_HW_H__

/*****************************************************************************/
/**
*
* @file xsysmonpsu_hw.h
* @brief Defines register identifiers and low-level access for SYSMONPSU.
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xsysmonpsu.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   kvn	  12/15/15 First release
* 2.0   vns    08/14/16  Added CFG_REG3, SEQ_INPUT_MODE2, SEQ_ACQ2,
*                        SEQ_CH2 and SEQ_AVG2 offsets and bit masks
* 2.1   sk     03/03/16 Check for PL reset before doing PL Sysmon reset.
* 2.9   cog    07/20/23 Added support for SDT flow.
* 3.0   se     07/30/24 OT Upper Alarm threshold requires 12-bit value update
*                       to enable over-temperature default value override.
* 3.1   sa     12/10/24 Modified code for MISRA-C:2012 Compliance
* 3.4   vn     09/29/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup Overview
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#ifndef SDT
#include "xparameters.h"
#endif

/**
 * XSysmonPsu Base Address
 */
#define XSYSMONPSU_BASEADDR      0xFFA50000U

/**
 * @name XSysmonPsuMisc register definitions
 * @{
 */
/** Offset of the MISC register. */
#define XSYSMONPSU_MISC_OFFSET   0x00000000U
/** Reset value of the MISC register. */
#define XSYSMONPSU_MISC_RSTVAL   0x00000000U

/** Bit shift of the SLVERR_EN_DRP field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_DRP_SHIFT   1U
/** Bit width of the SLVERR_EN_DRP field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_DRP_WIDTH   1U
/** Bit mask of the SLVERR_EN_DRP field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_DRP_MASK    0x00000002U

/** Bit shift of the SLVERR_EN field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_SHIFT   0U
/** Bit width of the SLVERR_EN field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_WIDTH   1U
/** Bit mask of the SLVERR_EN field in MISC. */
#define XSYSMONPSU_MISC_SLVERR_EN_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuIsr0 register definitions
 * @{
 */
/** Offset of the ISR_0 register. */
#define XSYSMONPSU_ISR_0_OFFSET   0x00000010U
/** Bit mask of the ISR_0 register. */
#define XSYSMONPSU_ISR_0_MASK	  0xffffffffU
/** Reset value of the ISR_0 register. */
#define XSYSMONPSU_ISR_0_RSTVAL   0x00000000U

/** Bit shift of the PL_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_15_SHIFT   31U
/** Bit width of the PL_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_15_WIDTH   1U
/** Bit mask of the PL_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_15_MASK    0x80000000U

/** Bit shift of the PL_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_14_SHIFT   30U
/** Bit width of the PL_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_14_WIDTH   1U
/** Bit mask of the PL_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_14_MASK    0x40000000U

/** Bit shift of the PL_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_13_SHIFT   29U
/** Bit width of the PL_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_13_WIDTH   1U
/** Bit mask of the PL_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_13_MASK    0x20000000U

/** Bit shift of the PL_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_12_SHIFT   28U
/** Bit width of the PL_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_12_WIDTH   1U
/** Bit mask of the PL_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_12_MASK    0x10000000U

/** Bit shift of the PL_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_11_SHIFT   27U
/** Bit width of the PL_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_11_WIDTH   1U
/** Bit mask of the PL_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_11_MASK    0x08000000U

/** Bit shift of the PL_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_10_SHIFT   26U
/** Bit width of the PL_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_10_WIDTH   1U
/** Bit mask of the PL_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_10_MASK    0x04000000U

/** Bit shift of the PL_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_9_SHIFT   25U
/** Bit width of the PL_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_9_WIDTH   1U
/** Bit mask of the PL_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_9_MASK    0x02000000U

/** Bit shift of the PL_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_8_SHIFT   24U
/** Bit width of the PL_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_8_WIDTH   1U
/** Bit mask of the PL_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_8_MASK    0x01000000U

/** Bit shift of the PL_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_7_SHIFT   23U
/** Bit width of the PL_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_7_WIDTH   1U
/** Bit mask of the PL_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_7_MASK    0x00800000U

/** Bit shift of the PL_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_6_SHIFT   22U
/** Bit width of the PL_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_6_WIDTH   1U
/** Bit mask of the PL_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_6_MASK    0x00400000U

/** Bit shift of the PL_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_5_SHIFT   21U
/** Bit width of the PL_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_5_WIDTH   1U
/** Bit mask of the PL_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_5_MASK    0x00200000U

/** Bit shift of the PL_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_4_SHIFT   20U
/** Bit width of the PL_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_4_WIDTH   1U
/** Bit mask of the PL_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_4_MASK    0x00100000U

/** Bit shift of the PL_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_3_SHIFT   19U
/** Bit width of the PL_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_3_WIDTH   1U
/** Bit mask of the PL_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_3_MASK    0x00080000U

/** Bit shift of the PL_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_2_SHIFT   18U
/** Bit width of the PL_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_2_WIDTH   1U
/** Bit mask of the PL_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_2_MASK    0x00040000U

/** Bit shift of the PL_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_1_SHIFT   17U
/** Bit width of the PL_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_1_WIDTH   1U
/** Bit mask of the PL_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_1_MASK    0x00020000U

/** Bit shift of the PL_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_0_SHIFT   16U
/** Bit width of the PL_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_0_WIDTH   1U
/** Bit mask of the PL_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PL_ALM_0_MASK    0x00010000U

/** Bit shift of the PS_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_15_SHIFT   15U
/** Bit width of the PS_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_15_WIDTH   1U
/** Bit mask of the PS_ALM_15 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_15_MASK    0x00008000U

/** Bit shift of the PS_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_14_SHIFT   14U
/** Bit width of the PS_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_14_WIDTH   1U
/** Bit mask of the PS_ALM_14 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_14_MASK    0x00004000U

/** Bit shift of the PS_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_13_SHIFT   13U
/** Bit width of the PS_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_13_WIDTH   1U
/** Bit mask of the PS_ALM_13 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_13_MASK    0x00002000U

/** Bit shift of the PS_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_12_SHIFT   12U
/** Bit width of the PS_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_12_WIDTH   1U
/** Bit mask of the PS_ALM_12 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_12_MASK    0x00001000U

/** Bit shift of the PS_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_11_SHIFT   11U
/** Bit width of the PS_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_11_WIDTH   1U
/** Bit mask of the PS_ALM_11 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_11_MASK    0x00000800U

/** Bit shift of the PS_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_10_SHIFT   10U
/** Bit width of the PS_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_10_WIDTH   1U
/** Bit mask of the PS_ALM_10 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_10_MASK    0x00000400U

/** Bit shift of the PS_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_9_SHIFT   9U
/** Bit width of the PS_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_9_WIDTH   1U
/** Bit mask of the PS_ALM_9 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_9_MASK    0x00000200U

/** Bit shift of the PS_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_8_SHIFT   8U
/** Bit width of the PS_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_8_WIDTH   1U
/** Bit mask of the PS_ALM_8 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_8_MASK    0x00000100U

/** Bit shift of the PS_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_7_SHIFT   7U
/** Bit width of the PS_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_7_WIDTH   1U
/** Bit mask of the PS_ALM_7 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_7_MASK    0x00000080U

/** Bit shift of the PS_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_6_SHIFT   6U
/** Bit width of the PS_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_6_WIDTH   1U
/** Bit mask of the PS_ALM_6 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_6_MASK    0x00000040U

/** Bit shift of the PS_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_5_SHIFT   5U
/** Bit width of the PS_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_5_WIDTH   1U
/** Bit mask of the PS_ALM_5 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_5_MASK    0x00000020U

/** Bit shift of the PS_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_4_SHIFT   4U
/** Bit width of the PS_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_4_WIDTH   1U
/** Bit mask of the PS_ALM_4 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_4_MASK    0x00000010U

/** Bit shift of the PS_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_3_SHIFT   3U
/** Bit width of the PS_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_3_WIDTH   1U
/** Bit mask of the PS_ALM_3 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_3_MASK    0x00000008U

/** Bit shift of the PS_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_2_SHIFT   2U
/** Bit width of the PS_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_2_WIDTH   1U
/** Bit mask of the PS_ALM_2 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_2_MASK    0x00000004U

/** Bit shift of the PS_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_1_SHIFT   1U
/** Bit width of the PS_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_1_WIDTH   1U
/** Bit mask of the PS_ALM_1 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_1_MASK    0x00000002U

/** Bit shift of the PS_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_0_SHIFT   0U
/** Bit width of the PS_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_0_WIDTH   1U
/** Bit mask of the PS_ALM_0 field in ISR_0. */
#define XSYSMONPSU_ISR_0_PS_ALM_0_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuIsr1 register definitions
 * @{
 */
/** Offset of the ISR_1 register. */
#define XSYSMONPSU_ISR_1_OFFSET   0x00000014U
/** Bit mask of the ISR_1 register. */
#define XSYSMONPSU_ISR_1_MASK	  0xe000001fU
/** Reset value of the ISR_1 register. */
#define XSYSMONPSU_ISR_1_RSTVAL   0x00000000U

/** Bit shift of the ADD_DECD_ERR field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_SHIFT   31U
/** Bit width of the ADD_DECD_ERR field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_MASK    0x80000000U

/** Bit shift of the ADD_DECD_ERR_PL_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PL_SYSMON_SHIFT   30U
/** Bit width of the ADD_DECD_ERR_PL_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PL_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PL_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PL_SYSMON_MASK    0x40000000U

/** Bit shift of the ADD_DECD_ERR_PS_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PS_SYSMON_SHIFT   29U
/** Bit width of the ADD_DECD_ERR_PS_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PS_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PS_SYSMON field in ISR_1. */
#define XSYSMONPSU_ISR_1_ADD_DECD_ERR_PS_SYSMON_MASK    0x20000000U

/** Bit shift of the EOS field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOS_SHIFT   4U
/** Bit width of the EOS field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOS_WIDTH   1U
/** Bit mask of the EOS field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOS_MASK    0x00000010U

/** Bit shift of the EOC field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOC_SHIFT   3U
/** Bit width of the EOC field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOC_WIDTH   1U
/** Bit mask of the EOC field in ISR_1. */
#define XSYSMONPSU_ISR_1_EOC_MASK    0x00000008U

/** Bit shift of the PL_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PL_OT_SHIFT   2U
/** Bit width of the PL_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PL_OT_WIDTH   1U
/** Bit mask of the PL_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PL_OT_MASK    0x00000004U

/** Bit shift of the PS_LPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_LPD_OT_SHIFT   1U
/** Bit width of the PS_LPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_LPD_OT_WIDTH   1U
/** Bit mask of the PS_LPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_LPD_OT_MASK    0x00000002U

/** Bit shift of the PS_FPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_FPD_OT_SHIFT   0U
/** Bit width of the PS_FPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_FPD_OT_WIDTH   1U
/** Bit mask of the PS_FPD_OT field in ISR_1. */
#define XSYSMONPSU_ISR_1_PS_FPD_OT_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuImr0 register definitions
 * @{
 */
/** Offset of the IMR_0 register. */
#define XSYSMONPSU_IMR_0_OFFSET   0x00000018U
/** Reset value of the IMR_0 register. */
#define XSYSMONPSU_IMR_0_RSTVAL   0xffffffffU

/** Bit shift of the PL_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_15_SHIFT   31U
/** Bit width of the PL_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_15_WIDTH   1U
/** Bit mask of the PL_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_15_MASK    0x80000000U

/** Bit shift of the PL_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_14_SHIFT   30U
/** Bit width of the PL_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_14_WIDTH   1U
/** Bit mask of the PL_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_14_MASK    0x40000000U

/** Bit shift of the PL_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_13_SHIFT   29U
/** Bit width of the PL_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_13_WIDTH   1U
/** Bit mask of the PL_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_13_MASK    0x20000000U

/** Bit shift of the PL_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_12_SHIFT   28U
/** Bit width of the PL_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_12_WIDTH   1U
/** Bit mask of the PL_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_12_MASK    0x10000000U

/** Bit shift of the PL_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_11_SHIFT   27U
/** Bit width of the PL_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_11_WIDTH   1U
/** Bit mask of the PL_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_11_MASK    0x08000000U

/** Bit shift of the PL_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_10_SHIFT   26U
/** Bit width of the PL_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_10_WIDTH   1U
/** Bit mask of the PL_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_10_MASK    0x04000000U

/** Bit shift of the PL_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_9_SHIFT   25U
/** Bit width of the PL_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_9_WIDTH   1U
/** Bit mask of the PL_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_9_MASK    0x02000000U

/** Bit shift of the PL_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_8_SHIFT   24U
/** Bit width of the PL_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_8_WIDTH   1U
/** Bit mask of the PL_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_8_MASK    0x01000000U

/** Bit shift of the PL_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_7_SHIFT   23U
/** Bit width of the PL_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_7_WIDTH   1U
/** Bit mask of the PL_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_7_MASK    0x00800000U

/** Bit shift of the PL_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_6_SHIFT   22U
/** Bit width of the PL_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_6_WIDTH   1U
/** Bit mask of the PL_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_6_MASK    0x00400000U

/** Bit shift of the PL_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_5_SHIFT   21U
/** Bit width of the PL_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_5_WIDTH   1U
/** Bit mask of the PL_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_5_MASK    0x00200000U

/** Bit shift of the PL_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_4_SHIFT   20U
/** Bit width of the PL_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_4_WIDTH   1U
/** Bit mask of the PL_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_4_MASK    0x00100000U

/** Bit shift of the PL_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_3_SHIFT   19U
/** Bit width of the PL_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_3_WIDTH   1U
/** Bit mask of the PL_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_3_MASK    0x00080000U

/** Bit shift of the PL_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_2_SHIFT   18U
/** Bit width of the PL_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_2_WIDTH   1U
/** Bit mask of the PL_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_2_MASK    0x00040000U

/** Bit shift of the PL_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_1_SHIFT   17U
/** Bit width of the PL_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_1_WIDTH   1U
/** Bit mask of the PL_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_1_MASK    0x00020000U

/** Bit shift of the PL_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_0_SHIFT   16U
/** Bit width of the PL_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_0_WIDTH   1U
/** Bit mask of the PL_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PL_ALM_0_MASK    0x00010000U

/** Bit shift of the PS_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_15_SHIFT   15U
/** Bit width of the PS_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_15_WIDTH   1U
/** Bit mask of the PS_ALM_15 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_15_MASK    0x00008000U

/** Bit shift of the PS_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_14_SHIFT   14U
/** Bit width of the PS_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_14_WIDTH   1U
/** Bit mask of the PS_ALM_14 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_14_MASK    0x00004000U

/** Bit shift of the PS_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_13_SHIFT   13U
/** Bit width of the PS_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_13_WIDTH   1U
/** Bit mask of the PS_ALM_13 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_13_MASK    0x00002000U

/** Bit shift of the PS_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_12_SHIFT   12U
/** Bit width of the PS_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_12_WIDTH   1U
/** Bit mask of the PS_ALM_12 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_12_MASK    0x00001000U

/** Bit shift of the PS_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_11_SHIFT   11U
/** Bit width of the PS_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_11_WIDTH   1U
/** Bit mask of the PS_ALM_11 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_11_MASK    0x00000800U

/** Bit shift of the PS_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_10_SHIFT   10U
/** Bit width of the PS_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_10_WIDTH   1U
/** Bit mask of the PS_ALM_10 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_10_MASK    0x00000400U

/** Bit shift of the PS_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_9_SHIFT   9U
/** Bit width of the PS_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_9_WIDTH   1U
/** Bit mask of the PS_ALM_9 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_9_MASK    0x00000200U

/** Bit shift of the PS_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_8_SHIFT   8U
/** Bit width of the PS_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_8_WIDTH   1U
/** Bit mask of the PS_ALM_8 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_8_MASK    0x00000100U

/** Bit shift of the PS_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_7_SHIFT   7U
/** Bit width of the PS_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_7_WIDTH   1U
/** Bit mask of the PS_ALM_7 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_7_MASK    0x00000080U

/** Bit shift of the PS_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_6_SHIFT   6U
/** Bit width of the PS_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_6_WIDTH   1U
/** Bit mask of the PS_ALM_6 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_6_MASK    0x00000040U

/** Bit shift of the PS_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_5_SHIFT   5U
/** Bit width of the PS_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_5_WIDTH   1U
/** Bit mask of the PS_ALM_5 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_5_MASK    0x00000020U

/** Bit shift of the PS_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_4_SHIFT   4U
/** Bit width of the PS_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_4_WIDTH   1U
/** Bit mask of the PS_ALM_4 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_4_MASK    0x00000010U

/** Bit shift of the PS_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_3_SHIFT   3U
/** Bit width of the PS_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_3_WIDTH   1U
/** Bit mask of the PS_ALM_3 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_3_MASK    0x00000008U

/** Bit shift of the PS_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_2_SHIFT   2U
/** Bit width of the PS_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_2_WIDTH   1U
/** Bit mask of the PS_ALM_2 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_2_MASK    0x00000004U

/** Bit shift of the PS_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_1_SHIFT   1U
/** Bit width of the PS_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_1_WIDTH   1U
/** Bit mask of the PS_ALM_1 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_1_MASK    0x00000002U

/** Bit shift of the PS_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_0_SHIFT   0U
/** Bit width of the PS_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_0_WIDTH   1U
/** Bit mask of the PS_ALM_0 field in IMR_0. */
#define XSYSMONPSU_IMR_0_PS_ALM_0_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuImr1 register definitions
 * @{
 */
/** Offset of the IMR_1 register. */
#define XSYSMONPSU_IMR_1_OFFSET   0x0000001CU
/** Reset value of the IMR_1 register. */
#define XSYSMONPSU_IMR_1_RSTVAL   0xe000001fU

/** Bit shift of the ADD_DECD_ERR field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_SHIFT   31U
/** Bit width of the ADD_DECD_ERR field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_MASK    0x80000000U

/** Bit shift of the ADD_DECD_ERR_PL_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PL_SYSMON_SHIFT   30U
/** Bit width of the ADD_DECD_ERR_PL_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PL_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PL_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PL_SYSMON_MASK    0x40000000U

/** Bit shift of the ADD_DECD_ERR_PS_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PS_SYSMON_SHIFT   29U
/** Bit width of the ADD_DECD_ERR_PS_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PS_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PS_SYSMON field in IMR_1. */
#define XSYSMONPSU_IMR_1_ADD_DECD_ERR_PS_SYSMON_MASK    0x20000000U

/** Bit shift of the EOS field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOS_SHIFT   4U
/** Bit width of the EOS field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOS_WIDTH   1U
/** Bit mask of the EOS field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOS_MASK    0x00000010U

/** Bit shift of the EOC field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOC_SHIFT   3U
/** Bit width of the EOC field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOC_WIDTH   1U
/** Bit mask of the EOC field in IMR_1. */
#define XSYSMONPSU_IMR_1_EOC_MASK    0x00000008U

/** Bit shift of the PL_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PL_OT_SHIFT   2U
/** Bit width of the PL_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PL_OT_WIDTH   1U
/** Bit mask of the PL_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PL_OT_MASK    0x00000004U

/** Bit shift of the PS_LPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_LPD_OT_SHIFT   1U
/** Bit width of the PS_LPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_LPD_OT_WIDTH   1U
/** Bit mask of the PS_LPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_LPD_OT_MASK    0x00000002U

/** Bit shift of the PS_FPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_FPD_OT_SHIFT   0U
/** Bit width of the PS_FPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_FPD_OT_WIDTH   1U
/** Bit mask of the PS_FPD_OT field in IMR_1. */
#define XSYSMONPSU_IMR_1_PS_FPD_OT_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuIer0 register definitions
 * @{
 */
/** Offset of the IER_0 register. */
#define XSYSMONPSU_IER_0_OFFSET   0x00000020U
/**
 * Mask of the valid bits in the interrupt registers ISR_0, IMR_0, IER_0, and
 * IDR_0.
 */
#define XSYSMONPSU_IXR_0_MASK     0xFFFFFFFFU
/** Reset value of the IER_0 register. */
#define XSYSMONPSU_IER_0_RSTVAL   0x00000000U

/** Bit shift of the PL_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_15_SHIFT   31U
/** Bit width of the PL_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_15_WIDTH   1U
/** Bit mask of the PL_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_15_MASK    0x80000000U

/** Bit shift of the PL_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_14_SHIFT   30U
/** Bit width of the PL_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_14_WIDTH   1U
/** Bit mask of the PL_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_14_MASK    0x40000000U

/** Bit shift of the PL_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_13_SHIFT   29U
/** Bit width of the PL_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_13_WIDTH   1U
/** Bit mask of the PL_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_13_MASK    0x20000000U

/** Bit shift of the PL_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_12_SHIFT   28U
/** Bit width of the PL_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_12_WIDTH   1U
/** Bit mask of the PL_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_12_MASK    0x10000000U

/** Bit shift of the PL_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_11_SHIFT   27U
/** Bit width of the PL_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_11_WIDTH   1U
/** Bit mask of the PL_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_11_MASK    0x08000000U

/** Bit shift of the PL_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_10_SHIFT   26U
/** Bit width of the PL_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_10_WIDTH   1U
/** Bit mask of the PL_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_10_MASK    0x04000000U

/** Bit shift of the PL_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_9_SHIFT   25U
/** Bit width of the PL_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_9_WIDTH   1U
/** Bit mask of the PL_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_9_MASK    0x02000000U

/** Bit shift of the PL_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_8_SHIFT   24U
/** Bit width of the PL_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_8_WIDTH   1U
/** Bit mask of the PL_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_8_MASK    0x01000000U

/** Bit shift of the PL_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_7_SHIFT   23U
/** Bit width of the PL_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_7_WIDTH   1U
/** Bit mask of the PL_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_7_MASK    0x00800000U

/** Bit shift of the PL_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_6_SHIFT   22U
/** Bit width of the PL_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_6_WIDTH   1U
/** Bit mask of the PL_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_6_MASK    0x00400000U

/** Bit shift of the PL_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_5_SHIFT   21U
/** Bit width of the PL_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_5_WIDTH   1U
/** Bit mask of the PL_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_5_MASK    0x00200000U

/** Bit shift of the PL_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_4_SHIFT   20U
/** Bit width of the PL_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_4_WIDTH   1U
/** Bit mask of the PL_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_4_MASK    0x00100000U

/** Bit shift of the PL_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_3_SHIFT   19U
/** Bit width of the PL_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_3_WIDTH   1U
/** Bit mask of the PL_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_3_MASK    0x00080000U

/** Bit shift of the PL_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_2_SHIFT   18U
/** Bit width of the PL_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_2_WIDTH   1U
/** Bit mask of the PL_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_2_MASK    0x00040000U

/** Bit shift of the PL_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_1_SHIFT   17U
/** Bit width of the PL_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_1_WIDTH   1U
/** Bit mask of the PL_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_1_MASK    0x00020000U

/** Bit shift of the PL_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_0_SHIFT   16U
/** Bit width of the PL_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_0_WIDTH   1U
/** Bit mask of the PL_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PL_ALM_0_MASK    0x00010000U

/** Bit shift of the PS_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_15_SHIFT   15U
/** Bit width of the PS_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_15_WIDTH   1U
/** Bit mask of the PS_ALM_15 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_15_MASK    0x00008000U

/** Bit shift of the PS_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_14_SHIFT   14U
/** Bit width of the PS_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_14_WIDTH   1U
/** Bit mask of the PS_ALM_14 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_14_MASK    0x00004000U

/** Bit shift of the PS_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_13_SHIFT   13U
/** Bit width of the PS_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_13_WIDTH   1U
/** Bit mask of the PS_ALM_13 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_13_MASK    0x00002000U

/** Bit shift of the PS_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_12_SHIFT   12U
/** Bit width of the PS_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_12_WIDTH   1U
/** Bit mask of the PS_ALM_12 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_12_MASK    0x00001000U

/** Bit shift of the PS_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_11_SHIFT   11U
/** Bit width of the PS_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_11_WIDTH   1U
/** Bit mask of the PS_ALM_11 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_11_MASK    0x00000800U

/** Bit shift of the PS_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_10_SHIFT   10U
/** Bit width of the PS_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_10_WIDTH   1U
/** Bit mask of the PS_ALM_10 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_10_MASK    0x00000400U

/** Bit shift of the PS_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_9_SHIFT   9U
/** Bit width of the PS_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_9_WIDTH   1U
/** Bit mask of the PS_ALM_9 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_9_MASK    0x00000200U

/** Bit shift of the PS_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_8_SHIFT   8U
/** Bit width of the PS_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_8_WIDTH   1U
/** Bit mask of the PS_ALM_8 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_8_MASK    0x00000100U

/** Bit shift of the PS_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_7_SHIFT   7U
/** Bit width of the PS_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_7_WIDTH   1U
/** Bit mask of the PS_ALM_7 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_7_MASK    0x00000080U

/** Bit shift of the PS_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_6_SHIFT   6U
/** Bit width of the PS_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_6_WIDTH   1U
/** Bit mask of the PS_ALM_6 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_6_MASK    0x00000040U

/** Bit shift of the PS_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_5_SHIFT   5U
/** Bit width of the PS_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_5_WIDTH   1U
/** Bit mask of the PS_ALM_5 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_5_MASK    0x00000020U

/** Bit shift of the PS_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_4_SHIFT   4U
/** Bit width of the PS_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_4_WIDTH   1U
/** Bit mask of the PS_ALM_4 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_4_MASK    0x00000010U

/** Bit shift of the PS_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_3_SHIFT   3U
/** Bit width of the PS_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_3_WIDTH   1U
/** Bit mask of the PS_ALM_3 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_3_MASK    0x00000008U

/** Bit shift of the PS_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_2_SHIFT   2U
/** Bit width of the PS_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_2_WIDTH   1U
/** Bit mask of the PS_ALM_2 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_2_MASK    0x00000004U

/** Bit shift of the PS_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_1_SHIFT   1U
/** Bit width of the PS_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_1_WIDTH   1U
/** Bit mask of the PS_ALM_1 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_1_MASK    0x00000002U

/** Bit shift of the PS_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_0_SHIFT   0U
/** Bit width of the PS_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_0_WIDTH   1U
/** Bit mask of the PS_ALM_0 field in IER_0. */
#define XSYSMONPSU_IER_0_PS_ALM_0_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuIer1 register definitions
 * @{
 */
/** Offset of the IER_1 register. */
#define XSYSMONPSU_IER_1_OFFSET   0x00000024U
/**
 * Mask of the valid bits in the interrupt registers ISR_1, IMR_1, IER_1, and
 * IDR_1.
 */
#define XSYSMONPSU_IXR_1_MASK     0xE000001FU
/** Reset value of the IER_1 register. */
#define XSYSMONPSU_IER_1_RSTVAL   0x00000000U

/** Bit shift of the ADD_DECD_ERR field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_SHIFT   31U
/** Bit width of the ADD_DECD_ERR field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_MASK    0x80000000U

/** Bit shift of the ADD_DECD_ERR_PL_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PL_SYSMON_SHIFT   30U
/** Bit width of the ADD_DECD_ERR_PL_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PL_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PL_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PL_SYSMON_MASK    0x40000000U

/** Bit shift of the ADD_DECD_ERR_PS_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PS_SYSMON_SHIFT   29U
/** Bit width of the ADD_DECD_ERR_PS_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PS_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PS_SYSMON field in IER_1. */
#define XSYSMONPSU_IER_1_ADD_DECD_ERR_PS_SYSMON_MASK    0x20000000U

/** Bit shift of the EOS field in IER_1. */
#define XSYSMONPSU_IER_1_EOS_SHIFT   4U
/** Bit width of the EOS field in IER_1. */
#define XSYSMONPSU_IER_1_EOS_WIDTH   1U
/** Bit mask of the EOS field in IER_1. */
#define XSYSMONPSU_IER_1_EOS_MASK    0x00000010U

/** Bit shift of the EOC field in IER_1. */
#define XSYSMONPSU_IER_1_EOC_SHIFT   3U
/** Bit width of the EOC field in IER_1. */
#define XSYSMONPSU_IER_1_EOC_WIDTH   1U
/** Bit mask of the EOC field in IER_1. */
#define XSYSMONPSU_IER_1_EOC_MASK    0x00000008U

/** Bit shift of the PL_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PL_OT_SHIFT   2U
/** Bit width of the PL_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PL_OT_WIDTH   1U
/** Bit mask of the PL_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PL_OT_MASK    0x00000004U

/** Bit shift of the PS_LPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_LPD_OT_SHIFT   1U
/** Bit width of the PS_LPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_LPD_OT_WIDTH   1U
/** Bit mask of the PS_LPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_LPD_OT_MASK    0x00000002U

/** Bit shift of the PS_FPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_FPD_OT_SHIFT   0U
/** Bit width of the PS_FPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_FPD_OT_WIDTH   1U
/** Bit mask of the PS_FPD_OT field in IER_1. */
#define XSYSMONPSU_IER_1_PS_FPD_OT_MASK    0x00000001U

/** Shift of the interrupt register 1 bits in the 64-bit interrupt mask. */
#define XSYSMONPSU_IXR_1_SHIFT  32U
/** @} */

/**
 * @name XSysmonPsuIdr0 register definitions
 * @{
 */
/** Offset of the IDR_0 register. */
#define XSYSMONPSU_IDR_0_OFFSET   0x00000028U
/** Reset value of the IDR_0 register. */
#define XSYSMONPSU_IDR_0_RSTVAL   0x00000000U

/** Bit shift of the PL_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_15_SHIFT   31U
/** Bit width of the PL_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_15_WIDTH   1U
/** Bit mask of the PL_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_15_MASK    0x80000000U

/** Bit shift of the PL_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_14_SHIFT   30U
/** Bit width of the PL_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_14_WIDTH   1U
/** Bit mask of the PL_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_14_MASK    0x40000000U

/** Bit shift of the PL_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_13_SHIFT   29U
/** Bit width of the PL_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_13_WIDTH   1U
/** Bit mask of the PL_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_13_MASK    0x20000000U

/** Bit shift of the PL_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_12_SHIFT   28U
/** Bit width of the PL_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_12_WIDTH   1U
/** Bit mask of the PL_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_12_MASK    0x10000000U

/** Bit shift of the PL_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_11_SHIFT   27U
/** Bit width of the PL_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_11_WIDTH   1U
/** Bit mask of the PL_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_11_MASK    0x08000000U

/** Bit shift of the PL_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_10_SHIFT   26U
/** Bit width of the PL_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_10_WIDTH   1U
/** Bit mask of the PL_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_10_MASK    0x04000000U

/** Bit shift of the PL_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_9_SHIFT   25U
/** Bit width of the PL_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_9_WIDTH   1U
/** Bit mask of the PL_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_9_MASK    0x02000000U

/** Bit shift of the PL_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_8_SHIFT   24U
/** Bit width of the PL_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_8_WIDTH   1U
/** Bit mask of the PL_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_8_MASK    0x01000000U

/** Bit shift of the PL_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_7_SHIFT   23U
/** Bit width of the PL_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_7_WIDTH   1U
/** Bit mask of the PL_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_7_MASK    0x00800000U

/** Bit shift of the PL_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_6_SHIFT   22U
/** Bit width of the PL_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_6_WIDTH   1U
/** Bit mask of the PL_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_6_MASK    0x00400000U

/** Bit shift of the PL_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_5_SHIFT   21U
/** Bit width of the PL_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_5_WIDTH   1U
/** Bit mask of the PL_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_5_MASK    0x00200000U

/** Bit shift of the PL_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_4_SHIFT   20U
/** Bit width of the PL_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_4_WIDTH   1U
/** Bit mask of the PL_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_4_MASK    0x00100000U

/** Bit shift of the PL_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_3_SHIFT   19U
/** Bit width of the PL_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_3_WIDTH   1U
/** Bit mask of the PL_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_3_MASK    0x00080000U

/** Bit shift of the PL_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_2_SHIFT   18U
/** Bit width of the PL_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_2_WIDTH   1U
/** Bit mask of the PL_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_2_MASK    0x00040000U

/** Bit shift of the PL_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_1_SHIFT   17U
/** Bit width of the PL_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_1_WIDTH   1U
/** Bit mask of the PL_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_1_MASK    0x00020000U

/** Bit shift of the PL_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_0_SHIFT   16U
/** Bit width of the PL_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_0_WIDTH   1U
/** Bit mask of the PL_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PL_ALM_0_MASK    0x00010000U

/** Bit shift of the PS_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_15_SHIFT   15U
/** Bit width of the PS_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_15_WIDTH   1U
/** Bit mask of the PS_ALM_15 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_15_MASK    0x00008000U

/** Bit shift of the PS_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_14_SHIFT   14U
/** Bit width of the PS_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_14_WIDTH   1U
/** Bit mask of the PS_ALM_14 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_14_MASK    0x00004000U

/** Bit shift of the PS_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_13_SHIFT   13U
/** Bit width of the PS_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_13_WIDTH   1U
/** Bit mask of the PS_ALM_13 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_13_MASK    0x00002000U

/** Bit shift of the PS_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_12_SHIFT   12U
/** Bit width of the PS_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_12_WIDTH   1U
/** Bit mask of the PS_ALM_12 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_12_MASK    0x00001000U

/** Bit shift of the PS_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_11_SHIFT   11U
/** Bit width of the PS_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_11_WIDTH   1U
/** Bit mask of the PS_ALM_11 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_11_MASK    0x00000800U

/** Bit shift of the PS_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_10_SHIFT   10U
/** Bit width of the PS_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_10_WIDTH   1U
/** Bit mask of the PS_ALM_10 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_10_MASK    0x00000400U

/** Bit shift of the PS_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_9_SHIFT   9U
/** Bit width of the PS_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_9_WIDTH   1U
/** Bit mask of the PS_ALM_9 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_9_MASK    0x00000200U

/** Bit shift of the PS_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_8_SHIFT   8U
/** Bit width of the PS_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_8_WIDTH   1U
/** Bit mask of the PS_ALM_8 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_8_MASK    0x00000100U

/** Bit shift of the PS_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_7_SHIFT   7U
/** Bit width of the PS_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_7_WIDTH   1U
/** Bit mask of the PS_ALM_7 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_7_MASK    0x00000080U

/** Bit shift of the PS_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_6_SHIFT   6U
/** Bit width of the PS_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_6_WIDTH   1U
/** Bit mask of the PS_ALM_6 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_6_MASK    0x00000040U

/** Bit shift of the PS_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_5_SHIFT   5U
/** Bit width of the PS_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_5_WIDTH   1U
/** Bit mask of the PS_ALM_5 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_5_MASK    0x00000020U

/** Bit shift of the PS_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_4_SHIFT   4U
/** Bit width of the PS_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_4_WIDTH   1U
/** Bit mask of the PS_ALM_4 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_4_MASK    0x00000010U

/** Bit shift of the PS_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_3_SHIFT   3U
/** Bit width of the PS_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_3_WIDTH   1U
/** Bit mask of the PS_ALM_3 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_3_MASK    0x00000008U

/** Bit shift of the PS_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_2_SHIFT   2U
/** Bit width of the PS_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_2_WIDTH   1U
/** Bit mask of the PS_ALM_2 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_2_MASK    0x00000004U

/** Bit shift of the PS_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_1_SHIFT   1U
/** Bit width of the PS_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_1_WIDTH   1U
/** Bit mask of the PS_ALM_1 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_1_MASK    0x00000002U

/** Bit shift of the PS_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_0_SHIFT   0U
/** Bit width of the PS_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_0_WIDTH   1U
/** Bit mask of the PS_ALM_0 field in IDR_0. */
#define XSYSMONPSU_IDR_0_PS_ALM_0_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuIdr1 register definitions
 * @{
 */
/** Offset of the IDR_1 register. */
#define XSYSMONPSU_IDR_1_OFFSET   0x0000002CU
/** Reset value of the IDR_1 register. */
#define XSYSMONPSU_IDR_1_RSTVAL   0x00000000U

/** Bit shift of the ADD_DECD_ERR field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_SHIFT   31U
/** Bit width of the ADD_DECD_ERR field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_MASK    0x80000000U

/** Bit shift of the ADD_DECD_ERR_PL_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PL_SYSMON_SHIFT   30U
/** Bit width of the ADD_DECD_ERR_PL_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PL_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PL_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PL_SYSMON_MASK    0x40000000U

/** Bit shift of the ADD_DECD_ERR_PS_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PS_SYSMON_SHIFT   29U
/** Bit width of the ADD_DECD_ERR_PS_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PS_SYSMON_WIDTH   1U
/** Bit mask of the ADD_DECD_ERR_PS_SYSMON field in IDR_1. */
#define XSYSMONPSU_IDR_1_ADD_DECD_ERR_PS_SYSMON_MASK    0x20000000U

/** Bit shift of the EOS field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOS_SHIFT   4U
/** Bit width of the EOS field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOS_WIDTH   1U
/** Bit mask of the EOS field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOS_MASK    0x00000010U

/** Bit shift of the EOC field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOC_SHIFT   3U
/** Bit width of the EOC field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOC_WIDTH   1U
/** Bit mask of the EOC field in IDR_1. */
#define XSYSMONPSU_IDR_1_EOC_MASK    0x00000008U

/** Bit shift of the PL_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PL_OT_SHIFT   2U
/** Bit width of the PL_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PL_OT_WIDTH   1U
/** Bit mask of the PL_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PL_OT_MASK    0x00000004U

/** Bit shift of the PS_LPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_LPD_OT_SHIFT   1U
/** Bit width of the PS_LPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_LPD_OT_WIDTH   1U
/** Bit mask of the PS_LPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_LPD_OT_MASK    0x00000002U

/** Bit shift of the PS_FPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_FPD_OT_SHIFT   0U
/** Bit width of the PS_FPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_FPD_OT_WIDTH   1U
/** Bit mask of the PS_FPD_OT field in IDR_1. */
#define XSYSMONPSU_IDR_1_PS_FPD_OT_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuPsSysmonSts register definitions
 * @{
 */
/** Offset of the PS_SYSMON_CSTS register. */
#define XSYSMONPSU_PS_SYSMON_CSTS_OFFSET   0x00000040U
/** Reset value of the PS_SYSMON_CSTS register. */
#define XSYSMONPSU_PS_SYSMON_CSTS_RSTVAL   0x00000000U

/** Bit shift of the STRTUP_STE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_STE_SHIFT   24U
/** Bit width of the STRTUP_STE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_STE_WIDTH   4U
/** Bit mask of the STRTUP_STE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_STE_MASK    0x0f000000U

/** Bit shift of the STRTUP_DNE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_DNE_SHIFT   16U
/** Bit width of the STRTUP_DNE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_DNE_WIDTH   1U
/** Bit mask of the STRTUP_DNE field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_DNE_MASK    0x00010000U

/** Bit shift of the AUTO_CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_AUTO_CONVST_SHIFT   3U
/** Bit width of the AUTO_CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_AUTO_CONVST_WIDTH   1U
/** Bit mask of the AUTO_CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_AUTO_CONVST_MASK    0x00000008U

/** Bit shift of the CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_CONVST_SHIFT   2U
/** Bit width of the CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_CONVST_WIDTH   1U
/** Bit mask of the CONVST field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_CONVST_MASK    0x00000004U

/** Bit shift of the RST_USR field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_RST_USR_SHIFT   1U
/** Bit width of the RST_USR field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_RST_USR_WIDTH   1U
/** Bit mask of the RST_USR field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_RST_USR_MASK    0x00000002U

/** Bit shift of the STRTUP_TRIG field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_TRIG_SHIFT   0U
/** Bit width of the STRTUP_TRIG field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_TRIG_WIDTH   1U
/** Bit mask of the STRTUP_TRIG field in PS_SYSMON_CSTS. */
#define XSYSMONPSU_PS_SYSMON_CSTS_STRTUP_TRIG_MASK    0x00000001U

/** PS SysMon control status value when the PS SysMon is ready. */
#define XSYSMONPSU_PS_SYSMON_READY 0x08010000U
/** @} */

/**
 * @name XSysmonPsuPlSysmonSts register definitions
 * @{
 */
/** Offset of the PL_SYSMON_CSTS register. */
#define XSYSMONPSU_PL_SYSMON_CSTS_OFFSET   0x00000044U
/** Reset value of the PL_SYSMON_CSTS register. */
#define XSYSMONPSU_PL_SYSMON_CSTS_RSTVAL   0x00000000U

/** Bit shift of the ACESBLE field in PL_SYSMON_CSTS. */
#define XSYSMONPSU_PL_SYSMON_CSTS_ACESBLE_SHIFT   0U
/** Bit width of the ACESBLE field in PL_SYSMON_CSTS. */
#define XSYSMONPSU_PL_SYSMON_CSTS_ACESBLE_WIDTH   1U
/** Bit mask of the ACESBLE field in PL_SYSMON_CSTS. */
#define XSYSMONPSU_PL_SYSMON_CSTS_ACESBLE_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuMonSts register definitions
 * @{
 */
/** Offset of the MON_STS register. */
#define XSYSMONPSU_MON_STS_OFFSET   0x00000050U
/** Reset value of the MON_STS register. */
#define XSYSMONPSU_MON_STS_RSTVAL   0x00000000U

/** Bit shift of the JTAG_LCKD field in MON_STS. */
#define XSYSMONPSU_MON_STS_JTAG_LCKD_SHIFT   23U
/** Bit width of the JTAG_LCKD field in MON_STS. */
#define XSYSMONPSU_MON_STS_JTAG_LCKD_WIDTH   1U
/** Bit mask of the JTAG_LCKD field in MON_STS. */
#define XSYSMONPSU_MON_STS_JTAG_LCKD_MASK    0x00800000U

/** Bit shift of the BSY field in MON_STS. */
#define XSYSMONPSU_MON_STS_BSY_SHIFT   22U
/** Bit width of the BSY field in MON_STS. */
#define XSYSMONPSU_MON_STS_BSY_WIDTH   1U
/** Bit mask of the BSY field in MON_STS. */
#define XSYSMONPSU_MON_STS_BSY_MASK    0x00400000U

/** Bit shift of the CH field in MON_STS. */
#define XSYSMONPSU_MON_STS_CH_SHIFT   16U
/** Bit width of the CH field in MON_STS. */
#define XSYSMONPSU_MON_STS_CH_WIDTH   6U
/** Bit mask of the CH field in MON_STS. */
#define XSYSMONPSU_MON_STS_CH_MASK    0x003f0000U

/** Bit shift of the DATA field in MON_STS. */
#define XSYSMONPSU_MON_STS_DATA_SHIFT   0U
/** Bit width of the DATA field in MON_STS. */
#define XSYSMONPSU_MON_STS_DATA_WIDTH   16U
/** Bit mask of the DATA field in MON_STS. */
#define XSYSMONPSU_MON_STS_DATA_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPspll0 register definitions
 * @{
 */
/** Offset of the VCC_PSPLL0 register. */
#define XSYSMONPSU_VCC_PSPLL0_OFFSET   0x00000060U
/** Reset value of the VCC_PSPLL0 register. */
#define XSYSMONPSU_VCC_PSPLL0_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSPLL0. */
#define XSYSMONPSU_VCC_PSPLL0_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSPLL0. */
#define XSYSMONPSU_VCC_PSPLL0_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSPLL0. */
#define XSYSMONPSU_VCC_PSPLL0_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPspll1 register definitions
 * @{
 */
/** Offset of the VCC_PSPLL1 register. */
#define XSYSMONPSU_VCC_PSPLL1_OFFSET   0x00000064U
/** Reset value of the VCC_PSPLL1 register. */
#define XSYSMONPSU_VCC_PSPLL1_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSPLL1. */
#define XSYSMONPSU_VCC_PSPLL1_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSPLL1. */
#define XSYSMONPSU_VCC_PSPLL1_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSPLL1. */
#define XSYSMONPSU_VCC_PSPLL1_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPspll2 register definitions
 * @{
 */
/** Offset of the VCC_PSPLL2 register. */
#define XSYSMONPSU_VCC_PSPLL2_OFFSET   0x00000068U
/** Reset value of the VCC_PSPLL2 register. */
#define XSYSMONPSU_VCC_PSPLL2_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSPLL2. */
#define XSYSMONPSU_VCC_PSPLL2_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSPLL2. */
#define XSYSMONPSU_VCC_PSPLL2_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSPLL2. */
#define XSYSMONPSU_VCC_PSPLL2_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPspll3 register definitions
 * @{
 */
/** Offset of the VCC_PSPLL3 register. */
#define XSYSMONPSU_VCC_PSPLL3_OFFSET   0x0000006CU
/** Reset value of the VCC_PSPLL3 register. */
#define XSYSMONPSU_VCC_PSPLL3_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSPLL3. */
#define XSYSMONPSU_VCC_PSPLL3_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSPLL3. */
#define XSYSMONPSU_VCC_PSPLL3_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSPLL3. */
#define XSYSMONPSU_VCC_PSPLL3_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPspll4 register definitions
 * @{
 */
/** Offset of the VCC_PSPLL4 register. */
#define XSYSMONPSU_VCC_PSPLL4_OFFSET   0x00000070U
/** Reset value of the VCC_PSPLL4 register. */
#define XSYSMONPSU_VCC_PSPLL4_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSPLL4. */
#define XSYSMONPSU_VCC_PSPLL4_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSPLL4. */
#define XSYSMONPSU_VCC_PSPLL4_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSPLL4. */
#define XSYSMONPSU_VCC_PSPLL4_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPsbatt register definitions
 * @{
 */
/** Offset of the VCC_PSBATT register. */
#define XSYSMONPSU_VCC_PSBATT_OFFSET   0x00000074U
/** Reset value of the VCC_PSBATT register. */
#define XSYSMONPSU_VCC_PSBATT_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSBATT. */
#define XSYSMONPSU_VCC_PSBATT_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSBATT. */
#define XSYSMONPSU_VCC_PSBATT_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSBATT. */
#define XSYSMONPSU_VCC_PSBATT_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccint register definitions
 * @{
 */
/** Offset of the VCCINT register. */
#define XSYSMONPSU_VCCINT_OFFSET   0x00000078U
/** Reset value of the VCCINT register. */
#define XSYSMONPSU_VCCINT_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCCINT. */
#define XSYSMONPSU_VCCINT_VAL_SHIFT   0U
/** Bit width of the VAL field in VCCINT. */
#define XSYSMONPSU_VCCINT_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCCINT. */
#define XSYSMONPSU_VCCINT_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccbram register definitions
 * @{
 */
/** Offset of the VCCBRAM register. */
#define XSYSMONPSU_VCCBRAM_OFFSET   0x0000007CU
/** Reset value of the VCCBRAM register. */
#define XSYSMONPSU_VCCBRAM_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCCBRAM. */
#define XSYSMONPSU_VCCBRAM_VAL_SHIFT   0U
/** Bit width of the VAL field in VCCBRAM. */
#define XSYSMONPSU_VCCBRAM_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCCBRAM. */
#define XSYSMONPSU_VCCBRAM_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccaux register definitions
 * @{
 */
/** Offset of the VCCAUX register. */
#define XSYSMONPSU_VCCAUX_OFFSET   0x00000080U
/** Reset value of the VCCAUX register. */
#define XSYSMONPSU_VCCAUX_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCCAUX. */
#define XSYSMONPSU_VCCAUX_VAL_SHIFT   0U
/** Bit width of the VAL field in VCCAUX. */
#define XSYSMONPSU_VCCAUX_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCCAUX. */
#define XSYSMONPSU_VCCAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccPsddrpll register definitions
 * @{
 */
/** Offset of the VCC_PSDDRPLL register. */
#define XSYSMONPSU_VCC_PSDDRPLL_OFFSET   0x00000084U
/** Reset value of the VCC_PSDDRPLL register. */
#define XSYSMONPSU_VCC_PSDDRPLL_RSTVAL   0x00000000U

/** Bit shift of the VAL field in VCC_PSDDRPLL. */
#define XSYSMONPSU_VCC_PSDDRPLL_VAL_SHIFT   0U
/** Bit width of the VAL field in VCC_PSDDRPLL. */
#define XSYSMONPSU_VCC_PSDDRPLL_VAL_WIDTH   16U
/** Bit mask of the VAL field in VCC_PSDDRPLL. */
#define XSYSMONPSU_VCC_PSDDRPLL_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuDdrphyVref register definitions
 * @{
 */
/** Offset of the DDRPHY_VREF register. */
#define XSYSMONPSU_DDRPHY_VREF_OFFSET   0x00000088U
/** Reset value of the DDRPHY_VREF register. */
#define XSYSMONPSU_DDRPHY_VREF_RSTVAL   0x00000000U

/** Bit shift of the VAL field in DDRPHY_VREF. */
#define XSYSMONPSU_DDRPHY_VREF_VAL_SHIFT   0U
/** Bit width of the VAL field in DDRPHY_VREF. */
#define XSYSMONPSU_DDRPHY_VREF_VAL_WIDTH   16U
/** Bit mask of the VAL field in DDRPHY_VREF. */
#define XSYSMONPSU_DDRPHY_VREF_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuDdrphyAto register definitions
 * @{
 */
/** Offset of the DDRPHY_ATO register. */
#define XSYSMONPSU_DDRPHY_ATO_OFFSET   0x0000008CU
/** Reset value of the DDRPHY_ATO register. */
#define XSYSMONPSU_DDRPHY_ATO_RSTVAL   0x00000000U

/** Bit shift of the VAL field in DDRPHY_ATO. */
#define XSYSMONPSU_DDRPHY_ATO_VAL_SHIFT   0U
/** Bit width of the VAL field in DDRPHY_ATO. */
#define XSYSMONPSU_DDRPHY_ATO_VAL_WIDTH   16U
/** Bit mask of the VAL field in DDRPHY_ATO. */
#define XSYSMONPSU_DDRPHY_ATO_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuPsgtAt0 register definitions
 * @{
 */
/** Offset of the PSGT_AT0 register. */
#define XSYSMONPSU_PSGT_AT0_OFFSET   0x00000090U
/** Reset value of the PSGT_AT0 register. */
#define XSYSMONPSU_PSGT_AT0_RSTVAL   0x00000000U

/** Bit shift of the VAL field in PSGT_AT0. */
#define XSYSMONPSU_PSGT_AT0_VAL_SHIFT   0U
/** Bit width of the VAL field in PSGT_AT0. */
#define XSYSMONPSU_PSGT_AT0_VAL_WIDTH   16U
/** Bit mask of the VAL field in PSGT_AT0. */
#define XSYSMONPSU_PSGT_AT0_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuPsgtAt1 register definitions
 * @{
 */
/** Offset of the PSGT_AT1 register. */
#define XSYSMONPSU_PSGT_AT1_OFFSET   0x00000094U
/** Reset value of the PSGT_AT1 register. */
#define XSYSMONPSU_PSGT_AT1_RSTVAL   0x00000000U

/** Bit shift of the VAL field in PSGT_AT1. */
#define XSYSMONPSU_PSGT_AT1_VAL_SHIFT   0U
/** Bit width of the VAL field in PSGT_AT1. */
#define XSYSMONPSU_PSGT_AT1_VAL_WIDTH   16U
/** Bit mask of the VAL field in PSGT_AT1. */
#define XSYSMONPSU_PSGT_AT1_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuReserve0 register definitions
 * @{
 */
/** Offset of the RESERVE0 register. */
#define XSYSMONPSU_RESERVE0_OFFSET   0x00000098U
/** Reset value of the RESERVE0 register. */
#define XSYSMONPSU_RESERVE0_RSTVAL   0x00000000U

/** Bit shift of the VAL field in RESERVE0. */
#define XSYSMONPSU_RESERVE0_VAL_SHIFT   0U
/** Bit width of the VAL field in RESERVE0. */
#define XSYSMONPSU_RESERVE0_VAL_WIDTH   16U
/** Bit mask of the VAL field in RESERVE0. */
#define XSYSMONPSU_RESERVE0_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuReserve1 register definitions
 * @{
 */
/** Offset of the RESERVE1 register. */
#define XSYSMONPSU_RESERVE1_OFFSET   0x0000009CU
/** Reset value of the RESERVE1 register. */
#define XSYSMONPSU_RESERVE1_RSTVAL   0x00000000U

/** Bit shift of the VAL field in RESERVE1. */
#define XSYSMONPSU_RESERVE1_VAL_SHIFT   0U
/** Bit width of the VAL field in RESERVE1. */
#define XSYSMONPSU_RESERVE1_VAL_WIDTH   16U
/** Bit mask of the VAL field in RESERVE1. */
#define XSYSMONPSU_RESERVE1_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuTemp register definitions
 * @{
 */
/** Offset of the TEMP register. */
#define XSYSMONPSU_TEMP_OFFSET   0x00000000U
/** Reset value of the TEMP register. */
#define XSYSMONPSU_TEMP_RSTVAL   0x00000000U

#define XSYSMONPSU_TEMP_SHIFT   0U /**< Bit shift of the TEMP register value. */
/** Bit width of the TEMP register value. */
#define XSYSMONPSU_TEMP_WIDTH   16U
/** Bit mask of the TEMP register. */
#define XSYSMONPSU_TEMP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup1 register definitions
 * @{
 */
/** Offset of the SUP1 register. */
#define XSYSMONPSU_SUP1_OFFSET   0x00000004U
/** Reset value of the SUP1 register. */
#define XSYSMONPSU_SUP1_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP1. */
#define XSYSMONPSU_SUP1_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP1. */
#define XSYSMONPSU_SUP1_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP1. */
#define XSYSMONPSU_SUP1_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup2 register definitions
 * @{
 */
/** Offset of the SUP2 register. */
#define XSYSMONPSU_SUP2_OFFSET   0x00000008U
/** Reset value of the SUP2 register. */
#define XSYSMONPSU_SUP2_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP2. */
#define XSYSMONPSU_SUP2_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP2. */
#define XSYSMONPSU_SUP2_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP2. */
#define XSYSMONPSU_SUP2_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVpVn register definitions
 * @{
 */
/** Offset of the VP_VN register. */
#define XSYSMONPSU_VP_VN_OFFSET   0x0000000CU
/** Reset value of the VP_VN register. */
#define XSYSMONPSU_VP_VN_RSTVAL   0x00000000U

/** Bit shift of the VP_VN register value. */
#define XSYSMONPSU_VP_VN_SHIFT   0U
/** Bit width of the VP_VN register value. */
#define XSYSMONPSU_VP_VN_WIDTH   16U
/** Bit mask of the VP_VN register. */
#define XSYSMONPSU_VP_VN_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVrefp register definitions
 * @{
 */
/** Offset of the VREFP register. */
#define XSYSMONPSU_VREFP_OFFSET   0x00000010U
/** Reset value of the VREFP register. */
#define XSYSMONPSU_VREFP_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in VREFP. */
#define XSYSMONPSU_VREFP_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in VREFP. */
#define XSYSMONPSU_VREFP_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in VREFP. */
#define XSYSMONPSU_VREFP_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVrefn register definitions
 * @{
 */
/** Offset of the VREFN register. */
#define XSYSMONPSU_VREFN_OFFSET   0x00000014U
/** Reset value of the VREFN register. */
#define XSYSMONPSU_VREFN_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in VREFN. */
#define XSYSMONPSU_VREFN_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in VREFN. */
#define XSYSMONPSU_VREFN_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in VREFN. */
#define XSYSMONPSU_VREFN_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup3 register definitions
 * @{
 */
/** Offset of the SUP3 register. */
#define XSYSMONPSU_SUP3_OFFSET   0x00000018U
/** Reset value of the SUP3 register. */
#define XSYSMONPSU_SUP3_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP3. */
#define XSYSMONPSU_SUP3_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP3. */
#define XSYSMONPSU_SUP3_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP3. */
#define XSYSMONPSU_SUP3_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuCalSupOff register definitions
 * @{
 */
/** Offset of the CAL_SUP_OFF register. */
#define XSYSMONPSU_CAL_SUP_OFF_OFFSET   0x00000020U
/** Reset value of the CAL_SUP_OFF register. */
#define XSYSMONPSU_CAL_SUP_OFF_RSTVAL   0x00000000U

/** Bit shift of the VAL field in CAL_SUP_OFF. */
#define XSYSMONPSU_CAL_SUP_OFF_VAL_SHIFT   0U
/** Bit width of the VAL field in CAL_SUP_OFF. */
#define XSYSMONPSU_CAL_SUP_OFF_VAL_WIDTH   16U
/** Bit mask of the VAL field in CAL_SUP_OFF. */
#define XSYSMONPSU_CAL_SUP_OFF_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuCalAdcBiplrOff register definitions
 * @{
 */
/** Offset of the CAL_ADC_BIPLR_OFF register. */
#define XSYSMONPSU_CAL_ADC_BIPLR_OFF_OFFSET   0x00000024U
/** Reset value of the CAL_ADC_BIPLR_OFF register. */
#define XSYSMONPSU_CAL_ADC_BIPLR_OFF_RSTVAL   0x00000000U

/** Bit shift of the VAL field in CAL_ADC_BIPLR_OFF. */
#define XSYSMONPSU_CAL_ADC_BIPLR_OFF_VAL_SHIFT   0U
/** Bit width of the VAL field in CAL_ADC_BIPLR_OFF. */
#define XSYSMONPSU_CAL_ADC_BIPLR_OFF_VAL_WIDTH   16U
/** Bit mask of the VAL field in CAL_ADC_BIPLR_OFF. */
#define XSYSMONPSU_CAL_ADC_BIPLR_OFF_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuCalGainErr register definitions
 * @{
 */
/** Offset of the CAL_GAIN_ERR register. */
#define XSYSMONPSU_CAL_GAIN_ERR_OFFSET   0x00000028U
/** Reset value of the CAL_GAIN_ERR register. */
#define XSYSMONPSU_CAL_GAIN_ERR_RSTVAL   0x00000000U

/** Bit shift of the VAL field in CAL_GAIN_ERR. */
#define XSYSMONPSU_CAL_GAIN_ERR_VAL_SHIFT   0U
/** Bit width of the VAL field in CAL_GAIN_ERR. */
#define XSYSMONPSU_CAL_GAIN_ERR_VAL_WIDTH   16U
/** Bit mask of the VAL field in CAL_GAIN_ERR. */
#define XSYSMONPSU_CAL_GAIN_ERR_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup4 register definitions
 * @{
 */
/** Offset of the SUP4 register. */
#define XSYSMONPSU_SUP4_OFFSET   0x00000034U
/** Reset value of the SUP4 register. */
#define XSYSMONPSU_SUP4_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP4. */
#define XSYSMONPSU_SUP4_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP4. */
#define XSYSMONPSU_SUP4_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP4. */
#define XSYSMONPSU_SUP4_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup5 register definitions
 * @{
 */
/** Offset of the SUP5 register. */
#define XSYSMONPSU_SUP5_OFFSET   0x00000038U
/** Reset value of the SUP5 register. */
#define XSYSMONPSU_SUP5_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP5. */
#define XSYSMONPSU_SUP5_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP5. */
#define XSYSMONPSU_SUP5_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP5. */
#define XSYSMONPSU_SUP5_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup6 register definitions
 * @{
 */
/** Offset of the SUP6 register. */
#define XSYSMONPSU_SUP6_OFFSET   0x0000003CU
/** Reset value of the SUP6 register. */
#define XSYSMONPSU_SUP6_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP6. */
#define XSYSMONPSU_SUP6_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP6. */
#define XSYSMONPSU_SUP6_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP6. */
#define XSYSMONPSU_SUP6_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux00 register definitions
 * @{
 */
/** Offset of the VAUX00 register. */
#define XSYSMONPSU_VAUX00_OFFSET   0x00000040U
/** Reset value of the VAUX00 register. */
#define XSYSMONPSU_VAUX00_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX00. */
#define XSYSMONPSU_VAUX00_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX00. */
#define XSYSMONPSU_VAUX00_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX00. */
#define XSYSMONPSU_VAUX00_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux01 register definitions
 * @{
 */
/** Offset of the VAUX01 register. */
#define XSYSMONPSU_VAUX01_OFFSET   0x00000044U
/** Reset value of the VAUX01 register. */
#define XSYSMONPSU_VAUX01_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX01. */
#define XSYSMONPSU_VAUX01_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX01. */
#define XSYSMONPSU_VAUX01_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX01. */
#define XSYSMONPSU_VAUX01_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux02 register definitions
 * @{
 */
/** Offset of the VAUX02 register. */
#define XSYSMONPSU_VAUX02_OFFSET   0x00000048U
/** Reset value of the VAUX02 register. */
#define XSYSMONPSU_VAUX02_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX02. */
#define XSYSMONPSU_VAUX02_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX02. */
#define XSYSMONPSU_VAUX02_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX02. */
#define XSYSMONPSU_VAUX02_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux03 register definitions
 * @{
 */
/** Offset of the VAUX03 register. */
#define XSYSMONPSU_VAUX03_OFFSET   0x0000004CU
/** Reset value of the VAUX03 register. */
#define XSYSMONPSU_VAUX03_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX03. */
#define XSYSMONPSU_VAUX03_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX03. */
#define XSYSMONPSU_VAUX03_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX03. */
#define XSYSMONPSU_VAUX03_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux04 register definitions
 * @{
 */
/** Offset of the VAUX04 register. */
#define XSYSMONPSU_VAUX04_OFFSET   0x00000050U
/** Reset value of the VAUX04 register. */
#define XSYSMONPSU_VAUX04_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX04. */
#define XSYSMONPSU_VAUX04_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX04. */
#define XSYSMONPSU_VAUX04_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX04. */
#define XSYSMONPSU_VAUX04_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux05 register definitions
 * @{
 */
/** Offset of the VAUX05 register. */
#define XSYSMONPSU_VAUX05_OFFSET   0x00000054U
/** Reset value of the VAUX05 register. */
#define XSYSMONPSU_VAUX05_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX05. */
#define XSYSMONPSU_VAUX05_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX05. */
#define XSYSMONPSU_VAUX05_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX05. */
#define XSYSMONPSU_VAUX05_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux06 register definitions
 * @{
 */
/** Offset of the VAUX06 register. */
#define XSYSMONPSU_VAUX06_OFFSET   0x00000058U
/** Reset value of the VAUX06 register. */
#define XSYSMONPSU_VAUX06_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX06. */
#define XSYSMONPSU_VAUX06_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX06. */
#define XSYSMONPSU_VAUX06_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX06. */
#define XSYSMONPSU_VAUX06_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux07 register definitions
 * @{
 */
/** Offset of the VAUX07 register. */
#define XSYSMONPSU_VAUX07_OFFSET   0x0000005CU
/** Reset value of the VAUX07 register. */
#define XSYSMONPSU_VAUX07_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX07. */
#define XSYSMONPSU_VAUX07_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX07. */
#define XSYSMONPSU_VAUX07_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX07. */
#define XSYSMONPSU_VAUX07_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux08 register definitions
 * @{
 */
/** Offset of the VAUX08 register. */
#define XSYSMONPSU_VAUX08_OFFSET   0x00000060U
/** Reset value of the VAUX08 register. */
#define XSYSMONPSU_VAUX08_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX08. */
#define XSYSMONPSU_VAUX08_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX08. */
#define XSYSMONPSU_VAUX08_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX08. */
#define XSYSMONPSU_VAUX08_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux09 register definitions
 * @{
 */
/** Offset of the VAUX09 register. */
#define XSYSMONPSU_VAUX09_OFFSET   0x00000064U
/** Reset value of the VAUX09 register. */
#define XSYSMONPSU_VAUX09_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX09. */
#define XSYSMONPSU_VAUX09_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX09. */
#define XSYSMONPSU_VAUX09_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX09. */
#define XSYSMONPSU_VAUX09_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0a register definitions
 * @{
 */
/** Offset of the VAUX0A register. */
#define XSYSMONPSU_VAUX0A_OFFSET   0x00000068U
/** Reset value of the VAUX0A register. */
#define XSYSMONPSU_VAUX0A_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0A. */
#define XSYSMONPSU_VAUX0A_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0A. */
#define XSYSMONPSU_VAUX0A_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0A. */
#define XSYSMONPSU_VAUX0A_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0b register definitions
 * @{
 */
/** Offset of the VAUX0B register. */
#define XSYSMONPSU_VAUX0B_OFFSET   0x0000006CU
/** Reset value of the VAUX0B register. */
#define XSYSMONPSU_VAUX0B_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0B. */
#define XSYSMONPSU_VAUX0B_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0B. */
#define XSYSMONPSU_VAUX0B_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0B. */
#define XSYSMONPSU_VAUX0B_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0c register definitions
 * @{
 */
/** Offset of the VAUX0C register. */
#define XSYSMONPSU_VAUX0C_OFFSET   0x00000070U
/** Reset value of the VAUX0C register. */
#define XSYSMONPSU_VAUX0C_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0C. */
#define XSYSMONPSU_VAUX0C_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0C. */
#define XSYSMONPSU_VAUX0C_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0C. */
#define XSYSMONPSU_VAUX0C_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0d register definitions
 * @{
 */
/** Offset of the VAUX0D register. */
#define XSYSMONPSU_VAUX0D_OFFSET   0x00000074U
/** Reset value of the VAUX0D register. */
#define XSYSMONPSU_VAUX0D_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0D. */
#define XSYSMONPSU_VAUX0D_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0D. */
#define XSYSMONPSU_VAUX0D_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0D. */
#define XSYSMONPSU_VAUX0D_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0e register definitions
 * @{
 */
/** Offset of the VAUX0E register. */
#define XSYSMONPSU_VAUX0E_OFFSET   0x00000078U
/** Reset value of the VAUX0E register. */
#define XSYSMONPSU_VAUX0E_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0E. */
#define XSYSMONPSU_VAUX0E_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0E. */
#define XSYSMONPSU_VAUX0E_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0E. */
#define XSYSMONPSU_VAUX0E_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVaux0f register definitions
 * @{
 */
/** Offset of the VAUX0F register. */
#define XSYSMONPSU_VAUX0F_OFFSET   0x0000007CU
/** Reset value of the VAUX0F register. */
#define XSYSMONPSU_VAUX0F_RSTVAL   0x00000000U

/** Bit shift of the VAUX_VAL field in VAUX0F. */
#define XSYSMONPSU_VAUX0F_VAUX_VAL_SHIFT   0U
/** Bit width of the VAUX_VAL field in VAUX0F. */
#define XSYSMONPSU_VAUX0F_VAUX_VAL_WIDTH   16U
/** Bit mask of the VAUX_VAL field in VAUX0F. */
#define XSYSMONPSU_VAUX0F_VAUX_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxTemp register definitions
 * @{
 */
/** Offset of the MAX_TEMP register. */
#define XSYSMONPSU_MAX_TEMP_OFFSET   0x00000080U
/** Reset value of the MAX_TEMP register. */
#define XSYSMONPSU_MAX_TEMP_RSTVAL   0x00000000U

/** Bit shift of the MAX_TEMP register value. */
#define XSYSMONPSU_MAX_TEMP_SHIFT   0U
/** Bit width of the MAX_TEMP register value. */
#define XSYSMONPSU_MAX_TEMP_WIDTH   16U
/** Bit mask of the MAX_TEMP register. */
#define XSYSMONPSU_MAX_TEMP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup1 register definitions
 * @{
 */
/** Offset of the MAX_SUP1 register. */
#define XSYSMONPSU_MAX_SUP1_OFFSET   0x00000084U
/** Reset value of the MAX_SUP1 register. */
#define XSYSMONPSU_MAX_SUP1_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP1. */
#define XSYSMONPSU_MAX_SUP1_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP1. */
#define XSYSMONPSU_MAX_SUP1_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP1. */
#define XSYSMONPSU_MAX_SUP1_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup2 register definitions
 * @{
 */
/** Offset of the MAX_SUP2 register. */
#define XSYSMONPSU_MAX_SUP2_OFFSET   0x00000088U
/** Reset value of the MAX_SUP2 register. */
#define XSYSMONPSU_MAX_SUP2_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP2. */
#define XSYSMONPSU_MAX_SUP2_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP2. */
#define XSYSMONPSU_MAX_SUP2_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP2. */
#define XSYSMONPSU_MAX_SUP2_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup3 register definitions
 * @{
 */
/** Offset of the MAX_SUP3 register. */
#define XSYSMONPSU_MAX_SUP3_OFFSET   0x0000008CU
/** Reset value of the MAX_SUP3 register. */
#define XSYSMONPSU_MAX_SUP3_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP3. */
#define XSYSMONPSU_MAX_SUP3_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP3. */
#define XSYSMONPSU_MAX_SUP3_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP3. */
#define XSYSMONPSU_MAX_SUP3_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinTemp register definitions
 * @{
 */
/** Offset of the MIN_TEMP register. */
#define XSYSMONPSU_MIN_TEMP_OFFSET   0x00000090U
/** Reset value of the MIN_TEMP register. */
#define XSYSMONPSU_MIN_TEMP_RSTVAL   0x0000ffffU

/** Bit shift of the MIN_TEMP register value. */
#define XSYSMONPSU_MIN_TEMP_SHIFT   0U
/** Bit width of the MIN_TEMP register value. */
#define XSYSMONPSU_MIN_TEMP_WIDTH   16U
/** Bit mask of the MIN_TEMP register. */
#define XSYSMONPSU_MIN_TEMP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup1 register definitions
 * @{
 */
/** Offset of the MIN_SUP1 register. */
#define XSYSMONPSU_MIN_SUP1_OFFSET   0x00000094U
/** Reset value of the MIN_SUP1 register. */
#define XSYSMONPSU_MIN_SUP1_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP1. */
#define XSYSMONPSU_MIN_SUP1_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP1. */
#define XSYSMONPSU_MIN_SUP1_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP1. */
#define XSYSMONPSU_MIN_SUP1_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup2 register definitions
 * @{
 */
/** Offset of the MIN_SUP2 register. */
#define XSYSMONPSU_MIN_SUP2_OFFSET   0x00000098U
/** Reset value of the MIN_SUP2 register. */
#define XSYSMONPSU_MIN_SUP2_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP2. */
#define XSYSMONPSU_MIN_SUP2_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP2. */
#define XSYSMONPSU_MIN_SUP2_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP2. */
#define XSYSMONPSU_MIN_SUP2_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup3 register definitions
 * @{
 */
/** Offset of the MIN_SUP3 register. */
#define XSYSMONPSU_MIN_SUP3_OFFSET   0x0000009CU
/** Reset value of the MIN_SUP3 register. */
#define XSYSMONPSU_MIN_SUP3_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP3. */
#define XSYSMONPSU_MIN_SUP3_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP3. */
#define XSYSMONPSU_MIN_SUP3_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP3. */
#define XSYSMONPSU_MIN_SUP3_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup4 register definitions
 * @{
 */
/** Offset of the MAX_SUP4 register. */
#define XSYSMONPSU_MAX_SUP4_OFFSET   0x000000A0U
/** Reset value of the MAX_SUP4 register. */
#define XSYSMONPSU_MAX_SUP4_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP4. */
#define XSYSMONPSU_MAX_SUP4_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP4. */
#define XSYSMONPSU_MAX_SUP4_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP4. */
#define XSYSMONPSU_MAX_SUP4_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup5 register definitions
 * @{
 */
/** Offset of the MAX_SUP5 register. */
#define XSYSMONPSU_MAX_SUP5_OFFSET   0x000000A4U
/** Reset value of the MAX_SUP5 register. */
#define XSYSMONPSU_MAX_SUP5_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP5. */
#define XSYSMONPSU_MAX_SUP5_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP5. */
#define XSYSMONPSU_MAX_SUP5_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP5. */
#define XSYSMONPSU_MAX_SUP5_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup6 register definitions
 * @{
 */
/** Offset of the MAX_SUP6 register. */
#define XSYSMONPSU_MAX_SUP6_OFFSET   0x000000A8U
/** Reset value of the MAX_SUP6 register. */
#define XSYSMONPSU_MAX_SUP6_RSTVAL   0x00000000U

/** Bit shift of the VAL field in MAX_SUP6. */
#define XSYSMONPSU_MAX_SUP6_VAL_SHIFT   0U
/** Bit width of the VAL field in MAX_SUP6. */
#define XSYSMONPSU_MAX_SUP6_VAL_WIDTH   16U
/** Bit mask of the VAL field in MAX_SUP6. */
#define XSYSMONPSU_MAX_SUP6_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup4 register definitions
 * @{
 */
/** Offset of the MIN_SUP4 register. */
#define XSYSMONPSU_MIN_SUP4_OFFSET   0x000000B0U
/** Reset value of the MIN_SUP4 register. */
#define XSYSMONPSU_MIN_SUP4_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP4. */
#define XSYSMONPSU_MIN_SUP4_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP4. */
#define XSYSMONPSU_MIN_SUP4_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP4. */
#define XSYSMONPSU_MIN_SUP4_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup5 register definitions
 * @{
 */
/** Offset of the MIN_SUP5 register. */
#define XSYSMONPSU_MIN_SUP5_OFFSET   0x000000B4U
/** Reset value of the MIN_SUP5 register. */
#define XSYSMONPSU_MIN_SUP5_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP5. */
#define XSYSMONPSU_MIN_SUP5_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP5. */
#define XSYSMONPSU_MIN_SUP5_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP5. */
#define XSYSMONPSU_MIN_SUP5_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup6 register definitions
 * @{
 */
/** Offset of the MIN_SUP6 register. */
#define XSYSMONPSU_MIN_SUP6_OFFSET   0x000000B8U
/** Reset value of the MIN_SUP6 register. */
#define XSYSMONPSU_MIN_SUP6_RSTVAL   0x0000ffffU

/** Bit shift of the VAL field in MIN_SUP6. */
#define XSYSMONPSU_MIN_SUP6_VAL_SHIFT   0U
/** Bit width of the VAL field in MIN_SUP6. */
#define XSYSMONPSU_MIN_SUP6_VAL_WIDTH   16U
/** Bit mask of the VAL field in MIN_SUP6. */
#define XSYSMONPSU_MIN_SUP6_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuStsFlag register definitions
 * @{
 */
/** Offset of the STS_FLAG register. */
#define XSYSMONPSU_STS_FLAG_OFFSET   0x000000FCU
/** Reset value of the STS_FLAG register. */
#define XSYSMONPSU_STS_FLAG_RSTVAL   0x00000000U

/** Bit shift of the CLK_OSC_USED field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_CLK_OSC_USED_SHIFT   15U
/** Bit width of the CLK_OSC_USED field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_CLK_OSC_USED_WIDTH   1U
/** Bit mask of the CLK_OSC_USED field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_CLK_OSC_USED_MASK    0x00008000U

/** Bit shift of the BLK_IN_RST field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_BLK_IN_RST_SHIFT   14U
/** Bit width of the BLK_IN_RST field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_BLK_IN_RST_WIDTH   1U
/** Bit mask of the BLK_IN_RST field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_BLK_IN_RST_MASK    0x00004000U

/** Bit shift of the JTAG_DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_DISD_SHIFT   11U
/** Bit width of the JTAG_DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_DISD_WIDTH   1U
/** Bit mask of the JTAG_DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_DISD_MASK    0x00000800U

/** Bit shift of the JTAG_RD_ONLY field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_RD_ONLY_SHIFT   10U
/** Bit width of the JTAG_RD_ONLY field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_RD_ONLY_WIDTH   1U
/** Bit mask of the JTAG_RD_ONLY field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_JTAG_RD_ONLY_MASK    0x00000400U

/** Bit shift of the INTRNL_REF field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_INTRNL_REF_SHIFT   9U
/** Bit width of the INTRNL_REF field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_INTRNL_REF_WIDTH   1U
/** Bit mask of the INTRNL_REF field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_INTRNL_REF_MASK    0x00000200U

/** Bit shift of the DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_DISD_SHIFT   8U
/** Bit width of the DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_DISD_WIDTH   1U
/** Bit mask of the DISD field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_DISD_MASK    0x00000100U

/** Bit shift of the ALM_6_3 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_6_3_SHIFT   4U
/** Bit width of the ALM_6_3 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_6_3_WIDTH   4U
/** Bit mask of the ALM_6_3 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_6_3_MASK    0x000000f0U

/** Bit shift of the OT field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_OT_SHIFT   3U
/** Bit width of the OT field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_OT_WIDTH   1U
/** Bit mask of the OT field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_OT_MASK    0x00000008U

/** Bit shift of the ALM_2_0 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_2_0_SHIFT   0U
/** Bit width of the ALM_2_0 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_2_0_WIDTH   3U
/** Bit mask of the ALM_2_0 field in STS_FLAG. */
#define XSYSMONPSU_STS_FLAG_ALM_2_0_MASK    0x00000007U
/** @} */

/**
 * @name XSysmonPsuCfgReg0 register definitions
 * @{
 */
/** Offset of the CFG_REG0 register. */
#define XSYSMONPSU_CFG_REG0_OFFSET   0x00000100U
/** Reset value of the CFG_REG0 register. */
#define XSYSMONPSU_CFG_REG0_RSTVAL   0x00000000U

/** Bit shift of the AVRGNG field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_AVRGNG_SHIFT   12U
/** Bit width of the AVRGNG field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_AVRGNG_WIDTH   2U
/** Bit mask of the AVRGNG field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_AVRGNG_MASK    0x00003000U

/** Bit shift of the XTRNL_MUX field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_XTRNL_MUX_SHIFT   11U
/** Bit width of the XTRNL_MUX field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_XTRNL_MUX_WIDTH   1U
/** Bit mask of the XTRNL_MUX field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_XTRNL_MUX_MASK    0x00000800U

/** Bit shift of the BU field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_BU_SHIFT   10U
/** Bit width of the BU field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_BU_WIDTH   1U
/** Bit mask of the BU field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_BU_MASK    0x00000400U

/** Bit shift of the EC field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_EC_SHIFT   9U
/** Bit width of the EC field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_EC_WIDTH   1U
/** Bit mask of the EC field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_EC_MASK    0x00000200U

#define XSYSMONPSU_EVENT_MODE	1 /**< Event sampling mode. */
#define XSYSMONPSU_CONTINUOUS_MODE 2 /**< Continuous sampling mode. */

/** Bit shift of the ACQ field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_ACQ_SHIFT   8U
/** Bit width of the ACQ field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_ACQ_WIDTH   1U
/** Bit mask of the ACQ field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_ACQ_MASK    0x00000100U

/** Bit shift of the MUX_CH field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_MUX_CH_SHIFT   0U
/** Bit width of the MUX_CH field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_MUX_CH_WIDTH   6U
/** Bit mask of the MUX_CH field in CFG_REG0. */
#define XSYSMONPSU_CFG_REG0_MUX_CH_MASK    0x0000003fU
/** @} */

/**
 * @name XSysmonPsuCfgReg1 register definitions
 * @{
 */
/** Offset of the CFG_REG1 register. */
#define XSYSMONPSU_CFG_REG1_OFFSET   0x00000104U
/** Reset value of the CFG_REG1 register. */
#define XSYSMONPSU_CFG_REG1_RSTVAL   0x00000000U

/** Bit shift of the SEQ_MDE field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_SEQ_MDE_SHIFT   12U
/** Bit width of the SEQ_MDE field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_SEQ_MDE_WIDTH   4U
/** Bit mask of the SEQ_MDE field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_SEQ_MDE_MASK    0x0000f000U

/** Bit shift of the ALRM_DIS6TO3 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS6TO3_SHIFT   8U
/** Bit width of the ALRM_DIS6TO3 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS6TO3_WIDTH   4U
/** Bit mask of the ALRM_DIS6TO3 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS6TO3_MASK    0x00000f00U

/** Bit shift of the ALRM_DIS2TO0 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS2TO0_SHIFT   1U
/** Bit width of the ALRM_DIS2TO0 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS2TO0_WIDTH   3U
/** Bit mask of the ALRM_DIS2TO0 field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_DIS2TO0_MASK    0x0000000eU

/** Bit shift of the OVR_TEMP_DIS field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_OVR_TEMP_DIS_SHIFT   0U
/** Bit width of the OVR_TEMP_DIS field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_OVR_TEMP_DIS_WIDTH   1U
/** Bit mask of the OVR_TEMP_DIS field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_OVR_TEMP_DIS_MASK    0x00000001U

/** Bit mask of the ALRM_ALL field in CFG_REG1. */
#define XSYSMONPSU_CFG_REG1_ALRM_ALL_MASK 0x00000f0fU
/** Supply 6 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP6_MASK		0x0800U
/** Supply 5 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP5_MASK		0x0400U
/** Supply 4 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP4_MASK		0x0200U
/** Supply 3 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP3_MASK		0x0100U
/** Supply 2 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP2_MASK		0x0008U
/** Supply 1 alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_SUP1_MASK		0x0004U
/** Temperature alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_TEMP_MASK		0x0002U
/** Over-temperature alarm bit mask in Configuration Register 1. */
#define XSYSMONPSU_CFR_REG1_ALRM_OT_MASK		0x0001U
/** @} */

/**
 * @name XSysmonPsuCfgReg2 register definitions
 * @{
 */
/** Offset of the CFG_REG2 register. */
#define XSYSMONPSU_CFG_REG2_OFFSET   0x00000108U
/** Reset value of the CFG_REG2 register. */
#define XSYSMONPSU_CFG_REG2_RSTVAL   0x00000000U

/** Bit shift of the CLK_DVDR field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_CLK_DVDR_SHIFT   8U
/** Bit width of the CLK_DVDR field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_CLK_DVDR_WIDTH   8U
/** Bit mask of the CLK_DVDR field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_CLK_DVDR_MASK    0x0000ff00U

/** Minimum clock divider value. */
#define XSYSMONPSU_CLK_DVDR_MIN_VAL			0U
/** Maximum clock divider value. */
#define XSYSMONPSU_CLK_DVDR_MAX_VAL			255U

/** Bit shift of the PWR_DOWN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_PWR_DOWN_SHIFT   4U
/** Bit width of the PWR_DOWN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_PWR_DOWN_WIDTH   4U
/** Bit mask of the PWR_DOWN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_PWR_DOWN_MASK    0x000000f0U

/** Bit shift of the TST_CH_EN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_CH_EN_SHIFT   2U
/** Bit width of the TST_CH_EN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_CH_EN_WIDTH   1U
/** Bit mask of the TST_CH_EN field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_CH_EN_MASK    0x00000004U

/** Bit shift of the TST_MDE field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_MDE_SHIFT   0U
/** Bit width of the TST_MDE field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_MDE_WIDTH   2U
/** Bit mask of the TST_MDE field in CFG_REG2. */
#define XSYSMONPSU_CFG_REG2_TST_MDE_MASK    0x00000003U
/** @} */

/**
 * @name XSysmonPsuCfgReg3 register definitions
 * @{
 */
/** Offset of the CFG_REG3 register. */
#define XSYSMONPSU_CFG_REG3_OFFSET		0x0000010CU
/** Bit mask of the ALRM_ALL field in CFG_REG3. */
#define XSYSMONPSU_CFG_REG3_ALRM_ALL_MASK	0x0000003FU

/**
 * Shift of the Configuration Register 3 alarm bits in the alarm enable mask.
 */
#define XSM_CFG_ALARM_SHIFT			16U
/** @} */

/**
 * @name XSysmonPsuSeqCh2 register definitions
 * @{
 */
/** Offset of the SEQ_CH2 register. */
#define XSYSMONPSU_SEQ_CH2_OFFSET   0x00000118U

/** Bit shift of the TEMP_RMT field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_TEMP_RMT_SHIFT	5U
/** Bit mask of the TEMP_RMT field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_TEMP_RMT_MASK	0x00000020U

/** Bit shift of the VCCAMS field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_VCCAMS_SHIFT		4U
/** Bit mask of the VCCAMS field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_VCCAMS_MASK		0x00000010U

/** Bit shift of the SUP10 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP10_SHIFT		3U
/** Bit mask of the SUP10 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP10_MASK		0x00000008U

/** Bit shift of the SUP9 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP9_SHIFT		2U
/** Bit mask of the SUP9 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP9_MASK		0x00000004U

/** Bit shift of the SUP8 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP8_SHIFT		1U
/** Bit mask of the SUP8 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP8_MASK		0x00000002U

/** Bit shift of the SUP7 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP7_SHIFT		0U
/** Bit mask of the SUP7 field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_SUP7_MASK		0x00000001U

/** Bit mask of the VALID field in SEQ_CH2. */
#define XSYSMONPSU_SEQ_CH2_VALID_MASK     	0x0000003FU
/** @} */

/**
 * @name XSysmonPsuSeqAverage2 register definitions
 * @{
 */
/** Offset of the SEQ_AVERAGE2 register. */
#define XSYSMONPSU_SEQ_AVERAGE2_OFFSET  0x0000011CU
/** Reset value of the SEQ_AVERAGE1 register. */
#define XSYSMONPSU_SEQ_AVERAGE1_RSTVAL  0x00000000U
/** Bit mask of the SEQ_AVERAGE2 register. */
#define XSYSMONPSU_SEQ_AVERAGE2_MASK	0x0000003FU
/** @} */

/**
 * @name XSysmonPsuSeqCh0 register definitions
 * @{
 */
/** Offset of the SEQ_CH0 register. */
#define XSYSMONPSU_SEQ_CH0_OFFSET   0x00000120U
/** Reset value of the SEQ_CH0 register. */
#define XSYSMONPSU_SEQ_CH0_RSTVAL   0x00000000U

/** Bit shift of the CUR_MON field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CUR_MON_SHIFT   15U
/** Bit width of the CUR_MON field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CUR_MON_WIDTH   1U
/** Bit mask of the CUR_MON field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CUR_MON_MASK    0x00008000U

/** Bit shift of the SUP3 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP3_SHIFT   14U
/** Bit width of the SUP3 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP3_WIDTH   1U
/** Bit mask of the SUP3 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP3_MASK    0x00004000U

/** Bit shift of the VREFN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFN_SHIFT   13U
/** Bit width of the VREFN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFN_WIDTH   1U
/** Bit mask of the VREFN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFN_MASK    0x00002000U

/** Bit shift of the VREFP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFP_SHIFT   12U
/** Bit width of the VREFP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFP_WIDTH   1U
/** Bit mask of the VREFP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VREFP_MASK    0x00001000U

/** Bit shift of the VP_VN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VP_VN_SHIFT   11U
/** Bit width of the VP_VN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VP_VN_WIDTH   1U
/** Bit mask of the VP_VN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VP_VN_MASK    0x00000800U

/** Bit shift of the SUP2 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP2_SHIFT   10U
/** Bit width of the SUP2 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP2_WIDTH   1U
/** Bit mask of the SUP2 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP2_MASK    0x00000400U

/** Bit shift of the SUP1 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP1_SHIFT   9U
/** Bit width of the SUP1 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP1_WIDTH   1U
/** Bit mask of the SUP1 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP1_MASK    0x00000200U

/** Bit shift of the TEMP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TEMP_SHIFT   8U
/** Bit width of the TEMP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TEMP_WIDTH   1U
/** Bit mask of the TEMP field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TEMP_MASK    0x00000100U

/** Bit shift of the SUP6 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP6_SHIFT   7U
/** Bit width of the SUP6 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP6_WIDTH   1U
/** Bit mask of the SUP6 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP6_MASK    0x00000080U

/** Bit shift of the SUP5 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP5_SHIFT   6U
/** Bit width of the SUP5 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP5_WIDTH   1U
/** Bit mask of the SUP5 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP5_MASK    0x00000040U

/** Bit shift of the SUP4 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP4_SHIFT   5U
/** Bit width of the SUP4 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP4_WIDTH   1U
/** Bit mask of the SUP4 field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_SUP4_MASK    0x00000020U

/** Bit shift of the TST_CH field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TST_CH_SHIFT   3U
/** Bit width of the TST_CH field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TST_CH_WIDTH   1U
/** Bit mask of the TST_CH field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_TST_CH_MASK    0x00000008U

/** Bit shift of the CALIBRTN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CALIBRTN_SHIFT   0U
/** Bit width of the CALIBRTN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CALIBRTN_WIDTH   1U
/** Bit mask of the CALIBRTN field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_CALIBRTN_MASK    0x00000001U

/** Bit mask of the VALID field in SEQ_CH0. */
#define XSYSMONPSU_SEQ_CH0_VALID_MASK     0x0000FFE9U
/** @} */

/**
 * @name XSysmonPsuSeqCh1 register definitions
 * @{
 */
/** Offset of the SEQ_CH1 register. */
#define XSYSMONPSU_SEQ_CH1_OFFSET         0x00000124U
/** Bit mask of the VALID field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VALID_MASK     0x0000FFFFU
/** Reset value of the SEQ_CH1 register. */
#define XSYSMONPSU_SEQ_CH1_RSTVAL         0x00000000U

/** Bit shift of the VAUX0F field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0F_SHIFT   15U
/** Bit width of the VAUX0F field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0F_WIDTH   1U
/** Bit mask of the VAUX0F field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0F_MASK    0x00008000U

/** Bit shift of the VAUX0E field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0E_SHIFT   14U
/** Bit width of the VAUX0E field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0E_WIDTH   1U
/** Bit mask of the VAUX0E field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0E_MASK    0x00004000U

/** Bit shift of the VAUX0D field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0D_SHIFT   13U
/** Bit width of the VAUX0D field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0D_WIDTH   1U
/** Bit mask of the VAUX0D field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0D_MASK    0x00002000U

/** Bit shift of the VAUX0C field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0C_SHIFT   12U
/** Bit width of the VAUX0C field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0C_WIDTH   1U
/** Bit mask of the VAUX0C field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0C_MASK    0x00001000U

/** Bit shift of the VAUX0B field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0B_SHIFT   11U
/** Bit width of the VAUX0B field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0B_WIDTH   1U
/** Bit mask of the VAUX0B field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0B_MASK    0x00000800U

/** Bit shift of the VAUX0A field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0A_SHIFT   10U
/** Bit width of the VAUX0A field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0A_WIDTH   1U
/** Bit mask of the VAUX0A field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX0A_MASK    0x00000400U

/** Bit shift of the VAUX09 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX09_SHIFT   9U
/** Bit width of the VAUX09 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX09_WIDTH   1U
/** Bit mask of the VAUX09 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX09_MASK    0x00000200U

/** Bit shift of the VAUX08 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX08_SHIFT   8U
/** Bit width of the VAUX08 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX08_WIDTH   1U
/** Bit mask of the VAUX08 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX08_MASK    0x00000100U

/** Bit shift of the VAUX07 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX07_SHIFT   7U
/** Bit width of the VAUX07 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX07_WIDTH   1U
/** Bit mask of the VAUX07 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX07_MASK    0x00000080U

/** Bit shift of the VAUX06 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX06_SHIFT   6U
/** Bit width of the VAUX06 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX06_WIDTH   1U
/** Bit mask of the VAUX06 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX06_MASK    0x00000040U

/** Bit shift of the VAUX05 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX05_SHIFT   5U
/** Bit width of the VAUX05 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX05_WIDTH   1U
/** Bit mask of the VAUX05 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX05_MASK    0x00000020U

/** Bit shift of the VAUX04 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX04_SHIFT   4U
/** Bit width of the VAUX04 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX04_WIDTH   1U
/** Bit mask of the VAUX04 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX04_MASK    0x00000010U

/** Bit shift of the VAUX03 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX03_SHIFT   3U
/** Bit width of the VAUX03 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX03_WIDTH   1U
/** Bit mask of the VAUX03 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX03_MASK    0x00000008U

/** Bit shift of the VAUX02 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX02_SHIFT   2U
/** Bit width of the VAUX02 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX02_WIDTH   1U
/** Bit mask of the VAUX02 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX02_MASK    0x00000004U

/** Bit shift of the VAUX01 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX01_SHIFT   1U
/** Bit width of the VAUX01 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX01_WIDTH   1U
/** Bit mask of the VAUX01 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX01_MASK    0x00000002U

/** Bit shift of the VAUX00 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX00_SHIFT   0U
/** Bit width of the VAUX00 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX00_WIDTH   1U
/** Bit mask of the VAUX00 field in SEQ_CH1. */
#define XSYSMONPSU_SEQ_CH1_VAUX00_MASK    0x00000001U

/** Shift of the SEQ_CH1 channel bits in the channel enable mask. */
#define XSM_SEQ_CH_SHIFT                  16U
/** Shift of the SEQ_CH2 channel bits in the channel enable mask. */
#define XSM_SEQ_CH2_SHIFT                 32U
/** @} */

/**
 * @name XSysmonPsuSeqAverage0 register definitions
 * @{
 */
/** Offset of the SEQ_AVERAGE0 register. */
#define XSYSMONPSU_SEQ_AVERAGE0_OFFSET   0x00000128U
/** Reset value of the SEQ_AVERAGE0 register. */
#define XSYSMONPSU_SEQ_AVERAGE0_RSTVAL   0x00000000U

/** Bit shift of the SEQ_AVERAGE0 register value. */
#define XSYSMONPSU_SEQ_AVERAGE0_SHIFT   0U
/** Bit width of the SEQ_AVERAGE0 register value. */
#define XSYSMONPSU_SEQ_AVERAGE0_WIDTH   16U
/** Bit mask of the SEQ_AVERAGE0 register. */
#define XSYSMONPSU_SEQ_AVERAGE0_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSeqAverage1 register definitions
 * @{
 */
/** Offset of the SEQ_AVERAGE1 register. */
#define XSYSMONPSU_SEQ_AVERAGE1_OFFSET   0x0000012CU
/** Reset value of the SEQ_AVERAGE1 register. */
#define XSYSMONPSU_SEQ_AVERAGE1_RSTVAL   0x00000000U

/** Bit shift of the SEQ_AVERAGE1 register value. */
#define XSYSMONPSU_SEQ_AVERAGE1_SHIFT   0U
/** Bit width of the SEQ_AVERAGE1 register value. */
#define XSYSMONPSU_SEQ_AVERAGE1_WIDTH   16U
/** Bit mask of the SEQ_AVERAGE1 register. */
#define XSYSMONPSU_SEQ_AVERAGE1_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSeqInputMde0 register definitions
 * @{
 */
/** Offset of the SEQ_INPUT_MDE0 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE0_OFFSET   0x00000130U
/** Reset value of the SEQ_INPUT_MDE0 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE0_RSTVAL   0x00000000U

/** Bit shift of the SEQ_INPUT_MDE0 register value. */
#define XSYSMONPSU_SEQ_INPUT_MDE0_SHIFT   0U
/** Bit width of the SEQ_INPUT_MDE0 register value. */
#define XSYSMONPSU_SEQ_INPUT_MDE0_WIDTH   16U
/** Bit mask of the SEQ_INPUT_MDE0 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE0_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSeqInputMde1 register definitions
 * @{
 */
/** Offset of the SEQ_INPUT_MDE1 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE1_OFFSET   0x00000134U
/** Reset value of the SEQ_INPUT_MDE1 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE1_RSTVAL   0x00000000U

/** Bit shift of the SEQ_INPUT_MDE1 register value. */
#define XSYSMONPSU_SEQ_INPUT_MDE1_SHIFT   0U
/** Bit width of the SEQ_INPUT_MDE1 register value. */
#define XSYSMONPSU_SEQ_INPUT_MDE1_WIDTH   16U
/** Bit mask of the SEQ_INPUT_MDE1 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE1_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSeqAcq0 register definitions
 * @{
 */
/** Offset of the SEQ_ACQ0 register. */
#define XSYSMONPSU_SEQ_ACQ0_OFFSET   0x00000138U
/** Reset value of the SEQ_ACQ0 register. */
#define XSYSMONPSU_SEQ_ACQ0_RSTVAL   0x00000000U

/** Bit shift of the SEQ_ACQ0 register value. */
#define XSYSMONPSU_SEQ_ACQ0_SHIFT   0U
/** Bit width of the SEQ_ACQ0 register value. */
#define XSYSMONPSU_SEQ_ACQ0_WIDTH   16U
/** Bit mask of the SEQ_ACQ0 register. */
#define XSYSMONPSU_SEQ_ACQ0_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSeqAcq1 register definitions
 * @{
 */
/** Offset of the SEQ_ACQ1 register. */
#define XSYSMONPSU_SEQ_ACQ1_OFFSET   0x0000013CU
/** Reset value of the SEQ_ACQ1 register. */
#define XSYSMONPSU_SEQ_ACQ1_RSTVAL   0x00000000U

/** Bit shift of the SEQ_ACQ1 register value. */
#define XSYSMONPSU_SEQ_ACQ1_SHIFT   0U
/** Bit width of the SEQ_ACQ1 register value. */
#define XSYSMONPSU_SEQ_ACQ1_WIDTH   16U
/** Bit mask of the SEQ_ACQ1 register. */
#define XSYSMONPSU_SEQ_ACQ1_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmTempUpr register definitions
 * @{
 */
/** Offset of the ALRM_TEMP_UPR register. */
#define XSYSMONPSU_ALRM_TEMP_UPR_OFFSET   0x00000140U
/** Reset value of the ALRM_TEMP_UPR register. */
#define XSYSMONPSU_ALRM_TEMP_UPR_RSTVAL   0x00000000U

/** Bit shift of the ALRM_TEMP_UPR register value. */
#define XSYSMONPSU_ALRM_TEMP_UPR_SHIFT   0U
/** Bit width of the ALRM_TEMP_UPR register value. */
#define XSYSMONPSU_ALRM_TEMP_UPR_WIDTH   16U
/** Bit mask of the ALRM_TEMP_UPR register. */
#define XSYSMONPSU_ALRM_TEMP_UPR_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup1Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP1_UPR register. */
#define XSYSMONPSU_ALRM_SUP1_UPR_OFFSET   0x00000144U
/** Reset value of the ALRM_SUP1_UPR register. */
#define XSYSMONPSU_ALRM_SUP1_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP1_UPR. */
#define XSYSMONPSU_ALRM_SUP1_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP1_UPR. */
#define XSYSMONPSU_ALRM_SUP1_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP1_UPR. */
#define XSYSMONPSU_ALRM_SUP1_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup2Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP2_UPR register. */
#define XSYSMONPSU_ALRM_SUP2_UPR_OFFSET   0x00000148U
/** Reset value of the ALRM_SUP2_UPR register. */
#define XSYSMONPSU_ALRM_SUP2_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP2_UPR. */
#define XSYSMONPSU_ALRM_SUP2_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP2_UPR. */
#define XSYSMONPSU_ALRM_SUP2_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP2_UPR. */
#define XSYSMONPSU_ALRM_SUP2_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmOtUpr register definitions
 * @{
 */
/** Offset of the ALRM_OT_UPR register. */
#define XSYSMONPSU_ALRM_OT_UPR_OFFSET   0x0000014CU
/** Reset value of the ALRM_OT_UPR register. */
#define XSYSMONPSU_ALRM_OT_UPR_RSTVAL   0x00000000U

/** Bit shift of the TEMP field in ALRM_OT_UPR. */
#define XSYSMONPSU_ALRM_OT_UPR_TEMP_SHIFT   4U
/** Bit width of the TEMP field in ALRM_OT_UPR. */
#define XSYSMONPSU_ALRM_OT_UPR_TEMP_WIDTH   12U
/** Bit mask of the TEMP field in ALRM_OT_UPR. */
#define XSYSMONPSU_ALRM_OT_UPR_TEMP_MASK    0x00000fffU

/**
 * Value written to the 4 LSBs of the OT upper threshold to enable the
 * over-temperature default value override.
 */
#define XSYSMONPSU_ALRM_OT_UPR_TEMP_OVERRIDE   0x3U
/** @} */

/**
 * @name XSysmonPsuAlrmTempLwr register definitions
 * @{
 */
/** Offset of the ALRM_TEMP_LWR register. */
#define XSYSMONPSU_ALRM_TEMP_LWR_OFFSET   0x00000150U
/** Reset value of the ALRM_TEMP_LWR register. */
#define XSYSMONPSU_ALRM_TEMP_LWR_RSTVAL   0x00000000U

/** Bit shift of the ALRM_TEMP_LWR register value. */
#define XSYSMONPSU_ALRM_TEMP_LWR_SHIFT   1U
/** Bit width of the ALRM_TEMP_LWR register value. */
#define XSYSMONPSU_ALRM_TEMP_LWR_WIDTH   15U
/** Bit mask of the ALRM_TEMP_LWR register. */
#define XSYSMONPSU_ALRM_TEMP_LWR_MASK    0x0000fffeU

/** Bit shift of the TSHLD_MDE field in ALRM_TEMP_LWR. */
#define XSYSMONPSU_ALRM_TEMP_LWR_TSHLD_MDE_SHIFT   0U
/** Bit width of the TSHLD_MDE field in ALRM_TEMP_LWR. */
#define XSYSMONPSU_ALRM_TEMP_LWR_TSHLD_MDE_WIDTH   1U
/** Bit mask of the TSHLD_MDE field in ALRM_TEMP_LWR. */
#define XSYSMONPSU_ALRM_TEMP_LWR_TSHLD_MDE_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuAlrmSup1Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP1_LWR register. */
#define XSYSMONPSU_ALRM_SUP1_LWR_OFFSET   0x00000154U
/** Reset value of the ALRM_SUP1_LWR register. */
#define XSYSMONPSU_ALRM_SUP1_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP1_LWR. */
#define XSYSMONPSU_ALRM_SUP1_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP1_LWR. */
#define XSYSMONPSU_ALRM_SUP1_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP1_LWR. */
#define XSYSMONPSU_ALRM_SUP1_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup2Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP2_LWR register. */
#define XSYSMONPSU_ALRM_SUP2_LWR_OFFSET   0x00000158U
/** Reset value of the ALRM_SUP2_LWR register. */
#define XSYSMONPSU_ALRM_SUP2_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP2_LWR. */
#define XSYSMONPSU_ALRM_SUP2_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP2_LWR. */
#define XSYSMONPSU_ALRM_SUP2_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP2_LWR. */
#define XSYSMONPSU_ALRM_SUP2_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmOtLwr register definitions
 * @{
 */
/** Offset of the ALRM_OT_LWR register. */
#define XSYSMONPSU_ALRM_OT_LWR_OFFSET   0x0000015CU
/** Reset value of the ALRM_OT_LWR register. */
#define XSYSMONPSU_ALRM_OT_LWR_RSTVAL   0x00000000U

/** Bit shift of the TEMP field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TEMP_SHIFT   1U
/** Bit width of the TEMP field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TEMP_WIDTH   15U
/** Bit mask of the TEMP field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TEMP_MASK    0x0000fffeU

/** Bit shift of the TSHLD_MDE field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TSHLD_MDE_SHIFT   0U
/** Bit width of the TSHLD_MDE field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TSHLD_MDE_WIDTH   1U
/** Bit mask of the TSHLD_MDE field in ALRM_OT_LWR. */
#define XSYSMONPSU_ALRM_OT_LWR_TSHLD_MDE_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuAlrmSup3Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP3_UPR register. */
#define XSYSMONPSU_ALRM_SUP3_UPR_OFFSET   0x00000160U
/** Reset value of the ALRM_SUP3_UPR register. */
#define XSYSMONPSU_ALRM_SUP3_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP3_UPR. */
#define XSYSMONPSU_ALRM_SUP3_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP3_UPR. */
#define XSYSMONPSU_ALRM_SUP3_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP3_UPR. */
#define XSYSMONPSU_ALRM_SUP3_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup4Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP4_UPR register. */
#define XSYSMONPSU_ALRM_SUP4_UPR_OFFSET   0x00000164U
/** Reset value of the ALRM_SUP4_UPR register. */
#define XSYSMONPSU_ALRM_SUP4_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP4_UPR. */
#define XSYSMONPSU_ALRM_SUP4_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP4_UPR. */
#define XSYSMONPSU_ALRM_SUP4_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP4_UPR. */
#define XSYSMONPSU_ALRM_SUP4_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup5Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP5_UPR register. */
#define XSYSMONPSU_ALRM_SUP5_UPR_OFFSET   0x00000168U
/** Reset value of the ALRM_SUP5_UPR register. */
#define XSYSMONPSU_ALRM_SUP5_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP5_UPR. */
#define XSYSMONPSU_ALRM_SUP5_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP5_UPR. */
#define XSYSMONPSU_ALRM_SUP5_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP5_UPR. */
#define XSYSMONPSU_ALRM_SUP5_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup6Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP6_UPR register. */
#define XSYSMONPSU_ALRM_SUP6_UPR_OFFSET   0x0000016CU
/** Reset value of the ALRM_SUP6_UPR register. */
#define XSYSMONPSU_ALRM_SUP6_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP6_UPR. */
#define XSYSMONPSU_ALRM_SUP6_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP6_UPR. */
#define XSYSMONPSU_ALRM_SUP6_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP6_UPR. */
#define XSYSMONPSU_ALRM_SUP6_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup3Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP3_LWR register. */
#define XSYSMONPSU_ALRM_SUP3_LWR_OFFSET   0x00000170U
/** Reset value of the ALRM_SUP3_LWR register. */
#define XSYSMONPSU_ALRM_SUP3_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP3_LWR. */
#define XSYSMONPSU_ALRM_SUP3_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP3_LWR. */
#define XSYSMONPSU_ALRM_SUP3_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP3_LWR. */
#define XSYSMONPSU_ALRM_SUP3_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup4Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP4_LWR register. */
#define XSYSMONPSU_ALRM_SUP4_LWR_OFFSET   0x00000174U
/** Reset value of the ALRM_SUP4_LWR register. */
#define XSYSMONPSU_ALRM_SUP4_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP4_LWR. */
#define XSYSMONPSU_ALRM_SUP4_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP4_LWR. */
#define XSYSMONPSU_ALRM_SUP4_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP4_LWR. */
#define XSYSMONPSU_ALRM_SUP4_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup5Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP5_LWR register. */
#define XSYSMONPSU_ALRM_SUP5_LWR_OFFSET   0x00000178U
/** Reset value of the ALRM_SUP5_LWR register. */
#define XSYSMONPSU_ALRM_SUP5_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP5_LWR. */
#define XSYSMONPSU_ALRM_SUP5_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP5_LWR. */
#define XSYSMONPSU_ALRM_SUP5_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP5_LWR. */
#define XSYSMONPSU_ALRM_SUP5_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup6Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP6_LWR register. */
#define XSYSMONPSU_ALRM_SUP6_LWR_OFFSET   0x0000017CU
/** Reset value of the ALRM_SUP6_LWR register. */
#define XSYSMONPSU_ALRM_SUP6_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP6_LWR. */
#define XSYSMONPSU_ALRM_SUP6_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP6_LWR. */
#define XSYSMONPSU_ALRM_SUP6_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP6_LWR. */
#define XSYSMONPSU_ALRM_SUP6_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup7Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP7_UPR register. */
#define XSYSMONPSU_ALRM_SUP7_UPR_OFFSET   0x00000180U
/** Reset value of the ALRM_SUP7_UPR register. */
#define XSYSMONPSU_ALRM_SUP7_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP7_UPR. */
#define XSYSMONPSU_ALRM_SUP7_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP7_UPR. */
#define XSYSMONPSU_ALRM_SUP7_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP7_UPR. */
#define XSYSMONPSU_ALRM_SUP7_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup8Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP8_UPR register. */
#define XSYSMONPSU_ALRM_SUP8_UPR_OFFSET   0x00000184U
/** Reset value of the ALRM_SUP8_UPR register. */
#define XSYSMONPSU_ALRM_SUP8_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP8_UPR. */
#define XSYSMONPSU_ALRM_SUP8_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP8_UPR. */
#define XSYSMONPSU_ALRM_SUP8_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP8_UPR. */
#define XSYSMONPSU_ALRM_SUP8_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup9Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP9_UPR register. */
#define XSYSMONPSU_ALRM_SUP9_UPR_OFFSET   0x00000188U
/** Reset value of the ALRM_SUP9_UPR register. */
#define XSYSMONPSU_ALRM_SUP9_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP9_UPR. */
#define XSYSMONPSU_ALRM_SUP9_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP9_UPR. */
#define XSYSMONPSU_ALRM_SUP9_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP9_UPR. */
#define XSYSMONPSU_ALRM_SUP9_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup10Upr register definitions
 * @{
 */
/** Offset of the ALRM_SUP10_UPR register. */
#define XSYSMONPSU_ALRM_SUP10_UPR_OFFSET   0x0000018CU
/** Reset value of the ALRM_SUP10_UPR register. */
#define XSYSMONPSU_ALRM_SUP10_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP10_UPR. */
#define XSYSMONPSU_ALRM_SUP10_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP10_UPR. */
#define XSYSMONPSU_ALRM_SUP10_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP10_UPR. */
#define XSYSMONPSU_ALRM_SUP10_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmVccamsUpr register definitions
 * @{
 */
/** Offset of the ALRM_VCCAMS_UPR register. */
#define XSYSMONPSU_ALRM_VCCAMS_UPR_OFFSET   0x00000190U
/** Reset value of the ALRM_VCCAMS_UPR register. */
#define XSYSMONPSU_ALRM_VCCAMS_UPR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_VCCAMS_UPR. */
#define XSYSMONPSU_ALRM_VCCAMS_UPR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_VCCAMS_UPR. */
#define XSYSMONPSU_ALRM_VCCAMS_UPR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_VCCAMS_UPR. */
#define XSYSMONPSU_ALRM_VCCAMS_UPR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmTremoteUpr register definitions
 * @{
 */
/** Offset of the ALRM_TREMOTE_UPR register. */
#define XSYSMONPSU_ALRM_TREMOTE_UPR_OFFSET   0x00000194U
/** Reset value of the ALRM_TREMOTE_UPR register. */
#define XSYSMONPSU_ALRM_TREMOTE_UPR_RSTVAL   0x00000000U

/** Bit shift of the TEMP field in ALRM_TREMOTE_UPR. */
#define XSYSMONPSU_ALRM_TREMOTE_UPR_TEMP_SHIFT   0U
/** Bit width of the TEMP field in ALRM_TREMOTE_UPR. */
#define XSYSMONPSU_ALRM_TREMOTE_UPR_TEMP_WIDTH   16U
/** Bit mask of the TEMP field in ALRM_TREMOTE_UPR. */
#define XSYSMONPSU_ALRM_TREMOTE_UPR_TEMP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup7Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP7_LWR register. */
#define XSYSMONPSU_ALRM_SUP7_LWR_OFFSET   0x000001A0U
/** Reset value of the ALRM_SUP7_LWR register. */
#define XSYSMONPSU_ALRM_SUP7_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP7_LWR. */
#define XSYSMONPSU_ALRM_SUP7_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP7_LWR. */
#define XSYSMONPSU_ALRM_SUP7_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP7_LWR. */
#define XSYSMONPSU_ALRM_SUP7_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup8Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP8_LWR register. */
#define XSYSMONPSU_ALRM_SUP8_LWR_OFFSET   0x000001A4U
/** Reset value of the ALRM_SUP8_LWR register. */
#define XSYSMONPSU_ALRM_SUP8_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP8_LWR. */
#define XSYSMONPSU_ALRM_SUP8_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP8_LWR. */
#define XSYSMONPSU_ALRM_SUP8_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP8_LWR. */
#define XSYSMONPSU_ALRM_SUP8_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup9Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP9_LWR register. */
#define XSYSMONPSU_ALRM_SUP9_LWR_OFFSET   0x000001A8U
/** Reset value of the ALRM_SUP9_LWR register. */
#define XSYSMONPSU_ALRM_SUP9_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP9_LWR. */
#define XSYSMONPSU_ALRM_SUP9_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP9_LWR. */
#define XSYSMONPSU_ALRM_SUP9_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP9_LWR. */
#define XSYSMONPSU_ALRM_SUP9_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmSup10Lwr register definitions
 * @{
 */
/** Offset of the ALRM_SUP10_LWR register. */
#define XSYSMONPSU_ALRM_SUP10_LWR_OFFSET   0x000001ACU
/** Reset value of the ALRM_SUP10_LWR register. */
#define XSYSMONPSU_ALRM_SUP10_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_SUP10_LWR. */
#define XSYSMONPSU_ALRM_SUP10_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_SUP10_LWR. */
#define XSYSMONPSU_ALRM_SUP10_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_SUP10_LWR. */
#define XSYSMONPSU_ALRM_SUP10_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmVccamsLwr register definitions
 * @{
 */
/** Offset of the ALRM_VCCAMS_LWR register. */
#define XSYSMONPSU_ALRM_VCCAMS_LWR_OFFSET   0x000001B0U
/** Reset value of the ALRM_VCCAMS_LWR register. */
#define XSYSMONPSU_ALRM_VCCAMS_LWR_RSTVAL   0x00000000U

/** Bit shift of the SUP field in ALRM_VCCAMS_LWR. */
#define XSYSMONPSU_ALRM_VCCAMS_LWR_SUP_SHIFT   0U
/** Bit width of the SUP field in ALRM_VCCAMS_LWR. */
#define XSYSMONPSU_ALRM_VCCAMS_LWR_SUP_WIDTH   16U
/** Bit mask of the SUP field in ALRM_VCCAMS_LWR. */
#define XSYSMONPSU_ALRM_VCCAMS_LWR_SUP_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuAlrmTremoteLwr register definitions
 * @{
 */
/** Offset of the ALRM_TREMOTE_LWR register. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_OFFSET   0x000001B4U
/** Reset value of the ALRM_TREMOTE_LWR register. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_RSTVAL   0x00000000U

/** Bit shift of the TEMP field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TEMP_SHIFT   1U
/** Bit width of the TEMP field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TEMP_WIDTH   15U
/** Bit mask of the TEMP field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TEMP_MASK    0x0000fffeU

/** Bit shift of the TSHLD_MDE field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TSHLD_MDE_SHIFT   0U
/** Bit width of the TSHLD_MDE field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TSHLD_MDE_WIDTH   1U
/** Bit mask of the TSHLD_MDE field in ALRM_TREMOTE_LWR. */
#define XSYSMONPSU_ALRM_TREMOTE_LWR_TSHLD_MDE_MASK    0x00000001U
/** @} */

/**
 * @name XSysmonPsuSeqInputMde2 register definitions
 * @{
 */
/** Offset of the SEQ_INPUT_MDE2 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE2_OFFSET	0x000001E0U
/** Reset value of the SEQ_INPUT_MDE2 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE2_RSTVAL	0x00000000U

/** Bit shift of the SEQ_INPUT_MDE2 register value. */
#define XSYSMONPSU_SEQ_INPUT_MDE2_SHIFT		0U
/** Bit mask of the SEQ_INPUT_MDE2 register. */
#define XSYSMONPSU_SEQ_INPUT_MDE2_MASK		0x0000003FU
/** @} */

/**
 * @name XSysmonPsuSeqAcq2 register definitions
 * @{
 */
/** Offset of the SEQ_ACQ2 register. */
#define XSYSMONPSU_SEQ_ACQ2_OFFSET		0x000001E4U
/** Reset value of the SEQ_ACQ2 register. */
#define XSYSMONPSU_SEQ_ACQ2_RSTVAL		0x00000000U

/** Bit shift of the SEQ_ACQ2 register value. */
#define XSYSMONPSU_SEQ_ACQ2_SHIFT		0U
/** Bit mask of the SEQ_ACQ2 register. */
#define XSYSMONPSU_SEQ_ACQ2_MASK		0x0000003FU
/** @} */

/**
 * @name XSysmonPsuSup7 register definitions
 * @{
 */
/** Offset of the SUP7 register. */
#define XSYSMONPSU_SUP7_OFFSET   0x00000200U
/** Reset value of the SUP7 register. */
#define XSYSMONPSU_SUP7_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP7. */
#define XSYSMONPSU_SUP7_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP7. */
#define XSYSMONPSU_SUP7_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP7. */
#define XSYSMONPSU_SUP7_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup8 register definitions
 * @{
 */
/** Offset of the SUP8 register. */
#define XSYSMONPSU_SUP8_OFFSET   0x00000204U
/** Reset value of the SUP8 register. */
#define XSYSMONPSU_SUP8_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP8. */
#define XSYSMONPSU_SUP8_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP8. */
#define XSYSMONPSU_SUP8_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP8. */
#define XSYSMONPSU_SUP8_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup9 register definitions
 * @{
 */
/** Offset of the SUP9 register. */
#define XSYSMONPSU_SUP9_OFFSET   0x00000208U
/** Reset value of the SUP9 register. */
#define XSYSMONPSU_SUP9_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP9. */
#define XSYSMONPSU_SUP9_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP9. */
#define XSYSMONPSU_SUP9_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP9. */
#define XSYSMONPSU_SUP9_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuSup10 register definitions
 * @{
 */
/** Offset of the SUP10 register. */
#define XSYSMONPSU_SUP10_OFFSET   0x0000020CU
/** Reset value of the SUP10 register. */
#define XSYSMONPSU_SUP10_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in SUP10. */
#define XSYSMONPSU_SUP10_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in SUP10. */
#define XSYSMONPSU_SUP10_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in SUP10. */
#define XSYSMONPSU_SUP10_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuVccams register definitions
 * @{
 */
/** Offset of the VCCAMS register. */
#define XSYSMONPSU_VCCAMS_OFFSET   0x00000210U
/** Reset value of the VCCAMS register. */
#define XSYSMONPSU_VCCAMS_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in VCCAMS. */
#define XSYSMONPSU_VCCAMS_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in VCCAMS. */
#define XSYSMONPSU_VCCAMS_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in VCCAMS. */
#define XSYSMONPSU_VCCAMS_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuTempRemte register definitions
 * @{
 */
/** Offset of the TEMP_REMTE register. */
#define XSYSMONPSU_TEMP_REMTE_OFFSET   0x00000214U
/** Reset value of the TEMP_REMTE register. */
#define XSYSMONPSU_TEMP_REMTE_RSTVAL   0x00000000U

/** Bit shift of the TEMP_REMTE register value. */
#define XSYSMONPSU_TEMP_REMTE_SHIFT   0U
/** Bit width of the TEMP_REMTE register value. */
#define XSYSMONPSU_TEMP_REMTE_WIDTH   16U
/** Bit mask of the TEMP_REMTE register. */
#define XSYSMONPSU_TEMP_REMTE_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup7 register definitions
 * @{
 */
/** Offset of the MAX_SUP7 register. */
#define XSYSMONPSU_MAX_SUP7_OFFSET   0x00000280U
/** Reset value of the MAX_SUP7 register. */
#define XSYSMONPSU_MAX_SUP7_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in MAX_SUP7. */
#define XSYSMONPSU_MAX_SUP7_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MAX_SUP7. */
#define XSYSMONPSU_MAX_SUP7_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MAX_SUP7. */
#define XSYSMONPSU_MAX_SUP7_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup8 register definitions
 * @{
 */
/** Offset of the MAX_SUP8 register. */
#define XSYSMONPSU_MAX_SUP8_OFFSET   0x00000284U
/** Reset value of the MAX_SUP8 register. */
#define XSYSMONPSU_MAX_SUP8_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in MAX_SUP8. */
#define XSYSMONPSU_MAX_SUP8_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MAX_SUP8. */
#define XSYSMONPSU_MAX_SUP8_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MAX_SUP8. */
#define XSYSMONPSU_MAX_SUP8_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup9 register definitions
 * @{
 */
/** Offset of the MAX_SUP9 register. */
#define XSYSMONPSU_MAX_SUP9_OFFSET   0x00000288U
/** Reset value of the MAX_SUP9 register. */
#define XSYSMONPSU_MAX_SUP9_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in MAX_SUP9. */
#define XSYSMONPSU_MAX_SUP9_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MAX_SUP9. */
#define XSYSMONPSU_MAX_SUP9_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MAX_SUP9. */
#define XSYSMONPSU_MAX_SUP9_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxSup10 register definitions
 * @{
 */
/** Offset of the MAX_SUP10 register. */
#define XSYSMONPSU_MAX_SUP10_OFFSET   0x0000028CU
/** Reset value of the MAX_SUP10 register. */
#define XSYSMONPSU_MAX_SUP10_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in MAX_SUP10. */
#define XSYSMONPSU_MAX_SUP10_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MAX_SUP10. */
#define XSYSMONPSU_MAX_SUP10_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MAX_SUP10. */
#define XSYSMONPSU_MAX_SUP10_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxVccams register definitions
 * @{
 */
/** Offset of the MAX_VCCAMS register. */
#define XSYSMONPSU_MAX_VCCAMS_OFFSET   0x00000290U
/** Reset value of the MAX_VCCAMS register. */
#define XSYSMONPSU_MAX_VCCAMS_RSTVAL   0x00000000U

/** Bit shift of the SUP_VAL field in MAX_VCCAMS. */
#define XSYSMONPSU_MAX_VCCAMS_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MAX_VCCAMS. */
#define XSYSMONPSU_MAX_VCCAMS_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MAX_VCCAMS. */
#define XSYSMONPSU_MAX_VCCAMS_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMaxTempRemte register definitions
 * @{
 */
/** Offset of the MAX_TEMP_REMTE register. */
#define XSYSMONPSU_MAX_TEMP_REMTE_OFFSET   0x00000294U
/** Reset value of the MAX_TEMP_REMTE register. */
#define XSYSMONPSU_MAX_TEMP_REMTE_RSTVAL   0x00000000U

/** Bit shift of the MAX_TEMP_REMTE register value. */
#define XSYSMONPSU_MAX_TEMP_REMTE_SHIFT   0U
/** Bit width of the MAX_TEMP_REMTE register value. */
#define XSYSMONPSU_MAX_TEMP_REMTE_WIDTH   16U
/** Bit mask of the MAX_TEMP_REMTE register. */
#define XSYSMONPSU_MAX_TEMP_REMTE_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup7 register definitions
 * @{
 */
/** Offset of the MIN_SUP7 register. */
#define XSYSMONPSU_MIN_SUP7_OFFSET   0x000002A0U
/** Reset value of the MIN_SUP7 register. */
#define XSYSMONPSU_MIN_SUP7_RSTVAL   0x0000ffffU

/** Bit shift of the SUP_VAL field in MIN_SUP7. */
#define XSYSMONPSU_MIN_SUP7_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MIN_SUP7. */
#define XSYSMONPSU_MIN_SUP7_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MIN_SUP7. */
#define XSYSMONPSU_MIN_SUP7_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup8 register definitions
 * @{
 */
/** Offset of the MIN_SUP8 register. */
#define XSYSMONPSU_MIN_SUP8_OFFSET   0x000002A4U
/** Reset value of the MIN_SUP8 register. */
#define XSYSMONPSU_MIN_SUP8_RSTVAL   0x0000ffffU

/** Bit shift of the SUP_VAL field in MIN_SUP8. */
#define XSYSMONPSU_MIN_SUP8_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MIN_SUP8. */
#define XSYSMONPSU_MIN_SUP8_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MIN_SUP8. */
#define XSYSMONPSU_MIN_SUP8_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup9 register definitions
 * @{
 */
/** Offset of the MIN_SUP9 register. */
#define XSYSMONPSU_MIN_SUP9_OFFSET   0x000002A8U
/** Reset value of the MIN_SUP9 register. */
#define XSYSMONPSU_MIN_SUP9_RSTVAL   0x0000ffffU

/** Bit shift of the SUP_VAL field in MIN_SUP9. */
#define XSYSMONPSU_MIN_SUP9_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MIN_SUP9. */
#define XSYSMONPSU_MIN_SUP9_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MIN_SUP9. */
#define XSYSMONPSU_MIN_SUP9_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinSup10 register definitions
 * @{
 */
/** Offset of the MIN_SUP10 register. */
#define XSYSMONPSU_MIN_SUP10_OFFSET   0x000002ACU
/** Reset value of the MIN_SUP10 register. */
#define XSYSMONPSU_MIN_SUP10_RSTVAL   0x0000ffffU

/** Bit shift of the SUP_VAL field in MIN_SUP10. */
#define XSYSMONPSU_MIN_SUP10_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MIN_SUP10. */
#define XSYSMONPSU_MIN_SUP10_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MIN_SUP10. */
#define XSYSMONPSU_MIN_SUP10_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinVccams register definitions
 * @{
 */
/** Offset of the MIN_VCCAMS register. */
#define XSYSMONPSU_MIN_VCCAMS_OFFSET   0x000002B0U
/** Reset value of the MIN_VCCAMS register. */
#define XSYSMONPSU_MIN_VCCAMS_RSTVAL   0x0000ffffU

/** Bit shift of the SUP_VAL field in MIN_VCCAMS. */
#define XSYSMONPSU_MIN_VCCAMS_SUP_VAL_SHIFT   0U
/** Bit width of the SUP_VAL field in MIN_VCCAMS. */
#define XSYSMONPSU_MIN_VCCAMS_SUP_VAL_WIDTH   16U
/** Bit mask of the SUP_VAL field in MIN_VCCAMS. */
#define XSYSMONPSU_MIN_VCCAMS_SUP_VAL_MASK    0x0000ffffU
/** @} */

/**
 * @name XSysmonPsuMinTempRemte register definitions
 * @{
 */
/** Offset of the MIN_TEMP_REMTE register. */
#define XSYSMONPSU_MIN_TEMP_REMTE_OFFSET   0x000002B4U
/** Reset value of the MIN_TEMP_REMTE register. */
#define XSYSMONPSU_MIN_TEMP_REMTE_RSTVAL   0x0000ffffU

/** Bit shift of the MIN_TEMP_REMTE register value. */
#define XSYSMONPSU_MIN_TEMP_REMTE_SHIFT   0U
/** Bit width of the MIN_TEMP_REMTE register value. */
#define XSYSMONPSU_MIN_TEMP_REMTE_WIDTH   16U
/** Bit mask of the MIN_TEMP_REMTE register. */
#define XSYSMONPSU_MIN_TEMP_REMTE_MASK    0x0000ffffU

/** @} */

#define XSYSMONPSU_CSU_BASEADDR		0xFFCA0000U /**< CSU base address. */
/** Offset of the PCAP status register in the CSU. */
#define PCAP_STATUS_OFFSET		0x00003010U
/** Bit mask of the PL configuration reset bit in PCAP status. */
#define PL_CFG_RESET_MASK		0x00000040U
/** Bit shift of the PL configuration reset bit in PCAP status. */
#define PL_CFG_RESET_SHIFT		6U

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]   RegisterAddr Register address in the address
*              space of the SYSMONPSU device.
*
* @return      The 32-bit value of the register.
*
*****************************************************************************/
static inline u32 XSysmonPsu_ReadReg(UINTPTR RegisterAddr) {
    return Xil_In32(RegisterAddr);
}

/****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]   RegisterAddr Register address in the address
*              space of the SYSMONPSU device.
* @param[in]   Data 32-bit value to write to the register.
*
*****************************************************************************/
static inline void XSysmonPsu_WriteReg(UINTPTR RegisterAddr, u32 Data) {
    Xil_Out32(RegisterAddr, Data);
}

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* XSYSMONPSU_HW_H__ */
