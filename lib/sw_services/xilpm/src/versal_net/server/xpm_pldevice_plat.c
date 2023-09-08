/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_pldevice.h"
#include "xpm_device.h"

void XPmPlDevice_ReleaseAieDevice(XPm_PlDevice *PlDevice){
	/* There is no AIE device in current Versal_Net devices family */
	(void)PlDevice;
}

void XPmPlDevice_GetAieParent(const XPm_Device *Device, const XPm_PlDevice **OutParent){
	/* There is no AIE device in current Versal_Net devices family */
	(void)Device;
	(void)OutParent;
}

static XStatus SavePlDeviceNode(XPm_PlDevice *ThisData)
{
	XStatus Status = XST_FAILURE;
	u32 *SavedData = NULL;

	BEGIN_SAVE_STRUCT(SavedData, XPmDevice_SaveDevice, (&(ThisData->Device)));
	SaveStruct(Status, done, (ThisData->Parent)? ThisData->Parent->Device.Node.Id: 0);
	SaveStruct(Status, done, (ThisData->Child)? ThisData->Child->Device.Node.Id: 0);
	SaveStruct(Status, done, (ThisData->NextPeer)? ThisData->NextPeer->Device.Node.Id: 0);
	SaveStruct(Status, done, (ThisData->PowerBitMask << 16) | (ThisData->WfPowerBitMask << 8) | (ThisData->MemCtrlrCount));
	END_SAVE_STRUCT(SavedData)

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus RestorePlDeviceNode(u32 *SavedData, XPm_PlDevice *ThisData)
{
	u32* NextSavedAddr = NULL;
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_RestoreDevice(SavedData, &(ThisData->Device), &NextSavedAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	u32 ParentId = 0;
	u32 ChildId = 0;
	u32 NextPeerId = 0;

	RestoreStruct(NextSavedAddr, ParentId);
	XPm_PlDevice* node = (XPm_PlDevice*)XPmDevice_GetById(ParentId);
	if (NULL == node && 0 != ParentId) {
		Status = XPM_UPDATE_NODE_DATA_NULL;
		goto done;
	}
	ThisData->Parent = node;

	RestoreStruct(NextSavedAddr, ChildId);
	node = (XPm_PlDevice*)XPmDevice_GetById(ChildId);
	if (NULL == node && 0 != ChildId) {
		Status = XPM_UPDATE_NODE_DATA_NULL;
		goto done;
	}
	ThisData->Child = node;

	RestoreStruct(NextSavedAddr, NextPeerId);
	node = (XPm_PlDevice*)XPmDevice_GetById(NextPeerId);
	if (NULL == node && 0 != NextPeerId) {
		Status = XPM_UPDATE_NODE_DATA_NULL;
		goto done;
	}
	ThisData->NextPeer = node;

	u32 tmp;
	RestoreStruct(NextSavedAddr, tmp);
	ThisData->PowerBitMask = (tmp >> 16) & 0xFFU;
	ThisData->WfPowerBitMask = (tmp >> 8) & 0xFFU;
	ThisData->MemCtrlrCount = (tmp) & 0xFFU;

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

XStatus XPmPlDevice_DoSaveRestore(u32 *SavedData, u32 *ThisData, u32 Op)
{
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE  == Op)
	{
		Status = SavePlDeviceNode((XPm_PlDevice *)ThisData);
		goto done;
	}

	if (XPLMI_RESTORE_DATABASE == Op)
	{
		Status = RestorePlDeviceNode(SavedData, (XPm_PlDevice *)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}
