
/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xil_hw.h
*
* This is the header file which contains definitions for the hardware
* registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sk   02/20/2025 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XIL_HW_H
#define XIL_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/**@cond xil_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*
 * PMC_GLOBAL Base Address
 */
#define PMC_GLOBAL_BASEADDR     (0XF1110000U)

/*
 * Register: PMC_GLOBAL_DOMAIN_ISO_CNTRL
 */
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL    (PMC_GLOBAL_BASEADDR + 0X00010000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_VCCRAM_MASK		(0x00040000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCRAM_SOC_MASK		(0x00020000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_VCCAUX_SOC_MASK		(0x00010000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PL_SOC_MASK			(0x00008000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK		(0x00004000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK		(0x00002000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_MASK			(0x00001000U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_TEST_MASK		(0x00000800U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK		(0X00000400U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_MASK		(0x00000200U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_LPD_DFX_MASK		(0x00000100U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_SOC_MASK		(0x00000080U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_MASK			(0x00000040U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_PL_TEST_MASK		(0x00000020U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_MASK		(0x00000010U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_LPD_CPM_DFX_MASK		(0x00000008U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK		(0x00000004U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK			(0x00000002U)
#define PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_TEST_MASK		(0x00000001U)
/*
 * Definitions required from pmc_tap.h
 */
#define PMC_TAP_BASEADDR		(0XF11A0000U)
#define PMC_TAP_IDCODE		(PMC_TAP_BASEADDR + 0X00000000U)
#define PMC_TAP_VERSION		(PMC_TAP_BASEADDR + 0X00000004U)
#define PMC_TAP_VERSION_PMC_VERSION_MASK		(0X000000FFU)
/*
 * Definitions required from crp.h
 */
#define CRP_BASEADDR		(0XF1260000U)
#define CRP_RESET_REASON		(CRP_BASEADDR + 0X00000220U)
#define CRP_RST_NONPS		(CRP_BASEADDR + 0X00000320U)
#define CRP_RST_NONPS_NPI_RESET_MASK		(0X10U)
/*
 * Register: CRP_RST_PS
 */
#define CRP_RST_PS		(CRP_BASEADDR + 0x0000031CU)
/*
 * Register: PMC_IOU_SLCR
 */
#define PMC_IOU_SLCR_BASEADDR      (0XF1060000U)

#define IPI_BASEADDR		(0xEB300000U)

#define PCSR_UNLOCK_VAL		(0xF9E8D7C6U)
#define PCSR_LOCK_VAL		(0U)

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XIL_HW_H */
