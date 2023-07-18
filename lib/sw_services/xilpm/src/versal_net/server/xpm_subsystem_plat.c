/******************************************************************************
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_subsystem.h"

#include "xpm_update.h"

static XStatus SaveSubsystem(XPm_Subsystem* ThisData)
{
	XStatus Status = XST_FAILURE;

	/* Allocating 2 words. One for ID , one for Info*/
	u32* SavedData = (u32*)XPmUpdate_DynAllocBytes(2*sizeof(u32));

	if (NULL == SavedData)
	{
		Status = XPM_UPDATE_BUFFER_TOO_SMALL;
		goto done;
	}

	SAVED_DATA_SET_ID(SavedData, ThisData->Id);
	u32* StartDataAddr = (u32*)XPmUpdate_GetDynFreeBytes();
	SaveStruct(Status, done, (ThisData->State << 8U) | (ThisData->Flags));
	SaveStruct(Status, done, ThisData->PendCb);
	SaveStruct(Status, done, ThisData->FrcPwrDwnReq);
	SaveStruct(Status, done, ThisData->IpiMask);
	SAVED_DATA_SET_SIZE(SavedData,((((u32)XPmUpdate_GetDynFreeBytes()) - (u32)StartDataAddr)>>2));

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus RestoreSubsystem(u32* SavedData, XPm_Subsystem* ThisData)
{
	XStatus Status = XST_FAILURE;

	u32* DataAddr = NULL;
	if (NULL == SavedData)
	{
		Status = XPM_UPDATE_SAVED_DATA_NULL;
		goto done;
	}

	if (NULL == ThisData)
	{
		Status = XPM_UPDATE_NODE_DATA_NULL;
		goto done;
	}

	if (ThisData->Id != SAVED_DATA_ID(SavedData))
	{
		Status = XPM_UPDATE_ID_MISMATCH;
		PmErr("Error: ID mismatch SaveDataId = 0x%x NodeId = 0x%x \n\r", SAVED_DATA_ID(SavedData), ThisData->Id);
		goto done;
	}

	DataAddr = &(SavedData[XPM_SAVED_DATA_RETORE_OFFSET]);
	u32 tmp = 0U;
	RestoreStruct(DataAddr, tmp);
	ThisData->State = (u8)((tmp >> 8U) & 0xFFU);
	ThisData->Flags = (u8)((tmp) & 0xFFU);
	RestoreStruct(DataAddr, ThisData->PendCb);
	RestoreStruct(DataAddr, ThisData->FrcPwrDwnReq);
	RestoreStruct(DataAddr, ThisData->IpiMask);

	Status = XST_SUCCESS;
done:
	return  Status;
}

XStatus XPmSubsystem_SaveRestore(u32* SavedData, u32* ThisData, u32 Op)
{
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE == Op){
		Status = SaveSubsystem((XPm_Subsystem*)ThisData);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = RestoreSubsystem(SavedData, (XPm_Subsystem*)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}
