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

#define LOGTAG "EXPV3DUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_exp_api_v3.h"

int VsiVvbenchParseExpv3Info
(
	cJSON *expv3Info,
	VvbenchModuleCfg_t *expv3Ctrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == expv3Ctrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s EXP v3 feature not supported!!", __func__);
	expv3Ctrl->isSupport = false;
	return result;
}

int VsiVvbenchExpv3Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}

int VsiVvbenchExpv3GetStatistic
(
	CamDeviceHandle_t hCamDevice
)
{
	return 0;
}
