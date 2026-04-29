/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_clock.h"
#include "xpm_debug.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_common.h"
#include "xplmi.h"
#include "xpm_subsystem.h"
#include "xpm_runtime_clock.h"
#include "xpm_runtime_device.h"
#include "xpm_pll.h"
#include "xpm_runtime_pll.h"

#define CLK_QUERY_NAME_LEN		(MAX_NAME_BYTES)
#define CLK_INIT_ENABLE_SHIFT		1U
#define CLK_TYPE_SHIFT			2U
#define CLK_NODETYPE_SHIFT		14U
#define CLK_NODESUBCLASS_SHIFT		20U
#define CLK_NODECLASS_SHIFT		26U
#define CLK_PARENTS_PAYLOAD_LEN		12U
#define CLK_TOPOLOGY_PAYLOAD_LEN	12U
#define CLK_CLKFLAGS_SHIFT		8U
#define CLK_TYPEFLAGS_SHIFT		24U
/** Maximum clock parent-chain depth for iterative rate calculation.
 *  Limits traversal in XPmClock_CalculateRate() to guard against
 *  runaway walks on malformed or unexpectedly deep topologies. */
#define MAX_CLK_CHAIN_DEPTH		32U


#ifdef XILPM_NG_ENABLE_CLK_SCMI
/* CLOCK_DESCRIBE_RATE response format macros */
#define CLK_RATE_FORMAT_SHIFT	12U
#define CLK_RATES_TRIPLET	3U

/* CLOCK_DESCRIBE_RATE FLAG macros */
#define CLK_RANGE_FORMAT	1U

/* Helper macro to format clock rate response */
#define CLK_DESCRIBE_RATE_FORMAT(num_rates, format_type) \
	((num_rates) | ((format_type) << CLK_RATE_FORMAT_SHIFT))

/* Rate change allowed for only few clocks (for now) */
#define IS_RATE_CHANGE_ALLOWED(ClkId) \
	((ClkId == PM_CLK_GEM0_TX) || (ClkId == PM_CLK_GEM1_TX) || \
	(ClkId == PM_CLK_MMIPLL) || (ClkId == PM_CLK_DC_REF) || \
	(ClkId == PM_CLK_DC_PIXEL) || (ClkId == PM_CLK_MMI_AUX1_REF))
#endif /* XILPM_NG_ENABLE_CLK_SCMI */

/* Clock UseCount overflow protection macro (use info print to avoid flooding logs) */
#define PmIncrementClk(NodeId, Name, UseCount) \
	do { \
		if (UseCount < 255) { \
			UseCount++; \
		} else { \
			PmInfo("Usecount is already 255, Clk: 0x%x (%s)\r\n", NodeId, Name); \
		} \
	} while (0)

/* Clock UseCount underflow protection macro (use warning to keep current behavior) */
#define PmDecrementClk(NodeId, Name, UseCount) \
	do { \
		if (UseCount > 0) { \
			UseCount--; \
		} else { \
			PmWarn("Usecount is already 0, Clk: 0x%x (%s)\r\n", NodeId, Name); \
		} \
	} while (0)

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
static void XPmClock_RequestInt(XPm_ClockNode *Clk)
{
	XStatus Status;

	if (Clk != NULL) {
		if (0U == Clk->UseCount) {
			/* Initialize the parent if not done before */
			if (CLOCK_PARENT_INVALID == Clk->ParentIdx) {
				XPmClock_InitParent((XPm_OutClockNode *)Clk);
			}

			/* Request the parent first */
			XPm_ClockNode *ParentClk = XPmClock_GetByIdx(Clk->ParentIdx);
			if (NULL == ParentClk) {
				PmWarn("Invalid parent clockIdx %d\r\n", Clk->ParentIdx);
				goto done;
			}

			if (ISOUTCLK(ParentClk->Node.Id)) {
				XPmClock_RequestInt(ParentClk);
			} else if (ISPLL(ParentClk->Node.Id)) {
				Status = XPmClockPll_Request(ParentClk->Node.Id);
				if (XST_SUCCESS != Status) {
					PmWarn("Error %d in request PLL of 0x%x\r\n", Status, ParentClk->Node.Id);
					goto done;
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
		PmIncrementClk(Clk->Node.Id, Clk->Name, Clk->UseCount);
	}

done:
	return;
}

XStatus XPmClock_SetGate(XPm_OutClockNode *Clk, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	const struct XPm_ClkTopologyNode *Ptr;
	u32 Val;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_GATE);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (Enable > 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Val = (CLK_GATE_ACTIVE_LOW & Ptr->Typeflags) ? !Enable : Enable;
	XPm_RMW32(Ptr->Reg, BITNMASK(Ptr->Param1.Shift,Ptr->Param2.Width), Val << Ptr->Param1.Shift);

	if (1U == Enable) {
		Clk->ClkNode.Node.State |= XPM_CLK_STATE_ON;
	} else {
		Clk->ClkNode.Node.State &= (u8)(~(XPM_CLK_STATE_ON));
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static void XPmClock_ReleaseInt(XPm_ClockNode *Clk)
{
	XStatus Status;

	if (Clk != NULL) {
		/* Decrease the use count of clock */
		PmDecrementClk(Clk->Node.Id, Clk->Name, Clk->UseCount);

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

XStatus XPmClock_Release(const XPm_ClockHandle *ClkHandle)
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


XStatus XPmClock_CheckPermissions(u32 SubsystemIdx, u32 ClockId)
{
	volatile XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk;
	const XPm_ClockHandle *DevHandle;
	u32 PermissionMask = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check for read-only flag with redundant verification */
	volatile u32 RoFlag = CLK_FLAG_READ_ONLY & Clk->Flags;
	volatile u32 RoFlagTmp = CLK_FLAG_READ_ONLY & Clk->Flags;
	if ((0U != RoFlag) || (0U != RoFlagTmp)) {
		DbgErr = XPM_INT_ERR_READ_ONLY_CLK;
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Check for power domain of clock */
	if ((NULL != Clk->PwrDomain) &&
	    ((u8)XPM_POWER_STATE_ON != Clk->PwrDomain->Node.State)) {
		DbgErr = XPM_INT_ERR_PWR_DOMAIN_OFF;
		Status = XST_FAILURE;
		goto done;
	}

	if (ISPLL(ClockId)) {
		if (PM_CLK_MMI_PLL == ClockId) {
			/* Allow MMI PLL to be reprogrammed runtime as required for
			 * accurate video clock frequencies for DC device */
			Status = XST_SUCCESS;
		} else {
			/* Do not allow permission by default when PLL is shared */
			DbgErr = XPM_INT_ERR_PLL_PERMISSION;
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	}

	DevHandle = Clk->ClkHandles;
	while (NULL != DevHandle) {
		u32 DevPermissionMask = 0U;
		/* Get permission mask which indicates permission for each subsystem */
		Status = XPmDevice_GetPermissions(DevHandle->Device,
						  &DevPermissionMask);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GET_DEVICE_PERMISSION;
			goto done;
		}
		PermissionMask |= DevPermissionMask;
		DevHandle = DevHandle->NextDevice;
	}

	/* Check permission for given subsystem with redundant verification */
	volatile u32 PermCheck = PermissionMask & ((u32)1U << SubsystemIdx);
	volatile u32 PermCheckTmp = PermissionMask & ((u32)1U << SubsystemIdx);
	if ((0U == PermCheck) || (0U == PermCheckTmp)) {
		DbgErr = XPM_INT_ERR_DEVICE_PERMISSION;
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Access is not allowed if resource is shared (multiple subsystems) */
	volatile u32 PopCnt = (u32)__builtin_popcount(PermissionMask);
	volatile u32 PopCntTmp = (u32)__builtin_popcount(PermissionMask);
	if ((SubsystemIdx != 0U) && ((PopCnt > 1U) || (PopCntTmp > 1U))) {
		PmErr("Resource is shared among multiple subsystems: 0x%x\r\n",
				PermissionMask);
		DbgErr = XPM_INT_ERR_SHARED_RESOURCE;
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
XStatus XPmClock_QueryTopology(u32 ClockId, u32 Index, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	const struct XPm_ClkTopologyNode *PtrNodes;
	const XPm_OutClockNode *Clk;
	u8 Type;
	u16 Typeflags;
	u16 Clkflags;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		goto done;
	}

	Status = Xil_SMemSet(Resp, CLK_TOPOLOGY_PAYLOAD_LEN, 0, CLK_TOPOLOGY_PAYLOAD_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

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
			Type =  PtrNodes[Index + i].Type;
			Clkflags = PtrNodes[Index + i].Clkflags;
			Typeflags = PtrNodes[Index + i].Typeflags;

			/**
			 * CCF should not disable unused clocks since all clocks
			 * are managed by PLM and when XPm_InitFinalize() is
			 * called PLM will release all unused devices and
			 * disables its clocks. So add CLK_IGNORE_UNUSED flag
			 * for all GATE to avoid disabling unused clocks from
			 * Linux CCF driver.
			 */
			if ((u8)TYPE_GATE == Type) {
				Clkflags |= CLK_IGNORE_UNUSED;
			}

			/* Set CCF flags to each nodes for read only clock */
			if (0U != (Clk->ClkNode.Flags & CLK_FLAG_READ_ONLY)) {
				if ((u8)TYPE_GATE == Type) {
					Clkflags |= CLK_IS_CRITICAL;
				} else if (((u8)TYPE_DIV1 == Type) || ((u8)TYPE_DIV2 == Type)) {
					Typeflags |= CLK_DIVIDER_READ_ONLY;
				} else if ((u8)TYPE_MUX == Type) {
					Typeflags |= CLK_MUX_READ_ONLY;
				} else {
					PmDbg("Unknown clock type\r\n");
				}
			}

			Resp[i] = Type;
			Resp[i] |= ((u32)Clkflags << CLK_CLKFLAGS_SHIFT);
			Resp[i] |= ((u32)Typeflags << CLK_TYPEFLAGS_SHIFT);
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
	const struct XPm_ClkTopologyNode *Ptr;
	const XPm_OutClockNode *Clk;

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
	const XPm_OutClockNode *Clk;
	u32 i;

	Clk = (XPm_OutClockNode *)XPmClock_GetById(ClockId);
	if ((NULL == Clk) || (!ISOUTCLK(ClockId))) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Status = Xil_SMemSet(Resp, CLK_PARENTS_PAYLOAD_LEN, 0, CLK_PARENTS_PAYLOAD_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Skip parent till index */
	for (i = 0; i < 3U; i++) {
		if (Clk->ClkNode.NumParents == (Index + i)) {
			Resp[i] = 0xFFFFFFFFU;
			break;
		}

		if (Clk->Topology.MuxSources[Index + i] == (u16)CLK_DUMMY_PARENT) {
			Resp[i] = (u32)CLK_DUMMY_PARENT;
		} else {
			Resp[i] = Clk->Topology.MuxSources[Index + i];
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_QueryAttributes(u32 ClockIndex, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Attr = 0;
	u32 InitEnable = 0;
	u32 ClockId = 0;
	const XPm_ClockNode *Clk;

	XPm_ClockNode** ClkNodeList = XPmClock_GetClkList();
	if (NULL == ClkNodeList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}
	u32 MaxClkNodes = XPmClock_GetMaxClkNodes();
	if (ClockIndex > MaxClkNodes) {
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

	/*
	 * CPM/CPM5 power domain may be turned off if CPM CDO is not loaded.
	 * This will break the linux boot as CCF is trying to get clock
	 * data for CPM/CPM5 clocks and firmware returns an error because clock
	 * power domain is not up.
	 * Mark CPM/CPM5 related clocks as invalid to avoid registration with
	 * CCF until this is handled in a more generic way.
	 *
	 * FIXME: This needs to be handled in a more generic way either in
	 * linux or firmware or both.
	 */
	if (ISCPMCLK(ClockIndex)) {
		Attr = 0;
	}

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

	*Resp = XPmClock_GetMaxClkNodes() + 1U;

	return XST_SUCCESS;
}

XStatus XPmClock_GetMaxDivisor(u32 ClockId, u32 DivType, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	const struct XPm_ClkTopologyNode *Ptr;
	const XPm_OutClockNode *Clk;

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
XStatus XPmClock_QueryName(u32 ClockId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	const XPm_ClockNode *Clk;
	const u32 CopySize = CLK_QUERY_NAME_LEN;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		goto done;
	}

	Status = Xil_SMemSet(Resp, CLK_QUERY_NAME_LEN, 0, CLK_QUERY_NAME_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Xil_SMemCpy((char *)Resp, CopySize, &Clk->Name[0], CopySize, CopySize);

done:
	return Status;
}

XStatus XPmClock_SetDivider(const XPm_OutClockNode *Clk, u32 Divider)
{
	volatile XStatus Status = XST_FAILURE;
	const struct XPm_ClkTopologyNode *Ptr;
	XPm_ClockNode *ParentClk = NULL;
	u32 Divider1, Width, Shift;

	Ptr = XPmClock_GetTopologyNode(Clk, (u32)TYPE_DIV1);
	if (Ptr == NULL) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Width = Ptr->Param2.Width;
	Shift = Ptr->Param1.Shift;

	if ((Divider == 0) && !(Ptr->Typeflags & CLK_DIVIDER_ALLOW_ZERO)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Divider1 = Divider & 0xFFFFU;
	if (Divider1 > BITMASK(Width)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Assert PLL reset if parent is PLL */
	ParentClk = XPmClock_GetByIdx(Clk->ClkNode.ParentIdx);
	if ((NULL != ParentClk) && ISPLL(ParentClk->Node.Id)) {
		Status = XPmClockPll_Reset((XPm_PllClockNode *)ParentClk, PLL_RESET_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	XPm_RMW32(Ptr->Reg, BITNMASK(Shift, Width), Divider1 << Shift);

	/* Release PLL reset if parent is PLL */
	if ((NULL != ParentClk) && ISPLL(ParentClk->Node.Id)) {
		Status = XPmClockPll_Reset((XPm_PllClockNode *)ParentClk, PLL_RESET_RELEASE);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Blind write check */
	PmChkRegMask32(Ptr->Reg, BITNMASK(Shift, Width),
		       Divider1 << Shift, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmClock_SetParent(XPm_OutClockNode *Clk, u32 ParentIdx)
{
	XStatus Status = XST_FAILURE;
	const struct XPm_ClkTopologyNode *Ptr;
	XPm_ClockNode *ParentClk = NULL;
	XPm_ClockNode *OldParentClk = NULL;
	u32 OldParentIdx = CLOCK_PARENT_INVALID;

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
	if (NULL == ParentClk) {
		Status = XPM_INVALID_PARENT_CLKID;
		goto done;
	}

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

	Status = XPmClock_GetClockData(Clk, (u32)TYPE_MUX, &OldParentIdx);
	if (XST_SUCCESS != Status) {
		PmErr("Error %d in GetClockData of 0x%x\r\n", Status, Clk->ClkNode.Node.Id);
		goto done;
	}

	/** Bound checking OldParentIdx */
	if (OldParentIdx >= MAX_MUX_PARENTS) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Release old parent */
	OldParentClk = XPmClock_GetByIdx(Clk->Topology.MuxSources[OldParentIdx]);
	if (NULL == OldParentClk) {
		Status = XPM_INVALID_PARENT_CLKID;
		goto done;
	}

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



XStatus XPmClock_Request(const XPm_ClockHandle *ClkHandle)
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

/**
 * @brief  Compute the clock rate for a single node given its parent's rate.
 *
 * @param  ClkNode	Pointer to the clock node
 * @param  ParentRate	Parent clock rate in Hz
 *
 * @return Computed clock rate in Hz, or 0 on failure
 *
 ****************************************************************************/
static u32 XPmClock_ComputeNodeRate(XPm_ClockNode *ClkNode, u32 ParentRate)
{
	XStatus Status = XST_FAILURE;
	u32 Rate = 0U;
	u32 Div = 0U;
	const struct XPm_ClkTopologyNode *DivNode = NULL;

	if ((ClkNode == NULL) || (ParentRate == 0U)) {
		goto done;
	}

	if (ISREFCLK(ClkNode->Node.Id)) {
		Rate = ParentRate;
	} else if (ISPLL(ClkNode->Node.Id)) {
		Status = XPmClockPll_GetParam((XPm_PllClockNode *)ClkNode,
					      (u32)PM_PLL_PARAM_ID_FBDIV, &Div);
		if ((Status != XST_SUCCESS) || (Div == 0U)) {
			goto done;
		}
		Rate = ParentRate * Div;
	} else if (ISOUTCLK(ClkNode->Node.Id)) {
		DivNode = XPmClock_GetTopologyNode((XPm_OutClockNode *)ClkNode,
						   (u32)TYPE_DIV1);
		if (NULL == DivNode) {
			Rate = ParentRate;
			goto done;
		}

		Status = XPmClock_GetClockData((XPm_OutClockNode *)ClkNode,
					       (u32)TYPE_DIV1, &Div);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		if (Div == 0U) {
			if (DivNode->Typeflags & CLK_DIVIDER_ALLOW_ZERO) {
				Rate = ParentRate;
			}
			goto done;
		}

		if (DivNode->Typeflags & CLK_DIVIDER_POWER_OF_TWO) {
			Rate = ParentRate >> Div;
		} else {
			Rate = ParentRate / Div;
		}
	} else {
		Rate = ParentRate;
	}

done:
	return Rate;
}

/**
 * @brief  Iteratively calculate the clock rate for a given clock node.
 *
 * Walks up the parent chain collecting unresolved nodes, then walks
 * back down computing and caching rates. Bounded by MAX_CLK_CHAIN_DEPTH
 * to prevent runaway traversal on malformed topologies.
 *
 * @param  Clk  Pointer to the clock node
 *
 * @return Calculated clock rate in Hz, or 0 on failure
 *
 ****************************************************************************/
u32 XPmClock_CalculateRate(XPm_ClockNode *Clk)
{
	XPm_ClockNode *ClkChain[MAX_CLK_CHAIN_DEPTH];
	u32 ChainDepth = 0U;
	u32 ParentRate = 0U;
	XPm_ClockNode *ClkNode = Clk;

	if (ClkNode == NULL) {
		goto done;
	}

	/* Phase 1: Walk up, collect unresolved clocks */
	while ((ClkNode != NULL) && (ClkNode->ClkRate == 0U) &&
	       (ChainDepth < MAX_CLK_CHAIN_DEPTH)) {
		if (ISOUTCLK(ClkNode->Node.Id) && (ClkNode->NumParents > 1U)) {
			XPmClock_InitParent((XPm_OutClockNode *)ClkNode);
		}
		ClkChain[ChainDepth] = ClkNode;
		ChainDepth++;
		ClkNode = XPmClock_GetByIdx(ClkNode->ParentIdx);
	}

	if (ChainDepth == MAX_CLK_CHAIN_DEPTH) {
		PmWarn("Clock chain depth exceeded %u for Clk 0x%x\r\n",
		       MAX_CLK_CHAIN_DEPTH, Clk->Node.Id);
	}

	/* Use cached rate */
	if (ClkNode != NULL) {
		ParentRate = ClkNode->ClkRate;
	}

	/* Phase 2: Walk back down, compute and cache each rate */
	while (ChainDepth > 0U) {
		ChainDepth--;
		ClkNode = ClkChain[ChainDepth];
		ClkNode->ClkRate = XPmClock_ComputeNodeRate(ClkNode, ParentRate);
		ParentRate = ClkNode->ClkRate;
	}

done:
	return (Clk != NULL) ? Clk->ClkRate : 0U;
}

/**
 * @brief  Get the cached clock rate for a given clock node.
 *
 * @param  ClockId	Clock node ID
 * @param  Rate		Pointer to store the clock rate in Hz
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
XStatus XPmClock_GetRate(u32 ClockId, u32 *Rate)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		goto done;
	}

	*Rate = Clk->ClkRate;
	Status = XST_SUCCESS;

done:
	return Status;
}

#ifdef XILPM_NG_ENABLE_CLK_SCMI

/**
 * XPmClockPll_EvaluateDivider() - Evaluate a divider value and calculate best PLL params
 * @TargetRate: Desired output rate
 * @Divider: Divider value to evaluate (1, 2, 4, 8)
 * @RefClkRate: Reference clock rate (input to PLL)
 * @Fbdiv: Output - calculated FBDIV
 * @FracData: Output - calculated FRAC_DATA
 * @ActualRate: Output - actual achievable rate
 *
 * Return: Absolute difference between ActualRate and TargetRate
 */
static u32 XPmClockPll_EvaluateDivider(u32 TargetRate, u32 Divider, u32 RefClkRate,
                                        u32 *Fbdiv, u32 *FracData, u32 *ActualRate)
{
	u64 RateDiv;
	u32 CalcFbdiv, CalcFracData;
	u64 PllRate;
	u32 OutputRate;
	u32 Error;
	u64 RequiredPllRate = (u64)TargetRate * Divider;

	/* Calculate FBDIV and FRAC_DATA based on required PLL rate */
	RateDiv = ((u64)RequiredPllRate * FRAC_DIV) / RefClkRate;
	CalcFbdiv = (u32)(RateDiv / FRAC_DIV);
	CalcFracData = (u32)(RateDiv % FRAC_DIV);

	/* Clamp FBDIV to valid range */
	if (CalcFbdiv < PLL_FBDIV_MIN) {
		CalcFbdiv = PLL_FBDIV_MIN;
	} else if (CalcFbdiv > PLL_FBDIV_MAX) {
		CalcFbdiv = PLL_FBDIV_MAX;
	}

	/* Calculate actual PLL rate using u64 to prevent overflow */
	PllRate = (u64)RefClkRate * CalcFbdiv;
	if (CalcFracData != 0U) {
		PllRate += ((u64)RefClkRate * CalcFracData) / FRAC_DIV;
	}

	/* Reject rates outside valid VCO operating range */
	if ((PllRate < (u64)PLL_VCO_MIN) || (PllRate > (u64)PLL_VCO_MAX)) {
		OutputRate = 0U;
		Error = 0xFFFFFFFFU;
	} else {
		/* Calculate actual output rate after divider */
		OutputRate = (u32)(PllRate / Divider);

		Error = (OutputRate >= TargetRate) ?
		        (OutputRate - TargetRate) : (TargetRate - OutputRate);
	}

	*Fbdiv = CalcFbdiv;
	*FracData = CalcFracData;
	*ActualRate = OutputRate;

	return Error;
}

/**
 * XPmClockOut_SetRateWithPll() - Set rate for divider clock with PLL parent
 * @Clk: The divider clock node (e.g., mmi_pll_out)
 * @ClkRate: Desired rate
 * @Flags: Rate setting flags
 *
 * Uses greedy algorithm to find best divider and PLL parameters:
 * 1. Try all possible divider values (1, 2, 4, 8)
 * 2. Select combination that gives closest rate
 * 3. Program PLL (FBDIV + FRAC_DATA) and divider
 * 4. Update clock rates
 *
 * Return: XST_SUCCESS or error code
 */
static XStatus XPmClockOut_SetRateWithPll(XPm_ClockNode *Clk, const u32 ClkRate, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *PllClk;
	XPm_ClockNode *RefClk;
	u32 RefClkRate;
	u32 BestDivIdx = 0U;
	u32 BestFbdiv = 0U;
	u32 BestFracData = 0U;
	u32 BestError = 0xFFFFFFFFU;
	u32 i, Fbdiv, FracData, ActualRate, Error;
	u64 PllRate;
	u32 PllMode;

	(void)Flags;

	/* Parent should be a PLL */
	PllClk = XPmClock_GetByIdx(Clk->ParentIdx);
	if ((NULL == PllClk) || (!ISPLL(PllClk->Node.Id))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	RefClk = XPmClock_GetById(PM_CLK_REF_CLK);
	if (NULL == RefClk) {
		goto done;
	}

	RefClkRate = RefClk->ClkRate;
	if (0U == RefClkRate) {
		Status = XST_FAILURE;
		goto done;
	}

	/* PLL_OUT valid divider values: 1, 2, 4, 8 */
	for (i = 0U; i < PLL_NUM_DIVIDERS; i++) {
		Error = XPmClockPll_EvaluateDivider(ClkRate, 1U << i, RefClkRate,
		                                     &Fbdiv, &FracData, &ActualRate);

		if (Error < BestError) {
			BestError = Error;
			BestDivIdx = i;
			BestFbdiv = Fbdiv;
			BestFracData = FracData;
		}

		/* Exit early if perfect match found */
		if (0U == Error) {
			break;
		}
	}

	/* Check if we found a valid solution */
	if (BestFbdiv == 0U) {
		Status = XST_FAILURE;
		goto done;
	}

	/* Determine PLL mode based on fractional data */
	PllMode = (BestFracData != 0U) ?
	          (u32)PM_PLL_MODE_FRACTIONAL :
	          (u32)PM_PLL_MODE_INTEGER;

	/* Set PLL mode */
	Status = XPmClockPll_SetMode((XPm_PllClockNode *)PllClk, PllMode);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Set FBDIV (common to both modes) */
	Status = XPmClockPll_SetParam((XPm_PllClockNode *)PllClk,
	                               (u32)PM_PLL_PARAM_ID_FBDIV, BestFbdiv);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Set fractional data only if in fractional mode */
	if (BestFracData != 0U) {
		Status = XPmClockPll_SetParam((XPm_PllClockNode *)PllClk,
		                               (u32)PM_PLL_PARAM_ID_DATA, BestFracData);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Program divide */
	Status = XPmClock_SetDivider((XPm_OutClockNode *)Clk, BestDivIdx);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Update clock rates using u64 to prevent overflow */
	PllRate = (u64)RefClkRate * BestFbdiv;
	if (BestFracData != 0U) {
		PllRate += ((u64)RefClkRate * BestFracData) / FRAC_DIV;
	}

	PllClk->ClkRate = (u32)PllRate;
	Clk->ClkRate = (u32)(PllRate / (1U << BestDivIdx));

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * XPmClockOut_SetRate() - Set the rate of an output clock
 * @Clk: Clock node to configure
 * @ClkRate: Desired clock rate in Hz
 * @Flags: Rate setting flags
 *
 * Return: XST_SUCCESS on success, error code otherwise
 */
static XStatus XPmClockOut_SetRate(XPm_ClockNode *Clk, const u32 ClkRate, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	const struct XPm_ClkTopologyNode *DivNode = NULL;
	u32 ParentRate = 0U;
	XPm_ClockNode *ParentClk = NULL;
	u32 Div = 0U;

	if ((Clk == NULL) || (ClkRate == 0U)) {
		goto done;
	}
	/**
	 * @todo Remove this once parent change is supported by SCMI.
	 *
	 * Since parent change is not supported by SCMI, initialise the
	 * parent explicitly for clocks that have multiple parents.
	 */
	if (Clk->NumParents > 1U) {
		XPmClock_InitParent((XPm_OutClockNode *)Clk);
	}

	ParentClk = XPmClock_GetByIdx(Clk->ParentIdx);
	if (NULL == ParentClk) {
		goto done;
	}

	DivNode = XPmClock_GetTopologyNode((XPm_OutClockNode *)Clk, (u32)TYPE_DIV1);
	if (NULL == DivNode) {
		/* No divider node - propagate rate change to parent for certain clocks */
		if (IS_RATE_CHANGE_ALLOWED(Clk->Node.Id)) {
			Status = XPmClockOut_SetRate(ParentClk, ClkRate, Flags);
			if (XST_SUCCESS == Status) {
				Clk->ClkRate = ParentClk->ClkRate;
			}
		}
		goto done;
	}

	/* Check if parent is a PLL - so clock is PLL_OUT */
	if (ISPLL(ParentClk->Node.Id)) {
		Status = XPmClockOut_SetRateWithPll(Clk, ClkRate, Flags);
		goto done;
	}

	/* Normal divider case - calculate required divider value */
	ParentRate = ParentClk->ClkRate;
	Div = ParentRate / ClkRate;

	/* Apply rounding based on divider type flags */
	if (DivNode->Typeflags & CLK_DIVIDER_ROUND_CLOSEST) {
		u32 Remainder = ParentRate % ClkRate;
		if ((Remainder << 1U) >= ClkRate) {
			Div += 1U;
		}
	} else if (ParentRate % ClkRate != 0U) {
		Div += 1U;
	}

	if (Div == 0U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_SetDivider((XPm_OutClockNode *)Clk, Div);
	if (XST_SUCCESS == Status) {
		Clk->ClkRate = ParentRate / Div;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the rate of the clock.
 *
 * @param ClockId	Clock node ID
 * @param ClkRate	Clock rate
 * @param Flags		Flags for setting clock rate
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmClock_ProgramRate(u32 ClockId, u32 ClkRate, u32 Flags)
{
	XStatus Status = XST_FAILURE;

	if (!IS_RATE_CHANGE_ALLOWED(ClockId)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClockOut_SetRate(Clk, ClkRate, Flags);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Describe supported clock rates for a given clock
 *
 * This function returns clock rate information in one of two formats
 * depending on the clock type.
 *
 * @param ClockId   Clock node ID to query
 * @param RateIdx   Index of the rate to describe (reserved for future use)
 * @param Response  Output array (minimum 4 elements):
 *                  Response[0]: Format and count flags
 *                               - Bits [11:0]:  Number of rates returned (3 for range, 1 for single)
 *                               - Bit  [12]:    Format (1 = range triplet, 0 = discrete list)
 *                               - Bits [15:13]: Reserved
 *                               - Bits [31:16]: Number of remaining rates after this call
 *
 *                  Range format (bit 12 = 1):
 *                    Response[1]: Minimum rate in Hz
 *                    Response[2]: Maximum rate in Hz
 *                    Response[3]: Step size in Hz
 *
 *                  Discrete format (bit 12 = 0):
 *                    Response[1]: Rate in Hz (or first discrete rate)
 *                    Response[2]: 0 (unused, or second discrete rate)
 *                    Response[3]: 0 (unused, or third discrete rate)
 *
 * @return XST_SUCCESS always
 *
 * @note Follows SCMI CLOCK_DESCRIBE_RATES format
 *
 ****************************************************************************/
XStatus XPmClock_DescribeRate(u32 ClockId, u32 RateIdx, u32 *Response)
{
	XStatus Status = XST_FAILURE;
	(void)RateIdx; /* RateIdx is not used for now, kept for future use */

	if ((PM_CLK_GEM0_TX == ClockId) || (PM_CLK_GEM1_TX == ClockId)) {
		Response[0] = CLK_DESCRIBE_RATE_FORMAT(CLK_RATES_TRIPLET, CLK_RANGE_FORMAT);
		Response[1] = 2500000U;    /* Min: 2.5 MHz */
		Response[2] = 125000000U;  /* Max: 125 MHz */
		Response[3] = 2500000U;    /* Step: 2.5 MHz */
	} else if (PM_CLK_MMIPLL == ClockId) {
		Response[0] = CLK_DESCRIBE_RATE_FORMAT(CLK_RATES_TRIPLET, CLK_RANGE_FORMAT);
		Response[1] = 189000000U;   /* Min: 189 MHz */
		Response[2] = 2997000000U;  /* Max: 2.997 GHz */
		Response[3] = 100000U;      /* Step: 100 kHz */
	} else if (PM_CLK_DC_PIXEL == ClockId) {
		Response[0] = CLK_DESCRIBE_RATE_FORMAT(CLK_RATES_TRIPLET, CLK_RANGE_FORMAT);
		Response[1] = 25175000U;   /* Min: 25.175 MHz */
		Response[2] = 597000000U;  /* Max: 597 MHz */
		Response[3] = 1000U;       /* Step: 1 kHz */
	} else if (PM_CLK_DC_REF == ClockId) {
		Response[0] = CLK_DESCRIBE_RATE_FORMAT(CLK_RATES_TRIPLET, CLK_RANGE_FORMAT);
		Response[1] = 2822400U;   /* Min: 2.8224 MHz */
		Response[2] = 27000000U;  /* Max: 27 MHz */
		Response[3] = 4800U;      /* Step: 4.8 kHz */
	} else if (PM_CLK_MMI_AUX1_REF == ClockId) {
		Response[0] = 1U;         /* Single discrete rate */
		Response[1] = 27000000U;  /* 27 MHz */
	} else {
		Response[0] = 1U;  /* Single rate */
		XPmClock_GetRate(ClockId, &Response[1]);
	}
	Status = XST_SUCCESS;

	return Status;
}

#endif /* XILPM_NG_ENABLE_CLK_SCMI */
