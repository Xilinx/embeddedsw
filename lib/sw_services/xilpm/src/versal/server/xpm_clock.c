/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_util.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_device.h"

/* Query related defines */
#define CLK_QUERY_NAME_LEN		(MAX_NAME_BYTES)
#define CLK_INIT_ENABLE_SHIFT		1U
#define CLK_TYPE_SHIFT			2U
#define CLK_NODETYPE_SHIFT		14U
#define CLK_NODESUBCLASS_SHIFT		20U
#define CLK_NODECLASS_SHIFT		26U
#define CLK_PARENTS_PAYLOAD_LEN		12U
#define CLK_TOPOLOGY_PAYLOAD_LEN	12U
#define CLK_NA_PARENT			-1
#define CLK_DUMMY_PARENT		-2
#define CLK_CLKFLAGS_SHIFT		8U
#define CLK_TYPEFLAGS_SHIFT		24U

#define CLOCK_PARENT_INVALID		0U

/* Clock Flags */
#define CLK_FLAG_READ_ONLY		(1U << 0U)

#define GENERIC_MUX						\
	{									\
		.Type = (u8)TYPE_MUX,				\
		.Param1.Shift = PERIPH_MUX_SHIFT,		\
		.Param2.Width = PERIPH_MUX_WIDTH,		\
		.Clkflags = CLK_SET_RATE_NO_REPARENT |	\
				CLK_IS_BASIC,			\
		.Typeflags = NA_TYPE_FLAGS,		\
	}

#define GENERIC_DIV						\
	{									\
		.Type = (u8)TYPE_DIV1,				\
		.Param1.Shift = PERIPH_DIV_SHIFT,		\
		.Param2.Width = PERIPH_DIV_WIDTH,		\
		.Clkflags = CLK_SET_RATE_NO_REPARENT |	\
				CLK_IS_BASIC,			\
		.Typeflags = CLK_DIVIDER_ONE_BASED |	\
				 CLK_DIVIDER_ALLOW_ZERO,		\
	}

#define GENERIC_GATE(id)				\
	{									\
		.Type = (u8)TYPE_GATE,				\
		.Param1.Shift = PERIPH_GATE##id##_SHIFT,		\
		.Param2.Width = PERIPH_GATE_WIDTH,				\
		.Clkflags = CLK_SET_RATE_PARENT |		\
				CLK_SET_RATE_GATE |		\
				CLK_IS_BASIC,			\
		.Typeflags = NA_TYPE_FLAGS,		\
	}
static struct XPm_ClkTopologyNode GenericMuxDivNodes[] = {
	GENERIC_MUX,
	GENERIC_DIV,
};

static struct XPm_ClkTopologyNode GenericMuxGate2Nodes[] = {
	GENERIC_MUX,
	GENERIC_GATE(2),
};
static struct XPm_ClkTopologyNode GenericDivGate2Nodes[] = {
	GENERIC_DIV,
	GENERIC_GATE(2),
};
static struct XPm_ClkTopologyNode GenericMuxDivGate1Nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV,
	GENERIC_GATE(1),
};

static struct XPm_ClkTopologyNode GenericMuxDivGate2Nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV,
	GENERIC_GATE(2),
};

static XPm_ClkTopology ClkTopologies[ ] = {
	 {&GenericMuxDivNodes, TOPOLOGY_GENERIC_MUX_DIV, ARRAY_SIZE(GenericMuxDivNodes), {0}},
	 {&GenericMuxGate2Nodes, TOPOLOGY_GENERIC_MUX_GATE, ARRAY_SIZE(GenericMuxGate2Nodes), {0}},
	 {&GenericDivGate2Nodes, TOPOLOGY_GENERIC_DIV_GATE, ARRAY_SIZE(GenericDivGate2Nodes), {0}},
	 {&GenericMuxDivGate1Nodes, TOPOLOGY_GENERIC_MUX_DIV_GATE_1, ARRAY_SIZE(GenericMuxDivGate1Nodes), {0}},
	 {&GenericMuxDivGate2Nodes, TOPOLOGY_GENERIC_MUX_DIV_GATE_2, ARRAY_SIZE(GenericMuxDivGate2Nodes), {0}},
	 {&GenericMuxDivGate2Nodes, TOPOLOGY_GENERIC_MUX_DIV_GATE_2, ARRAY_SIZE(GenericMuxDivGate2Nodes), {0}},
};

static XPm_ClockNode *ClkNodeList[(u32)XPM_NODEIDX_CLK_MAX];
static const u32 MaxClkNodes = (u32)XPM_NODEIDX_CLK_MAX;
static u32 PmNumClocks;

static XStatus XPmClock_Init(XPm_ClockNode *Clk, u32 Id, u32 ControlReg,
			     u8 TopologyType, u8 NumCustomNodes, u8 NumParents,
			     u32 PowerDomainId, u8 ClkFlags)
{
	XStatus Status = XST_FAILURE;
	u32 Subclass = NODESUBCLASS(Id);

	if (Subclass == (u32)XPM_NODETYPE_CLOCK_REF) {
		XPmNode_Init(&Clk->Node, Id, (u8)XPM_CLK_STATE_ON, 0);
	} else if (Subclass == (u32)XPM_NODETYPE_CLOCK_OUT) {
		if (NumParents > MAX_MUX_PARENTS) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		XPm_OutClockNode *OutClkPtr = (XPm_OutClockNode *)Clk;
		XPmNode_Init(&OutClkPtr->ClkNode.Node, Id, (u8)XPM_CLK_STATE_OFF, 0);
		OutClkPtr->ClkNode.Node.BaseAddress = ControlReg;
		OutClkPtr->ClkNode.ClkHandles = NULL;
		OutClkPtr->ClkNode.UseCount = 0;
		OutClkPtr->ClkNode.NumParents = NumParents;
		OutClkPtr->ClkNode.Flags = ClkFlags;
		if (TopologyType == TOPOLOGY_CUSTOM) {
			OutClkPtr->Topology.Id = TOPOLOGY_CUSTOM;
			OutClkPtr->Topology.NumNodes = NumCustomNodes;
			OutClkPtr->Topology.Nodes = XPm_AllocBytes((u32)NumCustomNodes * sizeof(struct XPm_ClkTopologyNode));
			if (OutClkPtr->Topology.Nodes == NULL) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
		} else {
			OutClkPtr->Topology.Id = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].Id;
			OutClkPtr->Topology.NumNodes = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].NumNodes;
			OutClkPtr->Topology.Nodes = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].Nodes;
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)XPM_NODECLASS_POWER != NODECLASS(PowerDomainId)) ||
	    ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(PowerDomainId))) {
		Clk->PwrDomain = NULL;
		Status = XST_SUCCESS;
		goto done;
	}

	Clk->PwrDomain = XPmPower_GetById(PowerDomainId);
	if (NULL == Clk->PwrDomain) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Clk->ClkRate = 0;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			 u8 NumCustomNodes, u8 NumParents, u32 PowerDomainId,
			 u8 ClkFlags)
{
	XStatus Status = XST_FAILURE;
	u32 Subclass = NODESUBCLASS(Id);
	XPm_ClockNode *Clk;

	if (NULL != XPmClock_GetById(Id)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (Subclass == (u32)XPM_NODETYPE_CLOCK_REF) {
		Clk = XPm_AllocBytes(sizeof(XPm_ClockNode));
		if (Clk==NULL) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else if (Subclass == (u32)XPM_NODETYPE_CLOCK_OUT) {
		if (TopologyType >= MAX_TOPOLOGY || TopologyType < TOPOLOGY_GENERIC_MUX_DIV) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Clk = XPm_AllocBytes(sizeof(XPm_OutClockNode));
		if (Clk == NULL) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_Init(Clk, Id, ControlReg, TopologyType,
			       NumCustomNodes, NumParents, PowerDomainId,
			       ClkFlags);

	if (XST_SUCCESS == Status) {
		Status = XPmClock_SetById(Id, Clk);
	} else {
		/* TODO: Free allocated memory */
	}

done:
	return Status;
}

XStatus XPmClock_AddClkName(u32 Id, char *Name)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(Id);

	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	(void)XPlmi_MemCpy(Clk->Name, Name, MAX_NAME_BYTES);

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_AddSubNode(u32 Id, u32 Type, u32 ControlReg, u8 Param1, u8 Param2, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0U;
	XPm_OutClockNode *OutClkPtr = (XPm_OutClockNode *)XPmClock_GetById(Id);
	struct XPm_ClkTopologyNode *SubNodes;

	if (OutClkPtr == NULL  || OutClkPtr->Topology.Id != TOPOLOGY_CUSTOM)	{
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (Type <= (u32)TYPE_INVALID || Type >= (u32)TYPE_MAX || Type == (u32)TYPE_PLL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	SubNodes = *OutClkPtr->Topology.Nodes;
	for (i=0; i<OutClkPtr->Topology.NumNodes; i++) {
		if (SubNodes[i].Type == 0U) {
			SubNodes[i].Type = (u8)Type;
			SubNodes[i].Reg = ControlReg;
			if (Type == (u32)TYPE_FIXEDFACTOR) {
				SubNodes[i].Param1.Mult = Param1;
				SubNodes[i].Param2.Div = Param2;
			} else {
				SubNodes[i].Param1.Shift = Param1;
				SubNodes[i].Param2.Width = Param2;
			}
			SubNodes[i].Clkflags = (u16)(Flags & 0xFFFFU);
			SubNodes[i].Typeflags = (u16)((Flags >> 16) & 0xFFFFU);
			SubNodes[i].Reg = ControlReg;
			break;
		}
	}
	if (i == OutClkPtr->Topology.NumNodes) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_AddParent(u32 Id, u32 *Parents, u8 NumParents)
{
	XStatus Status = XST_FAILURE;
	u32 Idx = 0;
	u32 LastParentIdx = 0;
	u16 ParentIdx = 0;
	XPm_ClockNode *ParentClk = NULL;
	XPm_OutClockNode *ClkPtr = (XPm_OutClockNode *)XPmClock_GetById(Id);

	if (ClkPtr == NULL || NumParents > MAX_MUX_PARENTS || NumParents == 0U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (Idx = 0; Idx < NumParents; Idx++) {
		u32 ParentId = Parents[Idx];

		/*
		 * FIXME: For GEM0_RX and GEM1_RX parents are EMIO and MIO
		 * clocks and their IDs are 0 which is not valid clock ID.
		 * Consider 0 as a valid parent ID for now.
		 * Remove this condition once EMIO and MIO clocks are added
		 * as valid clocks.
		 */
		if (0U == ParentId) {
			continue;
		}

		if (!ISOUTCLK(ParentId) && !ISREFCLK(ParentId) &&
		    !ISPLL(ParentId) && (u32)CLK_DUMMY_PARENT != ParentId) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}

	/*
	 * For clocks which has more than 5 parents add parent command will call
	 * multiple times. Because from single command only 5 parents can add.
	 * So find parent index for second command from there remaining parents
	 * should be stored.
	 */
	while ((0U != ClkPtr->Topology.MuxSources[LastParentIdx]) &&
	       (MAX_MUX_PARENTS != LastParentIdx)) {
		LastParentIdx++;
	}

	/* Parents count should not be greater than clock's numbed of parents */
	if ((LastParentIdx + NumParents > ClkPtr->ClkNode.NumParents) ||
	    (MAX_MUX_PARENTS == LastParentIdx)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* For clocks involving mux */
	for (Idx = 0; Idx < NumParents; Idx++) {
		if ((u32)CLK_DUMMY_PARENT == Parents[Idx]) {
			ParentIdx = (u16)CLK_DUMMY_PARENT;
		} else {
			ParentIdx = (u16)(NODEINDEX(Parents[Idx]));
		}
		ClkPtr->Topology.MuxSources[LastParentIdx++] = ParentIdx;
	}

	/* Assign default parent */
	if (ClkPtr->ClkNode.NumParents > 1U) {
		/*
		 * For mux clocks, parents are initialized when clock
		 * requested. So assign invalid clock parent by default.
		 */
		ClkPtr->ClkNode.ParentIdx = (u16)CLOCK_PARENT_INVALID;
	} else {
		ParentClk = XPmClock_GetByIdx(ClkPtr->Topology.MuxSources[0]);
		if (NULL != ParentClk) {
			ClkPtr->ClkNode.ParentIdx = (u16)(NODEINDEX(ParentClk->Node.Id));
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XPm_ClockNode* XPmClock_GetById(u32 ClockId)
{
	u32 ClockIndex = NODEINDEX(ClockId);
	u32 NodeType = NODETYPE(ClockId);
	XPm_ClockNode *Clk = NULL;
	u32 MaskId = ((u32)XPM_NODETYPE_CLOCK_SUBNODE == NodeType) ?
		(~((u32)NODE_TYPE_MASK)) : ((~(u32)0x0));

	if (((u32)XPM_NODECLASS_CLOCK != NODECLASS(ClockId)) ||
	    (MaxClkNodes <= ClockIndex)) {
		goto done;
	}

	Clk = ClkNodeList[ClockIndex];
	if (NULL == Clk) {
		goto done;
	}

	/* Check that Clock's ID is same as given ID or not.
	 * NOTE:
	 * For ADD_CLOCK_SUBNODE command, we add the subnodes to the existing
	 * clock nodes in the database. These "existing" clock nodes are stored
	 * with a different node type than the 'XPM_NODETYPE_CLOCK_SUBNODE'
	 * which is passed into this function for retrieval of such nodes.
	 * So, we need to mask the type for this special case while validating.
	 * This is what MaskId does. For all other cases, it is all ones.
	 */
	if ((ClockId & MaskId) != (Clk->Node.Id & MaskId)) {
		Clk = NULL;
	}

done:
	return Clk;
}

XPm_ClockNode* XPmClock_GetByIdx(u32 ClockIdx)
{
        XPm_ClockNode *Clk = NULL;

        if(MaxClkNodes <= ClockIdx) {
                goto done;
        }

        Clk = ClkNodeList[ClockIdx];

done:
        return Clk;
}


XStatus XPmClock_SetById(u32 ClockId, XPm_ClockNode *Clk)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(ClockId);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Clk) && (MaxClkNodes > NodeIndex)) {
		ClkNodeList[NodeIndex] = Clk;
		PmNumClocks++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static struct XPm_ClkTopologyNode* XPmClock_GetTopologyNode(XPm_OutClockNode *Clk, u32 Type)
{
	struct XPm_ClkTopologyNode *SubNodes;
	uint8_t NumNodes;
	u32 i;

	if (Clk == NULL) {
		return NULL;
	}

	SubNodes = *Clk->Topology.Nodes;
	NumNodes = Clk->Topology.NumNodes;

	for (i = 0; i < NumNodes; i++) {
		if (SubNodes[i].Type == Type) {
			/* For custom topology, nodes have correct control address but
			 * for other topologies, there's no separate node memory to fill
			 * register each time */
			if (Clk->Topology.Id != TOPOLOGY_CUSTOM) {
				SubNodes[i].Reg = Clk->ClkNode.Node.BaseAddress;
			}
			return &SubNodes[i];
		}
	}
	return NULL;
}

static void XPmClock_InitParent(XPm_OutClockNode *Clk)
{
	u32 ParentIdx = 0;
	struct XPm_ClkTopologyNode *Ptr;
	XPm_ClockNode *ParentClk = NULL;
	int Status;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_MUX);
	if (NULL != Ptr) {
		Status = XPmClock_GetClockData(Clk, (u32)TYPE_MUX, &ParentIdx);
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in GetClockData of 0x%x\r\n", Status, Clk->ClkNode.Node.Id);
		}

		/* Update new parent id */
		ParentClk = XPmClock_GetByIdx(Clk->Topology.MuxSources[ParentIdx]);
		if (NULL != ParentClk) {
			Clk->ClkNode.ParentIdx = (u16)(NODEINDEX(ParentClk->Node.Id));
		}
	}

	return;
}

static void XPmClock_RequestInt(XPm_ClockNode *Clk)
{
	int Status;

	if (Clk != NULL) {
		if (0U == Clk->UseCount) {
			/* Initialize the parent if not done before */
			if (CLOCK_PARENT_INVALID == Clk->ParentIdx) {
				XPmClock_InitParent((XPm_OutClockNode *)Clk);
			}

			/* Request the parent first */
			XPm_ClockNode *ParentClk = XPmClock_GetByIdx(Clk->ParentIdx);
			if (ISOUTCLK(ParentClk->Node.Id)) {
				XPmClock_RequestInt(ParentClk);
			} else if (ISPLL(ParentClk->Node.Id)) {
				Status = XPmClockPll_Request(ParentClk->Node.Id);
				if (XST_SUCCESS != Status) {
					PmWarn("Error %d in request PLL of 0x%x\r\n", Status, ParentClk->Node.Id);
				}
			} else {
				/* Required due to MISRA */
				PmDbg("Invalid clock type of clock 0x%x\r\n", ParentClk->Node.Id);
			}

			/* Mark it as requested. If clock has a gate, state will be changed to On when enabled */
			Clk->Node.State |= XPM_CLK_STATE_REQUESTED;
			/* Enable clock if gated */
			(void)XPmClock_SetGate((XPm_OutClockNode *)Clk, 1);
		}

		/* Increment the use count of clock */
		Clk->UseCount++;
	}
	return;
}

XStatus XPmClock_Request(XPm_ClockHandle *ClkHandle)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	u32 ClkId;

	if (NULL == ClkHandle) {
		Status = XST_SUCCESS;
		goto done;
	}

	while (NULL != ClkHandle) {
		Clk = ClkHandle->Clock;
		ClkId = Clk->Node.Id;
		if (ISOUTCLK(ClkId)) {
			XPmClock_RequestInt(Clk);
		} else if (ISPLL(ClkId)) {
			Status = XPmClockPll_Request(ClkId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			/* Required due to MISRA */
			PmDbg("Invalid clock type of clock 0x%x\r\n", ClkId);
		}
		ClkHandle = ClkHandle->NextClock;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static void XPmClock_ReleaseInt(XPm_ClockNode *Clk)
{
	int Status;

	if (Clk != NULL) {
		/* Decrease the use count of clock */
		Clk->UseCount--;

		if (0U == Clk->UseCount) {
			/* Clear the requested bit of clock */
			Clk->Node.State &= (u8)(~(XPM_CLK_STATE_REQUESTED));
			/* Disable clock */
			(void)XPmClock_SetGate((XPm_OutClockNode *)Clk, 0);

			/* Release the clock parent */
			XPm_ClockNode *ParentClk = XPmClock_GetByIdx(Clk->ParentIdx);
			if (ISOUTCLK(ParentClk->Node.Id)) {
				XPmClock_ReleaseInt(ParentClk);
			} else if (ISPLL(ParentClk->Node.Id)) {
				Status = XPmClockPll_Release(ParentClk->Node.Id);
				if (XST_SUCCESS != Status) {
					PmWarn("Error %d in release PLL of 0x%x\r\n", Status, ParentClk->Node.Id);
				}
			} else {
				/* Required due to MISRA */
				PmDbg("Invalid clock type of clock 0x%x\r\n", ParentClk->Node.Id);
			}
		}
	}
	return;
}

XStatus XPmClock_Release(XPm_ClockHandle *ClkHandle)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	u32 ClkId;

	if (NULL == ClkHandle) {
		Status = XST_SUCCESS;
		goto done;
	}

	while (NULL != ClkHandle) {
		Clk = ClkHandle->Clock;
		ClkId = Clk->Node.Id;
		if (ISOUTCLK(ClkId)) {
			XPmClock_ReleaseInt(Clk);
		} else if (ISPLL(ClkId)) {
			Status = XPmClockPll_Release(ClkId);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			/* Required due to MISRA */
		}
		ClkHandle = ClkHandle->NextClock;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}


XStatus XPmClock_SetGate(XPm_OutClockNode *Clk, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *Ptr;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_GATE);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Enable > 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPm_RMW32(Ptr->Reg, BITNMASK(Ptr->Param1.Shift,Ptr->Param2.Width), Enable << Ptr->Param1.Shift);

	if (1U == Enable) {
		Clk->ClkNode.Node.State |= XPM_CLK_STATE_ON;
	} else {
		Clk->ClkNode.Node.State &= (u8)(~(XPM_CLK_STATE_ON));
	}

	Status = XST_SUCCESS;

done:
	return Status;
}


XStatus XPmClock_SetParent(XPm_OutClockNode *Clk, u32 ParentIdx)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *Ptr;
	XPm_ClockNode *ParentClk = NULL;
	XPm_ClockNode *OldParentClk = NULL;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_MUX);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((ParentIdx > BITMASK(Ptr->Param2.Width)) ||
	    (ParentIdx > (((u32)Clk->ClkNode.NumParents) - 1U))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Request new parent */
	ParentClk = XPmClock_GetByIdx(Clk->Topology.MuxSources[ParentIdx]);
	if (ISOUTCLK(ParentClk->Node.Id)) {
		XPmClock_RequestInt(ParentClk);
	} else if (ISPLL(ParentClk->Node.Id)) {
		Status = XPmClockPll_Request(ParentClk->Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		/* Required due to MISRA */
		PmDbg("Invalid clock type of clock 0x%x\r\n", ParentClk->Node.Id);
	}

	XPm_RMW32(Ptr->Reg, BITNMASK(Ptr->Param1.Shift,Ptr->Param2.Width), ParentIdx << Ptr->Param1.Shift);

	/* Release old parent */
	OldParentClk = XPmClock_GetByIdx(Clk->ClkNode.ParentIdx);
	if (ISOUTCLK(OldParentClk->Node.Id)) {
		XPmClock_ReleaseInt(OldParentClk);
	} else if (ISPLL(OldParentClk->Node.Id)) {
		Status = XPmClockPll_Release(OldParentClk->Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	} else {
		/* Required due to MISRA */
		PmDbg("Invalid clock type of clock 0x%x\r\n", OldParentClk->Node.Id);
	}

	/* Update new parent idx */
	Clk->ClkNode.ParentIdx = (u16)(NODEINDEX(ParentClk->Node.Id));

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_SetDivider(XPm_OutClockNode *Clk, u32 Divider)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *Ptr;
	u32 Divider1;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_DIV1);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Divider1 = Divider & 0xFFFFU;
	if (Divider1 > BITMASK(Ptr->Param2.Width)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPm_RMW32(Ptr->Reg, BITNMASK(Ptr->Param1.Shift,Ptr->Param2.Width), Divider1 << Ptr->Param1.Shift);

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_GetClockData(XPm_OutClockNode *Clk, u32 Nodetype, u32 *Value)
{
	XStatus Status = XST_FAILURE;
	u32 Mask;
	struct XPm_ClkTopologyNode *Ptr;
	XPm_Power *PowerDomain = Clk->ClkNode.PwrDomain;

	Ptr = XPmClock_GetTopologyNode(Clk, Nodetype);
	if (Ptr == NULL) {
		Status = XPM_INVALID_CLK_SUBNODETYPE;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON != PowerDomain->Node.State) {
		Status = XST_NO_ACCESS;
		goto done;
	}

	Mask = BITNMASK(Ptr->Param1.Shift, Ptr->Param2.Width);
	*Value = (XPm_Read32(Ptr->Reg) &  Mask) >> Ptr->Param1.Shift;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_QueryName(u32 ClockId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	(void)memset(Resp, 0, CLK_QUERY_NAME_LEN);

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		goto done;
	}

	(void)memcpy((char *)Resp, &Clk->Name[0], CLK_QUERY_NAME_LEN);

	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmClock_QueryTopology(u32 ClockId, u32 Index, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	struct XPm_ClkTopologyNode *PtrNodes;
	XPm_OutClockNode *Clk;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);

	(void)memset(Resp, 0, CLK_TOPOLOGY_PAYLOAD_LEN);
	if (ISOUTCLK(ClockId)) {
		PtrNodes = *Clk->Topology.Nodes;

		/* Skip parent till index */
		if (Index >= Clk->Topology.NumNodes) {
			Status = XST_SUCCESS;
			goto done;
		}

		for (i = 0; i < 3U; i++) {
			if ((Index + i) == Clk->Topology.NumNodes) {
				break;
			}
			Resp[i] =  PtrNodes[Index + i].Type;
			Resp[i] |= ((u32)(PtrNodes[Index + i].Clkflags) << CLK_CLKFLAGS_SHIFT);
			Resp[i] |= ((u32)(PtrNodes[Index + i].Typeflags) << CLK_TYPEFLAGS_SHIFT);
		}
	} else if (ISPLL(ClockId)) {
		if (Index != 0U) {
			Status = XST_SUCCESS;
			goto done;
		}
		Resp[0] = (u32)TYPE_PLL;
		Resp[0] |= CLK_SET_RATE_NO_REPARENT << CLK_CLKFLAGS_SHIFT;
		Resp[0] |= ((u32)NA_TYPE_FLAGS) << CLK_TYPEFLAGS_SHIFT;
	} else {
		Status = XST_FAILURE;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_QueryFFParams(u32 ClockId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *Ptr;
	XPm_OutClockNode *Clk;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);

	if (!ISOUTCLK(ClockId)) {
		goto done;
	}

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_FIXEDFACTOR);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Resp[0] = Ptr->Param1.Mult;
	Resp[1] = Ptr->Param2.Div;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_QueryMuxSources(u32 ClockId, u32 Index, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	XPm_OutClockNode *Clk;
	u32 i;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);

	if (!ISOUTCLK(ClockId)) {
		goto done;
	}

	(void)memset(Resp, 0, CLK_PARENTS_PAYLOAD_LEN);

	/* Skip parent till index */
	for (i = 0; i < 3U; i++) {
		if (Clk->ClkNode.NumParents == (Index + i)) {
			Resp[i] = 0xFFFFFFFFU;
			break;
		}
		Resp[i] = Clk->Topology.MuxSources[Index + i];
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_QueryAttributes(u32 ClockIndex, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	unsigned int Attr = 0;
	u32 InitEnable = 0;
	u32 ClockId = 0;
	XPm_ClockNode *Clk;

	if (ClockIndex >= MaxClkNodes) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Clock valid bit. All clocks present in clock database is valid. */
	if (NULL != ClkNodeList[ClockIndex]) {
		Attr = 1U;
		Clk = ClkNodeList[ClockIndex];
		ClockId = Clk->Node.Id;
	} else {
		Attr = 0U;
	}

	//if (PLATFORM_VERSION_SILICON != Platform) {
	/*
	 * Mark CPM related clock as invalid because their registers
	 * are not accessible from PS DDR SPP.
	 * TODO: This code under platform version check needs to be
	 * removed when CPM registers are accessible.
	 */
	if (ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_LSBUS_REF ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_PLL ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_PRESRC ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_POSTCLK ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_PLL_OUT ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_CORE_REF ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_DBG_REF ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_AUX0_REF ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_AUX1_REF ||
	    ClockIndex == (u32)XPM_NODEIDX_CLK_CPM_TOPSW_REF) {
		//PmInfo("Marking Clock: %d as Invalid\r\n", ClockIndex);
		Attr = 0;
	}
	//}

	/* If clock needs to be enabled during init */
	/* TBD -  Decide InitEnable value */
	Attr |= InitEnable << CLK_INIT_ENABLE_SHIFT;
	/* Clock type (Output/External) */
	if (NODESUBCLASS(ClockId) == (u32)XPM_NODESUBCL_CLOCK_REF) {
		Attr |= 1U << CLK_TYPE_SHIFT;
	}
	/* Clock node type PLL, OUT or REF*/
	Attr |= NODETYPE(ClockId) << CLK_NODETYPE_SHIFT;
	/* Clock node subclass PLL, OUT or REF */
	Attr |= NODESUBCLASS(ClockId) << CLK_NODESUBCLASS_SHIFT;
	/* Node class, i.e Clock */
	Attr |= NODECLASS(ClockId) << CLK_NODECLASS_SHIFT;

	*Resp = Attr;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_GetNumClocks(u32 *Resp)
{
	*Resp = (u32)XPM_NODEIDX_CLK_MAX;

	return XST_SUCCESS;
}

XStatus XPmClock_CheckPermissions(u32 SubsystemIdx, u32 ClockId)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;
	XPm_ClockHandle *DevHandle;
	u32 PermissionMask = 0U;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check for read-only flag */
	if (0U != (CLK_FLAG_READ_ONLY & Clk->Flags)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Check for power domain of clock */
	if ((NULL != Clk->PwrDomain) &&
	    ((u8)XPM_POWER_STATE_ON != Clk->PwrDomain->Node.State)) {
		Status = XST_FAILURE;
		goto done;
	}

	if (ISPLL(ClockId)) {
		/* Do not allow permission by default when PLL is shared */
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	DevHandle = Clk->ClkHandles;
	while (NULL != DevHandle) {
		/* Get permission mask which indicates permission for each subsystem */
		Status = XPmDevice_GetPermissions(DevHandle->Device,
						  &PermissionMask);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		DevHandle = DevHandle->NextDevice;
	}

	/* Check permission for given subsystem */
	if (0U == (PermissionMask & ((u32)1U << SubsystemIdx))) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Access is not allowed if resource is shared (multiple subsystems) */
	if (__builtin_popcount(PermissionMask) > 1) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_GetMaxDivisor(u32 ClockId, u32 DivType, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *Ptr;
	XPm_OutClockNode *Clk;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Ptr = XPmClock_GetTopologyNode(Clk, DivType);
	if (NULL == Ptr) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*Resp = BITMASK(Ptr->Param2.Width);

	Status = XST_SUCCESS;

done:
	return Status;
}

int XPmClock_SetRate(XPm_ClockNode *Clk, const u32 ClkRate)
{
	Clk->ClkRate = ClkRate;

	return XST_SUCCESS;
}

int XPmClock_GetRate(XPm_ClockNode *Clk, u32 *ClkRate)
{
	*ClkRate = Clk->ClkRate;

	return XST_SUCCESS;
}
