/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CLOCK_H_
#define XPM_CLOCK_H_

#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISOUTCLK(id)	((NODECLASS(id) == (u32)XPM_NODECLASS_CLOCK) && \
			 (NODESUBCLASS(id) == (u32)XPM_NODESUBCL_CLOCK_OUT) && \
			 (NODEINDEX(id) < (u32)XPM_NODEIDX_CLK_MAX))
#define ISREFCLK(id)	((NODECLASS(id) == (u32)XPM_NODECLASS_CLOCK) && \
			 (NODESUBCLASS(id) == (u32)XPM_NODESUBCL_CLOCK_REF) && \
			 (NODEINDEX(id) < (u32)XPM_NODEIDX_CLK_MAX))

/* Topology types */
#define	TOPOLOGY_GENERIC_PLL		1U
#define	TOPOLOGY_NOC_PLL		2U
#define TOPOLOGY_GENERIC_MUX_DIV	3U
#define TOPOLOGY_GENERIC_MUX_GATE	4U
#define TOPOLOGY_GENERIC_DIV_GATE	5U
#define TOPOLOGY_GENERIC_MUX_DIV_GATE_1	6U
#define TOPOLOGY_GENERIC_MUX_DIV_GATE_2	7U
#define TOPOLOGY_CUSTOM			8U
#define MAX_TOPOLOGY			9U

#define MAX_MUX_PARENTS		8U
#define MAX_NAME_BYTES		16U

/* Clock Flags */
#define CLK_FLAG_READ_ONLY		(1U << 0U)

/**
 * The topology node class.	 This is the class to represent each node
 * in clock topology. It can be mux/div/gate/fixed factor.
 */
struct XPm_ClkTopologyNode {
	u32 Reg;
	uint16_t Clkflags;
	uint16_t Typeflags;
	union {uint8_t Shift; uint8_t Mult;}Param1;
	union {uint8_t Width; uint8_t Div;}Param2;
	uint8_t Type;
};

typedef struct XPm_ClkTopology {
	struct XPm_ClkTopologyNode(*Nodes)[];
	uint8_t Id;
	uint8_t NumNodes;
	u16 MuxSources[MAX_MUX_PARENTS]; /**< Clock index of mux sources */
}XPm_ClkTopology;

typedef struct XPm_ClockNode XPm_ClockNode;
typedef struct XPm_ClockHandle XPm_ClockHandle;

/**
 * The clock class.	 This is the base class for all the clocks.
 */
struct XPm_ClockNode {
	XPm_Node Node;
	char Name[MAX_NAME_BYTES];
	u16 ParentIdx;
	u8 NumParents;
	u8 Flags;
	u8 UseCount;
	XPm_ClockHandle *ClkHandles; /**< Pointer to the clock/device pairs */
	XPm_Power *PwrDomain;
	u32 ClkRate;
};

/**
 * XPm_ClockHandle - This models clock/device pair.
 */
struct XPm_ClockHandle {
	XPm_ClockNode *Clock; /**< Clock used by device */
	struct XPm_DeviceNode *Device; /**< Device which uses the clock */
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
#define NA_TYPE_FLAGS			0U
#define CLK_SET_RATE_GATE		BIT16(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE		BIT16(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT		BIT16(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED		BIT16(3) /* do not gate even if unused */
						 /* BIT(4) unused */
						 /* BIT(5) unused */
#define CLK_GET_RATE_NOCACHE		BIT16(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT	BIT16(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE	BIT16(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES		BIT16(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE		BIT16(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL			BIT16(11) /* do not gate, ever */

/* Type Flags for divider clock */
#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT16(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)

/* Type Flags for mux clock */
#define CLK_MUX_INDEX_ONE               BIT(0)
#define CLK_MUX_INDEX_BIT               BIT(1)
#define CLK_MUX_HIWORD_MASK             BIT(2)
#define CLK_MUX_READ_ONLY               BIT16(3)
#define CLK_MUX_ROUND_CLOSEST           BIT(4)
#define CLK_MUX_BIG_ENDIAN              BIT(5)

/************************** Function Prototypes ******************************/
XStatus XPmClock_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			 u8 NumCustomNodes, u8 NumParents, u32 PowerDomainId,
			 u8 ClkFlags);
XStatus XPmClock_AddClkName(u32 Id, const char *Name);
XStatus XPmClock_AddSubNode(u32 Id, u32 Type, u32 ControlReg, u8 Param1, u8 Param2, u32 Flags);
XStatus XPmClock_AddParent(u32 Id, const u32 *Parents, u8 NumParents);
void XPmClock_SetPlClockAsReadOnly(void);
XPm_ClockNode* XPmClock_GetById(u32 ClockId);
XPm_ClockNode* XPmClock_GetByIdx(u32 ClockIdx);
XStatus XPmClock_SetById(u32 ClockId, XPm_ClockNode *Clk);
XStatus XPmClock_Request(const XPm_ClockHandle *ClkHandle);
XStatus XPmClock_Release(const XPm_ClockHandle *ClkHandle);
XStatus XPmClock_SetGate(XPm_OutClockNode *Clk, u32 Enable);
XStatus XPmClock_SetParent(XPm_OutClockNode *Clk, u32 ParentIdx);
XStatus XPmClock_SetDivider(const XPm_OutClockNode *Clk, u32 Divider);
XStatus XPmClock_GetClockData(const XPm_OutClockNode *Clk, u32 Nodetype, u32 *Value);
XStatus XPmClock_QueryName(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryTopology(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryFFParams(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryMuxSources(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryAttributes(u32 ClockIndex, u32 *Resp);
XStatus XPmClock_GetNumClocks(u32 *Resp);
XStatus XPmClock_CheckPermissions(u32 SubsystemIdx, u32 ClockId);
XStatus XPmClock_GetMaxDivisor(u32 ClockId, u32 DivType, u32 *Resp);
XStatus XPmClock_SetRate(XPm_ClockNode *Clk, const u32 ClkRate);
XStatus XPmClock_GetRate(const XPm_ClockNode *Clk, u32 *ClkRate);

#ifdef __cplusplus
}
#endif

#endif /* XPM_CLOCK_H_ */
