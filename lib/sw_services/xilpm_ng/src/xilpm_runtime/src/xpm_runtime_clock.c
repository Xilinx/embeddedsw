/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
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
		Clk->UseCount++;
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
		PmDecrement(Clk->UseCount);

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
	XStatus Status = XST_FAILURE;
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

	/* Check for read-only flag */
	if (0U != (CLK_FLAG_READ_ONLY & Clk->Flags)) {
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
		/* Get permission mask which indicates permission for each subsystem */
		Status = XPmDevice_GetPermissions(DevHandle->Device,
						  &PermissionMask);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_GET_DEVICE_PERMISSION;
			goto done;
		}

		DevHandle = DevHandle->NextDevice;
	}

	/* Check permission for given subsystem */
	if (0U == (PermissionMask & ((u32)1U << SubsystemIdx))) {
		DbgErr = XPM_INT_ERR_DEVICE_PERMISSION;
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Access is not allowed if resource is shared (multiple subsystems) */
	if (SubsystemIdx != 0 && __builtin_popcount(PermissionMask) > 1) {
		PmErr("Resource is shared among multiple subsystems\r\n");
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
