/******************************************************************************
 * Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpm_access.h"
#include "xpm_common.h"
#include "xpm_api.h"
#include "xpm_device.h"
#include "xpm_subsystem.h"
#include "xplmi.h"
#ifdef XPM_ENABLE_PLM_TO_PSM_FORWARDING
#include "xpm_ipi.h"
#include "xpm_psm.h"
#endif

#ifdef VERSAL_NET
#include "xplmi_update.h"
#include "xpm_update_data.h"
#include "xpm_update.h"
#endif

static XPm_RegNode *PmRegnodes;
static u32 PmRegnodeCount;
static XPm_NodeAccess *PmNodeAccessTable;
static u32 PmNodeAccessCount;
static u32 PmApertureCount;

#ifdef VERSAL_NET
/**
 * Structure to save regnode and access table state for IPU.
 * The actual data is in ByteBuffer which is saved separately.
 */
typedef struct {
	u32 PmRegnodesPtr;        /**< Head of regnode linked list */
	u32 PmRegnodeCount;       /**< Number of regnodes in list */
	u32 PmNodeAccessTablePtr; /**< Head of access table linked list */
	u32 PmNodeAccessCount;    /**< Number of access table entries */
	u32 PmApertureCount;      /**< Total number of apertures across all entries */
} XPm_AccessPtrs;

static XPm_AccessPtrs AccessPtrs;

/**
 * @brief Handler for saving regnode and access table pointers during IPU
 *
 * Only saves pointers during STORE. The actual restore is done later
 * via XPmAccess_RestoreRegnodes() called from XPmUpdate_RestoreAllNodes()
 * after ByteBuffer has been fully restored.
 */
static int XPmAccess_PtrOps(u32 Op, u64 Addr, void *Data)
{
	int Status = XST_FAILURE;

	if (Op == XPLMI_STORE_DATABASE) {
		/* Save head pointers and counts */
		AccessPtrs.PmRegnodesPtr = (u32)(UINTPTR)PmRegnodes;
		AccessPtrs.PmRegnodeCount = PmRegnodeCount;
		AccessPtrs.PmNodeAccessTablePtr = (u32)(UINTPTR)PmNodeAccessTable;
		AccessPtrs.PmNodeAccessCount = PmNodeAccessCount;
		AccessPtrs.PmApertureCount = PmApertureCount;

		PmDbg("IPU: Save regnodes=%u access=%u apertures=%u\n\r",
			AccessPtrs.PmRegnodeCount, AccessPtrs.PmNodeAccessCount,
			AccessPtrs.PmApertureCount);

		/* Use default handler to copy to DDR */
		Status = XPlmi_DsOps(Op, Addr, Data);
	}
	else if (Op == XPLMI_RESTORE_DATABASE) {
		/* Just restore the saved pointers from DDR - actual pointer fixup
		 * happens later in XPmAccess_RestoreRegnodes() after ByteBuffer restore */
		Status = XPlmi_DsOps(Op, Addr, Data);
	}
	else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto done;
	}

done:
	return Status;
}

EXPORT_DS_W_HANDLER(AccessPtrs,
	XPLMI_MODULE_XILPM_ID, XPM_REGNODES_DS_ID,
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION,
	sizeof(AccessPtrs), (u32)(UINTPTR)(&AccessPtrs), XPmAccess_PtrOps);

/**
 * @brief Advance to the next node in a saved linked list
 *
 * Returns the offset-adjusted next pointer, or NULL if the raw
 * pointer is NULL or fails conversion.
 * Uses XPm_ConvertToSavedAddress() for the actual conversion.
 *
 * @param RawPtr The raw pointer value from saved data
 * @return Adjusted pointer, or NULL if invalid
 */
static inline void *XPmAccess_NextSaved(u32 RawPtr)
{
	u32 Adjusted;

	if (RawPtr == 0U) {
		return NULL;
	}

	Adjusted = XPm_ConvertToSavedAddress(RawPtr);
	if (0U == Adjusted) {
		return NULL;
	}

	return (void *)(UINTPTR)Adjusted;
}

/**
 * @brief Restore regnodes and access table after In-Place PLM Update
 *
 * Deep-copies regnode and access table linked lists from the DDR-saved
 * ByteBuffer into fresh ByteBuffer allocations. Must be called after
 * ByteBuffer restore is complete.
 *
 * @return XST_SUCCESS if all entries restored, error code otherwise
 */
XStatus XPmAccess_RestoreRegnodes(void)
{
	XStatus Status = XST_FAILURE;
	u32 RegnodeCount = 0U;
	u32 AccessCount = 0U;
	u32 ApertureCount = 0U;

	PmDbg("IPU: Restore regnodes=%u access=%u apertures=%u\n\r",
		AccessPtrs.PmRegnodeCount, AccessPtrs.PmNodeAccessCount,
		AccessPtrs.PmApertureCount);

	/*
	 * Deep copy regnodes from DDR saved ByteBuffer into fresh
	 * ByteBuffer allocations. The DDR reserved region is reused
	 * on the next IPU, so regnodes must live in ByteBuffer.
	 *
	 * Lists are rebuilt using append to preserve original order.
	 */
	PmRegnodes = NULL;
	PmNodeAccessTable = NULL;

	/* Restore regnodes */
	if (AccessPtrs.PmRegnodesPtr != 0U) {
		XPm_RegNode *SavedNode = (XPm_RegNode *)XPmAccess_NextSaved(
			AccessPtrs.PmRegnodesPtr);
		XPm_RegNode *Tail = NULL;
		if (NULL == SavedNode) {
			Status = XST_FAILURE;
			goto done;
		}

		while ((NULL != SavedNode) && (RegnodeCount < AccessPtrs.PmRegnodeCount)) {
			u32 SavedPowerPtr = (u32)(UINTPTR)SavedNode->Power;
			u32 SavedNextRegnodePtr = (u32)(UINTPTR)SavedNode->NextRegnode;
			void *PowerAddr;
			u32 PowerId;
			XPm_RegNode *NewNode = (XPm_RegNode *)XPm_AllocBytes(sizeof(XPm_RegNode));
			if (NULL == NewNode) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			NewNode->Id = SavedNode->Id;
			NewNode->BaseAddress = SavedNode->BaseAddress;
			NewNode->Requirements = SavedNode->Requirements;
			NewNode->NextRegnode = NULL;

			/* Resolve power parent by ID. All regnodes must have valid power. */
			if (SavedPowerPtr == 0U) {
				PmErr("IPU: Regnode 0x%08x has NULL power parent\n\r",
					SavedNode->Id);
				Status = XST_FAILURE;
				goto done;
			}
			PowerAddr = XPmAccess_NextSaved(SavedPowerPtr);
			if (NULL == PowerAddr) {
				PmErr("IPU: Cannot resolve power ptr 0x%08x for regnode 0x%08x\n\r",
					SavedPowerPtr, SavedNode->Id);
				Status = XST_FAILURE;
				goto done;
			}
			PowerId = ((XPm_Node *)PowerAddr)->Id;
			NewNode->Power = XPmPower_GetById(PowerId);
			if (NULL == NewNode->Power) {
				PmErr("IPU: Power ID 0x%08x not found for regnode 0x%08x\n\r",
					PowerId, SavedNode->Id);
				Status = XST_FAILURE;
				goto done;
			}

			/* Append to preserve original list order */
			if (NULL == PmRegnodes) {
				PmRegnodes = NewNode;
			} else {
				Tail->NextRegnode = NewNode;
			}
			Tail = NewNode;

			PmDbg("  RegNode: 0x%08x @ 0x%08x\n\r",
				NewNode->Id, NewNode->BaseAddress);

			if (SavedNextRegnodePtr != 0U) {
				SavedNode = (XPm_RegNode *)XPmAccess_NextSaved(
					SavedNextRegnodePtr);
				if (NULL == SavedNode) {
					Status = XST_FAILURE;
					goto done;
				}
			}
			else {
				SavedNode = NULL;
			}
			RegnodeCount++;
		}
	}

	/* Validate regnode count matches saved count */
	if (RegnodeCount != AccessPtrs.PmRegnodeCount) {
		PmErr("IPU: regnode count %u != %u\n\r",
			RegnodeCount, AccessPtrs.PmRegnodeCount);
		Status = XST_FAILURE;
		goto done;
	}

	/* Restore access table */
	if (AccessPtrs.PmNodeAccessTablePtr != 0U) {
		XPm_NodeAccess *SavedEntry = (XPm_NodeAccess *)XPmAccess_NextSaved(
			AccessPtrs.PmNodeAccessTablePtr);
		XPm_NodeAccess *TableTail = NULL;
		if (NULL == SavedEntry) {
			Status = XST_FAILURE;
			goto done;
		}

		while ((NULL != SavedEntry) && (AccessCount < AccessPtrs.PmNodeAccessCount)) {
			u32 SavedAperturePtr = (u32)(UINTPTR)SavedEntry->Aperture;
			u32 SavedNextEntryPtr = (u32)(UINTPTR)SavedEntry->NextNode;
			XPm_NodeAccess *NewEntry = (XPm_NodeAccess *)XPm_AllocBytes(sizeof(XPm_NodeAccess));
			if (NULL == NewEntry) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			NewEntry->Id = SavedEntry->Id;
			NewEntry->Aperture = NULL;
			NewEntry->NextNode = NULL;

			/* Deep copy aperture chain */
			if (SavedAperturePtr != 0U) {
				XPm_NodeAper *SavedAper = (XPm_NodeAper *)XPmAccess_NextSaved(
					SavedAperturePtr);
				XPm_NodeAper *LastAper = NULL;
				if (NULL == SavedAper) {
					Status = XST_FAILURE;
					goto done;
				}

				while ((NULL != SavedAper) && (ApertureCount < AccessPtrs.PmApertureCount)) {
					u32 SavedNextAperPtr = (u32)(UINTPTR)SavedAper->NextAper;
					XPm_NodeAper *NewAper = (XPm_NodeAper *)XPm_AllocBytes(sizeof(XPm_NodeAper));
					if (NULL == NewAper) {
						Status = XST_BUFFER_TOO_SMALL;
						goto done;
					}
					NewAper->Offset = SavedAper->Offset;
					NewAper->Size = SavedAper->Size;
					NewAper->Access = SavedAper->Access;
					NewAper->NextAper = NULL;
					if (NULL == NewEntry->Aperture) {
						NewEntry->Aperture = NewAper;
					} else {
						LastAper->NextAper = NewAper;
					}
					LastAper = NewAper;

					if (SavedNextAperPtr != 0U) {
						SavedAper = (XPm_NodeAper *)XPmAccess_NextSaved(
							SavedNextAperPtr);
						if (NULL == SavedAper) {
							Status = XST_FAILURE;
							goto done;
						}
					}
					else {
						SavedAper = NULL;
					}
					ApertureCount++;
				}
			}

			/* Append to preserve original list order */
			if (NULL == PmNodeAccessTable) {
				PmNodeAccessTable = NewEntry;
			} else {
				TableTail->NextNode = NewEntry;
			}
			TableTail = NewEntry;

			PmDbg("  Access: 0x%08x\n\r", NewEntry->Id);

			if (SavedNextEntryPtr != 0U) {
				SavedEntry = (XPm_NodeAccess *)XPmAccess_NextSaved(
					SavedNextEntryPtr);
				if (NULL == SavedEntry) {
					Status = XST_FAILURE;
					goto done;
				}
			}
			else {
				SavedEntry = NULL;
			}
			AccessCount++;
		}
	}

	/* Validate access table count matches saved count */
	if (AccessCount != AccessPtrs.PmNodeAccessCount) {
		PmErr("IPU: access count %u != %u\n\r",
			AccessCount, AccessPtrs.PmNodeAccessCount);
		Status = XST_FAILURE;
		goto done;
	}

	/* Validate aperture count matches saved count */
	if (ApertureCount != AccessPtrs.PmApertureCount) {
		PmErr("IPU: aperture count %u != %u\n\r",
			ApertureCount, AccessPtrs.PmApertureCount);
		Status = XST_FAILURE;
		goto done;
	}

	/* Update static counts to match restored state */
	PmRegnodeCount = RegnodeCount;
	PmNodeAccessCount = AccessCount;
	PmApertureCount = ApertureCount;

	/* Update AccessPtrs to reflect new addresses for test validation */
	AccessPtrs.PmRegnodesPtr = (u32)(UINTPTR)PmRegnodes;
	AccessPtrs.PmNodeAccessTablePtr = (u32)(UINTPTR)PmNodeAccessTable;

	PmDbg("IPU: Restored %u regnodes, %u access, %u apertures\n\r",
		RegnodeCount, AccessCount, ApertureCount);
	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		/* Reset to clean empty state on partial restore failure */
		PmRegnodes = NULL;
		PmNodeAccessTable = NULL;
	}
	return Status;
}

#endif /* VERSAL_NET */

/* Match found in the "Node Access Table" */
typedef struct XPm_NodeAccessMatch {
	XPm_NodeAccess *Entry;
	XPm_NodeAper *Aper;
} XPm_NodeAccessMatch;

static XStatus XPmAccess_LookupEntry(u32 NodeId, u32 Offset,
				     XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccess *Entry = PmNodeAccessTable;

	/* Check for a matching entry with given node id */
	while (NULL != Entry) {
		if (Entry->Id == NodeId) {
			/* Matching entry found, Look for aperture containing given offset */
			XPm_NodeAper *Aper = Entry->Aperture;
			while (NULL != Aper) {
				if ((Offset >=	Aper->Offset) &&
				    (Offset < (Aper->Offset + (Aper->Size * 4U)))) {
					/* Found matching aperture */
					Match->Entry = Entry;
					Match->Aper = Aper;
					Status = XST_SUCCESS;
					break;
				}
				Aper = Aper->NextAper;
			}
			/* Stop the search at first node match */
			break;
		}
		Entry = Entry->NextNode;
	}

	return Status;
}

static XStatus XPmAccess_CheckRequirement(u32 SubsystemId, u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_RegNode *Regnode;
	u32 SubsysIdx = NODEINDEX(SubsystemId);

	/**
	 * NOTE:
	 *  No need for checking if requirement is allocated to the caller subsystem;
	 *  mere presence of a requirement on the given node from caller subsystem
	 *  is enough to pass the access criteria.
	 *
	 *  Therefore, simply check if the caller has requirement (aka permission)
	 *  to access the given node.
	 */
	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		/* Check power parent status */
		Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		/* Check if caller subsystem has a req on this node */
		if (NULL == XPmDevice_FindRequirement(DeviceId, SubsystemId)) {
			Status = XST_FAILURE;
			goto done;
		}
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Regnode = PmRegnodes;
		while (NULL != Regnode) {
			if (DeviceId == Regnode->Id) {
				/* Check if caller subsystem has a req on this node */
				if (0U != (Regnode->Requirements & BIT32(SubsysIdx))) {
					Status = XST_SUCCESS;
				}
				break;
			}
			Regnode = Regnode->NextRegnode;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus XPmAccess_BaseHandler(pm_ioctl_id Op, XPm_NodeAccessTypes AccessType)
{
	XStatus Status = XPM_PM_NO_ACCESS;

	switch (AccessType) {
	case ACCESS_ANY_RO:
	case ACCESS_SEC_RO:
	case ACCESS_SEC_NS_SUBSYS_RO:
	case ACCESS_SEC_SUBSYS_RO:
		/* When access is set to RO, only RO operation is allowed */
		if (IOCTL_READ_REG == Op) {
			Status = XST_SUCCESS;
		}
		break;
	case ACCESS_ANY_RW:
	case ACCESS_SEC_RW:
	case ACCESS_SEC_NS_SUBSYS_RW:
	case ACCESS_SEC_SUBSYS_RW:
		/* When access is set to RW, either RO/WR operations are allowed */
		Status = XST_SUCCESS;
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

static XStatus XPmAccess_AnyHandler(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				    XPm_NodeAccessTypes AccessType,
				    const XPm_NodeAccessMatch *const Match)
{
	(void)SubsystemId;
	(void)CmdType;
	(void)Match;

	return XPmAccess_BaseHandler(Op, AccessType);
}

static XStatus XPmAccess_SecHandler(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				    XPm_NodeAccessTypes AccessType,
				    const XPm_NodeAccessMatch *const Match)
{
	(void)SubsystemId;
	(void)Match;

	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_BaseHandler(Op, AccessType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if incoming command is Secure */
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;

done:
	return Status;
}

static XStatus XPmAccess_NSecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					   u32 CmdType,
					   XPm_NodeAccessTypes AccessType,
					   const XPm_NodeAccessMatch *const Match)
{
	(void)CmdType;

	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_BaseHandler(Op, AccessType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if caller subsystem has access to this entry */
	Status = XPmAccess_CheckRequirement(SubsystemId, Match->Entry->Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

static XStatus XPmAccess_SecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					  u32 CmdType,
					  XPm_NodeAccessTypes AccessType,
					  const XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XPM_PM_NO_ACCESS;

	Status = XPmAccess_NSecSubsysHandler(SubsystemId, Op, CmdType,
					     AccessType, Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Check if incoming command is Secure */
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;

done:
	return Status;
}

static XStatus XPmAccess_EnforcePolicy(u32 SubsystemId, pm_ioctl_id IoctlId,
				       u32 CmdType,
				       const XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XST_NO_FEATURE;
	XPm_NodeAccessTypes AccessType = (XPm_NodeAccessTypes) Match->Aper->Access;

	/* Handlers for access policy checking */
	static const struct XPm_NodeAccessHandler {
		XStatus (*Handler)(u32 SubsystemId, pm_ioctl_id Op, u32 CmdType,
				   XPm_NodeAccessTypes AccessType,
				   const XPm_NodeAccessMatch *const Match);
	} AccessPolicy[ACCESS_TYPE_MAX] = {
		[ACCESS_ANY_RO] = { .Handler = &XPmAccess_AnyHandler },
		[ACCESS_ANY_RW] = { .Handler = &XPmAccess_AnyHandler },
		[ACCESS_SEC_RO] = { .Handler = &XPmAccess_SecHandler },
		[ACCESS_SEC_RW] = { .Handler = &XPmAccess_SecHandler },
		[ACCESS_SEC_NS_SUBSYS_RO] = { .Handler = &XPmAccess_NSecSubsysHandler },
		[ACCESS_SEC_NS_SUBSYS_RW] = { .Handler = &XPmAccess_NSecSubsysHandler },
		[ACCESS_SEC_SUBSYS_RO] = { .Handler = &XPmAccess_SecSubsysHandler },
		[ACCESS_SEC_SUBSYS_RW] = { .Handler = &XPmAccess_SecSubsysHandler },
	};

	switch (AccessType) {
	case ACCESS_ANY_RO:
	case ACCESS_ANY_RW:
	case ACCESS_SEC_RO:
	case ACCESS_SEC_RW:
	case ACCESS_SEC_NS_SUBSYS_RO:
	case ACCESS_SEC_NS_SUBSYS_RW:
	case ACCESS_SEC_SUBSYS_RO:
	case ACCESS_SEC_SUBSYS_RW:
		Status = AccessPolicy[AccessType].Handler(SubsystemId,
					IoctlId, CmdType, AccessType, Match);
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_NO_FEATURE;
		break;
	}

	return Status;
}

static XStatus XPmAccess_IsAllowed(u32 SubsystemId, u32 DeviceId,
				   pm_ioctl_id IoctlId,
				   u32 Offset, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccessMatch Match = { NULL, NULL };

	/**
	 * Check sanity of given offset:
	 *  - Max offset width must be NODE_APER_OFFSET_BIT_FIELD_SIZE
	 *  - Offset must be aligned on a word boundary
	 */
	if ((0U != (Offset & ~NODE_APER_OFFSET_MASK)) ||
	    (0U != (Offset & (sizeof(int) - 1U)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmAccess_LookupEntry(DeviceId, Offset, &Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	PmDbg("Matching Entry Found:\r\n");
	PmDbg("    Id: 0x%08x\r\n", Match.Entry->Id);
	PmDbg("    Offset: 0x%x, Size: 0x%x, Access 0x%x\r\n",
			Match.Aper->Offset,
			Match.Aper->Size,
			Match.Aper->Access);

	Status = XPmAccess_EnforcePolicy(SubsystemId, IoctlId, CmdType, &Match);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	PmDbg("Access policy successfully enforced\r\n");

done:
	return Status;
}


static XStatus XPmAccess_CheckParent(u32 DeviceId, u32 *BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Base = 0U;
	const XPm_Device *Device;
	const XPm_RegNode *Regnode;

	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		/* Check power parent status */
		Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
		if (NULL == Device) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		if ((u32)XPM_POWER_STATE_ON != Device->Power->Node.State) {
			Status = XST_FAILURE;
			goto done;
		}
		/*
		 * Get device base address. XPm_GetDeviceBaseAddr() always returns
		 * XST_SUCCESS when Device is non-NULL (verified above) and
		 * BaseAddr is non-NULL (stack pointer &Base). Return value
		 * check is intentionally omitted as it is unreachable here.
		 */
		Status = XPm_GetDeviceBaseAddr(DeviceId, &Base);
		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Regnode = PmRegnodes;
		while (NULL != Regnode) {
			if (DeviceId == Regnode->Id) {
				/* Get base for a regnode */
				Base = Regnode->BaseAddress;
				/* Check parent power domain state */
				if (NULL == Regnode->Power) {
					PmErr("Regnode 0x%08x has NULL power parent\n\r",
						DeviceId);
					Status = XST_FAILURE;
				} else if ((u32)XPM_POWER_STATE_ON == Regnode->Power->Node.State) {
					Status = XST_SUCCESS;
				}
				break;
			}
			Regnode = Regnode->NextRegnode;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		Base = 0U;
		break;
	}

done:
	*BaseAddress = Base;
	return Status;
}

#ifdef XPM_ENABLE_PLM_TO_PSM_FORWARDING
/****************************************************************************/
/**
 * @brief This Function will forward plm read event to psm using IPI.
 *
 * @param BaseAddress			Base Address of the register to read from
 * @param Offset				Offset value to the base address
 * @param DataIn				Reference variable to hold read value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_ReadAccessForwarding(u32 BaseAddress, u32 Offset, u32 *DataIn)
{
	XStatus Status = XST_FAILURE;

	u32 RegAddress = BaseAddress + Offset;
	/* check whether LPD and PSM FW is loaded properly or not */
	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* construct IPI Payload for read command */
	u32 Payload[PAYLOAD_ARG_CNT];
	Payload[0] = PSM_API_READ_ACCESS;
	Payload[1] = RegAddress;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status){
		goto done;
	}
	Status = XPm_IpiRead(PSM_IPI_INT_MASK, &Payload);
	if (XST_SUCCESS != Status){
		goto done;
	}
	*DataIn = Payload[1];

done:
	/* We have an error condition from either of the if-cond */
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function will forward plm mask write event to psm using IPI.
 *
 * @param BaseAddress			Base Address of the register to read from
 * @param Offset				Offset value to the base address
 * @param Mask					Mask value to the base address
 * @param Value					Value to be written
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_MaskWriteAccessForwarding(u32 BaseAddress, u32 Offset, u32 Mask, u32 Value)
{
	XStatus Status = XST_FAILURE;

	u32 RegAddress = BaseAddress + Offset;
	/* check whether LPD and PSM FW is loaded properly or not */
	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* construct IPI Payload for mask write command */
	u32 Payload[PAYLOAD_ARG_CNT];
	Payload[0] = PSM_API_MASK_WRITE_ACCESS;
	Payload[1] = RegAddress;
	Payload[2] = Mask;
	Payload[3] = Value;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status){
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
#endif

/****************************************************************************/
/**
 * @brief  IOCTL read action handler (Reads given offset - 1 32-bit word)
 *
 * @param  SubsystemId: Subsystem Id
 * @param  DeviceId: Device Id (Device, Regnode etc.)
 * @param  IoctlId: IOCTL Id (Rd/Wr etc.)
 * @param  Offset: Offset to write to in base address of the given node
 * @param  Count: Count (Must be 1 for now, may change in future)
 * @param  CmdType: Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_ReadReg(u32 SubsystemId, u32 DeviceId,
			  pm_ioctl_id IoctlId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u32 DataIn = 0U;
	(void)Count;

	u32 NumArgs = 4U;
	u32 ArgBuf[4];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Offset;
	ArgBuf[3] = 1U;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs, CmdType, Response);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	Status = XPmAccess_IsAllowed(SubsystemId, DeviceId, IoctlId,
				     Offset, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmAccess_CheckParent(DeviceId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmDbg("RD @ (0x%x + 0x%x)\r\n", BaseAddress, Offset);

	u32 RegAddress = BaseAddress + Offset;
	/**
	 * Check if PLM is accessing either of PSM Address Space:
	 * - PSMX_LOCAL_REG
	 * - PSM_RAM_INSTR_ECC_CTRL
	 * - PSM_RAM_DATA_ECC_CTRL
	 * - PSM_TMR_MANAGER
	 * - PSM_TMR_INJECT
	 */
	if (((RegAddress >= PSM_LOCAL_REG_BASEADDR) && (RegAddress <= (PSM_LOCAL_REG_BASEADDR + PSM_LOCAL_REG_SIZE))) ||
	    ((RegAddress >= PSM_RAM_INSTR_ECC_CTRL_REG_BASEADDR) && (RegAddress <= (PSM_RAM_INSTR_ECC_CTRL_REG_BASEADDR + PSM_RAM_INSTR_ECC_CTRL_SIZE))) ||
	    ((RegAddress >= PSM_RAM_DATA_ECC_CTRL_REG_BASEADDR)	&& (RegAddress <= (PSM_RAM_DATA_ECC_CTRL_REG_BASEADDR + PSM_RAM_DATA_ECC_CTRL_SIZE))) ||
	    ((RegAddress >= PSM_TMR_MANAGER_REG_BASEADDR) && (RegAddress <= (PSM_TMR_MANAGER_REG_BASEADDR + PSM_TMR_MANAGER_SIZE))) ||
	    ((RegAddress >= PSM_TMR_INJECT_REG_BASEADDR) && (RegAddress <= (PSM_TMR_INJECT_REG_BASEADDR + PSM_TMR_INJECT_SIZE)))) {
#ifdef XPM_ENABLE_PLM_TO_PSM_FORWARDING
		Status = XPm_ReadAccessForwarding(BaseAddress, Offset, &DataIn);
		*Response = DataIn;
		if (XST_SUCCESS != Status) {
			*Response = 0U;
		}
		goto done;
#else
		Status = XST_NO_FEATURE;
		goto done;
#endif
	}
	PmIn32((BaseAddress + Offset), DataIn);
	*Response = DataIn;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  IOCTL read action handler (Mask writes to given offset - 1 32-bit word)
 *
 * @param  SubsystemId: Subsystem Id
 * @param  DeviceId: Device Id (Device, Regnode etc.)
 * @param  IoctlId: IOCTL Id (Rd/Wr etc.)
 * @param  Offset: Offset to write to in base address of the given node
 * @param  Mask: Mask to be applied to Value
 * @param  Value: Data to be written to given offset
 * @param  CmdType: Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_MaskWriteReg(u32 SubsystemId, u32 DeviceId,
			       pm_ioctl_id IoctlId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;

	u32 NumArgs = 5U;
	u32 ArgBuf[5U];
	ArgBuf[0] = DeviceId;
	ArgBuf[1] = (u32)IoctlId;
	ArgBuf[2] = Offset;
	ArgBuf[3] = Mask;
	ArgBuf[4] = Value;

	/* Forward message event to secondary SLR if required */
	Status = XPm_SsitForwardApi(PM_IOCTL, ArgBuf, NumArgs, CmdType, NULL);
	if (XST_DEVICE_NOT_FOUND != Status) {
		/* API is forwarded, nothing else to be done */
		goto done;
	}

	Status = XPmAccess_IsAllowed(SubsystemId, DeviceId, IoctlId,
				     Offset, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmAccess_CheckParent(DeviceId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmDbg("RMW M:0x%x V:0x%x @ (0x%x + 0x%x)\r\n",
			Mask, Value, BaseAddress, Offset);

	u32 RegAddress = BaseAddress + Offset;
	/**
	 * Check if PLM is accessing either of PSM Address Space:
	 * - PSMX_LOCAL_REG
	 * - PSM_RAM_INSTR_ECC_CTRL
	 * - PSM_RAM_DATA_ECC_CTRL
	 * - PSM_TMR_MANAGER
	 * - PSM_TMR_INJECT
	 */
	if (((RegAddress >= PSM_LOCAL_REG_BASEADDR) 				&& (RegAddress <= (PSM_LOCAL_REG_BASEADDR + PSM_LOCAL_REG_SIZE))) ||
		((RegAddress >= PSM_RAM_INSTR_ECC_CTRL_REG_BASEADDR)	&& (RegAddress <= (PSM_RAM_INSTR_ECC_CTRL_REG_BASEADDR + PSM_RAM_INSTR_ECC_CTRL_SIZE))) ||
		((RegAddress >= PSM_RAM_DATA_ECC_CTRL_REG_BASEADDR) 	&& (RegAddress <= (PSM_RAM_DATA_ECC_CTRL_REG_BASEADDR + PSM_RAM_DATA_ECC_CTRL_SIZE))) ||
		((RegAddress >= PSM_TMR_MANAGER_REG_BASEADDR) 		&& (RegAddress <= (PSM_TMR_MANAGER_REG_BASEADDR + PSM_TMR_MANAGER_SIZE))) ||
		((RegAddress >= PSM_TMR_INJECT_REG_BASEADDR)			&& (RegAddress <= (PSM_TMR_INJECT_REG_BASEADDR + PSM_TMR_INJECT_SIZE)))) {
#ifdef XPM_ENABLE_PLM_TO_PSM_FORWARDING
		Status = XPm_MaskWriteAccessForwarding(BaseAddress, Offset, Mask, Value);
		goto done;
#else
		Status = XST_NO_FEATURE;
		goto done;
#endif
	}

	/**
	 * If for all the 32bit writes (i.e mask = 0xffffffff), simply write to entire address
	 * for any other mask, use PmRmw32()
	 */
	if (0xFFFFFFFFU == Mask) {
		PmOut32((BaseAddress + Offset), Value);
	}
	else {
		PmRmw32((BaseAddress + Offset), Mask, Value);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add a new node access entry to the "Node Access Table"
 *
 * @param  NodeEntry: Pointer to an uninitialized XPm_NodeAccess struct
 * @param  Args: command arguments
 * @param  NumArgs: Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAccess_UpdateTable(XPm_NodeAccess *NodeEntry,
			      const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAper *NodeApertures = NULL;
	u32 AperCount = 0U;

	/* SET_NODE_ACCESS <NodeId: Arg0> <Arg 1,2> <Arg 3,4> ... */
	for (u32 i = 1U; i < NumArgs; i += 2U) {
		XPm_NodeAper *Aper = (XPm_NodeAper *)XPm_AllocBytes(sizeof(XPm_NodeAper));
		if (NULL == Aper) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}

		/* Setup aperture */
		Aper->Offset = NODE_APER_OFFSET(Args[i]);
		Aper->Size = (u8)NODE_APER_SIZE(Args[i]);
		Aper->Access = NODE_APER_ACCESS(Args[i + 1U]);

		/* Add new aperture entry to the list */
		Aper->NextAper = NodeApertures;
		NodeApertures = Aper;
		AperCount++;
	}
	NodeEntry->Aperture = NodeApertures;

	/* Add new node entry to the access table */
	NodeEntry->NextNode = PmNodeAccessTable;
	PmNodeAccessTable = NodeEntry;
	PmNodeAccessCount++;

	/* Commit aperture count only after successful addition */
	PmApertureCount += AperCount;

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Print "Node Access Table" and "Regnodes" table to console
 *
 * @param  None
 *
 * @return None
 *
 * @note This is only meant for debugging; in a usual case, this will get
 * optimized during link time since there are no references to it
 *
 ****************************************************************************/
void XPmAccess_PrintTable(void)
{
	const XPm_NodeAccess *Table = PmNodeAccessTable;
	const XPm_NodeAper *Aper = NULL;
	const XPm_RegNode *Regnode = PmRegnodes;

	while (NULL != Regnode) {
		PmDbg("Id: 0x%08x, Base: 0x%08x, Req: 0x%08x, Power: 0x%08x\r\n",
				Regnode->Id, Regnode->BaseAddress,
				Regnode->Requirements,
				(NULL != Regnode->Power) ? Regnode->Power->Node.Id : 0U);
		Regnode = Regnode->NextRegnode;
	}

	while (NULL != Table) {
		PmDbg("Id: 0x%08x\r\n", Table->Id);
		Aper = Table->Aperture;
		while (NULL != Aper) {
			PmDbg("    Offset: 0x%x, Size: 0x%x, Access 0x%x\r\n",
					Aper->Offset, Aper->Size, Aper->Access);
			Aper = Aper->NextAper;
		}
		Table = Table->NextNode;
	}
}

/****************************************************************************/
/**
 * @brief  Add requirements on a regnode from different subsystems
 *
 * @param  SubsystemId: Node Id assigned to a Subsystem
 * @param  RegnodeId: Node Id assigned to a Regnode
 *
 * @return XST_SUCCESS if successful, XST_DEVICE_NOT_FOUND otherwise
 *
 * @note   SubsystemId must be validated on caller side
 *
 ****************************************************************************/
XStatus XPmAccess_AddRegnodeRequirement(u32 SubsystemId, u32 RegnodeId)
{
	XStatus Status = XST_DEVICE_NOT_FOUND;
	u32 SubsysIdx = NODEINDEX(SubsystemId);
	XPm_RegNode *Regnode = PmRegnodes;

	while (NULL != Regnode) {
		if (RegnodeId == Regnode->Id) {
			Regnode->Requirements |= BIT32(SubsysIdx);
			Status = XST_SUCCESS;
			break;
		}
		Regnode = Regnode->NextRegnode;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize RegNode and add to database
 *
 * @param  RegNode: Pointer to an uninitialized XPm_RegNode struct
 * @param  NodeId: Node Id assigned to a Regnode
 * @param  BaseAddress: Baseaddress of given RegNode
 * @param  Power: Power parent dependency
 *
 * @return None
 *
 * @note None
 *
 ****************************************************************************/
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power)
{
	RegNode->Id = NodeId;
	RegNode->BaseAddress = BaseAddress;
	RegNode->Power = Power;
	RegNode->Requirements = 0U;

	/* Add to list of regnodes */
	RegNode->NextRegnode = PmRegnodes;
	PmRegnodes = RegNode;
	PmRegnodeCount++;
}
