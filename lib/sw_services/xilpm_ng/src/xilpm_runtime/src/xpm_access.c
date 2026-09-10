/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpm_access.h"
#include "xpm_common.h"
#include "xpm_api.h"
#include "xpm_device.h"
#include "xpm_mem.h"
#include "xpm_subsystem.h"
#include "xplmi.h"
#include "xplmi_err_common.h"
#include "xpm_runtime_device.h"
#include "xpm_requirement.h"
#include "xpm_ioctl.h"
#include "xpm_update.h"

/* XPlmi_GetLinkedNodes() is the cross-module accessor into xilplmi's linked-node
 * store; its prototype comes from xplmi_err_common.h above. */

/* Match found in the "Node Access Table" */
typedef struct {
	XPm_NodeAccess *Entry;
	XPm_NodeAper *Aper;
} XPm_NodeAccessMatch;

static XPm_NodeAccess *PmNodeAccessTable[XPM_NODE_ACCESS_TABLE_SIZE] XPM_INIT_DATA(PmNodeAccessTable) = { NULL };

static XPm_NodeAccess* XPmAccess_FindEntry(u32 NodeId)
{
	XPm_NodeAccess *Entry = PmNodeAccessTable[0];
	XPm_NodeAccess *Result = NULL;
	while (NULL != Entry) {
		if (Entry->Id == NodeId) {
			Result = Entry;
			break;
		}
		Entry = Entry->NextNode;
	}
	return Result;
}

static XStatus XPmAccess_LookupEntry(u32 NodeId, u32 Offset,
				     XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccess *Entry = PmNodeAccessTable[0];

	/* Check for a matching entry with given node id */
	while (NULL != Entry) {
		if (Entry->Id == NodeId) {
			/* Matching entry found, Look for aperture containing given offset */
			XPm_NodeAper *Aper = Entry->Aperture;
			while (NULL != Aper) {
				if ((Offset >=  Aper->Offset) &&
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
		Regnode = XPmRegNode_GetNodes();
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
	volatile XStatus Status = XPM_PM_NO_ACCESS;

	/* Redundant verification of AccessType against glitch */
	volatile XPm_NodeAccessTypes AccessTypeCheck = AccessType;
	volatile XPm_NodeAccessTypes AccessTypeCheckTmp = AccessType;
	if (AccessTypeCheck != AccessTypeCheckTmp) {
		Status = XST_GLITCH_ERROR;
		goto done;
	}

	switch (AccessType) {
	case ACCESS_ANY_RO:
	case ACCESS_SEC_RO:
	case ACCESS_SEC_NS_SUBSYS_RO:
	case ACCESS_SEC_SUBSYS_RO:
		/* Verify switch landed on a RO case */
		if ((ACCESS_ANY_RO != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_RO != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_NS_SUBSYS_RO != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_SUBSYS_RO != AccessTypeCheckTmp)) {
			Status = XST_GLITCH_ERROR;
			goto done;
		}
		/* When access is set to RO, only RO operation is allowed */
		{
			volatile pm_ioctl_id OpCheck = Op;
			volatile pm_ioctl_id OpCheckTmp = Op;
			if ((IOCTL_READ_REG == OpCheck) &&
			    (IOCTL_READ_REG == OpCheckTmp)) {
				Status = XST_SUCCESS;
			}
		}
		break;
	case ACCESS_ANY_RW:
	case ACCESS_SEC_RW:
	case ACCESS_SEC_NS_SUBSYS_RW:
	case ACCESS_SEC_SUBSYS_RW:
		/* Verify switch landed on a RW case */
		if ((ACCESS_ANY_RW != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_RW != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_NS_SUBSYS_RW != AccessTypeCheckTmp) &&
		    (ACCESS_SEC_SUBSYS_RW != AccessTypeCheckTmp)) {
			Status = XST_GLITCH_ERROR;
			goto done;
		}
		/* When access is set to RW, either RO/WR operations are allowed */
		Status = XST_SUCCESS;
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
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

	volatile XStatus Status = XPM_PM_NO_ACCESS;
	volatile XStatus StatusTmp = XPM_PM_NO_ACCESS;

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_BaseHandler, Op, AccessType);

	/* Check if incoming command is Secure */
	Status = XPM_PM_NO_ACCESS;
	StatusTmp = XPM_PM_NO_ACCESS;
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;
	StatusTmp = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;
	Status = (XStatus)((u32)Status | (u32)StatusTmp);

done:
	return Status;
}

static XStatus XPmAccess_NSecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					   u32 CmdType,
					   XPm_NodeAccessTypes AccessType,
					   const XPm_NodeAccessMatch *const Match)
{
	(void)CmdType;

	volatile XStatus Status = XPM_PM_NO_ACCESS;

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_BaseHandler, Op, AccessType);

	/* A missing match entry cannot satisfy the subsystem requirement. */
	if ((NULL == Match) || (NULL == Match->Entry)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Check if caller subsystem has access to this entry */
	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_CheckRequirement, SubsystemId, Match->Entry->Id);
done:
	return Status;
}

static XStatus XPmAccess_SecSubsysHandler(u32 SubsystemId, pm_ioctl_id Op,
					  u32 CmdType,
					  XPm_NodeAccessTypes AccessType,
					  const XPm_NodeAccessMatch *const Match)
{
	volatile XStatus Status = XPM_PM_NO_ACCESS;
	volatile XStatus StatusTmp = XPM_PM_NO_ACCESS;

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_NSecSubsysHandler, SubsystemId, Op, CmdType, AccessType, Match);

	/* Check if incoming command is Secure */
	Status = XPM_PM_NO_ACCESS;
	StatusTmp = XPM_PM_NO_ACCESS;
	Status = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;
	StatusTmp = (CmdType == XPLMI_CMD_SECURE) ? XST_SUCCESS : XPM_PM_NO_ACCESS;
	Status = (XStatus)((u32)Status | (u32)StatusTmp);

done:
	return Status;
}

static XStatus XPmAccess_EnforcePolicy(u32 SubsystemId, pm_ioctl_id IoctlId,
				       u32 CmdType,
				       const XPm_NodeAccessMatch *const Match)
{
	XStatus Status = XPM_PM_NO_ACCESS;
	XPm_NodeAccessTypes AccessType;

	/* A missing aperture has no policy and must not grant access. */
	if ((NULL == Match) || (NULL == Match->Aper)) {
		goto done;
	}
	AccessType = (XPm_NodeAccessTypes) Match->Aper->Access;

	/* Handlers for access policy checking */
	static const struct {
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
		if (NULL != AccessPolicy[AccessType].Handler) {
			Status = AccessPolicy[AccessType].Handler(SubsystemId,
					IoctlId, CmdType, AccessType, Match);
		} else {
			Status = XPM_PM_NO_ACCESS;
		}
		break;
	case ACCESS_RESERVED:
	case ACCESS_TYPE_MAX:
	default:
		Status = XST_NO_FEATURE;
		break;
	}

done:
	return Status;
}

static XStatus XPmAccess_IsAllowed(u32 SubsystemId, u32 DeviceId,
				   pm_ioctl_id IoctlId,
				   u32 Offset, u32 CmdType)
{
	volatile XStatus Status = XST_FAILURE;
	XPm_NodeAccessMatch Match = { NULL, NULL };

	/**
	 * Check sanity of given offset:
	 *  - Max offset width must be NODE_APER_OFFSET_BIT_FIELD_SIZE
	 *  - Offset must be aligned on a word boundary
	 */
	if ((0U != (Offset & ~NODE_APER_OFFSET_MASK)) ||
	    (0U != (Offset & (sizeof(u32) - 1U)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_LookupEntry, DeviceId, Offset, &Match);

	/* A successful lookup must yield both the entry and its aperture */
	if ((NULL == Match.Entry) || (NULL == Match.Aper)) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	PmDbg("Matching Entry Found:\r\n");
	PmDbg("    Id: 0x%08x\r\n", Match.Entry->Id);
	PmDbg("    Offset: 0x%x, Size: 0x%x, Access 0x%x\r\n",
			Match.Aper->Offset,
			Match.Aper->Size,
			Match.Aper->Access);

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_EnforcePolicy, SubsystemId, IoctlId, CmdType, &Match);

	PmDbg("Access policy successfully enforced\r\n");

done:
	return Status;
}


static XStatus XPmAccess_CheckParent(u32 DeviceId, u32 *BaseAddress)
{
	volatile XStatus Status = XST_FAILURE;
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
		if ((NULL == Device->Power) ||
		    ((u32)XPM_POWER_STATE_ON != Device->Power->Node.State)) {
			Status = XST_FAILURE;
			goto done;
		}
		/* Get device base */
		XSECURE_TEMPORAL_CHECK(done, Status, XPm_GetDeviceBaseAddr, DeviceId, &Base);

		break;
	case (u32)XPM_NODECLASS_REGNODE:
		Regnode = XPmRegNode_GetNodes();
		while (NULL != Regnode) {
			if (DeviceId == Regnode->Id) {
				/* Get base for a regnode */
				Base = Regnode->BaseAddress;
				/* Check parent power domain state */
				if ((u32)XPM_POWER_STATE_ON == Regnode->Power->Node.State) {
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
	volatile XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;
	u32 DataIn = 0U;
	(void)Count;

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_IsAllowed, SubsystemId, DeviceId, IoctlId, Offset, CmdType);

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_CheckParent, DeviceId, &BaseAddress);

	PmDbg("RD @ (0x%x + 0x%x)\r\n", BaseAddress, Offset);

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
	volatile XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0U;

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_IsAllowed, SubsystemId, DeviceId, IoctlId, Offset, CmdType);

	XSECURE_TEMPORAL_CHECK(done, Status, XPmAccess_CheckParent, DeviceId, &BaseAddress);

	PmDbg("RMW M:0x%x V:0x%x @ (0x%x + 0x%x)\r\n",
			Mask, Value, BaseAddress, Offset);

	/**
	 * If for all the 32bit writes (i.e mask = 0xffffffff),
	 * simply write to entire address without RMW
	 * For any other mask, use PmRmw32()
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

#ifdef XPLMI_EM_LINK_NODES_SUPPORT
/*
 * The GET_EVENT_STATUS machinery below reads the error->node linkage table via
 * XPlmi_GetLinkedNodes(), which is provided only by the platform xilplmi
 * backend that also defines XPLMI_EM_LINK_NODES_SUPPORT (the same macro that
 * gates the link_nodes store command). Platforms/forks without that backend
 * (e.g. imt versal_gen2_tr) omit the macro, so the whole feature - request
 * parsing, node union, aperture reads and deferred ISR clears - compiles out
 * cleanly instead of leaving an unresolved reference at link time.
 */
/*
 * Reply-buffer builder used by GET_EVENT_STATUS. It packs 32-bit words into a
 * caller-provided reply and always tracks the number of words that *would* be
 * required, so on overflow the caller can report the required size and retry.
 */
typedef struct {
	u32 *Buf;		/**< Reply buffer base (word0 = capacity) */
	u32 CapWords;		/**< Capacity in 32-bit words */
	u32 UsedWords;		/**< Words actually written (<= CapWords) */
	u32 ReqWords;		/**< Words required (may exceed CapWords) */
	u8 Overflow;		/**< Set once a word did not fit */
} XPmAccess_ReplyBuilder;

/**
 * @brief  Append one 32-bit word to a reply builder, tracking overflow.
 *
 * @param  Rb:  Reply builder state
 * @param  Val: Word to append to the reply
 *
 * @note   ReqWords always advances so the caller can learn the total required
 *         size even after the buffer fills; a word is stored (and UsedWords
 *         bumped) only while it still fits within the capacity. Once a word
 *         does not fit, the Overflow flag is set.
 */
static void XPmAccess_ReplyPut(XPmAccess_ReplyBuilder *Rb, u32 Val)
{
	/*
	 * ReqWords always advances so the caller can learn the total required
	 * size even after the buffer fills; a word is stored (and UsedWords
	 * bumped) only while it still fits within the capacity.
	 */
	if (Rb->ReqWords < Rb->CapWords) {
		Rb->Buf[Rb->ReqWords] = Val;
		Rb->UsedWords = Rb->ReqWords + 1U;
	} else {
		/* Past capacity: record that the reply overflowed. */
		Rb->Overflow = 1U;
	}
	Rb->ReqWords++;
}

/**
 * @brief  Select the apertures of a node for GET_EVENT_STATUS based on the
 *         requested EventStatusFlag0/1 bits.
 *
 * @param  Aper:  Aperture descriptor to test
 * @param  Flags: XPM_EVENT_STATUS_FLAG_* selection bits
 *
 * @return 1U if the aperture is selected, 0U otherwise.
 *
 * @note   An aperture is selected when its EFlagN matches a requested flag. If
 *         neither flag is requested, all apertures are selected (caller wants
 *         the full status set for the node).
 */
static u32 XPmAccess_AperSelected(const XPm_NodeAper *Aper, u32 Flags)
{
	u32 WantF0 = (0U != (Flags & XPM_EVENT_STATUS_FLAG_EFLAG0)) ? 1U : 0U;
	u32 WantF1 = (0U != (Flags & XPM_EVENT_STATUS_FLAG_EFLAG1)) ? 1U : 0U;
	u32 Selected;

	if ((0U == WantF0) && (0U == WantF1)) {
		/* No flag requested: select all apertures (pure-read default) */
		Selected = 1U;
	} else if ((1U == WantF0) && (0U != Aper->EFlag0)) {
		Selected = 1U;
	} else if ((1U == WantF1) && (0U != Aper->EFlag1)) {
		Selected = 1U;
	} else {
		Selected = 0U;
	}

	return Selected;
}

/*
 * Pack the 2-bit clear type alongside the aperture byte offset in one word.
 * Offset is a 20-bit field (NODE_APER_OFFSET_MASK, bits [19:0]); ClearType is
 * parked in bits [21:20], safely above the offset, so the two never collide.
 * This trims XPmAccess_DeferredClear from 16 to 12 bytes (PmEventClears store).
 */
#define XPM_CLR_TYPE_SHIFT	(20U)	/**< Shift of ClearType in a packed OffClear word */
#define XPM_CLR_TYPE_MASK	(0x3U)	/**< 2-bit mask for the ClearType field */
/**
 * @brief Pack a byte offset (bits [19:0]) and a 2-bit clear type (bits [21:20])
 *        into one word.
 */
#define XPM_CLR_PACK(Off, Ct)	(((Off) & NODE_APER_OFFSET_MASK) | \
				 (((Ct) & XPM_CLR_TYPE_MASK) << XPM_CLR_TYPE_SHIFT))
/** @brief Extract the byte offset (bits [19:0]) from a packed OffClear word. */
#define XPM_CLR_OFFSET(Pk)	((Pk) & NODE_APER_OFFSET_MASK)
/** @brief Extract the 2-bit clear type (bits [21:20]) from a packed OffClear word. */
#define XPM_CLR_TYPE(Pk)	(((Pk) >> XPM_CLR_TYPE_SHIFT) & XPM_CLR_TYPE_MASK)

/*
 * A single deferred clear recorded during the (side-effect-free) read/pack
 * pass. Clears are applied only after the full reply is known to fit, so an
 * undersized reply buffer can never destroy the CE/UE evidence: no register
 * is written on the XST_BUFFER_TOO_SMALL path.
 */
typedef struct {
	u32 NodeId;		/**< Regnode owning the aperture */
	u32 OffClear;		/**< byte offset[19:0] | ClearType[21:20], see XPM_CLR_* */
	u32 Snapshot;		/**< Value read (written back for w1tc) */
} XPmAccess_DeferredClear;

/**
 * @brief Upper bound on the number of nodes handled by one GET_EVENT_STATUS
 *        request.
 *
 * The linked-node lists live in xilplmi's ErrLinkPool[] (XPLMI_ERRLINK_POOL_WORDS
 * entries), so a deduped union across per-bit lists can never exceed that.
 */
#define XPM_EVENT_STATUS_MAX_NODES	(XPLMI_ERRLINK_POOL_WORDS)

/**
 * @brief Size of the deferred ISR-clear store.
 *
 * The correctness guarantee comes from
 * the explicit PmEventClearCnt < XPM_EVENT_STATUS_MAX_CLEARS check at the record
 * site, NOT from this value. One clear is recorded per cleared aperture DATA
 * word, so the true clear count is the total clearable words across the unioned
 * nodes - it is NOT bounded by the node count. This is sized to the node-union
 * cap (XPM_EVENT_STATUS_MAX_NODES) because the shipping DDR-EDAC topology
 * authors a single clearable ISR word per linked node (versal_2ve_2vm: 5 clears). A
 * topology that clears more words than this cap fails safe with the distinct,
 * non-retryable XPM_PM_EVENT_TOO_MANY_CLEARS rather than a faked reply overflow
 * (which would livelock a resized retry). Replaces the former reply-word bound
 * (512), shrinking the store to 64 entries.
 */
#define XPM_EVENT_STATUS_MAX_CLEARS	(XPM_EVENT_STATUS_MAX_NODES)

/*
 * Deferred ISR clears staged by XPmAccess_GetEventStatus() and applied by
 * XPmAccess_ApplyEventClears() only AFTER the reply has been DMAed to the
 * caller. File scope (not a function-local) so the two phases share the store;
 * PM command dispatch is serialized, so a single global staging area is safe.
 */
static XPmAccess_DeferredClear PmEventClears[XPM_EVENT_STATUS_MAX_CLEARS];
/* Transient scratch counter (reset at the top of every GetEventStatus call);
 * NOT restart/live-update state, so it is plain BSS (no XPM_INIT_DATA). */
static u32 PmEventClearCnt = 0U;

/**
 * @brief  Clear a single aperture word using its ClearType by writing back the
 *         read snapshot through the access choke-point (so access policy is
 *         re-enforced).
 *
 * @param  SubsystemId: Caller subsystem Id (access policy enforced)
 * @param  NodeId:      Regnode owning the aperture
 * @param  Offset:      Byte offset of the word within the node base
 * @param  Snapshot:    Value previously read from the aperture word
 * @param  ClearType:   Clear style (CLEAR_W1TC, CLEAR_W0TC or CLEAR_NONE)
 * @param  CmdType:     Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if the clear succeeds or is a no-op; else an error code.
 *
 * @note   w1tc writes the snapshot value (set bits clear); w0tc writes the
 *         inverse of the snapshot (zero bits clear).
 */
static XStatus XPmAccess_ClearWord(u32 SubsystemId, u32 NodeId, u32 Offset,
				   u32 Snapshot, u32 ClearType, u32 CmdType)
{
	/* Default result is success: an aperture with no clear is a no-op. */
	XStatus Status = XST_SUCCESS;
	u32 WrVal = 0U;
	u32 DoWrite = 0U;

	/* Derive the value to write back from the post-read snapshot. */
	switch (ClearType) {
	case (u32)CLEAR_W1TC:
		/* Write-1-to-clear: write the snapshot back verbatim. */
		WrVal = Snapshot;
		DoWrite = 1U;
		break;
	case (u32)CLEAR_W0TC:
		/* Write-0-to-clear: write the inverse of the snapshot. */
		WrVal = ~Snapshot;
		DoWrite = 1U;
		break;
	case (u32)CLEAR_NONE:
	default:
		/* CLEAR_NONE or unexpected encoding: nothing to clear. */
		break;
	}

	/* Only issue the register write when a clear style was selected. */
	if (0U != DoWrite) {
		Status = XPmAccess_MaskWriteReg(SubsystemId, NodeId,
						IOCTL_MASK_WRITE_REG, Offset,
						0xFFFFFFFFU, WrVal, CmdType);
	}

	return Status;
}

/*
 * Fixed DDRMC5E block strides: the DDRMC device node's base is the UB block,
 * which sits ABOVE the MAIN and NOC register windows this feature reads. The
 * intra-controller offsets are a hard-IP silicon constant (register-DB
 * confirmed identical across all controllers and parts).
 */
#define XPM_DDRMC_UB_TO_MAIN	(0x30000U)	/**< MAIN base = UB base - 0x30000 */
#define XPM_DDRMC_UB_TO_NOC	(0x10000U)	/**< NOC  base = UB base - 0x10000 */

/**
 * @brief  Presence gate for a linked DDRMC regnode: a DDR-EDAC MAIN/NOC regnode
 *         must only be read when its owning controller is PRESENT in the design
 *         and RUNNING.
 *
 * @param  NodeId: Node id to test (regnode or otherwise)
 *
 * @return 1U to keep the node (present+RUNNING controller, or not a regnode);
 *         0U to skip it (absent/not-RUNNING DDRMC regnode).
 *
 * @note   Reading an absent/unpopulated controller's window would fault (NoC
 *         SLVERR). The shared NoC power domain (checked by
 *         XPmAccess_CheckParent) cannot tell an absent controller from a
 *         present one - only the per-controller DDRMC DEVICE state can.
 *
 *         Resolve the regnode's register base, then walk the DDRMC device nodes
 *         (mirroring the loop in XPmPlDevice_MemCtrlrMap, honouring the
 *         non-contiguous DDRMC index gap) and match that base to a RUNNING
 *         controller's MAIN (UB - 0x30000) or NOC (UB - 0x10000) window.
 */
static u32 XPmAccess_RegNodePresent(u32 NodeId)
{
	const XPm_RegNode *Regnode = XPmRegNode_GetNodes();
	u32 RegBase = 0U;
	u32 Found = 0U;
	u32 Ret = 0U;
	u32 Idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN;

	while (NULL != Regnode) {
		if (NodeId == Regnode->Id) {
			RegBase = Regnode->BaseAddress;
			Found = 1U;
			break;
		}
		Regnode = Regnode->NextRegnode;
	}

	if (0U == Found) {
		/* Not a regnode: not gated by the DDRMC presence filter. */
		Ret = 1U;
	} else {
		/*
		 * Walk the DDRMC device indices (two ranges either side of the
		 * index gap) using a while loop that advances the index
		 * explicitly, so the loop counter is not modified in the body.
		 */
		while (Idx <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX) {
			const XPm_Device *Dev = XPmDevice_GetById(DDRMC_DEVID(Idx));

			if ((NULL != Dev) &&
			    ((u8)XPM_DEVSTATE_RUNNING == Dev->Node.State)) {
				u32 Ub = Dev->Node.BaseAddress;
				if ((RegBase == (Ub - XPM_DDRMC_UB_TO_MAIN)) ||
				    (RegBase == (Ub - XPM_DDRMC_UB_TO_NOC))) {
					/* backed by a present+RUNNING controller */
					Ret = 1U;
					break;
				}
			}
#ifdef XPM_NODEIDX_DEV_DDRMC_MAX_INT_1
			if ((u32)XPM_NODEIDX_DEV_DDRMC_MAX_INT_1 == Idx) {
				Idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN_INT_2;
			} else {
				Idx++;
			}
#else
			Idx++;
#endif
		}
	}

	return Ret;	/* 1 = keep (present+RUNNING or not a regnode), 0 = skip */
}

/**
 * @brief  Build the union of node IDs linked to the set bits of Mask, in
 *         first-seen (CDO) order, collected into Nodes[].
 *
 * @param  EventNodeId: Error/event node id whose linkage is queried
 * @param  Mask:        Event bit mask (set bits are unioned)
 * @param  Nodes:       Out - array receiving the collected node IDs
 * @param  MaxNodes:    Capacity of Nodes[] in entries
 * @param  NoDedup:     Non-zero to pass the stored list(s) through verbatim
 *
 * @return Number of nodes collected into Nodes[].
 *
 * @note   DDRMC regnodes whose controller is absent/not RUNNING are silently
 *         skipped (XPmAccess_RegNodePresent); MAIN_N and NOC_N share a
 *         controller, so both are dropped or kept together and the MAIN/NOC
 *         pairing stays intact.
 *
 *         Dedup rule: a single stored {EventId,Mask} link list must round-trip
 *         EXACTLY as authored, duplicates included. So when NoDedup is set -
 *         i.e. a single-bit Mask or a ReadNodeList request - the stored list(s)
 *         pass through verbatim. De-dup is only applied when unioning ACROSS
 *         multiple set bits (e.g. DDRMC_CR and DDRMC_NCR both linked to the
 *         same MAIN/NOC regnodes), where a shared node must be read or cleared
 *         at most once.
 */
static u32 XPmAccess_BuildNodeUnion(u32 EventNodeId, u32 Mask,
				    u32 *Nodes, u32 MaxNodes, u32 NoDedup)
{
	u32 BitMask = Mask;
	u32 UnionCnt = 0U;

	while (0U != BitMask) {
		u32 Bit = BitMask & (~BitMask + 1U);	/* lowest set bit */
		const u32 *NodeList = NULL;
		u32 NodeCount = 0U;
		u32 NodeIdx;

		BitMask &= ~Bit;

		if ((XST_SUCCESS != (XStatus)XPlmi_GetLinkedNodes(EventNodeId,
				Bit, &NodeList, &NodeCount)) ||
		    (NULL == NodeList)) {
			/* No linkage for this bit; skip it */
			continue;
		}

		for (NodeIdx = 0U; NodeIdx < NodeCount; NodeIdx++) {
			u32 NodeId = NodeList[NodeIdx];

			/*
			 * Omit a DDRMC regnode whose controller is absent or not
			 * RUNNING - never enters the union, so no header, no
			 * aperture reads, and no clears are produced for it.
			 */
			if (0U == XPmAccess_RegNodePresent(NodeId)) {
				continue;
			}

			if (0U == NoDedup) {
				u32 SeenIdx;
				u32 Seen = 0U;

				/* Skip if already present (first-seen order) */
				for (SeenIdx = 0U; SeenIdx < UnionCnt; SeenIdx++) {
					if (Nodes[SeenIdx] == NodeId) {
						Seen = 1U;
						break;
					}
				}
				if (0U != Seen) {
					continue;
				}
			}
			if (UnionCnt >= MaxNodes) {
				/* Union exceeds staging bound; stop collecting */
				goto done;
			}
			Nodes[UnionCnt] = NodeId;
			UnionCnt++;
		}
	}

done:
	return UnionCnt;
}

/****************************************************************************/
/**
 * @brief  GET_EVENT_STATUS core: source-trace the given event by reading the
 *         raw register snapshot of every node linked to {EventNodeId, Mask},
 *         optionally clearing the ISR aperture afterwards.
 *
 * @param  SubsystemId: Caller subsystem Id (access policy enforced per-aperture)
 * @param  EventNodeId: Error/event node id (e.g. PMC_ERR1)
 * @param  Mask:        Event bit mask (may be multi-bit; set bits are unioned)
 * @param  Flags:       XPM_EVENT_STATUS_FLAG_* selection/clear flags
 * @param  ReqArgs:     Optional {size/offset} range descriptors (ReadNodes)
 * @param  ReqArgCnt:   Number of ReqArgs words
 * @param  Reply:       Reply buffer (word0 = capacity in bytes, caller-set)
 * @param  ReplyCap:    Reply buffer capacity in bytes
 * @param  ReplyUsed:   Out - bytes used, or bytes required on overflow
 * @param  CmdType:     Secure/Non-Secure command type
 *
 * @return XST_SUCCESS on success; XST_BUFFER_TOO_SMALL (with *ReplyUsed set to
 *         the required byte count) on reply overflow; else an error code.
 *
 * @note   Reads go through XPmAccess_ReadReg so the per-aperture ACCESS_*
 *         policy is always enforced. Post-read ISR clears are only recorded
 *         (with their write permission verified up front); the caller applies
 *         them via XPmAccess_ApplyEventClears() after the reply is delivered.
 *
 ****************************************************************************/
XStatus XPmAccess_GetEventStatus(u32 SubsystemId, u32 EventNodeId, u32 Mask,
				 u32 Flags, const u32 *ReqArgs, u32 ReqArgCnt,
				 u32 *Reply, u32 ReplyCap, u32 *ReplyUsed,
				 u32 CmdType)
{
	volatile XStatus Status = XST_FAILURE;
	XPmAccess_ReplyBuilder Rb;
	u32 ClearAfterRead = (0U != (Flags & XPM_EVENT_STATUS_FLAG_CLEAR_AFTER_READ)) ? 1U : 0U;
	u32 ReadNodeList = (0U != (Flags & XPM_EVENT_STATUS_FLAG_READ_NODE_LIST)) ? 1U : 0U;
	u32 ReadNodes = (0U != (Flags & XPM_EVENT_STATUS_FLAG_READ_NODES)) ? 1U : 0U;
	static u32 UnionNodes[XPM_EVENT_STATUS_MAX_NODES];
	u32 NodeCount;
	u32 NodeIdx;

	if ((NULL == Reply) || (NULL == ReplyUsed) || (0U == Mask)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Reject unknown/reserved flag bits before acting on them */
	if (0U != (Flags & ~XPM_EVENT_STATUS_FLAG_VALID_MASK)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * ClearAfterRead must name at least one EFlag: "no flags = all
	 * apertures" is a pure-read convenience only. Allowing it to combine
	 * with CLEAR_AFTER_READ would wipe every clearable register in the node.
	 */
	if ((1U == ClearAfterRead) &&
	    (0U == (Flags & (XPM_EVENT_STATUS_FLAG_EFLAG0 |
			     XPM_EVENT_STATUS_FLAG_EFLAG1)))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * CLEAR_AFTER_READ only has meaning on the aperture-walk read path. The
	 * READ_NODES (request-specified ranges) and READ_NODE_LIST (flat node-id
	 * list) paths record NO clears, so pairing either with CLEAR_AFTER_READ
	 * would report a misleading "success" with nothing cleared - reject it.
	 */
	if ((1U == ClearAfterRead) &&
	    ((1U == ReadNodes) || (1U == ReadNodeList))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Start a fresh deferred-clear batch for this request */
	PmEventClearCnt = 0U;

	/*
	 * Default *ReplyUsed to 0 up front so every subsequent early-return error
	 * path leaves a defined value; only the reply-overflow path overwrites it
	 * with the required byte count (so the caller can resize and retry). Other
	 * error paths, including too-many-clears, leave it 0.
	 */
	*ReplyUsed = 0U;

	/* word0 of the reply is the caller-set capacity; skip it when packing */
	Rb.Buf = &Reply[1];
	Rb.CapWords = (ReplyCap / (u32)sizeof(u32));
	Rb.CapWords = (Rb.CapWords > 0U) ? (Rb.CapWords - 1U) : 0U;
	Rb.UsedWords = 0U;
	Rb.ReqWords = 0U;
	Rb.Overflow = 0U;

	/*
	 * Resolve the CDO-ordered union of node IDs across the set bits of Mask
	 * BEFORE reading. A single-bit Mask (or a ReadNodeList request) must
	 * round-trip its stored list verbatim (duplicates preserved); only a
	 * multi-bit Mask de-dups so a shared node is read/cleared at most
	 * once. Mask != 0 here, so single-bit == (Mask & (Mask-1))==0.
	 */
	{
		u32 SingleBit = (0U == (Mask & (Mask - 1U))) ? 1U : 0U;
		u32 NoDedup = ((1U == SingleBit) || (1U == ReadNodeList)) ? 1U : 0U;
		NodeCount = XPmAccess_BuildNodeUnion(EventNodeId, Mask, UnionNodes,
						     XPM_EVENT_STATUS_MAX_NODES,
						     NoDedup);
	}

	for (NodeIdx = 0U; NodeIdx < NodeCount; NodeIdx++) {
		u32 NodeId = UnionNodes[NodeIdx];
		const XPm_NodeAccess *Entry = XPmAccess_FindEntry(NodeId);
		u32 NodeBase = 0U;
		u32 HdrIdx;
		u32 NumEntries = 0U;	/* aperture descriptors, not data words */
		const XPm_NodeAper *Aper;

		if (0U != ReadNodeList) {
			/*
			 * Flat node-id list mode: only list a node the caller may
			 * actually READ. Probe the real per-aperture access policy
			 * via XPmAccess_IsAllowed (IOCTL_READ_REG) against the
			 * offset of the node's first aperture - NOT merely a
			 * requirement - so a secure-only aperture (ACCESS_SEC_*)
			 * is never leaked to a non-secure caller. A
			 * node with no aperture entry has no readable register and
			 * is skipped.
			 */
			if ((NULL == Entry) || (NULL == Entry->Aperture)) {
				continue;
			}
			if (XST_SUCCESS != XPmAccess_IsAllowed(SubsystemId,
					NodeId, IOCTL_READ_REG,
					(u32)Entry->Aperture->Offset, CmdType)) {
				continue;
			}
			XPmAccess_ReplyPut(&Rb, NodeId);
			continue;
		}

		if (NULL == Entry) {
			/* Node has no apertures; skip */
			continue;
		}

		/* Resolve base for the per-node reply header (best effort) */
		(void)XPmAccess_CheckParent(NodeId, &NodeBase);

		/* Node block header: NodeID, {NumOffsetEntries}, base_addr */
		XPmAccess_ReplyPut(&Rb, NodeId);
		HdrIdx = Rb.ReqWords;		/* remember count slot */
		XPmAccess_ReplyPut(&Rb, 0U);	/* patched with NumEntries */
		XPmAccess_ReplyPut(&Rb, NodeBase);

		if (0U != ReadNodes) {
			/*
			 * Read request-specified {offset,size} ranges against
			 * this node. Each range yields exactly ONE descriptor
			 * followed by its data words. Per-word
			 * reads still go through the choke-point, which bounds
			 * each offset against the node's apertures (OOB rejected).
			 */
			u32 ArgIdx;
			for (ArgIdx = 0U; ArgIdx < ReqArgCnt; ArgIdx++) {
				/*
				 * Request offset is a 10-bit WORD offset (4 KB
				 * reach); scale to a byte offset here. Reply and
				 * SET_NODE_ACCESS use 20-bit byte offsets.
				 */
				u32 Off = XPM_READNODES_OFFSET(ReqArgs[ArgIdx]) * 4U;
				u32 Sz = XPM_READNODES_SIZE(ReqArgs[ArgIdx]);
				u32 WordIdx;

				if (0U == Sz) {
					continue;
				}

				/* ONE descriptor per range: { size[31:24], offset[19:0] } */
				XPmAccess_ReplyPut(&Rb,
					((Sz << NODE_APER_SIZE_SHIFT) &
					 ((u32)NODE_APER_SIZE_MASK << NODE_APER_SIZE_SHIFT)) |
					(Off & NODE_APER_OFFSET_MASK));
				NumEntries++;

				for (WordIdx = 0U; WordIdx < Sz; WordIdx++) {
					u32 WordOff = Off + (WordIdx * 4U);
					u32 Val = 0U;
					Status = XPmAccess_ReadReg(SubsystemId,
							NodeId, IOCTL_READ_REG,
							WordOff, 1U, &Val, CmdType);
					if (XST_SUCCESS != Status) {
						goto done;
					}
					XPmAccess_ReplyPut(&Rb, Val);
				}
			}
		} else {
			/* Walk apertures selected by EFlag0/EFlag1 */
			Aper = Entry->Aperture;
			while (NULL != Aper) {
				u32 WordIdx;
				if (0U == XPmAccess_AperSelected(Aper, Flags)) {
					Aper = Aper->NextAper;
					continue;
				}

				/* ONE descriptor per aperture: { size[31:24], offset[19:0] } */
				XPmAccess_ReplyPut(&Rb,
					(((u32)Aper->Size << NODE_APER_SIZE_SHIFT) &
					 ((u32)NODE_APER_SIZE_MASK << NODE_APER_SIZE_SHIFT)) |
					((u32)Aper->Offset & NODE_APER_OFFSET_MASK));
				NumEntries++;

				for (WordIdx = 0U; WordIdx < (u32)Aper->Size; WordIdx++) {
					u32 WordOff = (u32)Aper->Offset + (WordIdx * 4U);
					u32 Val = 0U;
					Status = XPmAccess_ReadReg(SubsystemId,
							NodeId, IOCTL_READ_REG,
							WordOff, 1U, &Val, CmdType);
					if (XST_SUCCESS != Status) {
						goto done;
					}
					XPmAccess_ReplyPut(&Rb, Val);

					/*
					 * Record (do NOT perform) any post-read
					 * clear. Clears are applied only after the
					 * whole reply is proven to fit AND has been
					 * delivered, so an overflow-triggered retry
					 * never sees destroyed CE/UE evidence.
					 */
					if ((1U == ClearAfterRead) &&
					    ((u32)CLEAR_NONE != Aper->ClearType)) {
						/*
						 * Verify WRITE permission for this
						 * aperture NOW, during the
						 * side-effect-free read pass. A
						 * clearable aperture the caller cannot
						 * write is rejected cleanly here
						 * BEFORE any clear is performed, so a
						 * mid-clear permission failure can never
						 * leave a partially-cleared ISR.
						 */
						Status = XPmAccess_IsAllowed(
								SubsystemId, NodeId,
								IOCTL_MASK_WRITE_REG,
								WordOff, CmdType);
						if (XST_SUCCESS != Status) {
							goto done;
						}
						/* WordOff packs to 20 bits; it is pre-validated
						 * by XPmAccess_IsAllowed, but guard so a mispacked
						 * offset cannot clear a wrong register. */
						if (0U != (WordOff & ~(u32)NODE_APER_OFFSET_MASK)) {
							Status = XST_INVALID_PARAM;
							goto done;
						}
						if (PmEventClearCnt < XPM_EVENT_STATUS_MAX_CLEARS) {
							PmEventClears[PmEventClearCnt].NodeId = NodeId;
							PmEventClears[PmEventClearCnt].OffClear =
								XPM_CLR_PACK(WordOff,
									     (u32)Aper->ClearType);
							PmEventClears[PmEventClearCnt].Snapshot = Val;
							PmEventClearCnt++;
						} else {
							/*
							 * Clear store exhausted. Sized to the
							 * node-union cap, this is unreachable for
							 * any real versal_2ve_2vm topology. If reached, fail
							 * with a DISTINCT, non-retryable error - NOT
							 * XST_BUFFER_TOO_SMALL: enlarging the reply
							 * cannot reduce the clear count, so reporting
							 * a reply size would livelock a resized retry.
							 * Apply no clears (ApplyEventClears is never
							 * called on an error return); the caller must
							 * drop CLEAR_AFTER_READ or split the request
							 * by fewer mask bits.
							 */
							Status = XPM_PM_EVENT_TOO_MANY_CLEARS;
							goto done;
						}
					}
				}
				Aper = Aper->NextAper;
			}
		}

		/* Patch NumOffsetEntries (descriptor count) in the header if it fit */
		if (HdrIdx < Rb.CapWords) {
			Rb.Buf[HdrIdx] = (NumEntries & 0xFFFU);
		}
	}

	if (0U != Rb.Overflow) {
		/*
		 * Report required size (including reply word0) and bail out.
		 * CRITICAL: perform NO clears on overflow - the caller will
		 * resize and retry, and the source evidence must survive.
		 */
		*ReplyUsed = (Rb.ReqWords + 1U) * (u32)sizeof(u32);
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/*
	 * Reply fits and every recorded clear already passed a write-permission
	 * check. The clears themselves are NOT applied here: the caller DMAs the
	 * reply out first, then invokes XPmAccess_ApplyEventClears() so a clear
	 * failure can never destroy a snapshot the caller already received.
	 */

	/* Bytes used = packed words + reply word0 */
	*ReplyUsed = (Rb.UsedWords + 1U) * (u32)sizeof(u32);
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Apply the ISR clears deferred by the most recent successful
 *         XPmAccess_GetEventStatus() call.
 *
 * @param  SubsystemId: Caller subsystem Id (access policy re-enforced per clear)
 * @param  CmdType:     Secure/Non-Secure command type
 *
 * @return XST_SUCCESS if all clears succeed (or none were recorded); else an
 *         error code from the first failing clear.
 *
 * @note   Must be called only after the reply has been delivered to the caller.
 *         Clears go through XPmAccess_ClearWord -> XPmAccess_MaskWriteReg so the
 *         per-aperture ACCESS_* policy is re-enforced at write time. The batch
 *         is consumed (count reset) whether or not a clear fails.
 *
 ****************************************************************************/
XStatus XPmAccess_ApplyEventClears(u32 SubsystemId, u32 CmdType)
{
	XStatus Status = XST_SUCCESS;
	u32 ClearIdx;

	for (ClearIdx = 0U; ClearIdx < PmEventClearCnt; ClearIdx++) {
		Status = XPmAccess_ClearWord(SubsystemId,
					     PmEventClears[ClearIdx].NodeId,
					     XPM_CLR_OFFSET(PmEventClears[ClearIdx].OffClear),
					     PmEventClears[ClearIdx].Snapshot,
					     XPM_CLR_TYPE(PmEventClears[ClearIdx].OffClear),
					     CmdType);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	/* Consume the batch so a later call cannot re-apply stale clears */
	PmEventClearCnt = 0U;
	return Status;
}
#endif /* XPLMI_EM_LINK_NODES_SUPPORT */

/**
 * @brief  Validate the clear_type field of a SET_NODE_ACCESS word2.
 *
 * @param  Word2: The SET_NODE_ACCESS word2 value to validate
 *
 * @return XST_SUCCESS for a defined clear type; XST_INVALID_PARAM for the
 *         reserved encoding.
 *
 * @note   Kept as a small pure function so the caller can invoke it
 *         independently over a genuine double (volatile) read of word2 for
 *         glitch redundancy.
 */
static XStatus XPmAccess_ValidateClearType(u32 Word2)
{
	XStatus Status = XST_INVALID_PARAM;

	if ((u32)CLEAR_TYPE_MAX > NODE_APER_CLEARTYPE(Word2)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**
 * @brief  Validate the access field of a SET_NODE_ACCESS word2.
 *
 * @param  Word2: The SET_NODE_ACCESS word2 value to validate
 *
 * @return XST_SUCCESS for a valid access encoding; XST_INVALID_PARAM for
 *         ACCESS_RESERVED (0) or any encoding >= ACCESS_TYPE_MAX.
 *
 * @note   Rejecting reserved values ensures they can never be stored. See
 *         XPmAccess_ValidateClearType for why this is a function.
 */
static XStatus XPmAccess_ValidateAccess(u32 Word2)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 Access = NODE_APER_ACCESS(Word2);

	if (((u32)ACCESS_RESERVED != Access) &&
	    ((u32)ACCESS_TYPE_MAX > Access)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Add a new node access entry to the "Node Access Table"
 *
 * @param  Args: command arguments
 * @param  NumArgs: Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note This function implements an prepend/update behavior:
 * - If an aperture with the same offset and size exists, it updates the access field
 * - If it doesn't exist, it prepends the new aperture to the list
 *
 ****************************************************************************/
XStatus XPmAccess_UpdateTable(const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_NodeAccess *Entry;

	/* Validate: <NodeId: Arg0> <Arg1,2> <Arg3,4> ... */
	if ((NumArgs < NODE_ACCESS_ARG_MIN_LEN) || ((NumArgs % 2U) == 0U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	u32 NodeId = Args[0];
	/* Find or create the node entry */
	Entry = XPmAccess_FindEntry(NodeId);
	if (NULL == Entry) {
		Entry = (XPm_NodeAccess *)XPm_AllocBytesOthers(sizeof(XPm_NodeAccess));
		if (NULL == Entry) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Entry->Id = NodeId;
		Entry->Aperture = NULL;
		Entry->NextNode = PmNodeAccessTable[0];
		PmNodeAccessTable[0] = Entry;
	}

	/*
	 * Process each aperture from Args:
	 * 1. Search for existing aperture with same offset and size
	 * 2. If found, update access field only
	 * 3. If not found, prepend new aperture to the end of list
	 */
	for (u32 i = 1U; i < NumArgs; i += 2U) {
		u32 Offset = NODE_APER_OFFSET(Args[i]);
		u32 Size = (u32)NODE_APER_SIZE(Args[i]);
		/*
		 * access + extended attributes (clear_type, flags) from word2.
		 * Assigned ONLY from the glitch-validated Word2A below,
		 * so the committed aperture policy derives from the same checked
		 * read, not an earlier unchecked load.
		 * Legacy CDOs leave the extended bits 0 => CLEAR_NONE, flags = 0.
		 */
		u32 access = 0U;
		u32 ClearType = 0U;
		u32 EFlag0 = 0U;
		u32 EFlag1 = 0U;

		if (0U == Size) {
			/* Size cannot be zero */
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/*
		 * Reject non-zero reserved bits (word2 [31:8]). This makes the
		 * backward-compat contract self-enforcing regardless of the CDO
		 * generator instead of trusting generators to zero the free bits.
		 * The shipping DDR-EDAC topology/overlay CDOs set only word2 access
		 * values (0x1/0x6/0x40/0x45/0x56), all < 0x100, so no reserved bit
		 * is ever set by a real CDO.
		 */
		if (0U != (Args[i + 1U] & NODE_APER_WORD2_RSVD_MASK)) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		/*
		 * Validate clear_type (reserved encoding rejected) and access
		 * (ACCESS_RESERVED / out-of-range rejected) with GENUINE glitch
		 * redundancy: read the source word2 TWICE through a volatile
		 * pointer (the compiler may not fold the second load), validate
		 * both copies independently, and require both passes AND the two
		 * reads to agree. XSECURE_TEMPORAL_CHECK only COPIES its argument
		 * in this tree (xil_sutil.h XSECURE_TEMPORAL_IMPL: VarTmp = Var),
		 * so it would give no independent re-read here.
		 */
		{
			volatile const u32 *Word2Ptr = &Args[i + 1U];
			u32 Word2A = *Word2Ptr;
			u32 Word2B = *Word2Ptr;

			Status = XPmAccess_ValidateClearType(Word2A);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPmAccess_ValidateClearType(Word2B);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPmAccess_ValidateAccess(Word2A);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = XPmAccess_ValidateAccess(Word2B);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			if (Word2A != Word2B) {
				Status = XST_GLITCH_ERROR;
				goto done;
			}

			/*
			 * Commit fields ONLY from the validated word:
			 * the stored aperture policy must derive from the same
			 * glitch-checked read, not an earlier unchecked load.
			 */
			access = NODE_APER_ACCESS(Word2A);
			ClearType = NODE_APER_CLEARTYPE(Word2A);
			EFlag0 = NODE_APER_EFLAG0(Word2A);
			EFlag1 = NODE_APER_EFLAG1(Word2A);
		}

		/* Search for existing aperture with matching offset and size */
		XPm_NodeAper* FoundAper = NULL;
		XPm_NodeAper* CurrAper = Entry->Aperture;
		while (NULL != CurrAper) {
			if ((CurrAper->Offset == Offset) &&
			    (CurrAper->Size == (u8)Size)) {
				/* Found matching aperture */
				FoundAper = CurrAper;
				break;
			}
			CurrAper = CurrAper->NextAper;
		}

		if (NULL != FoundAper) {
			/* Update existing aperture's access field */
			FoundAper->Access = ((u32)access) & NODE_APER_ACCESS_MASK;
			/*
			 * Preserve existing extended attributes when the incoming
			 * word carries none: a legacy CDO re-touching an aperture
			 * (all extended bits 0) must not silently clobber the
			 * clear_type/flags a newer CDO already programmed.
			 */
			if ((0U != ClearType) || (0U != EFlag0) ||
			    (0U != EFlag1)) {
				FoundAper->ClearType = ClearType & NODE_APER_CLEARTYPE_MASK;
				FoundAper->EFlag0 = EFlag0 & NODE_APER_FLAG_MASK;
				FoundAper->EFlag1 = EFlag1 & NODE_APER_FLAG_MASK;
			}
		} else {
			/* Prepend new aperture to the head of the list */
			XPm_NodeAper *NewAper = (XPm_NodeAper *)XPm_AllocBytesOthers(sizeof(XPm_NodeAper));
			if (NULL == NewAper) {
				Status = XST_BUFFER_TOO_SMALL;
				goto done;
			}
			NewAper->Offset = Offset & NODE_APER_OFFSET_MASK;
			NewAper->Size = ((u32)Size) & NODE_APER_SIZE_MASK;
			NewAper->Access = ((u32)access) & NODE_APER_ACCESS_MASK;
			NewAper->ClearType = ClearType & NODE_APER_CLEARTYPE_MASK;
			NewAper->EFlag0 = EFlag0 & NODE_APER_FLAG_MASK;
			NewAper->EFlag1 = EFlag1 & NODE_APER_FLAG_MASK;

			/* Insert at the beginning */
			NewAper->NextAper = Entry->Aperture;
			Entry->Aperture = NewAper;
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("Failed to update Node Access Table: 0x%08x\r\n", Status);
	}
	return Status;
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
	XPm_RegNode *Regnode = XPmRegNode_GetNodes();

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
