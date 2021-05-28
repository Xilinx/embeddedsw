/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CPMDOMAIN_H_
#define XPM_CPMDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CPM domain node class.
 */
typedef struct XPm_CpmDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 CpmSlcrBaseAddr; /**< CPM SLCR Base Address */
	u32 CpmSlcrSecureBaseAddr; /**< CPM SLCR Secure base address */
	u32 CpmPcsrBaseAddr; /**< CPM PCSR Base address */
	u32 CpmCrCpmBaseAddr; /**< CPM CRCPM Base address */
} XPm_CpmDomain;

/*****************************************************************************/
/**
 * @brief This function unlocks the CPM PCSR registers.
 *
 *****************************************************************************/
static inline void XPmCpmDomain_UnlockPcsr(u32 BaseAddress)
{
	PmOut32(BaseAddress + CPM_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This function locks the CPM PCSR registers.
 *
 *****************************************************************************/
static inline void XPmCpmDomain_LockPcsr(u32 BaseAddress)
{
	PmOut32(BaseAddress + CPM_PCSR_LOCK_OFFSET, PCSR_LOCK_VAL);
}

/************************** Function Prototypes ******************************/
XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressesCnt);
#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_CPMDOMAIN_H_ */
