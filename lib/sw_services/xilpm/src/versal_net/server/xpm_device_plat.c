/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_pmc.h"

#define SD_DLL_DIV_MAP_RESET_VAL	(0x50505050U)

static XPm_Requirement *FindReqm(const XPm_Device *Device, const XPm_Subsystem *Subsystem)
{
	XPm_Requirement *Reqm = NULL;

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (Reqm->Subsystem == Subsystem) {
			break;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Reqm;
}
static u32 IsReqmModified(XPm_Requirement* Reqm)
{
	if (
		(Reqm->Allocated == 1) \
		|| (Reqm->Curr.Capabilities != XPM_MIN_CAPABILITY) \
		|| (Reqm->Curr.Latency != XPM_MAX_LATENCY) \
		|| (Reqm->Curr.QoS != XPM_MAX_QOS) \
		|| (Reqm->Next.Capabilities != XPM_MIN_CAPABILITY) \
		|| (Reqm->Next.Latency != XPM_MAX_LATENCY) \
		|| (Reqm->Next.QoS != XPM_MAX_QOS))
	{
		return 1;
	}
	return 0;

}

static XStatus XPmDevice_SaveReqm(XPm_Requirement* Reqm)
{
	XStatus Status = XST_FAILURE;
	/* Save Subsystem Id and Node Id*/
	SaveStruct(Status, done, Reqm->Subsystem->Id);
	SaveStruct(Status, done, Reqm->Device->Node.Id);

	/* Save Curr's Capabilities and Lattency */
	u32 SaveCap = (Reqm->Curr.Capabilities) & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
	u32 SaveLat = (Reqm->Curr.Latency) & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
	SaveStruct(Status, done, (SaveLat << REQ_INFO_CAPS_BIT_FIELD_SIZE)| SaveCap);

	/* Save Next's Capabilities and Lattency */
	SaveCap = (Reqm->Next.Capabilities) & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
	SaveLat = (Reqm->Next.Latency) & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
	SaveStruct(Status, done, (SaveLat << REQ_INFO_CAPS_BIT_FIELD_SIZE)| SaveCap);

	/* Save Next's QoS*/
	SaveStruct(Status, done, Reqm->Next.QoS);

	/* Save Allocated and Flags */
	SaveStruct(Status, done, ((Reqm->Flags) << 16U) | Reqm->Allocated);

	/* Save SetLatReq, PreallocCaps, AttrCaps*/
	SaveStruct(Status, done, ((Reqm->SetLatReq) << 16U) | (Reqm->PreallocCaps << 8) | (Reqm->AttrCaps));


	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus GetReqmFromIds(u32 SubsystemId, u32 DeviceId, XPm_Requirement** Reqm)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem)
	{
		PmErr("ERR:Subsystem 0x%x Not Found!", SubsystemId);
		goto done;
	}
	XPm_Device* Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device)
	{
		PmErr("ERR:Device 0x%x Not Found!", DeviceId);
		goto done;
	}
	*Reqm = FindReqm(Device , Subsystem);
	if (NULL == (*Reqm))
	{
		Status = XPM_UPDATE_PENDREQ_NULL;
		PmErr("ERR:Requirement of Device 0x%x and Subsystem 0x%x Not Found!", DeviceId, SubsystemId);
		goto done;
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus XPmDevice_RestoreReqm(u32** NextSavedData)
{
	XStatus Status = XST_FAILURE;
	u32 SubsystemId,NodeId,tmp = 0U;

	/* Restore Subsystem and Device Node */
	RestoreStruct((*NextSavedData), SubsystemId);
	RestoreStruct((*NextSavedData), NodeId);

	/* Restored Requirements */
	XPm_Requirement* Reqm;
	Status = GetReqmFromIds(SubsystemId, NodeId, &Reqm);
	if (XST_SUCCESS != Status)
	{
		goto done;
	}

	/* Restored Curr's Capabilities and Latency */
	RestoreStruct((*NextSavedData), tmp);
	Reqm->Curr.Capabilities = tmp & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
	Reqm->Curr.Latency	=  (tmp >> REQ_INFO_CAPS_BIT_FIELD_SIZE) &  BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);

	/* Restored Next's Capabilities and Latency */
	RestoreStruct((*NextSavedData), tmp);
	Reqm->Next.Capabilities = tmp & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
	Reqm->Next.Latency	=  (tmp >> REQ_INFO_CAPS_BIT_FIELD_SIZE) &  BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);

	/* Restored Next's QoS */
	RestoreStruct((*NextSavedData),Reqm->Next.QoS);

	/* Restored Allocated */

	RestoreStruct((*NextSavedData), tmp);
	Reqm->Flags = (u16)((tmp >> 16) & 0xFFFF);
	Reqm->Allocated = tmp & 0xFF;

	/* Save SetLatReq, PreallocCaps, AttrCaps*/
	RestoreStruct((*NextSavedData), tmp);
	Reqm->AttrCaps = tmp & 0xFF;
	Reqm->PreallocCaps = (tmp >> 8) & 0xFF;
	Reqm->SetLatReq = (tmp >> 16) & 0xFF;

	Status = XST_SUCCESS;
done:
	return Status;

}

XStatus XPmDevice_SaveDevice(XPm_Device* ThisData, u32** SavedData)
{
	XStatus Status = XST_FAILURE;

	BEGIN_SAVE_STRUCT((*SavedData), XPmNode_SaveNode, ((XPm_Node*)ThisData));

	u32 *SavedReqmInfo = (u32*)XPmUpdate_DynAllocBytes(sizeof(u32));
	u32 NumSavedReqm = 0U, HasPending = 0U;
	SaveStruct(Status, done, (ThisData->WfDealloc << 8) | (ThisData->WfPwrUseCnt) );

	for (XPm_Requirement* curReqm = ThisData->Requirements; curReqm != NULL; curReqm= curReqm->NextSubsystem){
		if (IsReqmModified(curReqm))
		{
			Status = XPmDevice_SaveReqm(curReqm);
			if (XST_SUCCESS != Status)
			{
				goto done;
			}
			NumSavedReqm++;
			if (NumSavedReqm >= 0xFFFFU)
			{
				Status = XPM_UPDATE_MAX_NUM_REQM;
			}
		}
	}

	if (((ThisData->PendingReqm ) != NULL)  && ((ThisData->PendingReqm->Subsystem)!= NULL) && ((ThisData->PendingReqm->Device)!= NULL))
	{
		SaveStruct(Status, done, ThisData->PendingReqm->Subsystem->Id);
		SaveStruct(Status, done, ThisData->PendingReqm->Device->Node.Id);
		HasPending = 1;
	}
	*SavedReqmInfo = ((HasPending << 16) | (NumSavedReqm & 0xFFFFU));
	END_SAVE_STRUCT((*SavedData));

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

XStatus XPmDevice_RestoreDevice(u32* SavedData, XPm_Device* ThisData, u32** NextSavedData)
{
	XStatus Status = XPmNode_RestoreNode(SavedData, &(ThisData->Node), NextSavedData);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	u32 tmp = 0U ,NumSavedReqm = 0U, HasPending = 0U;
	RestoreStruct((*NextSavedData), tmp);
	NumSavedReqm = (tmp & 0xFFFFU);
	HasPending = (tmp >> 16) & 0x1U;

	RestoreStruct((*NextSavedData), tmp);
	ThisData->WfPwrUseCnt = (tmp & 0xFFU);
	ThisData->WfDealloc = ((tmp >> 8) & 0xFFU);

	for(u32 i = 0U; i < NumSavedReqm; i++){
		Status = XPmDevice_RestoreReqm(NextSavedData);
		if (XST_SUCCESS != Status)
		{
			goto done;
		}
	}

	if (1U == HasPending)
	{
		u32 SubsystemId,NodeId;
		RestoreStruct((*NextSavedData), SubsystemId);
		RestoreStruct((*NextSavedData), NodeId);
		Status = GetReqmFromIds(SubsystemId, NodeId, &(ThisData->PendingReqm));
		if (XST_SUCCESS != Status)
		{
			goto done;
		}
	}

done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

XStatus XPmDevice_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op)
{
	XStatus Status = XST_FAILURE;
	u32* NextSavedData = NULL;

	if (XPLMI_STORE_DATABASE  == Op){
		Status = XPmDevice_SaveDevice((XPm_Device*)ThisData, &NextSavedData);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = XPmDevice_RestoreDevice(SavedData, (XPm_Device*)ThisData, &NextSavedData);
		goto done;
	}

	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}

static XStatus ResetSdDllRegs(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	u32 Value;
	u32 BaseAddress;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	BaseAddress = Pmc->PmcIouSlcrBaseAddr;
	if (PM_DEV_SDIO_0 == Device->Node.Id) {
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			Value);
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			Value);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmDevice_SdResetWorkaround(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	/*
	 * As per EDT-1054057 SD/eMMC DLL modes are failing after
	 * SD controller reset. Reset SD_DLL_MAP registers after
	 * reset release as a workaround.
	 */
	/* SDIO1 is EMMC and there is no DLL_RESET for SDIO1 */
	if ((PM_DEV_SDIO_0 == Device->Node.Id)) {
		Status = ResetSdDllRegs(Device);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}
