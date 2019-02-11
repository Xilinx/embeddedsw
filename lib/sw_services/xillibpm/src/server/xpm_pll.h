/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPM_PLL_H_
#define XPM_PLL_H_

#include "xpm_clock.h"
#include "xillibpm_defs.h"

/**
 * The PLL class.  This is the class to represent pll nodes.
 */
typedef struct XPm_PllParam {
	uint8_t Shift;
	uint8_t Width;
} XPm_PllParam;

struct XPm_PllTopology {
	uint16_t Id;
	XPm_PllParam ConfigParams[PLL_PARAM_MAX];
	uint8_t ResetShift;
	uint8_t BypassShift;
	uint8_t LockShift;
	uint8_t StableShift;
};

typedef struct XPm_PllClockNode XPm_PllClockNode;

struct XPm_PllClockNode {
	XPm_ClockNode ClkNode;
	u32 StatusReg;
	u32 ConfigReg;
	u32 FracConfigReg;
	u8 PllMode;
	struct XPm_PllTopology *Topology;
};


#define XPM_NODEIDX_CLK_PLL_MIN		XPM_NODEIDX_CLK_MIN
#define XPM_NODEIDX_CLK_PLL_MAX		XPM_NODEIDX_CLK_PLL_MAX
#define PLLCLOCK_MASK \
	(((XPM_NODECLASS_CLOCK & NODE_CLASS_MASK_BITS) << NODE_CLASS_SHIFT) | \
	((XPM_NODESUBCL_CLOCK_PLL & NODE_SUBCLASS_MASK_BITS) << NODE_SUBCLASS_SHIFT))
#define ISPLL(id) \
	( (((id & PLLCLOCK_MASK) == PLLCLOCK_MASK) && \
	((id & NODE_INDEX_MASK_BITS) > XPM_NODEIDX_CLK_PLL_MIN) && \
	((id & NODE_INDEX_MASK_BITS) < XPM_NODEIDX_CLK_PLL_MAX)) ? 1 : 0)

#define RESET_SHIFT		0U
#define BYPASS_SHIFT		3U
#define GEN_LOCK_SHIFT		0U
#define NPLL_LOCK_SHIFT		1U
#define GEN_STABLE_SHIFT	2U
#define NPLL_STABLE_SHIFT	3U

#define PLLPARAMS {	\
	[PLL_PARAM_ID_CLKOUTDIV] = {	\
		.Shift = 16U,		\
		.Width = 2U,		\
	},				\
	[PLL_PARAM_ID_FBDIV] = {	\
		.Shift = 8U,		\
		.Width = 7U,		\
	},				\
	[PLL_PARAM_ID_FRAC_DATA] = {	\
		.Shift = 0U,		\
		.Width = 16U,		\
	},				\
	[PLL_PARAM_ID_PRE_SRC] = {	\
		.Shift = 20U,		\
		.Width = 3U,		\
	},				\
	[PLL_PARAM_ID_POST_SRC] = {	\
		.Shift = 24U,		\
		.Width = 3U,		\
	},				\
	[PLL_PARAM_ID_LOCK_DLY] = {	\
		.Shift = 25U,		\
		.Width = 7U,		\
	},				\
	[PLL_PARAM_ID_LOCK_CNT] = {	\
		.Shift = 13U,		\
		.Width = 10U,		\
	},				\
	[PLL_PARAM_ID_LFHF] = {		\
		.Shift = 10U,		\
		.Width = 2U,		\
	},				\
	[PLL_PARAM_ID_CP] = {		\
		.Shift = 5U,		\
		.Width = 4U,		\
	},				\
	[PLL_PARAM_ID_RES] = {		\
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

/* PLL states: */
#define PM_PLL_STATE_RESET	0U
#define PM_PLL_STATE_LOCKED	1U
#define PM_PLL_STATE_SUSPENDED	2U

/* PLL modes: */
#define PM_PLL_MODE_RESET		0U
#define PM_PLL_MODE_INTEGER		1U
#define PM_PLL_MODE_FRACTIONAL		2U

#define PLL_FRAC_CFG_ENABLED_MASK	(0x80000000U)

/************************** Function Prototypes ******************************/
XStatus XPmClockPll_AddNode(u32 Id, u32 ControlReg, u8 TopologyType, u16 *Offsets);
XStatus XPmClockPll_AddParent(u32 Id, u32 *Parents, u32 NumParents);
XStatus XPmClockPll_Request(u32 PllId);
XStatus XPmClockPll_Release(u32 PllId);
XStatus XPmClockPll_SetMode(XPm_PllClockNode *Pll, u32 Mode);
XStatus XPmClockPll_GetMode(XPm_PllClockNode *Pll, u32 *Mode);
XStatus XPmClockPll_Suspend(XPm_PllClockNode *Pll);
XStatus XPmClockPll_Resume(XPm_PllClockNode *Pll);
XStatus XPmClockPll_Reset(XPm_PllClockNode *Pll, uint8_t Flags);
XStatus XPmClockPll_SetParam(XPm_PllClockNode *Pll, u32 Param,u32 Value);
XStatus XPmClockPll_GetParam(XPm_PllClockNode *Pll, u32 Param,u32 *Val);

/** @} */
#endif /* XPM_PLL_H_ */
