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

#define LOGTAG "TPG"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_tpg_api.h"

int VsiVvbenchTpgFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice || NULL == caseCtx) {
		return -1;
	}

	result = VsiCamDeviceTpgDisable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Disable TPG failed!");
		return -1;
	}

	bool_t enable = caseCtx->instanceCfgCtx[index].tpgCfg.enable;
	if (!enable) {
		LOGI("Vvbench TPG not enabled\n");
		return 0;
	}

	uint32_t version;
	CamDeviceTpgConfig_t config;
	MEMSET(&config, 0, sizeof(CamDeviceTpgConfig_t));

	if (caseCtx->instanceCfgCtx[index].tpgCfg.useCfg) {

		config.imageType = caseCtx->instanceCfgCtx[index].tpgCfg.imageType;
		config.bayerPattern = caseCtx->instanceCfgCtx[index].tpgCfg.bayerPattern;
		config.colorDepth = caseCtx->instanceCfgCtx[index].tpgCfg.colorDepth;
		config.resolution = caseCtx->instanceCfgCtx[index].tpgCfg.resolution;

		config.pixleGap = caseCtx->instanceCfgCtx[index].tpgCfg.pixleGap;
		config.lineGap = caseCtx->instanceCfgCtx[index].tpgCfg.lineGap;
		config.gapStandard = caseCtx->instanceCfgCtx[index].tpgCfg.gapStandard;
		config.randomSeed = caseCtx->instanceCfgCtx[index].tpgCfg.randomSeed;
		config.frameNum = caseCtx->instanceCfgCtx[index].tpgCfg.frameNum;

		result = VsiCamDeviceTpgSetConfig(hCamDevice, &config);
		if (0 != result) {
			LOGE("Vvbench Set TPG config failed!");
			return -1;
		}
	}

	result = VsiCamDeviceTpgEnable(hCamDevice);
	if (0 != result) {
		LOGE("Vvbench Enable TPG failed!");
		return -1;
	}

	result = VsiCamDeviceTpgGetVersion(hCamDevice, &version);
	if (0 != result) {
		LOGE("Vvbench Get TPG version failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench TPG version: %x", version);
	}

	LOGI("%s exit ", __func__);
	return result;
}
