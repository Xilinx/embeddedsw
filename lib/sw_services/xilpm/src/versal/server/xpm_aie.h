/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECC_SCRUB_ENABLE	(1U)
#define ECC_SCRUB_DISABLE	(0U)

#define AIE_GENV1		(1U)
#define AIE_GENV2		(2U)

typedef struct XPm_AieArray XPm_AieArray;
typedef struct XPm_AieDomain XPm_AieDomain;
typedef struct XPm_AieDomainOpHooks XPm_AieDomainOpHooks;

#define ARR_GENV(ArrWord)	((u16)((ArrWord) & 0xFFFFU))
#define ARR_ROWS(ArrWord)	((u16)((ArrWord) & 0xFFFFU))
#define ARR_COLS(ArrWord)	((u16)((ArrWord) >> 16U) & 0xFFFFU)
#define ARR_AIEROWS(ArrWord)	((u16)((ArrWord) >> 16U) & 0xFFFFU)
#define ARR_MEMROWS(ArrWord)	((u16)((ArrWord) >> 8U) & 0xFFU)
#define ARR_SHMROWS(ArrWord)	((u16)((ArrWord) & 0xFFU))

#define AIE_START_COL_MASK		(0x0000FFFFU)
#define AIE_NUM_COL_MASK		(0xFFFF0000U)

/**
 * AIE Operations.
 */
#define AIE_OPS_COL_RST			BIT(0U)
#define AIE_OPS_SHIM_RST		BIT(1U)
#define AIE_OPS_ENB_COL_CLK_BUFF	BIT(2U)
#define AIE_OPS_ZEROIZATION		BIT(3U)
#define AIE_OPS_DIS_COL_CLK_BUFF	BIT(4U)
#define AIE_OPS_ENB_AXI_MM_ERR_EVENT	BIT(5U)
#define AIE_OPS_SET_L2_CTRL_NPI_INTR	BIT(6U)
#define AIE_OPS_MAX			(0x7FU)

/**
 * AI Engine array.
 */
struct XPm_AieArray {
	u64 NocAddress;
	u16 GenVersion;
	u16 NumCols;
	u16 NumRows;
	u16 StartCol;
	u16 StartRow;
	u16 NumAieRows;
	u16 NumMemRows;
	u16 NumShimRows;
};

struct XPm_AieDomainOpHooks {
	XStatus (*PostScanClearHook)(const XPm_AieDomain *AieDomain, u32 BaseAddress);
	XStatus (*PreBisrHook)(const XPm_AieDomain *AieDomain, u32 BaseAddress);
};

/**
 * AI Engine domain node class.
 */
struct XPm_AieDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	XPm_AieArray Array;	/**< AIE device instance */
	XPm_AieDomainOpHooks Hooks;	/**< Hooks for AIE domain ops */
};

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
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs);
XStatus Aie_Operations(u32 Part, u32 Ops);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
