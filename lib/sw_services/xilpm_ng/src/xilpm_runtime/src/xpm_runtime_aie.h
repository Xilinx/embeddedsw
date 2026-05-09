/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_AIE_H_
#define XPM_RUNTIME_AIE_H_

#include "xstatus.h"
#include "xil_types.h"
#include "xpm_aiedevice.h"
#include "xpm_subsystem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum UC Private data memory size is 16KB */
#define AIE2PS_UC_PRIVATE_DM_MAX_SIZE   (16U * 1024U)

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

struct XPm_AieCtrlPktTlastErr {
	u16 Type;	/* Operation Type */
	u16 Len;	/* Operation struct length */
	u16 State;	/* Value to set CTRL_PKT_TLAST_ERR bit in module clock control register */
} __attribute__ ((aligned(4)));

XStatus XPmAie_Operations(u32 SubsystemId, u32 Size, u32 HighAddr, u32 LowAddr);
XStatus XPmAieDevice_UpdateClockDiv(const u32 Divider);
XStatus XPmAieDevice_QueryDivider(u32 *Response);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_AIE_H*/
