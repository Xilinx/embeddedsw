/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_AIE_H_
#define XPM_AIE_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_AieArray XPm_AieArray;
typedef struct XPm_AieDomain XPm_AieDomain;
typedef struct XPm_AieDomainOpHooks XPm_AieDomainOpHooks;

/**
 * AI Engine array.
 */
struct XPm_AieArray {
	u64 NocAddress;		/**< NoC Address of AIE */
	u16 GenVersion;		/**< AIE Gen: 1, 2 etc. */
	u16 NumCols;		/**< Total number of columns */
	u16 NumRows;		/**< Total number of rows (excluding shim) */
	u16 NumColsAdjusted;	/**< Total number of columns (adjusted after offset) */
	u16 NumRowsAdjusted;	/**< Total number of rows (adjusted after offset) */
	u16 NumAieRows;		/**< Total number of AieTile rows */
	u16 NumMemRows;		/**< Total number of MemTile rows */
	u16 NumShimRows;	/**< Total number of Shim rows */
	u16 StartCol;		/**< Start col */
	u16 StartRow;		/**< Start row */
	u8 LColOffset;		/**< Offset for start col (i.e. left col offset) */
	u8 RColOffset;		/**< Offset for end col (i.e. right col offset) */
	u8 TRowOffset;		/**< Offset for end row (i.e. top row offset) */
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

/************************** Function Prototypes ******************************/
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
