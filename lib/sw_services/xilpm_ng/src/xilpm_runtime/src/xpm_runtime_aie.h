/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_AIE_H_
#define XPM_RUNTIME_AIE_H_

#include "xstatus.h"
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum UC Private data memory size is 16KB */
#define AIE2PS_UC_PRIVATE_DM_MAX_SIZE   (16U * 1024U)

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
	u16 Len;	/* Operation struct length + handshake data length */
	u32 Offset;     /* Offset in the handshake region */
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
XStatus XPmAie_Operations(u32 Part, u32 Ops, u32 Arg3);
#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_AIE_H*/
