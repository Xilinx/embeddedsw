// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2026 Vivantec Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************/


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
