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


#define LOGTAG "LSCV3"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_lsc_api.h"

int VsiVvbenchParseLscInfo
(
	cJSON *lscInfo,
	VvbenchModuleCfg_t *lscCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == lscCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	lscCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(lscInfo, "enable")) {
		lscCtrl->enable = cJSON_GetObjectItem(lscInfo, "enable")->valueint;
	}
	if (!lscCtrl->enable) {
		LOGI("LSC feature not enable!");
	}
	else {
		LOGI("LSC feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(lscInfo, "useCfg")) {
		lscCtrl->useCfg = cJSON_GetObjectItem(lscInfo, "useCfg")->valueint;
	}
	if (lscCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(lscInfo, "mode")) {
			lscCtrl->mode = cJSON_GetObjectItem(lscInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == lscCtrl->mode) {
			LOGI("LSC use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == lscCtrl->mode) {
			LOGI("LSC use auto config");
		}
		else {
			LOGW("invalid LSC mode!");
			lscCtrl->useCfg = false;
			lscCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchLscGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceLscConfig_t config;
	CamDeviceLscStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceLscConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceLscStatus_t));

	result = VsiCamDeviceLscGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get LSC config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0; i < sizeof(config.manualCfg.xSize) / sizeof(config.manualCfg.xSize[0]); ++i) {
			LOGI("xSize[%d]: %d ", i, config.manualCfg.xSize[i]);
		}
	}

	result = VsiCamDeviceLscGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get LSC status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("LSC enable: %d\n", status.enable);
	}

	result = VsiCamDeviceLscGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get LSC version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("LSC version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchLscFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice || 0 == caseCtx->instanceCfgCtx[index].hCamCommon || NULL == caseCtx) {
		return -1;
	}

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.lsc.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.lsc.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.lsc.useCfg) {
			result = VsiCamDeviceLscEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable LSC Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonLscSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set LSC State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonLscRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonLscLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load LSC Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonLscGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get LSC Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonLscRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonLscLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load LSC Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchLscGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get LSC Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit", __func__);
	return result;
}
