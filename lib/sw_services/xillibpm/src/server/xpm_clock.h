/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_CLOCK_H_
#define XPM_CLOCK_H_

#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	XPM_NODEIDX_CLK_OUT_MIN	XPM_NODEIDX_CLK_PLL_MAX
#define	XPM_NODEIDX_CLK_OUT_MAX	XPM_NODEIDX_CLK_OUTCLK_MAX
#define XPM_NODEIDX_CLK_REF_MIN	XPM_NODEIDX_CLK_OUTCLK_MAX
#define XPM_NODEIDX_CLK_REF_MAX	XPM_NODEIDX_CLK_MAX

#define REFCLOCK_MASK \
	(((XPM_NODECLASS_CLOCK & NODE_CLASS_MASK_BITS) << NODE_CLASS_SHIFT) | \
	((XPM_NODESUBCL_CLOCK_REF & NODE_SUBCLASS_MASK_BITS) << NODE_SUBCLASS_SHIFT))
#define OUTCLOCK_MASK \
	(((XPM_NODECLASS_CLOCK & NODE_CLASS_MASK_BITS) << NODE_CLASS_SHIFT) | \
	((XPM_NODESUBCL_CLOCK_OUT & NODE_SUBCLASS_MASK_BITS) << NODE_SUBCLASS_SHIFT))
#define ISOUTCLK(id) \
	( (((id & OUTCLOCK_MASK) == OUTCLOCK_MASK) && \
	((id & NODE_INDEX_MASK_BITS) > XPM_NODEIDX_CLK_OUT_MIN) && \
	((id & NODE_INDEX_MASK_BITS) < XPM_NODEIDX_CLK_OUT_MAX) ) ? 1 : 0)
#define ISREFCLK(id) \
	( (((id & REFCLOCK_MASK) == REFCLOCK_MASK) && \
	((id & NODE_INDEX_MASK_BITS) > XPM_NODEIDX_CLK_REF_MIN) && \
	((id & NODE_INDEX_MASK_BITS) < XPM_NODEIDX_CLK_REF_MAX) ) ? 1 : 0)

/* Topology types */
#define	TOPOLOGY_GENERIC_PLL		1
#define	TOPOLOGY_NOC_PLL		2
#define TOPOLOGY_GENERIC_MUX_DIV	3
#define TOPOLOGY_GENERIC_MUX_GATE	4
#define TOPOLOGY_GENERIC_DIV_GATE	5
#define TOPOLOGY_GENERIC_MUX_DIV_GATE_1	6
#define TOPOLOGY_GENERIC_MUX_DIV_GATE_2	7
#define TOPOLOGY_CUSTOM			8
#define MAX_TOPOLOGY			9

#define MAX_MUX_PARENTS		8
#define MAX_NAME_BYTES		16

/**
 * The topology node class.	 This is the class to represent each node
 * in clock topology. It can be mux/div/gate/fixed factor.
 */
struct XPm_ClkTopologyNode {
	uint8_t Type;
	uint16_t Clkflags;
	uint16_t Typeflags;
	u32 Reg;
	union {uint8_t Shift; uint8_t Mult;}Param1;
	union {uint8_t Width; uint8_t Div;}Param2;
};

typedef struct XPm_ClkTopology {
	uint16_t Id;
	uint16_t NumNodes;
	int32_t MuxSources[MAX_MUX_PARENTS];
	struct XPm_ClkTopologyNode(*Nodes)[];
}XPm_ClkTopology;

typedef struct XPm_ClockNode XPm_ClockNode;
typedef struct XPm_ClockHandle XPm_ClockHandle;

/**
 * The clock class.	 This is the base class for all the clocks.
 */
struct XPm_ClockNode {
	XPm_Node Node;
	char Name[MAX_NAME_BYTES];
	u16 NumParents;
	u8 Flags;
	u32 ParentId;
	XPm_ClockHandle *ClkHandles; /**< Pointer to the clock/device pairs */
	u32 UseCount;
	XPm_Power *PwrDomain;
};

/**
 * XPm_ClockHandle - This models clock/device pair.
 */
struct XPm_ClockHandle {
	XPm_ClockNode *Clock; /**< Clock used by device */
	struct XPm_Device *Device; /**< Device which uses the clock */
	XPm_ClockHandle *NextClock; /**< Next handle of same device */
	XPm_ClockHandle *NextDevice; /**< Next handle of same clock */
};

typedef struct XPm_OutClockNode {
	XPm_ClockNode ClkNode;
	XPm_ClkTopology Topology;
}XPm_OutClockNode;

/* Common topology definitions */

enum XPm_ClockSubnodeType {
	TYPE_INVALID,
	TYPE_MUX,
	TYPE_PLL,
	TYPE_FIXEDFACTOR,
	TYPE_DIV1,
	TYPE_DIV2,
	TYPE_GATE,
	TYPE_MAX,
};

/* Clock states: */
#define	XPM_CLK_STATE_OFF		0U
#define XPM_CLK_STATE_REQUESTED	1U			//For all clocks with/without gate
#define XPM_CLK_STATE_ON			2U			//For clocks with gate

/* Peripheral Clocks */
#define PERIPH_MUX_SHIFT			0
#define PERIPH_MUX_WIDTH			3
#define PERIPH_DIV_SHIFT			8
#define PERIPH_DIV_WIDTH			10
#define PERIPH_GATE1_SHIFT			25
#define PERIPH_GATE2_SHIFT			24
#define PERIPH_GATE_WIDTH			1

/* Common Flags */
#define NA_TYPE_FLAGS				0U
#define CLK_SET_RATE_GATE		BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE		BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT		BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED		BIT(3) /* do not gate even if unused */
#define CLK_IS_BASIC			BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT	BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE	BIT(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES	BIT(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE		BIT(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL			BIT(11) /* do not gate, ever */

/* Type Flags */
#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)

/************************** Function Prototypes ******************************/
XStatus XPmClock_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			 u8 NumCustomNodes, u8 NumParents, u32 PowerDomainId,
			 u8 ClkFlags);
XStatus XPmClock_AddClkName(u32 Id, char *Name);
XStatus XPmClock_AddSubNode(u32 Id, u32 Type, u32 ControlReg, u8 Param1, u8 Param2, u32 Flags);
XStatus XPmClock_AddParent(u32 Id, u32 *Parents, u32 NumParents);
XPm_ClockNode* XPmClock_GetById(u32 ClockId);
XPm_ClockNode* XPmClock_GetByIdx(u32 ClockIdx);
XStatus XPmClock_SetById(u32 ClockId, XPm_ClockNode *Clk);
XStatus XPmClock_Request(XPm_ClockHandle *ClkHandle);
XStatus XPmClock_Release(XPm_ClockHandle *ClkHandle);
XStatus XPmClock_SetGate(XPm_OutClockNode *Clk, u32 Enable);
XStatus XPmClock_SetParent(XPm_OutClockNode *Clk, u32 ParentIdx);
XStatus XPmClock_SetDivider(XPm_OutClockNode *Clk, u32 Divider);
XStatus XPmClock_GetClockData(XPm_OutClockNode *Clk, u32 NodeType, u32 *Value);
XStatus XPmClock_QueryName(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryTopology(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryFFParams(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryMuxSources(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryAttributes(u32 ClockIndex, u32 *Resp);
XStatus XPmClock_GetNumClocks(u32 *Resp);
XStatus XPmClock_CheckPermissions(u32 SubsystemIdx, u32 ClockId);
XStatus XPmClock_GetMaxDivisor(u32 ClockId, u32 DivType, u32 *Resp);

#ifdef __cplusplus
}
#endif

#endif /* XPM_CLOCK_H_ */
