/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_NPDOMAIN_H_
#define XPM_NPDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The NOC power domain node class.
 */
typedef struct XPm_NpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u8 BisrDataCopied;
} XPm_NpDomain;

/*****************************************************************************/
/**
 * @brief This function unlocks the NPI PCSR registers.
 *
 *****************************************************************************/
static inline void XPmNpDomain_UnlockNpiPcsr(u32 BaseAddr)
{
	PmOut32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This function locks the NPI PCSR registers.
 *
 *****************************************************************************/
static inline void XPmNpDomain_LockNpiPcsr(u32 BaseAddr)
{
	PmOut32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_LOCK_VAL);
}

/************************** Function Prototypes ******************************/
XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);
XStatus XPmNpDomain_MemIcInit(u32 DeviceId, u32 BaseAddr);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NPDOMAIN_H_ */
