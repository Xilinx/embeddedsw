/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <xpm_clock.h>

XStatus XPmClock_SaveClockNode(XPm_ClockNode* ClockNodeData, u32** SavedData)
{
	XStatus Status = XST_FAILURE;
	*SavedData = NULL;

	BEGIN_SAVE_STRUCT((*SavedData), XPmNode_SaveNode, ((XPm_Node*)ClockNodeData));
	SaveStruct(Status, done, ((u32)((ClockNodeData->ParentIdx)<<16U)|(ClockNodeData->UseCount)));
	SaveStruct(Status, done, ClockNodeData->ClkRate);
	END_SAVE_STRUCT((*SavedData));
	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ClockNodeData);
	return Status;
}

XStatus XPmClock_RestoreClockNode(u32* SavedData, XPm_ClockNode* ThisData, u32** NextSavedData)
{
	XStatus Status = XST_FAILURE;
	Status = XPmNode_RestoreNode(SavedData, &(ThisData->Node), NextSavedData);

	if (XST_SUCCESS != Status) {
		goto done;
	}
	u32 tmp;
	RestoreStruct((*NextSavedData), tmp);
	ThisData->ParentIdx = (u16)((tmp >> 16) & 0xFFFFU);
	ThisData->UseCount = (tmp) & 0xFFU;
	RestoreStruct((*NextSavedData), ThisData->ClkRate);
done:
	return Status;
}

XStatus XPmClock_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op)
{
	XStatus Status = XST_FAILURE;
	u32* NextSavedData = NULL;
	if (XPLMI_STORE_DATABASE  == Op){
		Status = XPmClock_SaveClockNode((XPm_ClockNode*)ThisData, &NextSavedData);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = XPmClock_RestoreClockNode(SavedData, (XPm_ClockNode*)ThisData, &NextSavedData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}