/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_AIE_H_
#define XPM_AIE_H_

#include "xpm_pldevice.h"
#include "xpm_powerdomain.h"

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

/* TODO: Move to xpm_err.h when code is re-factored for versal_aiepg2 */
/* AIE ERRORS */
#define XPM_ERR_AIE_OPS_COL_RST                 (2120L) /**< Error while Column Reset */
#define XPM_ERR_AIE_OPS_SHIM_RST                (2121L) /**< Error while Shim Reset */
#define XPM_ERR_AIE_OPS_ENB_COL_CLK_BUFF        (2122L) /**< Error while Enabling of column clock buffer */
#define XPM_ERR_AIE_OPS_ZEROIZATION             (2123L) /**< Error while Zeroization */
#define XPM_ERR_AIE_OPS_DIS_COL_CLK_BUFF        (2124L) /**< Error while Disabling of column clock buffer */
#define XPM_ERR_AIE_OPS_ENB_AXI_MM_ERR_EVENT    (2125L) /**< Error while Enabling of AXI-MM error events */
#define XPM_ERR_AIE_OPS_SET_L2_CTRL_NPI_INTR    (2126L) /**< Error while Setting of L2 controller NPI INTR */
#define XPM_ERR_AIE_OPS_UC_ZEROIZATION		(2127L) /**< Error during UC zeroization */
#define XPM_ERR_AIE_OPS_NMU_CONFIG		(2128L) /**< Error during nmu config */

/**
 * AIE Run time Operations
 */
enum XPmAieOperations {
        AIE_OPS_MIN = 0U,
        AIE_OPS_COL_RST = 1U,
        AIE_OPS_SHIM_RST = 2U,
        AIE_OPS_UC_ZEROIZATION = 3U,
        AIE_OPS_ENB_COL_CLK_BUFF = 4U,
        AIE_OPS_HANDSHAKE = 5U,
        AIE_OPS_CLR_HW_ERR_STS = 6U,
        AIE_OPS_START_NUM_COL = 7U,
        AIE_OPS_ALL_MEM_ZEROIZATION = 8U,
        AIE_OPS_AXIMM_ISOLATION = 9U,
        AIE_OPS_NMU_CONFIG = 10U,
        AIE_OPS_DIS_MEM_PRIV = 11U,
        AIE_OPS_DIS_MEM_INTERLEAVE = 12U,
        AIE_OPS_ENB_UC_DMA_PAUSE = 13U,
        AIE_OPS_ENB_NOC_DMA_PAUSE = 14U,
        AIE_OPS_SET_ECC_SCRUB_PERIOD = 15U,
        AIE_OPS_DIS_COL_CLK_BUFF = 16U,
        AIE_OPS_HW_ERR_INT = 17U,
        AIE_OPS_HW_ERR_MASK = 18U,
        AIE_OPS_ENB_MEM_PRIV = 19U,
        AIE_OPS_ENB_AXI_MM_ERR_EVENT = 32U,     /* Backward compatibility for AIE1/AIE2 */
        AIE_OPS_SET_L2_CTRL_NPI_INTR = 64U,     /* Backward compatibility for AIE1/AIE2 */
        AIE_OPS_PROG_MEM_ZEROIZATION = 128U,    /* Backward compatibility for AIE1/AIE2 */
        AIE_OPS_DATA_MEM_ZEROIZATION = 256U,    /* Backward compatibility for AIE1/AIE2 */
        AIE_OPS_MEM_TILE_ZEROIZATION = 512U,    /* Backward compatibility for AIE1/AIE2 */
};
/** @} */

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

/**
 * AIE base device node
 */
typedef struct XPm_AieNode {
	XPm_Device Device;              /**< Device: Base class */
	u32 DefaultClockDiv;            /**< Default AIE clock divider at boot */
} XPm_AieNode;

struct XPm_AieOpStartNumCol {
        u16 Type;         /* Operation Type */
        u16 Len;          /* Operation struct length */
        u16 StartCol;    /* Start Column */
        u16 NumCol;      /* Number of Columns */
} __attribute__ ((aligned(4)));

struct XPm_AieOpL2CtrlIrq {
        u16 Type;	/* Operation Type */
        u16 Len;	/* Operation struct length */
        u16 Irq;	/* Value to be written to the L2 intr controller register. */
} __attribute__ ((aligned(4)));

/*
 * This structure is used for operation types that needs constant values to be
 * written. Eg. XILINX_AIE_OPS_ZEROISATION
 */
struct XPm_AieTypeLen {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length */
} __attribute__ ((aligned(4)));

struct XPm_AieOpHwErrSts {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length */
	u16 Val;
} __attribute__ ((aligned(4)));

struct XPm_AieOpUcZeroisation {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length*/
	u16 Flag;	/* Value to be written to the uc zeroization register */
} __attribute__ ((aligned(4)));

struct XPm_AieOpHandShake {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length*/
	u32 HighAddr;	/* physical address of the buffer that has handshake data */
	u32 LowAddr;
} __attribute__ ((aligned(4)));

struct XPm_AieOpNmuSwitch {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length */
	u16 C0Route;	/* Value to be written to column 0 nmu switch register */
	u16 C1Route;	/* Value to be written to column 1 nmu switch register */
} __attribute__ ((aligned(4)));

struct XPm_AieOpAximmIso {
	u16 Type;      /* Operation Type */
	u16 Len;       /* Operation struct length */
	u16 Traffic;   /* Value to be written to the aximm isolation register */
} __attribute__ ((aligned(4)));

struct XPm_AieOpEccScrubPeriod {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length */
	u16 ScrubPeriod; /* Value to be written to the ecc scrub period register */
} __attribute__ ((aligned(4)));

/************************** Function Prototypes ******************************/
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent, const u32 *Args, u32 NumArgs);
XStatus Aie_Operations(u32 Part, u32 Ops, u32 Arg3);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
