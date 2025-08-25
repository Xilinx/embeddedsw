/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_AIE_H_
#define XPM_AIE_H_

#include "xpm_powerdomain.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif
#define AIE_GENV2                (2U)

/* AI Engine dimensions */
#define ARR_GENV(ArrWord)      ((u16)((ArrWord) & 0xFFFFU))
#define ARR_ROWS(ArrWord)      ((u16)((ArrWord) & 0xFFFFU))
#define ARR_COLS(ArrWord)      ((u16)(((ArrWord) >> 16U) & 0xFFFFU))
#define ARR_AIEROWS(ArrWord)   ((u16)(((ArrWord) >> 16U) & 0xFFFFU))
#define ARR_MEMROWS(ArrWord)   ((u16)(((ArrWord) >> 8U) & 0xFFU))
#define ARR_SHMROWS(ArrWord)   ((u16)((ArrWord) & 0xFFU))
#define ARR_TROWOFF(ArrWord)   ((u8)(((ArrWord) >> 16U) & 0xFFU))
#define ARR_RCOLOFF(ArrWord)   ((u8)(((ArrWord) >> 8U) & 0xFFU))
#define ARR_LCOLOFF(ArrWord)   ((u8)((ArrWord) & 0xFFU))

typedef struct XPm_AieArray XPm_AieArray;
typedef struct XPm_AieDomain XPm_AieDomain;

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


/**
 * AI Engine domain node class.
 */
struct XPm_AieDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	XPm_AieArray Array;	/**< AIE device instance */
};

/**
 * AIE base device node
 */
typedef struct XPm_AieNode {
	XPm_Device Device;              /**< Device: Base class */
} XPm_AieNode;

/************************** Function Prototypes ******************************/
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs);
XStatus XPmAie_AddPeriphNode(const u32 *Args, u32 PowerId);
#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
