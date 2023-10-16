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
