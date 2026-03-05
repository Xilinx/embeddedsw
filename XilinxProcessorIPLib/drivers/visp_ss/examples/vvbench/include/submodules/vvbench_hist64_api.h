/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __VVDEV_HIST64_H__
#define __VVDEV_HIST64_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_hist64_api.h"

typedef uint32_t VVbenchDeviceHist64Bin_t[32];

int VsiVvbenchParseHist64Info
(
	cJSON *histInfo,
	VvbenchModuleCfg_t *histCtrl
);

int VsiVvbenchHist64GetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchHist64Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchHist64GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	VVbenchDeviceHist64Bin_t *pVvHistBin
);


#endif //__VVDEV_HIST64_H__
