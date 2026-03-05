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

#define LOGTAG "LUT3DDUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_lut3d_api.h"

int VsiVvbenchParseLut3dInfo
(
	cJSON *lut3dInfo,
	VvbenchModuleCfg_t *lut3dCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == lut3dCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s LUT3D feature not supported!!", __func__);
	lut3dCtrl->isSupport = false;
	return result;
}

int VsiVvbenchLut3dFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}
