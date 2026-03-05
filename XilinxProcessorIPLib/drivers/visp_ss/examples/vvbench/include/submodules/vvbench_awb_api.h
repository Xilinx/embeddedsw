/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __VVDEV_AWB_H__
#define __VVDEV_AWB_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_awb_api.h"

int VsiVvbenchParseAwbInfo
(
	cJSON *awbInfo,
	VvbenchModuleCfg_t *awbCtrl
);

int VsiVvbenchAwbGetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchAwbFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchAwbGetStatistic
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchAwbGetResult
(
	CamDeviceHandle_t hCamDevice
);

#endif //__VVDEV_AWB_H__
