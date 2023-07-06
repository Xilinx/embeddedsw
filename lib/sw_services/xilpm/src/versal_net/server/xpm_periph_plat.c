/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_periph.h"

static XStatus XPmPeriph_SavePeriph(XPm_Periph* ThisData)
{
	XStatus Status = XST_FAILURE;
	u32* SavedData = NULL;
	BEGIN_SAVE_STRUCT(SavedData, XPmDevice_SaveDevice, &(ThisData->Device));
	SaveStruct(Status, done, ThisData->WakeProcId);
	END_SAVE_STRUCT(SavedData);

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus XPmPeriph_RestorePeriph(u32* SavedData, XPm_Periph* ThisData){
	XStatus Status = XST_FAILURE;
	u32* DataAddr = NULL;
	Status = XPmDevice_RestoreDevice(SavedData, &(ThisData->Device), &DataAddr);
	if (XST_SUCCESS != Status)
	{
		goto done;
	}
	RestoreStruct(DataAddr, ThisData->WakeProcId);
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmPeriph_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op){
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE  == Op){
		Status = XPmPeriph_SavePeriph((XPm_Periph*)ThisData);
		goto done;
	}

	if (XPLMI_RESTORE_DATABASE == Op){
		Status = XPmPeriph_RestorePeriph(SavedData, (XPm_Periph*)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}