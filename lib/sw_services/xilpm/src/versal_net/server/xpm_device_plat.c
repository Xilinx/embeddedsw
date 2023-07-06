/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_requirement.h"
#include "xpm_device.h"

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