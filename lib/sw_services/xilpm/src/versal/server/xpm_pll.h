/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PLL_H_
#define XPM_PLL_H_

#include "xpm_clock.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The PLL class.  This is the class to represent pll nodes.
 */
typedef struct XPm_PllParam {
	uint8_t Shift;
	uint8_t Width;
} XPm_PllParam;

struct XPm_PllTopology {
	uint16_t Id;
	XPm_PllParam ConfigParams[PM_PLL_PARAM_MAX];
	uint8_t ResetShift;
	uint8_t BypassShift;
	uint8_t LockShift;
	uint8_t StableShift;
	uint8_t PllReg3Offset;
};

typedef struct XPm_PllClockNode XPm_PllClockNode;

/**
 * PmPllContext - Structure for saving context of PLL registers.
 *              Contains variable to store default content of:
 * @ctrl        Control register
 * @cfg         Configuration register
 * @frac        Fractional control register
 * @flag        Indicates context saved or not
 *
 * Note: context of the PLL is saved when PM framework suspends a PLL (when
 * no node requires PLL to be locked).
 */
typedef struct PmPllContext {
	u32 Ctrl;
	u32 Cfg;
	u32 Frac;
	u8 Flag;
} PmPllContext;

struct XPm_PllClockNode {
	XPm_ClockNode ClkNode;
	u32 StatusReg;
	u32 ConfigReg;
	u32 FracConfigReg;
	u8 PllMode;
	struct XPm_PllTopology *Topology;
	PmPllContext Context;
};

#define ISPLL(id)	((NODECLASS(id) == (u32)XPM_NODECLASS_CLOCK) && \
			 (NODESUBCLASS(id) == (u32)XPM_NODESUBCL_CLOCK_PLL) && \
			 (NODEINDEX(id) < (u32)XPM_NODEIDX_CLK_MAX))

#define RESET_SHIFT		0U
#define BYPASS_SHIFT		3U
#define GEN_LOCK_SHIFT		0U
#define NPLL_LOCK_SHIFT		1U
#define GEN_STABLE_SHIFT	2U
#define NPLL_STABLE_SHIFT	3U
#define GEN_REG3_OFFSET		0x68U
#define NPLL_REG3_OFFSET	0xA8U
#define PPLL_REG3_OFFSET	0x78U

#define PLL_REG3_CP_RES_H_SHIFT  20U
#define PLL_REG3_CP_RES_H_WIDTH  2U

#define PLLPARAMS {	\
	[PM_PLL_PARAM_ID_DIV2] = {	\
		.Shift = 16U,		\
		.Width = 2U,		\
	},				\
	[PM_PLL_PARAM_ID_FBDIV] = {	\
		.Shift = 8U,		\
		.Width = 7U,		\
	},				\
	[PM_PLL_PARAM_ID_DATA] = {	\
		.Shift = 0U,		\
		.Width = 16U,		\
	},				\
	[PM_PLL_PARAM_ID_PRE_SRC] = {	\
		.Shift = 20U,		\
		.Width = 3U,		\
	},				\
	[PM_PLL_PARAM_ID_POST_SRC] = {	\
		.Shift = 24U,		\
		.Width = 3U,		\
	},				\
	[PM_PLL_PARAM_ID_LOCK_DLY] = {	\
		.Shift = 25U,		\
		.Width = 7U,		\
	},				\
	[PM_PLL_PARAM_ID_LOCK_CNT] = {	\
		.Shift = 13U,		\
		.Width = 10U,		\
	},				\
	[PM_PLL_PARAM_ID_LFHF] = {	\
		.Shift = 10U,		\
		.Width = 2U,		\
	},				\
	[PM_PLL_PARAM_ID_CP] = {	\
		.Shift = 5U,		\
		.Width = 4U,		\
	},				\
	[PM_PLL_PARAM_ID_RES] = {	\
		.Shift = 0U,		\
		.Width = 4U,		\
	},				\
}

/* TBD: TImeout value need to be defined as per spec */
#define PLL_LOCK_TIMEOUT		0x10000U

/* PLL Flags */
#define PLL_RESET_ASSERT			1U
#define PLL_RESET_RELEASE			2U
#define PLL_RESET_PULSE	(PLL_RESET_ASSERT | PLL_RESET_RELEASE)
#define PM_PLL_CONTEXT_SAVED	1U

/* PLL states: */
#define PM_PLL_STATE_RESET	0U
#define PM_PLL_STATE_LOCKED	1U
#define PM_PLL_STATE_SUSPENDED	2U

#define PLL_FRAC_CFG_ENABLED_MASK	(0x80000000U)

/************************** Function Prototypes ******************************/
XStatus XPmClockPll_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			    const u16 *Offsets, u32 PowerDomainId, u8 ClkFlags);
XStatus XPmClockPll_AddParent(u32 Id, const u32 *Parents, u8 NumParents);
XStatus XPmClockPll_Request(u32 PllId);
XStatus XPmClockPll_Release(u32 PllId);
XStatus XPmClockPll_SetMode(XPm_PllClockNode *Pll, u32 Mode);
XStatus XPmClockPll_GetMode(XPm_PllClockNode *Pll, u32 *Mode);
XStatus XPmClockPll_Suspend(XPm_PllClockNode *Pll);
XStatus XPmClockPll_Resume(XPm_PllClockNode *Pll);
XStatus XPmClockPll_Reset(XPm_PllClockNode *Pll, uint8_t Flags);
XStatus XPmClockPll_SetParam(const XPm_PllClockNode *Pll, u32 Param,u32 Value);
XStatus XPmClockPll_GetParam(const XPm_PllClockNode *Pll, u32 Param,u32 *Val);
XStatus XPmClockPll_QueryMuxSources(u32 Id, u32 Index, u32 *Resp);
XStatus XPmClockPll_GetWakeupLatency(const u32 Id, u32 *Latency);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PLL_H_ */
