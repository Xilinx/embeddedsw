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

XStatus XPmDevice_SaveDevice(XPm_Device* ThisData, u32** SavedData)
{
	XStatus Status = XST_FAILURE;

	BEGIN_SAVE_STRUCT((*SavedData), XPmNode_SaveNode, ((XPm_Node*)ThisData));
	SaveStruct(Status, done, (ThisData->WfDealloc << 8) | (ThisData->WfPwrUseCnt) );
	/* TODO: need to save all requirements */
	/* Saving  Pending Requirement */
	if (((ThisData->PendingReqm ) != NULL)  && ((ThisData->PendingReqm->Subsystem)!= NULL) && ((ThisData->PendingReqm->Device)!= NULL))
	{
		SaveStruct(Status, done, ThisData->PendingReqm->Subsystem->Id);
		SaveStruct(Status, done, ThisData->PendingReqm->Device->Node.Id);
		u32 SaveCap = (ThisData->PendingReqm->Curr.Capabilities) & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
		u32 SaveLat = (ThisData->PendingReqm->Curr.Latency) & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
		SaveStruct(Status, done, (SaveLat << REQ_INFO_CAPS_BIT_FIELD_SIZE)| SaveCap);
		SaveCap = (ThisData->PendingReqm->Next.Capabilities) & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
		SaveLat = (ThisData->PendingReqm->Next.Latency) & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
		SaveStruct(Status, done, (SaveLat << REQ_INFO_CAPS_BIT_FIELD_SIZE)| SaveCap);
		SaveStruct(Status, done, ThisData->PendingReqm->Next.QoS);
	}
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
	u32 tmp;
	RestoreStruct((*NextSavedData), tmp);
	ThisData->WfPwrUseCnt = (tmp & 0xFFU);
	ThisData->WfDealloc = ((tmp >> 8) & 0xFFU);
	/* TODO: need to restore all requirements */
	/* Restoring Pending Requirement */
	if (SAVED_DATA_GET_SIZE(SavedData) > 5)
	{
		u32 SubsystemId;
		RestoreStruct((*NextSavedData), SubsystemId);
		ThisData->PendingReqm->Subsystem = XPmSubsystem_GetById(SubsystemId);
		u32 NodeId;
		RestoreStruct((*NextSavedData), NodeId);
		ThisData->PendingReqm->Device = XPmDevice_GetById(NodeId);
		ThisData->PendingReqm = FindReqm(ThisData->PendingReqm->Device , ThisData->PendingReqm->Subsystem);
		if (ThisData->PendingReqm == NULL)
		{
			Status = XPM_UPDATE_PENDREQ_NULL;
			goto done;
		}
		RestoreStruct((*NextSavedData), tmp);
		ThisData->PendingReqm->Curr.Capabilities = tmp & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
		ThisData->PendingReqm->Curr.Latency	=  (tmp >> REQ_INFO_CAPS_BIT_FIELD_SIZE) &  BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
		RestoreStruct((*NextSavedData), tmp);
		ThisData->PendingReqm->Next.Capabilities = tmp & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE);
		ThisData->PendingReqm->Next.Latency	=  (tmp >> REQ_INFO_CAPS_BIT_FIELD_SIZE) &  BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE);
		RestoreStruct((*NextSavedData),ThisData->PendingReqm->Next.QoS);
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
