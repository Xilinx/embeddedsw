/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_reset.h"
#include "xpm_runtime_reset.h"
#include "xpm_debug.h"
#include "xpm_domain_iso.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_common.h"
#include "xplmi.h"
#include "xpm_subsystem.h"
#include "xpm_runtime_device.h"
#include "xpm_update.h"
#include "xzdma_hw.h"
#include <stdlib.h>

static XPmRuntime_ResetList *RuntimeResetList XPM_INIT_DATA(RuntimeResetList) =  NULL;

/*
 * In the case of default subsystem with a pre-defined set of requirements,
 * add entire list of resets that are allowed to have permissions to the
 * default subsystem.
 */
static const u32 PermissionResets[] = {
	PM_RST_PMC_POR,
	PM_RST_PMC,
	PM_RST_SYS_RST_1,
	PM_RST_SYS_RST_2,
	PM_RST_SYS_RST_3,
	PM_RST_PL_POR,
	PM_RST_NOC_POR,
	PM_RST_PL_SRST,
	PM_RST_NOC,
	PM_RST_NPI,
	PM_RST_PL0,
	PM_RST_PL1,
	PM_RST_PL2,
	PM_RST_PL3,
	PM_RST_ADMA,
	PM_RST_PCIE_CORE0,
	PM_RST_PCIE_CORE1,
	PM_RST_PCIE_0,
	PM_RST_PCIE_1,
	PM_RST_PCIE_2,
	PM_RST_PCIE_3,
};

static XStatus XPmReset_SetAllowedSubsystems(XPm_ResetNode *Reset, const u32 AllowSubsystems);
static XStatus XPmReset_GetAllowedSubsystems(const XPm_ResetNode *Reset, u32 *AllowSubsystems);


#define XILPM_ZDMA_CH_ISR_OFFSET	(0x408U)	/* Channel ISR offset */
#define XILPM_ZDMA_CH_IDS_OFFSET	(0x414U)	/* Channel IDS offset */
#define XPM_MAX_TIMEOUT			(0x1FFFFFFFU)	/* Timeout */

static XStatus AdmaResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal = 0U, LocalTimeout;
	u32 BaseAddress;

	BaseAddress = Rst->Node.BaseAddress;

	/* Disable/stop the Channel */
	XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL2_OFFSET, XZDMA_CH_CTRL2_DIS_MASK);

	/* Wait till transfers are not completed or halted */
	LocalTimeout = XPM_MAX_TIMEOUT;
	do {
		RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_STS_OFFSET) & XZDMA_STS_BUSY_MASK;
		LocalTimeout--;
	} while ((0U != RegVal) && (LocalTimeout > 0U));

	if (0U != RegVal) {
		goto done;
	}

	/* Disable and clear all interrupts */
	XZDma_WriteReg(BaseAddress, XILPM_ZDMA_CH_IDS_OFFSET, XZDMA_IXR_ALL_INTR_MASK);

	RegVal = XZDma_ReadReg(BaseAddress, XILPM_ZDMA_CH_ISR_OFFSET);
	XZDma_WriteReg(BaseAddress, XILPM_ZDMA_CH_ISR_OFFSET, (RegVal & XZDMA_IXR_ALL_INTR_MASK));

	/* Reset all the configurations */
	XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL0_OFFSET, XZDMA_CTRL0_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL1_OFFSET, XZDMA_CTRL1_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DATA_ATTR_OFFSET, XZDMA_DATA_ATTR_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, XZDMA_DSCR_ATTR_RESET_VALUE);

	/* Clears total byte transferred */
	RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET, RegVal);

	/*
	 * Read interrupt counts to clear it on both source and
	 * destination Channels
	 */
	(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_SRC_ACCT_OFFSET);
	(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_DST_ACCT_OFFSET);

	/* Reset the channel's coherent attributes. */
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, 0x0);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_SRC_DSCR_WORD3_OFFSET, 0x0);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DST_DSCR_WORD3_OFFSET, 0x0);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AdmaResetPulse(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;

	Status = AdmaResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPm_RMW32(ControlReg, Mask, 0U);

done:
	return Status;
}

static const void *GetResetCustomOps(u32 ResetId)
{
	u16 Idx;
	const struct ResetCustomOps *RstCustomStatus = NULL;
	static const struct ResetCustomOps Reset_Custom[] = {
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_0,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_1,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_2,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_3,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_4,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_5,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_6,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_7,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
	};

	for (Idx = 0U; Idx < ARRAY_SIZE(Reset_Custom); Idx++) {
		if (Reset_Custom[Idx].ResetIdx == NODEINDEX(ResetId)) {
			RstCustomStatus = &Reset_Custom[Idx];
			break;
		}
	}
	return RstCustomStatus;
}

static XStatus Reset_AssertCommon(XPm_ResetNode *Rst, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;
	const struct ResetCustomOps *Ops = GetResetCustomOps(Rst->Node.Id);

	switch (Action) {
	case (u32)PM_RESET_ACTION_RELEASE:
		if ((NULL != Ops) && (NULL != Ops->ActionRelease)) {
			Status = Ops->ActionRelease(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, 0U);
		}
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_ASSERT:
		if ((NULL != Ops) && (NULL != Ops->ActionAssert)) {
			Status = Ops->ActionAssert(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, Mask);
		}
		Rst->Node.State = XPM_RST_STATE_ASSERTED;
		Status = XST_SUCCESS;
		break;
	case (u32)PM_RESET_ACTION_PULSE:
		if ((NULL != Ops) && (NULL != Ops->ActionPulse)) {
			Status = Ops->ActionPulse(Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			XPm_RMW32(ControlReg, Mask, Mask);
			//Wait for xms ??
			XPm_RMW32(ControlReg, Mask, 0U);
		}
		Rst->Node.State = XPM_RST_STATE_DEASSERTED;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	};

done:
	if (XST_SUCCESS != Status) {
		PmErr("RstId: 0x%x, Action: 0x%x Status: 0x%x\n\r",
			Rst->Node.Id, Action, Status);
	}

	return Status;
}

XStatus XPmReset_AssertbyId(u32 ResetId, const u32 Action)
{
	XStatus Status = XST_FAILURE;
	XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (NULL != Rst) {
		Status = Reset_AssertCommon(Rst, Action);
	} else {
		Status = XST_FAILURE;
	}
	if (XST_SUCCESS != Status) {
		PmErr("ResetId = %x 0x%x\n\r", ResetId, Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Read the hardware assertion state of a reset node.
 *
 * @param  ResetId  Reset node ID
 * @param  OutState Required non-NULL pointer to receive 1 when all reset bits
 *                  are asserted, or 0 otherwise
 *
 * @return XST_SUCCESS on success, or XST_INVALID_PARAM when the reset node
 *         does not exist
 *
 ****************************************************************************/
XStatus XPmReset_GetStateById(u32 ResetId, u32* OutState)
{
	XStatus Status = XST_FAILURE;

	/* Reject IDs that do not resolve to a reset node */
	const XPm_ResetNode* Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Report assertion only when every hardware bit in the reset field is set */
	u32 Mask = BITNMASK(Reset->Shift, Reset->Width);
	if ((XPm_Read32(Reset->Node.BaseAddress) & Mask) == Mask) {
		*OutState = 1U;
	} else {
		*OutState = 0U;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

/*
 * These resets are specifically allowed and none else for permissions
 * policy as these are the most common cases for users.
 *
 * The list of nodes may have elements removed or added in the future.
 */
XStatus XPmReset_IsPermissionReset(const u32 ResetId)
{
	u32 Index;
	XStatus Status = XPM_PM_NO_ACCESS;

	for (Index = 0; Index < (ARRAY_SIZE(PermissionResets)); Index++) {
		if (ResetId == PermissionResets[Index]) {
			Status = XST_SUCCESS;
			goto done;
		}
	}

done:
	return Status;
}

XStatus XPmReset_AddPermission(XPm_ResetNode *Rst,
			       const XPm_Subsystem *Subsystem,
			       const u32 Operations)
{
	XStatus Status = XST_FAILURE;
	u32 AllowedSubsystems = 0U;

	/* PMC and default subsystem can always enact operations */
	if ((NULL == Subsystem) || (PM_SUBSYS_PMC == Subsystem->Id)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmReset_IsPermissionReset(Rst->Node.Id);
	if (XST_SUCCESS != Status) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	AllowedSubsystems |=  PERM_BITMASK(Operations, RESET_PERM_SHIFT_NS,
					SUBSYS_TO_NS_BITPOS(Subsystem->Id));
	AllowedSubsystems |=  PERM_BITMASK(Operations, RESET_PERM_SHIFT_S,
					SUBSYS_TO_S_BITPOS(Subsystem->Id));

	Status = XPmReset_SetAllowedSubsystems(Rst, AllowedSubsystems);

done:
	return Status;
}

/**
 * @brief  Grant the subsystem secure and non-secure permission for every
 *         reset in the global permission-reset list.
 *
 * @param  Subsystem	Subsystem to be given the reset permissions
 *
 * @return XST_SUCCESS on success, error code otherwise
 */
XStatus XPmReset_AddPermForGlobalResets(const XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	u32 Index, Flags;
	XPm_ResetNode *Rst = NULL;
	Flags = (1UL << RESET_PERM_SHIFT_NS) | (1UL << RESET_PERM_SHIFT_S);

	for (Index = 0U; Index < (ARRAY_SIZE(PermissionResets)); Index++) {
		Rst = XPmReset_GetById( PermissionResets[Index]  );
		if (NULL != Rst) {
			Status = XPmReset_AddPermission(Rst, Subsystem, Flags);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

done:
	return Status;
}
XStatus XPmReset_CheckPermissions(const XPm_Subsystem *Subsystem, u32 ResetId)
{
	XStatus Status = XST_FAILURE;
	u32 DevId;
	const XPm_ResetHandle *DevHandle;
	const XPm_ResetNode *Rst = XPmReset_GetById(ResetId);

	if (NULL == Rst) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * AIE resets have no device association since PM_DEV_AIE is deprecated.
	 * Allow these resets unconditionally for any subsystem.
	 */
	if ((PM_RST_AIE_ARRAY == ResetId) || (PM_RST_AIE_SHIM == ResetId)) {
		Status = XST_SUCCESS;
		goto done;
	}

	DevHandle = Rst->RstHandles;
	while (NULL != DevHandle) {
		DevId = DevHandle->Device->Node.Id;
		if ((u32)XPM_DEVSTATE_RUNNING == DevHandle->Device->Node.State) {
			Status = XPmDevice_CheckPermissions(Subsystem, DevId);
			if (XST_SUCCESS == Status) {
				goto done;
			}
		}
		DevHandle = DevHandle->NextDevice;
	}

done:
	return Status;
}
XStatus XPmReset_IsOperationAllowed(const u32 SubsystemId,
				    const XPm_ResetNode *Rst,
				    const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}
	if (NULL == Rst) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	u32 AllowedSubsystems = 0;
	Status = XPmReset_GetAllowedSubsystems(Rst, &AllowedSubsystems);
	if (XST_SUCCESS != Status) {;
		goto done;
	}
	/* Have Target check if Host can enact the operation */
	if ((XPLMI_CMD_SECURE == CmdType) &&
	    (0U != (AllowedSubsystems & ((u32)1U << SUBSYS_TO_S_BITPOS(SubsystemId))))) {
		Status = XST_SUCCESS;
	} else if (0U != (AllowedSubsystems & ((u32)1U << SUBSYS_TO_NS_BITPOS(SubsystemId)))) {
		Status = XST_SUCCESS;
	} else {
		Status = XPM_PM_NO_ACCESS;
	}

done:
	return Status;
}
XStatus XPmReset_SystemReset(void)
{
	XStatus Status = XST_FAILURE;

	/* TODO: Confirm if idling is required here or not */

	Status = XPmReset_AssertbyId(PM_RST_PMC, (u32)PM_RESET_ACTION_ASSERT);

	return Status;
}

/**
 * @brief Find the runtime permission entry associated with a reset node.
 *
 * Keep list traversal in one place so permission updates and queries use
 * the same matching and NULL-entry handling.
 *
 * @param Reset Reset node to look up.
 *
 * @return Matching runtime entry, or NULL when none exists.
 */
static XPmRuntime_Reset *XPmReset_GetRuntimeReset(const XPm_ResetNode *Reset)
{
	XPmRuntime_Reset *RuntimeReset = NULL;

	/* An uninitialized list has no reset entries to search. */
	if (NULL != RuntimeResetList) {
		LIST_FOREACH(RuntimeResetList, RstNode) {
			/* Ignore empty list nodes and match entries by reset identity. */
			if ((NULL != RstNode->Data) &&
			    (RstNode->Data->Device == Reset)) {
				RuntimeReset = RstNode->Data;
				break;
			}
		}
	}

	return RuntimeReset;
}

/****************************************************************************/
/**
 * @brief  Add subsystem permission bits to a reset's runtime entry.
 *
 * A runtime list and entry are allocated when needed. Existing permissions
 * are preserved and combined with AllowSubsystems.
 *
 * @param  Reset            Pointer to the reset node to update
 * @param  AllowSubsystems  Permission bits to add
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL reset, or
 *         XST_BUFFER_TOO_SMALL if runtime storage cannot be allocated
 *
 ****************************************************************************/
static XStatus XPmReset_SetAllowedSubsystems(XPm_ResetNode *Reset, const u32 AllowSubsystems)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_Reset *RuntimeReset = NULL;

	/* Reject an update that has no reset node to identify its entry */
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Create the shared runtime list on its first permission update */
	if (NULL == RuntimeResetList) {
		RuntimeResetList = Make_XPmRuntime_ResetList();
		if (NULL == RuntimeResetList) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
	}

	/* Reuse an existing entry so its permission bits are preserved */
	RuntimeReset = XPmReset_GetRuntimeReset(Reset);

	/* Allocate and initialize storage only for a new reset entry */
	if (NULL == RuntimeReset) {
		RuntimeReset = (XPmRuntime_Reset* )XPm_AllocBytesDevOps(sizeof(XPmRuntime_Reset));
		if (NULL == RuntimeReset) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		/* Set the allowed subsystems */
		RuntimeReset->Device = Reset;
		RuntimeReset->AllowedSubsystems = AllowSubsystems;
		LIST_PREPEND(RuntimeResetList, RuntimeReset);
	}
	else {
		/* Update the allowed subsystems */
		RuntimeReset->AllowedSubsystems |= AllowSubsystems;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Retrieve a reset node's stored subsystem permission bits.
 *
 * @param  Reset            Pointer to the reset node to look up
 * @param  AllowSubsystems  Pointer to receive the permission bit mask
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL argument, or
 *         XST_DEVICE_NOT_FOUND when no runtime entry exists for the reset
 *
 ****************************************************************************/
static XStatus XPmReset_GetAllowedSubsystems(const XPm_ResetNode *Reset, u32 *AllowSubsystems)
{
	XStatus Status = XST_FAILURE;
	const XPmRuntime_Reset *RuntimeReset = NULL;

	/* Validate the lookup key and result destination together */
	if ((NULL == Reset) || (NULL == AllowSubsystems)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* An absent runtime list cannot contain permissions for this reset */
	if (NULL == RuntimeResetList) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Locate the runtime entry associated with the reset node */
	RuntimeReset = XPmReset_GetRuntimeReset(Reset);
	if (NULL == RuntimeReset) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Return the complete permission mask stored for the matched entry */
	*AllowSubsystems = RuntimeReset->AllowedSubsystems;
	Status = XST_SUCCESS;
done:
	return Status;
}
