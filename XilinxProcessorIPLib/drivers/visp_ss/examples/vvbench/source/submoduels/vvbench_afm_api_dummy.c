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

#define LOGTAG "AFMDUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_afm_api.h"

int VsiVvbenchParseAfmInfo
(
	cJSON *afmInfo,
	VvbenchModuleCfg_t *afmCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == afmCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s AFM feature not supported!!", __func__);
	afmCtrl->isSupport = false;
	return result;
}

int VsiVvbenchAfmFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}

int VsiVvbenchAfmGetStatistic
(
	CamDeviceHandle_t hCamDevice
)
{
	return 0;
}
