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

#ifndef __VVDEV_WDR52_H__
#define __VVDEV_WDR52_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_wdr_api.h"

int VsiVvbenchParseWdrInfo
(
	cJSON *wdrInfo,
	VvbenchModuleCfg_t *wdrCtrl
);

int VsiVvbenchWdrGetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchWdrFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

#endif //__VVDEV_WDR52_H__
