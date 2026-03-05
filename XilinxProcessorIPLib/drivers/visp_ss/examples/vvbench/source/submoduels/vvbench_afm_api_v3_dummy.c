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

#define LOGTAG "AFM3DUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_afm_api_v3.h"

int VsiVvbenchParseAfmv3Info
(
	cJSON *afmv3Info,
	VvbenchModuleCfg_t *afmv3Ctrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == afmv3Ctrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s AFM v3 feature not supported!!", __func__);
	afmv3Ctrl->isSupport = false;
	return result;
}

int VsiVvbenchAfmv3Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}

int VsiVvbenchAfmv3GetStatistic
(
	CamDeviceHandle_t hCamDevice
)
{
	return 0;
}
