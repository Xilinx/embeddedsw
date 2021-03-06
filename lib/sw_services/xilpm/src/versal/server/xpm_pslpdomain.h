/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSLPDOMAIN_H_
#define XPM_PSLPDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PMC_ANALOG Base Address
 */
#define PMC_ANALOG_BASEADDR		0XF1160000U

/**
 * Register: PMC_ANALOG_OD_MBIST_RST
 */
#define PMC_ANALOG_OD_MBIST_RST			( ( PMC_ANALOG_BASEADDR ) + 0X00020100U )
#define PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_RST_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_PG_EN
 */
#define PMC_ANALOG_OD_MBIST_PG_EN		( ( PMC_ANALOG_BASEADDR ) + 0X00020104U )
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_SETUP
 */
#define PMC_ANALOG_OD_MBIST_SETUP		( ( PMC_ANALOG_BASEADDR ) + 0X00020108U )
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_DONE
 */
#define PMC_ANALOG_OD_MBIST_DONE		( ( PMC_ANALOG_BASEADDR ) + 0X00020110U )
#define PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_DONE_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_GOOD
 */
#define PMC_ANALOG_OD_MBIST_GOOD		( ( PMC_ANALOG_BASEADDR ) + 0X00020114U )
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_TRIGGER
 */
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER			( ( PMC_ANALOG_BASEADDR ) + 0X00020120U )
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK		0X00000010U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK		0X00000100U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_DONE
 */
#define PMC_ANALOG_SCAN_CLEAR_DONE			( ( PMC_ANALOG_BASEADDR ) + 0X00020128U )
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK		0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK		0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK		0X00000010U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_PASS
 */
#define PMC_ANALOG_SCAN_CLEAR_PASS			( ( PMC_ANALOG_BASEADDR ) + 0X0002012CU )
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK		0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK		0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK		0X00000010U

/**
 * Register: PMC_ANALOG_LBIST_ENABLE
 */
#define PMC_ANALOG_LBIST_ENABLE			( ( PMC_ANALOG_BASEADDR ) + 0X00020200U )
#define PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_ENABLE_LPD_MASK	0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_RST_N
 */
#define PMC_ANALOG_LBIST_RST_N			( ( PMC_ANALOG_BASEADDR ) + 0X00020204U )
#define PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_RST_N_LPD_MASK		0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_ISOLATION_EN
 */
#define PMC_ANALOG_LBIST_ISOLATION_EN			( ( PMC_ANALOG_BASEADDR ) + 0X00020208U )
#define PMC_ANALOG_LBIST_ISOLATION_EN_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_ISOLATION_EN_LPD_MASK		0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_DONE
 */
#define PMC_ANALOG_LBIST_DONE			( ( PMC_ANALOG_BASEADDR ) + 0X00020210U )
#define PMC_ANALOG_LBIST_DONE_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_DONE_LPD_MASK		0X00000001U

#define LPD_BISR_DONE		BIT(0)
#define LPD_BISR_DATA_COPIED	BIT(1)

typedef struct XPm_PsLpDomain XPm_PsLpDomain;

/**
 * The PS low power domain node class.
 */
struct XPm_PsLpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 LpdIouSlcrBaseAddr; /**< LPD IOU SLCR Base address */
	u32 LpdSlcrBaseAddr; /**< LPD SLCR Base address */
	u32 LpdSlcrSecureBaseAddr; /**< LPD SLCR Secure base address */
	u8 LpdBisrFlags;
};

/*****************************************************************************/
/**
 * @brief This function unlocks the XRAM PCSR registers.
 *
 *****************************************************************************/
static inline void XPmPsLpDomain_UnlockPcsr(u32 BaseAddr)
{
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This function locks the XRAM PCSR registers.
 *
 *****************************************************************************/
static inline void XPmPsLpDomain_LockPcsr(u32 BaseAddr)
{
	PmOut32(BaseAddr + XRAM_SLCR_PCSR_LOCK_OFFSET, 0x0U);
}

/************************** Function Prototypes ******************************/
XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressesCnt);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSLPDOMAIN_H_ */
