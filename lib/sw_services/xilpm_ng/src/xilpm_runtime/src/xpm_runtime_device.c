/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_subsystem.h"
#include "xpm_requirement.h"
#include "xil_types.h"
#include "xpm_device.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xplmi.h"
#include "xpm_runtime_api.h"
#include "xpm_runtime_device.h"
#include "xpm_runtime_core.h"
#include "xpm_runtime_reset.h"
#include "xpm_mem_tcm.h"
#include "xpm_aiedevice.h"
#include "xpm_update.h"
#include "xpm_runtime_aie.h"
#include "xpm_rpucore.h"

/* Device security bit in register */
#define DEV_NONSECURE			(1U)
#define DEV_SECURE			(0U)
static XPmRuntime_DeviceOpsList* DevOpsList XPM_INIT_DATA(DevOpsList) = NULL;
static const u32 IpiMasks[][2] = {
	{ PM_DEV_IPI_0, IPI_0_MASK },
	{ PM_DEV_IPI_1, IPI_1_MASK },
	{ PM_DEV_IPI_2, IPI_2_MASK },
	{ PM_DEV_IPI_3, IPI_3_MASK },
	{ PM_DEV_IPI_4, IPI_4_MASK },
	{ PM_DEV_IPI_5, IPI_5_MASK },
	{ PM_DEV_IPI_6, IPI_6_MASK },
};
static XStatus DevRequest(XPm_Device *Device, XPm_Subsystem *Subsystem,
			  u32 Capabilities, u32 QoS, u32 CmdType);
static XStatus DevRelease(XPm_Device *Device,  XPm_Subsystem *Subsystem, u32 CmdType);
static XStatus SetDevRequirement(XPm_Device *Device, const XPm_Subsystem *Subsystem,
			      u32 Capabilities, const u32 QoS);

maybe_unused static XStatus CheckSecurityAccess(const XPm_Requirement *Reqm, u32 ReqCaps, u32 CmdType)
{
	XStatus Status = XPM_PM_NO_ACCESS;
	struct XPm_SecPolicy { u16 CmdType; u16 Allowed; };
	u16 SlaveTz = SECURITY_POLICY(Reqm->Flags);
	u16 MasterTz = (0U != (ReqCaps & (u32)PM_CAP_SECURE)) ? 1U : 0U;

	static const struct XPm_SecPolicy SecPolicy[2U][2U] = {
		[REQ_ACCESS_SECURE] = {
			/* Slave: secure, master: non-secure, allowed for S/NS commands */
			[0U] = { .CmdType = BIT16(XPLMI_CMD_SECURE) | BIT16(XPLMI_CMD_NON_SECURE), .Allowed = 1U },
			/* Slave: secure, master: secure, allowed for only S commands */
			[1U] = { .CmdType = BIT16(XPLMI_CMD_SECURE), .Allowed = 1U },
		},
		[REQ_ACCESS_SECURE_NONSECURE] = {
			/* Slave: non-secure, master: non-secure, allowed for S/NS commands */
			[0U] = { .CmdType = BIT16(XPLMI_CMD_SECURE) | BIT16(XPLMI_CMD_NON_SECURE), .Allowed = 1U },
			/* Slave: non-secure, master: secure, not allowed */
			[1U] = { .CmdType = 0U, .Allowed = 0U },
		},
	};

	if ((0U != (BIT16(CmdType) & SecPolicy[SlaveTz][MasterTz].CmdType)) &&
	    (0U != SecPolicy[SlaveTz][MasterTz].Allowed)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static inline XStatus XPmRequirement_IsAllocated(const XPm_Requirement *Reqm)
{
	volatile XStatus Status = XPM_FAILURE;
	if ((Reqm != NULL) && (1U == Reqm->Allocated)) {
		Status = XPM_SUCCESS;
	}
	return Status;
}

static u8 XPmDevice_IsExcluded(const u32 NodeId)
{
	u8 IsExcluded = 0U;

	if (((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(NodeId)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(NodeId)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_ASU == NODETYPE(NodeId)) ||
	    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(NodeId)) ||
	    ((u32)XPM_NODETYPE_DEV_XRAM == NODETYPE(NodeId)) ||
	    ((u32)XPM_NODESUBCL_DEV_PHY == NODESUBCLASS(NodeId)) ||
	    ((u32)XPM_NODEIDX_DEV_AMS_ROOT == NODEINDEX(NodeId))) {
		IsExcluded = 1U;
	}

	return IsExcluded;
}

u8 XPmDevice_IsRequestable(u32 NodeId)
{
	u8 Requestable = 0U;

	if (XPmDevice_IsExcluded(NodeId)) {
		/* Excluded device is not requestable */
		return Requestable;
	}

	switch (NODESUBCLASS(NodeId)) {
	case (u32)XPM_NODESUBCL_DEV_CORE:
	case (u32)XPM_NODESUBCL_DEV_PERIPH:
	case (u32)XPM_NODESUBCL_DEV_MEM:
	case (u32)XPM_NODESUBCL_DEV_PL:
	case (u32)XPM_NODESUBCL_DEV_AIE:
	case (u32)XPM_NODESUBCL_DEV_MEM_CTRLR:
		Requestable = 1U;
		break;
	default:
		Requestable = 0U;
		break;
	}

	return Requestable;
}

/**
 * @brief Sets the device operations for a device based on its ID.
 *
 * This function sets the device operations for a device identified by its ID.
 * If the device is not found or if the device operations are already set, the function does nothing.
 * If memory allocation for the device operations fails, an error message is printed and the function returns with NULL pointer
 *
 * @param DeviceId The ID of the device.
 * @return A pointer to the device operations structure on success, or NULL on failure.
 */
XPmRuntime_DeviceOps * XPmDevice_SetDevOps_ById(u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	if (NULL == DevOpsList){
		DevOpsList = Make_XPmRuntime_DeviceOpsList();
	}
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(DeviceId);
	if (NULL == Device) {
		PmErr("Device not found: 0x%x\r\n", DeviceId);
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (NULL != DevOps) {
		/** Nothing to do here */
		Status = XST_SUCCESS;
		goto done;
	}
	DevOps = (XPmRuntime_DeviceOps *)XPm_AllocBytesDevOps(sizeof(XPmRuntime_DeviceOps));
	if (NULL == DevOps) {
		PmErr("Failed to allocate memory for DevOps\r\n");
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	DevOps->Device = Device;

	LIST_PREPEND(DevOpsList, DevOps);
	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("Status = 0x%x\r\n", Status);
		DevOps = NULL;
	}

	return DevOps;
}

XPmRuntime_DeviceOps *XPm_GetDevOps_ById(u32 DeviceId) {
	LIST_FOREACH(DevOpsList, DevOpsNode){
		if (DevOpsNode->Data->Device->Node.Id == DeviceId) {
			return DevOpsNode->Data;
		}
	}
	return NULL;
}
static XStatus SetSecurityAttr(XPm_Requirement *Reqm, u32 ReqCaps, u32 PrevState)
{
	XStatus Status = XST_FAILURE;
	u32 CurrSecState;
	u32 Is_CapSecure = 0U;
	u32 BaseAddr, Offset1, Mask1, Offset2, Mask2;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;
	const XPm_PsLpDomain *Lpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	const XPm_PsFpDomain *Fpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);

	/**
	 * Skip if device does not have any security attributes.
	 * Security[1] attribute is optional.
	 */
	if ((NULL == DevAttr) || (0U == DevAttr->Security[0].Mask) ||
	    (0U == DevAttr->SecurityBaseAddr)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((NULL == Lpd) || (NULL == Fpd)) {
		Status = XST_FAILURE;
		goto done;
	}

	BaseAddr = DevAttr->SecurityBaseAddr;
	Offset1 = DevAttr->Security[0].Offset;
	Mask1 = DevAttr->Security[0].Mask;
	Offset2 = DevAttr->Security[1].Offset;
	Mask2 = DevAttr->Security[1].Mask;

	if (0U != (ReqCaps & (u32)(PM_CAP_SECURE))) {
		Is_CapSecure = 1U;
	}

	/**
	 * Do not touch the LPD/FPD registers if domain is not ON.
	 * This is the generalized solution but applicable during the last
	 * device release of the domain.
	 */
	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Lpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Fpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Required due to MISRA */
	}

	/* Here 1 value in bit corresponds to non-secure config */
	CurrSecState = XPm_In32(BaseAddr + Offset1) & Mask1;

	/* Do nothing if device is still ON after release */
	if ((0U == Reqm->Allocated) &&
	    ((u8)XPM_DEVSTATE_RUNNING == Reqm->Device->Node.State)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((1U == Reqm->Allocated) &&
	    ((u32)XPM_DEVSTATE_RUNNING == PrevState)) {
		/**
		 * Return error if current device state does not match
		 * with the received request.
		 */
		if (((1U == Is_CapSecure) && (DEV_NONSECURE == CurrSecState)) ||
		    ((0U == Is_CapSecure) && (DEV_SECURE == CurrSecState))) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else {
		/* Required due to MISRA */
	}

	if (1U == Is_CapSecure) {
		PmRmw32(BaseAddr + Offset1, Mask1, 0);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, 0);
		}
		Reqm->AttrCaps |= (u8)(PM_CAP_SECURE);
	} else {
		PmRmw32(BaseAddr + Offset1, Mask1, Mask1);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, Mask2);
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else {
		/* Required due to MISRA */
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus SetDevCohVirtAttr(XPm_Requirement *Reqm, u32 ReqCaps,
				 u8 Capability, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u8 CurrCaps = 0U;
	u32 BaseAddr, Offset, Mask;
	XPm_Power *FpdPower = NULL;
	XPm_Requirement *DevReqm = NULL;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;

	/* Skip if device does not have any attributes */
	if (NULL == DevAttr) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Get base address, mask and offset based on capability */
	BaseAddr = DevAttr->CohVirtBaseAddr;
	if ((u8)(PM_CAP_COHERENT) == Capability) {
		Offset = DevAttr->Coherency.Offset;
		Mask = DevAttr->Coherency.Mask;
	} else {
		Offset = DevAttr->Virtualization.Offset;
		Mask = DevAttr->Virtualization.Mask;
	}

	/* Do nothing if required attributes are not present */
	if ((0U == Mask) || (0U == BaseAddr)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (1U == Enable) {
		/* Do nothing if capability is not requested */
		if (0U == (ReqCaps & Capability)) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Update AttrCaps flag of requirement */
		if (0U != (Reqm->AttrCaps & Capability)) {
			Reqm->AttrCaps &= (u8)(~Capability);
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	FpdPower = XPmPower_GetById(PM_POWER_FPD);
	if (NULL == FpdPower) {
		goto done;
	}
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Reqm->Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		DevReqm = ReqmNode->Data;
		if (1U == DevReqm->Allocated) {
			CurrCaps |= DevReqm->AttrCaps;
		}
	}
	/* Do nothing if attribute is already set */
	if (0U != (CurrCaps & Capability)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (1U == Enable) {
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		PmRmw32(BaseAddr + Offset, Mask, Mask);
		Reqm->AttrCaps |= Capability;
	} else {
		PmRmw32(BaseAddr + Offset, Mask, 0U);
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	return Status;
}

maybe_unused static XStatus HandleDeviceAttr(struct XPm_Reqm *Reqm, u32 ReqCaps, u32 PrevState, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_COHERENT, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_VIRTUALIZED, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetSecurityAttr(Reqm, ReqCaps, PrevState);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get maximum of all requested capabilities of device
 * @param Device	Device whose maximum required capabilities should be
 *			determined
 *
 * @return	32bit value encoding the capabilities
 *
 * @note	None
 *
 ****************************************************************************/
static u32 GetMaxCapabilities(const XPm_Device* const Device)
{
	u32 MaxCaps = 0U;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode){
		XPm_Requirement* Reqm = ReqmNode->Data;
		MaxCaps |= Reqm->Curr.Capabilities;
	}
done:
	return MaxCaps;
}

XStatus XPmDevice_Request(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_REQ;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;
	u32 Idx;

	/* Todo: Check if policy allows this request */
	/* If not allowed XPM_PM_NO_ACCESS error should be returned */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}


	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State != (u8)ONLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Status = DevRequest(Device, Subsystem, Capabilities,
					    QoS, CmdType);
	if (XST_SUCCESS == Status) {
		/**
		 * More than one IPI channels can be associated with a subsystem;
		 * Update its IPI mask accordingly when an IPI channel is requested.
		 */
		for (Idx = 0; Idx < ARRAY_SIZE(IpiMasks); Idx++) {
			if (IpiMasks[Idx][0] == Device->Node.Id) {
				Subsystem->IpiMask |= IpiMasks[Idx][1];
				break;
			}
		}
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}


XStatus XPmDevice_Release(u32 SubsystemId, u32 DeviceId, u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_RELEASE;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;

	/* TODO: Check if subsystem has permission */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State == (u8)OFFLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	Status = DevRelease(Device, Subsystem, CmdType);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}



XStatus XPmDevice_Reset(const XPm_Device *Device, const XPm_ResetActions Action)
{
	XStatus Status = XST_FAILURE;
	const XPm_ResetHandle *RstHandle;
	const XPm_ResetHandle *DeviceHandle;
	XPm_ResetNode *Reset;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	/* TODO: Skip handling for PL resets until PL topology is available */
	if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Device->Node.Id)) {
		Status = XST_SUCCESS;
		goto done;
	}

#ifdef DEBUG_UART_PS
	/* Reset LPD init flag to stop debug prints which is using UART */
	if ((NODE_UART == Device->Node.Id) &&
	    (PM_RESET_ACTION_ASSERT == Action)) {
		PmDbg("Disabling UART prints\r\n");
		/* Wait for UART buffer to flush */
		usleep(10000);
		XPlmi_ResetLpdInitialized();
	}
#endif

	RstHandle = Device->RstHandles;
	/*
	 * Some devices do not have reset handlers, so exit without failure.
	 * In the case of TCM, there is special handling to release reset, so
	 * do not skip if it's a TCM device.
	 */
	if ((NULL == RstHandle) &&
	    ((u32)XPM_NODETYPE_DEV_TCM != NODETYPE(Device->Node.Id))) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (PM_RESET_ACTION_RELEASE != Action) {
		while (NULL != RstHandle) {
			Reset = RstHandle->Reset;
			DeviceHandle = Reset->RstHandles;
			while (NULL != DeviceHandle) {
				if ((Device->Node.Id !=
				    DeviceHandle->Device->Node.Id) &&
				    (((u32)XPM_DEVSTATE_RUNNING ==
				    DeviceHandle->Device->Node.State) ||
				    ((u32)XPM_DEVSTATE_PENDING_PWR_DWN ==
				    DeviceHandle->Device->Node.State))) {
					break;
				}
				DeviceHandle = DeviceHandle->NextDevice;
			}
			if (NULL == DeviceHandle) {
				Status = XPmReset_AssertbyId(Reset->Node.Id, Action);
				if (XST_SUCCESS != Status) {

					goto done;
				}
			}
			RstHandle = RstHandle->NextReset;
		}
	} else {
		/** If Device is TCM we handle TCM reset differently */
		if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Device->Node.Id)) &&
		    ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(Device->Node.Id)) &&
		    ((u32)XPM_NODETYPE_DEV_TCM == NODETYPE(Device->Node.Id))) {
			Status = XPmMem_TcmResetReleaseById(Device->Node.Id);
			if (XST_SUCCESS != Status) {
				PmErr("Failed to release TCM reset\r\n");
				goto done;
			}
			goto done;
		}
		/* Release reset for all devices in the reset handle */
		while (NULL != RstHandle) {
			/* Release reset for all devices in the reset handle */
			Status = XPmReset_AssertbyId(RstHandle->Reset->Node.Id, Action);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			RstHandle = RstHandle->NextReset;
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static XStatus DevRequest(XPm_Device *Device, XPm_Subsystem *Subsystem,
		       u32 Capabilities, u32 QoS, u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_REQ;
	XPm_Requirement *Reqm = NULL;
	u16 UsagePolicy = 0;
	u32 PrevState;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}
	PrevState = Device->Node.State;
	(void)PrevState;
	(void)CmdType;

	/* Check whether this device assigned to the subsystem */
	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		/* HACK: AIE device requirement is added implicitly when requested */
		Reqm = XPmDevice_GetAieReqmDefault(Device, Subsystem);
		if (NULL == Reqm) {
			goto done;
		}
	}

	/* TODO:: Enable this handling separately for eemi and subsys */
	// Status = CheckSecurityAccess(Reqm, Capabilities, CmdType);
	// if (XST_SUCCESS != Status) {
	// 	goto done;
	// }

	if (1U == Reqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check whether this device is shareable */
	UsagePolicy = USAGE_POLICY(Reqm->Flags);
	if ((UsagePolicy == (u16)REQ_TIME_SHARED) || (UsagePolicy == (u16)REQ_NONSHARED)) {
		XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
		if (NULL == DevOps) {
			PmErr("DeviceOps is NULL\r\n");
			goto done;
		}
		//Check if it already requested by other subsystem. If yes, return
		LIST_FOREACH(DevOps->Requirements, ReqmNode){
			XPm_Requirement *NextReqm = ReqmNode->Data;
			if (1 == NextReqm->Allocated) {
				Status = XPM_PM_NODE_USED;
				goto done;
			}
		}
	}

	/* Allocated device for the subsystem */
	Reqm->Allocated = 1U;

	Status = SetDevRequirement(Device, Subsystem, Capabilities, QoS);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO:: Enable this handling separately for eemi and subsys */
	// Status = HandleDeviceAttr(Reqm, Capabilities, PrevState, 1U);
	// if (XST_SUCCESS != Status) {
	// 	goto done;
	// }

done:
	if (XST_SUCCESS != Status) {
		XPmRequirement_Clear(Reqm);
		PmErr("0x%x, Id: 0x%x\r\n", Status, Device->Node.Id);
	}
	return Status;
}

static XStatus DevRelease(XPm_Device *Device,  XPm_Subsystem *Subsystem, u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_RELEASE;
	XPm_Requirement *Reqm = NULL;
	u32 PrevState;

	PmInfo("Device: 0x%x, Subsystem: 0x%x, State: 0x%x\r\n",
		Device->Node.Id, Subsystem->Id, Device->Node.State);

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}
	PrevState = Device->Node.State;
	(void)PrevState;
	(void)CmdType;

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		goto done;
	}
	PmInfo("Device State: 0x%x, Reqm->Allocated: 0x%x\r\n", Device->Node.State, Reqm->Allocated);

	/* TODO:: Enable this handling separately for eemi and subsys */
	// Status = CheckSecurityAccess(Reqm, 0U, CmdType);
	// if (XST_SUCCESS != Status) {
	// 	goto done;
	// }

	if (0U == Reqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}
	Status = SetDevRequirement(Device, Subsystem, 0, XPM_DEF_QOS);
	if (XST_SUCCESS != Status) {
		Status = XPM_ERR_DEVICE_RELEASE;
		goto done;
	}
	XPmRequirement_Clear(Reqm);

	/* TODO:: Enable this handling separately for eemi and subsys */
	// Status = HandleDeviceAttr(Reqm, 0U, PrevState, 0U);
	// if (XST_SUCCESS != Status) {
	// 	goto done;
	// }

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x, Id: 0x%x\r\n", Status, Device->Node.Id);
	}
	return Status;
}

static XStatus SetDevRequirement(XPm_Device *Device, const XPm_Subsystem *Subsystem,
			      u32 Capabilities, const u32 QoS)
{
	XStatus Status = XPM_ERR_SET_REQ;;
	XPm_ReqmInfo TempReqm;
	XPm_Requirement *PendingReqm = NULL;

	PmInfo("Device: 0x%x, State: 0x%x\r\n", Device->Node.Id, Device->Node.State);

	/** HACK!! FIXME: Linux drivers can never request UART0 or UART1
	 * after releasing them; therefore, we have do this hack:
	 * Only make transition to RUNNING state for UART0 and UART1, any other state transtion will be ignored
	 */
	if (((PM_DEV_UART_0 == Device->Node.Id) || (PM_DEV_UART_1 == Device->Node.Id)) && (1 != Capabilities)) {
		PmInfo("Ignoring request for UART device: 0x%x\r\n", Device->Node.Id);
		Status = XST_SUCCESS;
		goto done;
	}

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
		((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
		PmErr("Device 0x%x is busy, Sate: 0x%x\r\n", Device->Node.Id, Device->Node.State);
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	PendingReqm = FindReqm(Device, Subsystem);
	if (NULL == PendingReqm) {
		goto done;
	}
	PendingReqm->IsPending = 1;
	/*
	 * If subsystem state is suspending then do not change device's state
	 * according to capabilities, only schedule requirements by setting
	 * device's next requirements.
	 */
	if ((u8)SUSPENDING == Subsystem->State) {
		PendingReqm->Next.Capabilities =
			(Capabilities & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE));
		PendingReqm->Next.QoS = QoS;
		Status = XST_SUCCESS;
		goto done;
	} else {
		/*
		 * Store current requirements as a backup in case something
		 * fails.
		 */
		TempReqm.Capabilities = PendingReqm->Curr.Capabilities;
		TempReqm.QoS = PendingReqm->Curr.QoS;

		PendingReqm->Curr.Capabilities =
			(Capabilities & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE));
		PendingReqm->Curr.QoS = QoS;
	}
	//Status = XST_SUCCESS;
	Status = XPmDevice_UpdateStatus(Device);

	if (XST_SUCCESS != Status) {
		PendingReqm->Curr.Capabilities = TempReqm.Capabilities;
		PendingReqm->Curr.QoS = TempReqm.QoS;
	} else if ((u32)PM_CAP_UNUSABLE == Capabilities) {
		/* Schedule next requirement to 0 */
		PendingReqm->Next.Capabilities = 0U;
		PendingReqm->Next.QoS = QoS;
	} else {
		XPm_RequiremntUpdate(PendingReqm);
	}

done:
	if (XST_SUCCESS != Status) {
		if (NULL != PendingReqm) {
			PendingReqm->IsPending = 0;
		}
		PmErr("0x%x, Id: 0x%x\r\n", Status, Device->Node.Id);
	}
	return Status;
}

XStatus XPmDevice_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
				 const u32 Capabilities, const u32 QoS)
{
	XStatus Status = XPM_ERR_SET_REQ;
	XPm_Device *Device;
	const XPm_Subsystem *Subsystem;

	/* TODO: Check if subsystem has permission */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}
	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State == (u8)OFFLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	Status = SetDevRequirement(Device, Subsystem, Capabilities, QoS);

	/*
	 * SetRequirement is used for dynamic AIE clock frequency scaling where
	 * QOS is the new divider value.
	 */
	if (IS_DEV_AIE(DeviceId)) {
		Status = XPmAieDevice_UpdateClockDiv(Device, QoS);
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get state with provided capabilities
 *
 * @param Device	Device whose states are searched
 * @param Caps		Capabilities the state must have
 * @param State		Pointer to a u32 variable where the result is put if
 *			state is found
 *
 * @return	Status of the operation
 *		- XST_SUCCESS if state is found
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus GetStateWithCaps(const XPm_Device* const Device, const u32 Caps,
				u32* const State)
{
	u32 Idx;
	XStatus Status = XPM_PM_CONFLICT;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	XPm_Fsm *Fsm = NULL;
	Status = XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get FSM for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}
	if (NULL == Fsm) {
		goto done;
	}

	for (Idx = 0U; Idx < Fsm->StatesCnt; Idx++) {
		/* Find the first state that contains all capabilities */
		if ((Caps & Fsm->States[Idx].Cap) == Caps) {
			Status = XST_SUCCESS;
			if (NULL != State) {
				*State = Fsm->States[Idx].State;
			}
			break;
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Find minimum of all latency requirements
 *
 * @Param  Device	Device whose min required latency is requested
 *
 * @return Latency in microseconds
 *
 ****************************************************************************/
static u32 GetMinRequestedLatency(const XPm_Device *const Device)
{
	u32 MinLatency = XPM_MAX_LATENCY;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode) {
		XPm_Requirement *NextReqm = ReqmNode->Data;
		if ((1U == NextReqm->SetLatReq) &&
		    (MinLatency > NextReqm->Next.Latency)) {
			MinLatency = NextReqm->Next.Latency;
		}
	}
done:
	return MinLatency;
}
/****************************************************************************/
/**
 * @brief  Get latency from given state to the highest state
 *
 * @param  Device	Pointer to the device whose states are in question
 * @param  State	State from which the latency is calculated
 *
 * @return Return value for the found latency
 *
 ****************************************************************************/
static u32 GetLatencyFromState(const XPm_Device *const Device, const u32 State)
{
	u32 Idx;
	u32 Latency = 0U;
	XStatus Status = XST_FAILURE;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	XPm_Fsm *Fsm = NULL;
	Status = XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get FSM for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}
	u32 HighestStateIdx = Fsm->StatesCnt - (u32)1U;
	u32 HighestState = Fsm->States[HighestStateIdx].State;

	for (Idx = 0U; Idx < Fsm->TransCnt; Idx++) {
		if ((State == Fsm->Trans[Idx].FromState) &&
		    (HighestState == Fsm->Trans[Idx].ToState)) {
			Latency = Fsm->Trans[Idx].Latency;
			break;
		}
	}
done:
	return Latency;
}

/****************************************************************************/
/**
 * @brief  Find a higher power state which satisfies latency requirements
 *
 * @param  Device	Device whose state may be constrained
 * @param  State	Chosen state which does not satisfy latency requirements
 * @param  CapsToSet	Capabilities that the state must have
 * @param  MinLatency	Latency requirements to be satisfied
 *
 * @return Status showing whether the higher power state is found or not.
 * State may not be found if multiple subsystem have contradicting requirements,
 * then XST_FAILURE is returned. Otherwise, function returns success.
 *
 ****************************************************************************/
static XStatus ConstrainStateByLatency(const XPm_Device *const Device,
				   u32 *const State, const u32 CapsToSet,
				   const u32 MinLatency)
{
	XStatus Status = XST_FAILURE;
	u32 WkupLat;
	u32 Idx = 0;

	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	XPm_Fsm *Fsm = NULL;
	Status = XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get FSM for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}
	/*
	 * Need to find higher power state, so ignore lower power states
	 * and find index for chosen state
	 */
	while (Fsm->States[Idx].State != *State)
	{
		Idx++;
	}

	for (; Idx < Fsm->StatesCnt; Idx++) {
		if ((CapsToSet & Fsm->States[Idx].Cap) != CapsToSet) {
			/* State candidate has no required capabilities */
			continue;
		}
		WkupLat = GetLatencyFromState(Device, Fsm->States[Idx].State);
		if (WkupLat > MinLatency) {
			/* State does not satisfy latency requirement */
			continue;
		}

		Status = XST_SUCCESS;
		*State = Fsm->States[Idx].State;
		break;
	}
done:
	return Status;
}


static XStatus HandleDeviceEvent(XPm_Device* Device, const u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("Runtime Device Ops is not initalized. Device ID = 0x%x\n", Device->Node.Id);
		Status = XST_FAILURE;
		goto done;
	}

	XPm_Fsm *Fsm = NULL;
	// Get the FSM for the device
	if (XST_SUCCESS != XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm)) {
		PmErr("Failed to get FSM for Device ID = 0x%x\n", Device->Node.Id);
		Status = XST_FAILURE;
		goto done;
	}

	const XPmFsm_Tran* EventTransitions = Fsm->Trans;
	u8 TransCnt = Fsm->TransCnt;

	// Find the transition for the current state and event
	for (u8 i = 0; i < TransCnt; i++) {
		if (EventTransitions[i].Event == Event && EventTransitions[i].FromState == Device->Node.State) {
			u8 NextState = EventTransitions[i].ToState;
			// Perform the necessary actions to transition to the next state
			if (EventTransitions[i].Action == NULL) {
				PmWarn("Action is NULL for transition FromState=%d ToState=%d Event=%d\n", EventTransitions[i].FromState, EventTransitions[i].ToState, EventTransitions[i].Event);
				Status = XST_SUCCESS;
				goto done;
			}
			if (EventTransitions[i].Action(Device) == XST_SUCCESS) {
				// Update the current state of the device
				Device->Node.State = NextState;
				return XST_SUCCESS;
		} else {
			return XST_FAILURE;
			}
		}
	}
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Change state of a device
 *
 * @param Device	Device pointer whose state should be changed
 * @param NextState		New state
 *
 * @return	XST_SUCCESS if transition was performed successfully.
 *              Error otherwise.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDevice_ChangeState(XPm_Device *Device, const u32 NextState)
{
	XStatus Status = XPM_ERR_SETSTATE;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	PmInfo("Device: 0x%x, State: 0x%x, NextState: 0x%x\r\n", Device->Node.Id,
		Device->Node.State, NextState);

	XPm_Fsm* Fsm = NULL;
	Status = XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get FSM for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}
	u32 OldState = Device->Node.State;
	u32 Trans;
	if (NULL == Fsm) {
		Status = XST_FAILURE;
		PmErr("FSM is Null");
		goto done;
	}
	if (0U == Fsm->TransCnt) {
		/* Device's FSM has no transitions when it has only one state */
		Status = XST_SUCCESS;
		goto done;
	}

	for (Trans = 0U; Trans < Fsm->TransCnt; Trans++) {
		/* Find transition from current state to next state */
		if ((Fsm->Trans[Trans].FromState != Device->Node.State) ||
			(Fsm->Trans[Trans].ToState != NextState)) {
			continue;
		}

		Status = HandleDeviceEvent(Device, Fsm->Trans[Trans].Event);
		break;
	}

	if ((OldState != NextState) && (XST_SUCCESS == Status)) {
		Device->Node.State = (u8)NextState;
		/** TODO: Add Notificiation on this state transition*/
		// /* Send notification about device state change */
		// XPmNotifier_Event(Device->Node.Id, (u32)EVENT_STATE_CHANGE);
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  Device updates its power parent about latency req
 *
 * @param  Device	Device whose latency requirement have changed
 *
 * @return If the change of the latency requirement caused the power up of the
 * power parent, the status of performing power up operation is returned,
 * otherwise XST_SUCCESS is returned.
 *
 ****************************************************************************/
static XStatus UpdatePwrLatencyReq(const XPm_Device *const Device)
{
	XStatus Status = XST_FAILURE;
	XPm_Power* Power = Device->Power;

	if ((u8)XPM_POWER_STATE_ON == Power->Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Power is down, check if latency requirements trigger the power up */
	if (Device->Node.LatencyMarg <
	    (Power->PwrDnLatency + Power->PwrUpLatency)) {
		Power->Node.LatencyMarg = 0U;
		Status = Power->HandleEvent(&Power->Node, XPM_POWER_EVENT_PWR_UP);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief	Update the device's state according to the current requirements
 *		from all subsystems
 * @param Device	Device whose state is about to be updated
 *
 * @return      Status of operation of updating device's state.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDevice_UpdateStatus(XPm_Device *Device)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	u32 Caps = GetMaxCapabilities(Device);
	u32 WkupLat, MinLat;
	u32 State = 0;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_PENDING_PWR_DWN != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
		((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Status = GetStateWithCaps(Device, Caps, &State);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get caps for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}

	MinLat = GetMinRequestedLatency(Device);
	WkupLat = GetLatencyFromState(Device, State);
	if (WkupLat > MinLat) {
		/* State does not satisfy latency requirement, find another */
		Status = ConstrainStateByLatency(Device, &State, Caps, MinLat);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		WkupLat = GetLatencyFromState(Device, State);
	}

	Device->Node.LatencyMarg = (u16)(MinLat - WkupLat);

	if (State != Device->Node.State) {
		Status = XPmDevice_ChangeState(Device, State);
	} else {
		if (((u8)XPM_DEVSTATE_UNUSED == Device->Node.State) &&
		    (NULL != Device->Power)) {
			/* Notify power parent (changed latency requirement) */
			Status = UpdatePwrLatencyReq(Device);
		}
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x, Id: 0x%x\r\n", Status, Device->Node.Id);
	}
	return Status;
}

XStatus XPmDevice_CheckPermissions(const XPm_Subsystem *Subsystem, u32 DeviceId)
{
	volatile XStatus Status = XPM_PM_NO_ACCESS;
	volatile XStatus StatusTmp = XPM_PM_NO_ACCESS;
	const XPm_Requirement *Reqm;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);

	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		goto done;
	}

	/* Redundant call on simple inline check to detect glitches */
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, XPmRequirement_IsAllocated, Reqm);

	if (XST_SUCCESS != Status || XST_SUCCESS != StatusTmp) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
XStatus XPmDevice_GetPermissions(const XPm_Device *Device, u32 *PermissionMask)
{
	XStatus Status = XST_FAILURE;
	u32 Idx;
	u32 SubsysIdx = XPmSubsystem_GetMaxSubsysIdx();

	if ((NULL == Device) || (NULL == PermissionMask)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode) {
		XPm_Requirement *Reqm = ReqmNode->Data;
		if (1U == Reqm->Allocated) {
			for (Idx = 0; Idx <= SubsysIdx; Idx++) {
				if (Reqm->Subsystem == XPmSubsystem_GetByIndex(Idx)) {
					*PermissionMask |= ((u32)1U << Idx);
				}
			}
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  Get the current usage status for a given device.
 * @param  Subsystem   Subsystem for which usage status is in query
 * @slave  Device      Device for which usage status need to be calculated
 *
 * @return  Usage status:
 *          - 0: No subsystem is currently using the device
 *          - 1: Only requesting subsystem is currently using the device
 *          - 2: Only other subsystems are currently using the device
 *          - 3: Both the current and at least one other subsystem is currently
 *               using the device
 *
 ****************************************************************************/
u32 XPmDevice_GetUsageStatus(const XPm_Subsystem *Subsystem, const XPm_Device *Device)
{
	u32 UsageStatus = 0;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("Device Id = 0x%x is not initialized properly\r\n", Device->Node.Id);
		return UsageStatus;
	}
	LIST_FOREACH(DevOps->Requirements, ReqmNode) {
		XPm_Requirement *Reqm = ReqmNode->Data;
		if (1U == Reqm->Allocated){
			/* This subsystem is currently using this device */
			if (Subsystem == Reqm->Subsystem) {
				UsageStatus |= (u32)PM_USAGE_CURRENT_SUBSYSTEM;
			} else {
				UsageStatus |= (u32)PM_USAGE_OTHER_SUBSYSTEM;
			}
		}
	}

	return UsageStatus;
}

XStatus XPmDevice_GetStatus(XPm_Subsystem *Subsystem,
			    u32 DeviceId,
			    XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	const XPm_Device *Device;
	const XPm_Requirement *Reqm;
	u32 ClkStatus = XPM_CLK_STATE_INVALID, RstStatus = XPM_RST_STATE_INVALID;

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}


	/*
	 * For RPU cores, we need to verify the following conditions:
	 * 1. Check if the core is in halt state
	 * 2. Verify that reset is released
	 * 3. Confirm that clock is enabled
	 * This ensures the RPU core is in halt state
	 */
	/* If RstStatus is invalid it returns current device state */
	if(NODETYPE(Device->Node.Id) == (u32)XPM_NODETYPE_DEV_CORE_RPU){
		Status = XPm_GetClockState(Device->ClkHandles->Clock->Node.Id, &ClkStatus);
		const XPm_ResetHandle *RstHandle = Device->RstHandles;
		while (NULL != RstHandle) {
			Status = XPmReset_GetStateById(RstHandle->Reset->Node.Id, &RstStatus);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			if(0x1U == RstStatus){
				break;
			}
			RstHandle = RstHandle->NextReset;
		}
		if((((XPm_In32(((XPm_RpuCore *)Device)->ResumeCfg) & XPM_RPU_CPUHALT_MASK) == XPM_RPU_CPUHALT_MASK)) &&
			(0x1U == ClkStatus) && (0U == RstStatus)){
				DeviceStatus->Status = (u32)XPM_DEVSTATE_HALT;
		}
		else{
			DeviceStatus->Status = Device->Node.State;
		}
	}
	else{
		DeviceStatus->Status = Device->Node.State;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL != Reqm) {
		DeviceStatus->Requirement = Reqm->Curr.Capabilities;
	}

	DeviceStatus->Usage = XPmDevice_GetUsageStatus(Subsystem, Device);

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function checks device capability
 *
 * @param Device	Device for capability check
 * @param Caps		Capability
 *
 * @return XST_SUCCESS if desired Caps is available in Device
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_CheckCapabilities(const XPm_Device *Device, u32 Caps)
{
	u32 Idx;
	XStatus Status = XST_FAILURE;
	XPmRuntime_DeviceOps* DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("DeviceOps is NULL\r\n");
		goto done;
	}
	XPm_Fsm *Fsm = NULL;
	Status = XPmFsm_GetFsmByType(DevOps->FsmType, &Fsm);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get FSM for device 0x%x\r\n", Device->Node.Id);
		goto done;
	}

	for (Idx = 0U; Idx <  Fsm->StatesCnt; Idx++) {
		/* Find the first state that contains all capabilities */
		if ((Caps &  Fsm->States[Idx].Cap) == Caps) {
			Status = XST_SUCCESS;
			break;
		}
	}

done:
	if (Status != XST_SUCCESS) {
		Status = XST_NO_FEATURE;
	}
	return Status;
}

XStatus XPmDevice_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 Lat = 0;

	*Latency = 0;

	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u8)XPM_DEVSTATE_RUNNING == Device->Node.State) {
		goto done;
	}

	*Latency = GetLatencyFromState(Device, Device->Node.State);

	if (NULL != Device->Power) {
		Status = XPmPower_GetWakeupLatency(Device->Power->Node.Id, &Lat);
		if (XST_SUCCESS != Status) {
			*Latency += Lat;
		}
	}

done:
	return Status;
}


struct XPm_Reqm *XPmDevice_FindRequirement(const u32 DeviceId, const u32 SubsystemId)
{
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	XPm_Requirement *Reqm = NULL;

	if ((NULL == Device) || (NULL == Subsystem)) {
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
done:
	return Reqm;
}

/****************************************************************************/
/**
 * @brief	Add AIE device requirement if one has not already been added
 *
 * @param	Device		AIE device to add requirement for
 * @param	Subsystem	Subsystem which requirement will belong to
 *
 * @return	Requirement node
 *
 ****************************************************************************/
XPm_Requirement *XPmDevice_GetAieReqmDefault(XPm_Device *Device, XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm = NULL;

	if (IS_DEV_AIE(Device->Node.Id)) {
		Status = XPmRequirement_Add(Subsystem, Device,
					    (u32)REQUIREMENT_FLAGS(0U,
						(u32)REQ_ACCESS_SECURE_NONSECURE,
						(u32)REQ_NO_RESTRICTION),
					    0U, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			PmErr("Error adding AIE device requirement\r\n");
			goto done;
		}

		/* Get requirement */
		Reqm = XPmDevice_FindRequirement(Device->Node.Id, Subsystem->Id);
	}

done:
	return Reqm;
}

/****************************************************************************/
/**
 * @brief	Get subsystem ID of processor
 *
 * @param  Device	Processor whose subsystem needs to found
 *
 * @return	Subsystem ID of that processor
 *
 * @note	Core must be requested from single subsystem. If it is
 *		requested from multiple subsystems then it returns only one
 *		subsystem ID and if it is not requested from any subsystem
 *		then this function returns maximum subsystem ID which is
 *		invalid.
 *
 ****************************************************************************/
u32 XPmDevice_GetSubsystemIdOfCore(const XPm_Device *Device)
{
	const XPm_Requirement *Reqm;
	const XPm_Subsystem *Subsystem = NULL;
	u32 Idx, SubSystemId;
	u32 SubsysIdx = XPmSubsystem_GetMaxSubsysIdx();

	for (Idx = 0; Idx <= SubsysIdx; Idx++) {
		Subsystem = XPmSubsystem_GetByIndex(Idx);
		if (NULL != Subsystem) {
			Reqm = FindReqm(Device, Subsystem);
			if ((NULL != Reqm) && (1U == Reqm->Allocated)) {
				break;
			}
		}
	}

	if (SubsysIdx < Idx) {
		SubSystemId = INVALID_SUBSYSID;
	} else {
		SubSystemId = Subsystem->Id;
	}

	return SubSystemId;
}

/****************************************************************************/
/**
 * @brief  Set maximum allowed latency for the device
 *
 * @param  SubsystemId	Initiator of the request who must previously requested
 *			the device
 * @param  DeviceId	Device whose latency is specified
 * @param  Latency	Maximum allowed latency in microseconds
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPmDevice_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
			    const u32 Latency)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	XPm_Device *Device = XPmDevice_GetById(DeviceId);

	if ((NULL == Subsystem) || (NULL == Device)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		Status = XPM_ERR_DEVICE_REQ;
		goto done;
	}

	Reqm->Next.Latency =
		(Latency & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE));
	Reqm->SetLatReq = 1;

	Status = XPmDevice_UpdateStatus(Device);
	if (XST_SUCCESS != Status) {
		Reqm->SetLatReq = 0;
		goto done;
	}

	Reqm->Curr.Latency =
		(Latency & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE));

done:
	return Status;
}
