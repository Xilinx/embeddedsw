/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xplmi_util.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_alloc.h"
#include "xpm_update.h"

/* Macros to initialize individual node types without creating temporaries */
#define INIT_MUX_NODE(node, idx) \
	(node)[(idx)].Type = (u8)TYPE_MUX; \
	(node)[(idx)].Param1.Shift = PERIPH_MUX_SHIFT; \
	(node)[(idx)].Param2.Width = PERIPH_MUX_WIDTH; \
	(node)[(idx)].Clkflags = CLK_SET_RATE_NO_REPARENT; \
	(node)[(idx)].Typeflags = NA_TYPE_FLAGS; \
	(node)[(idx)].Reg = 0U;

#define INIT_DIV_NODE(node, idx) \
	(node)[(idx)].Type = (u8)TYPE_DIV1; \
	(node)[(idx)].Param1.Shift = PERIPH_DIV_SHIFT; \
	(node)[(idx)].Param2.Width = PERIPH_DIV_WIDTH; \
	(node)[(idx)].Clkflags = CLK_SET_RATE_NO_REPARENT; \
	(node)[(idx)].Typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO; \
	(node)[(idx)].Reg = 0U;

#define INIT_GATE_NODE(node, idx, gate_id) \
	(node)[(idx)].Type = (u8)TYPE_GATE; \
	(node)[(idx)].Param1.Shift = PERIPH_GATE##gate_id##_SHIFT; \
	(node)[(idx)].Param2.Width = PERIPH_GATE_WIDTH; \
	(node)[(idx)].Clkflags = CLK_SET_RATE_PARENT | CLK_SET_RATE_GATE; \
	(node)[(idx)].Typeflags = NA_TYPE_FLAGS; \
	(node)[(idx)].Reg = 0U;

/* Macro to allocate and check for a topology */
#define ALLOC_TOPOLOGY_NODES(num_nodes, alloc_var) \
	alloc_var = (struct XPm_ClkTopologyNode *)XPm_AllocBytes( \
		(num_nodes) * sizeof(struct XPm_ClkTopologyNode)); \
	if (NULL == alloc_var) { \
		DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL; \
		Status = XST_BUFFER_TOO_SMALL; \
		goto done; \
	}

/* Macro to set a topology in the array */
#define SET_TOPOLOGY(topo_type, nodes_ptr, num) \
	ClkTopologies[(topo_type) - TOPOLOGY_GENERIC_MUX_DIV].Nodes = \
		(struct XPm_ClkTopologyNode(*)[])(nodes_ptr); \
	ClkTopologies[(topo_type) - TOPOLOGY_GENERIC_MUX_DIV].NumNodes = (num);

/* Macro for 2-node topologies */
#define INIT_TOPOLOGY_2(topo_type, node1_init, node2_init) do { \
	ALLOC_TOPOLOGY_NODES(2U, AllocatedNodes); \
	node1_init(AllocatedNodes, 0); \
	node2_init(AllocatedNodes, 1); \
	SET_TOPOLOGY(topo_type, AllocatedNodes, 2U); \
} while(0)

/* Macro for 3-node topologies */
#define INIT_TOPOLOGY_3(topo_type, node1_init, node2_init, node3_init) do { \
	ALLOC_TOPOLOGY_NODES(3U, AllocatedNodes); \
	node1_init(AllocatedNodes, 0); \
	node2_init(AllocatedNodes, 1); \
	node3_init(AllocatedNodes, 2); \
	SET_TOPOLOGY(topo_type, AllocatedNodes, 3U); \
} while(0)

/* Wrapper macros for GATE nodes with specific IDs */
#define INIT_GATE1_NODE(node, idx) INIT_GATE_NODE(node, idx, 1)
#define INIT_GATE2_NODE(node, idx) INIT_GATE_NODE(node, idx, 2)

static XPm_ClkTopology ClkTopologies[] XPM_INIT_DATA(ClkTopologies)= {
	 {NULL, TOPOLOGY_GENERIC_MUX_DIV, 0, {0}},
	 {NULL, TOPOLOGY_GENERIC_MUX_GATE, 0, {0}},
	 {NULL, TOPOLOGY_GENERIC_DIV_GATE, 0, {0}},
	 {NULL, TOPOLOGY_GENERIC_MUX_DIV_GATE_1, 0, {0}},
	 {NULL, TOPOLOGY_GENERIC_MUX_DIV_GATE_2, 0, {0}},
};

XStatus XPmClock_InitGenericTopology(void)
{
	XStatus Status = XST_FAILURE;
	struct XPm_ClkTopologyNode *AllocatedNodes;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	INIT_TOPOLOGY_2(TOPOLOGY_GENERIC_MUX_DIV, INIT_MUX_NODE, INIT_DIV_NODE);
	INIT_TOPOLOGY_2(TOPOLOGY_GENERIC_MUX_GATE, INIT_MUX_NODE, INIT_GATE2_NODE);
	INIT_TOPOLOGY_2(TOPOLOGY_GENERIC_DIV_GATE, INIT_DIV_NODE, INIT_GATE2_NODE);
	INIT_TOPOLOGY_3(TOPOLOGY_GENERIC_MUX_DIV_GATE_1, INIT_MUX_NODE, INIT_DIV_NODE, INIT_GATE1_NODE);
	INIT_TOPOLOGY_3(TOPOLOGY_GENERIC_MUX_DIV_GATE_2, INIT_MUX_NODE, INIT_DIV_NODE, INIT_GATE2_NODE);

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XPm_ClockNode *ClkNodeList[(u32)XPM_NODEIDX_CLK_MAX] XPM_INIT_DATA(ClkNodeList) = { NULL };
static u32 MaxClkNodes XPM_INIT_DATA(MaxClkNodes) = 0U;

XPm_ClockNode** XPmClock_GetClkList(void)
{
	return ClkNodeList;
}

u32 XPmClock_GetMaxClkNodes(void)
{
	return MaxClkNodes;
}
static XStatus XPmClock_Init(XPm_ClockNode *Clk, u32 Id, u32 ControlReg,
			     u8 TopologyType, u8 NumCustomNodes, u8 NumParents,
			     u32 PowerDomainId, u8 ClkFlags)
{
	XStatus Status = XST_FAILURE;
	u32 Subclass = NODESUBCLASS(Id);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (Subclass == (u32)XPM_NODETYPE_CLOCK_REF) {
		XPmNode_Init(&Clk->Node, Id, (u8)XPM_CLK_STATE_ON, 0);
	} else if (Subclass == (u32)XPM_NODETYPE_CLOCK_OUT) {
		if (NumParents > MAX_MUX_PARENTS) {
			DbgErr = XPM_INT_ERR_MAX_CLK_PARENTS;
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
				DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
		} else {
			OutClkPtr->Topology.Id = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].Id;
			OutClkPtr->Topology.NumNodes = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].NumNodes;
			OutClkPtr->Topology.Nodes = ClkTopologies[TopologyType-TOPOLOGY_GENERIC_MUX_DIV].Nodes;
		}
	} else {
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
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
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Clk->ClkRate = 0;

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmClock_AddNode(u32 Id, u32 ControlReg, u8 TopologyType,
			 u8 NumCustomNodes, u8 NumParents, u32 PowerDomainId,
			 u8 ClkFlags)
{
	XStatus Status = XST_FAILURE;
	u32 Subclass = NODESUBCLASS(Id);
	XPm_ClockNode *Clk;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (NULL != XPmClock_GetById(Id)) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (Subclass == (u32)XPM_NODETYPE_CLOCK_REF) {
		Clk = XPm_AllocBytes(sizeof(XPm_ClockNode));
		if (Clk==NULL) {
			DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else if (Subclass == (u32)XPM_NODETYPE_CLOCK_OUT) {
		if ((TopologyType >= MAX_TOPOLOGY) ||
		    (TopologyType < TOPOLOGY_GENERIC_MUX_DIV)) {
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			goto done;
		}
		Clk = XPm_AllocBytes(sizeof(XPm_OutClockNode));
		if (Clk == NULL) {
			DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	} else {
		DbgErr = XPM_INT_ERR_INVALID_SUBCLASS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_Init(Clk, Id, ControlReg, TopologyType,
			       NumCustomNodes, NumParents, PowerDomainId,
			       ClkFlags);

	if (XST_SUCCESS == Status) {
		Status = XPmClock_SetById(Id, Clk);
	} else {
		DbgErr = XPM_INT_ERR_CLK_INIT;
		/* TODO: Free allocated memory */
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmClock_AddClkName(u32 Id, const char *Name)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(Id);
	const u32 CopySize = MAX_NAME_BYTES;

	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Status = Xil_SMemCpy(Clk->Name, CopySize, Name, CopySize, CopySize);

done:
	return Status;
}

XStatus XPmClock_AddSubNode(u32 Id, u32 Type, u32 ControlReg, u8 Param1, u8 Param2, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0U;
	const XPm_OutClockNode *OutClkPtr = (XPm_OutClockNode *)XPmClock_GetById(Id);
	struct XPm_ClkTopologyNode *SubNodes;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((OutClkPtr == NULL) ||
	    (OutClkPtr->Topology.Id != TOPOLOGY_CUSTOM))	{
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if ((Type <= (u32)TYPE_INVALID) || (Type >= (u32)TYPE_MAX) ||
	    (Type == (u32)TYPE_PLL)) {
		DbgErr = XPM_INT_ERR_INVALID_CLK_TYPE;
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
		DbgErr = XPM_INT_ERR_CLK_TOPOLOGY_MAX_NUM_NODES;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmClock_AddParent(u32 Id, const u32 *Parents, u8 NumParents)
{
	XStatus Status = XST_FAILURE;
	u32 Idx = 0;
	u32 LastParentIdx = 0;
	u16 ParentIdx = 0;
	const XPm_ClockNode *ParentClk = NULL;
	XPm_OutClockNode *ClkPtr = (XPm_OutClockNode *)XPmClock_GetById(Id);
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((ClkPtr == NULL) ||
	    (NumParents > MAX_MUX_PARENTS) ||
	    (NumParents == 0U)) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
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

		if ((!ISOUTCLK(ParentId)) && (!ISREFCLK(ParentId)) &&
		    (!ISPLL(ParentId)) &&
		    ((u32)CLK_DUMMY_PARENT != ParentId)) {
			DbgErr = XPM_INT_ERR_INVALID_CLK_PARENT;
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
	if (((LastParentIdx + NumParents) > ClkPtr->ClkNode.NumParents) ||
	    (MAX_MUX_PARENTS == LastParentIdx)) {
		DbgErr = XPM_INT_ERR_MAX_CLK_PARENTS;
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
		ClkPtr->Topology.MuxSources[LastParentIdx] = ParentIdx;
		LastParentIdx++;
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
	XPm_PrintDbgErr(Status, DbgErr);
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
	    ((u32)XPM_NODEIDX_CLK_MAX <= ClockIndex)) {
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

        if(MaxClkNodes < ClockIdx) {
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
	if ((NULL != Clk) && ((u32)XPM_NODEIDX_CLK_MAX > NodeIndex)) {
		ClkNodeList[NodeIndex] = Clk;
		if (NodeIndex > MaxClkNodes) {
			MaxClkNodes = NodeIndex;
		}
		Status = XST_SUCCESS;
	}

	return Status;
}

static struct XPm_ClkTopologyNode* XPmClock_GetTopologyNode(const XPm_OutClockNode *Clk, u32 Type)
{
	struct XPm_ClkTopologyNode *SubNodes;
	struct XPm_ClkTopologyNode *ClkSubNodes = NULL;
	uint8_t NumNodes;
	u32 i;

	if (Clk == NULL) {
		goto done;
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
			ClkSubNodes = &SubNodes[i];
			break;
		}
	}
done:
	return ClkSubNodes;
}

void XPmClock_InitParent(XPm_OutClockNode *Clk)
{
	u32 ParentIdx = 0;
	const struct XPm_ClkTopologyNode *Ptr;
	const XPm_ClockNode *ParentClk = NULL;
	XStatus Status;

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


XStatus XPmClock_GetClockData(const XPm_OutClockNode *Clk, u32 Nodetype, u32 *Value)
{
	XStatus Status = XST_FAILURE;
	u32 Mask;
	const struct XPm_ClkTopologyNode *Ptr;
	const XPm_Power *PowerDomain = Clk->ClkNode.PwrDomain;

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


XStatus XPmClock_SetRate(XPm_ClockNode *Clk, const u32 ClkRate)
{
	Clk->ClkRate = ClkRate;

	return XST_SUCCESS;
}
