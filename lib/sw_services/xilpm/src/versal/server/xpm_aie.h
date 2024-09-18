/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
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
#define ARR_COLS(ArrWord)	((u16)(((ArrWord) >> 16U) & 0xFFFFU))
#define ARR_AIEROWS(ArrWord)	((u16)(((ArrWord) >> 16U) & 0xFFFFU))
#define ARR_MEMROWS(ArrWord)	((u16)(((ArrWord) >> 8U) & 0xFFU))
#define ARR_SHMROWS(ArrWord)	((u16)((ArrWord) & 0xFFU))
#define ARR_TROWOFF(ArrWord)	((u8)(((ArrWord) >> 16U) & 0xFFU))
#define ARR_RCOLOFF(ArrWord)	((u8)(((ArrWord) >> 8U) & 0xFFU))
#define ARR_LCOLOFF(ArrWord)	((u8)((ArrWord) & 0xFFU))

#define AIE_START_COL_MASK		(0x0000FFFFU)
#define AIE_NUM_COL_MASK		(0xFFFF0000U)

/* Type of Tile */
#define AIE_TILE_TYPE_UNDEFINED		(0U)
#define AIE_TILE_TYPE_SHIMPL		(1U)
#define AIE_TILE_TYPE_SHIMNOC		(2U)

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

/**
 * AIE base device node
 */
typedef struct XPm_AieNode {
	XPm_Device Device;		/**< Device: Base class */
	u32 DefaultClockDiv;		/**< Default AIE clock divider at boot */
} XPm_AieNode;

/************************** Function Prototypes ******************************/
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs);
XStatus Aie_Operations(u32 Part, u32 Ops);
XStatus AddAieDeviceNode(void);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
