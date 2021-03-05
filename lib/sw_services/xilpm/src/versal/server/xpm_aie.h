/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_AIE_H_
#define XPM_AIE_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_ipi.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECC_SCRUB_ENABLE	(1U)
#define ECC_SCRUB_DISABLE	(0U)

/**
 * AI Engine domain node class.
 */
typedef struct XPm_AieDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_AieDomain;

/*****************************************************************************/
/**
 * @brief This function unlocks the AIE PCSR registers.
 *
 *****************************************************************************/
static inline void XPmAieDomain_UnlockPcsr(u32 BaseAddress)
{
	u32 NpiPcsrLockReg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
	PmOut32(NpiPcsrLockReg, NPI_PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This function locks the AIE PCSR registers.
 *
 *****************************************************************************/
static inline void XPmAieDomain_LockPcsr(u32 BaseAddress)
{
	u32 NpiPcsrLockReg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
	PmOut32(NpiPcsrLockReg, 0x00000000U);
}

/************************** Function Prototypes ******************************/
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
