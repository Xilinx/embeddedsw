/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file psm_global.h
*
* This file contains PSM Global definitions used by PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  rp   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_PMC_GLOBAL_H_
#define XPSMFW_PMC_GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup pmc_global_module PMC Global register definitions
 * @{
 */
/**
 * Register: PMC_GLOBAL_PWR_SUPPLY_STATUS
 */
#define PMC_GLOBAL_PWR_SUPPLY_STATUS    ( ( PMC_GLOBAL_BASEADDR ) + 0X0000010CU )

/**
 * @name PMC Global Power Supply Status register operations
 * @ingroup pmc_global_module
 * @{
 */
/**
 * PMC Global Power Supply Status register operation
 */
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_SHIFT   7U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_RAM_MASK    0X00000080U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_SHIFT   6U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PL_MASK    0X00000040U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_SHIFT   5U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_MASK    0X00000020U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_SHIFT   4U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_SOC_MASK    0X00000010U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_SHIFT   3U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_LPD_MASK    0X00000008U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_SHIFT   2U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_FPD_MASK    0X00000004U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PMC_SHIFT   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PMC_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCINT_PMC_MASK    0X00000002U

#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_PMC_SHIFT   0U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_PMC_WIDTH   1U
#define PMC_GLOBAL_PWR_SUPPLY_STATUS_VCCAUX_PMC_MASK    0X00000001U
/** @} */

/**
 * @name PMC Global Domain Isolation Control register operations
 * @ingroup pmc_global_module
 * @{
 */
/**
 * PMC Global Domain Isolation Control register operation
 */
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_SHIFT   18U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_MASK    0X00040000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_SHIFT   17U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_MASK    0X00020000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_SHIFT   16U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_MASK    0X00010000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_SHIFT   15U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_MASK    0X00008000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_SHIFT   14U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK    0X00004000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_SHIFT   13U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK    0X00002000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_SHIFT   12U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_MASK    0X00001000U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_SHIFT   11U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK    0X00000800U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_SHIFT   10U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK    0X00000400U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_SHIFT   9U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_MASK    0X00000200U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_SHIFT   8U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_MASK    0X00000100U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_SHIFT   7U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_MASK    0X00000080U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_SHIFT   6U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_MASK    0X00000040U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_SHIFT   5U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_MASK    0X00000020U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_SHIFT   4U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_MASK    0X00000010U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_SHIFT   3U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_MASK    0X00000008U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_SHIFT   2U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_WIDTH   1U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_SHIFT   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_WIDTH   1U

#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_SHIFT   0U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_WIDTH   1U
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK    0X00000001U
/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_PMC_GLOBAL_H_ */
