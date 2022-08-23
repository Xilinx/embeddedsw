/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_pldevice.h"
#include "xpm_aiedevice.h"

void XPmPlDevice_ReleaseAieDevice(XPm_PlDevice *PlDevice){
	/*  Release AIE device from PlDevice*/
	XPm_AieDevice *AieToUnlink= PlDevice->AieDevice;
	if (NULL != AieToUnlink) {
		AieToUnlink->Parent = NULL;
		PlDevice->AieDevice = NULL;
	}
}

void XPmPlDevice_GetAieParent(const XPm_Device *Device, const XPm_PlDevice **OutParent){
	/* Get parent of an AIE device */
	*OutParent = ((XPm_AieDevice *)Device)->Parent;
}