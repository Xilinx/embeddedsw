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


#define LOGTAG "GE"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_ge_api.h"

int VsiVvbenchParseGeInfo
(
	cJSON *geInfo,
	VvbenchModuleCfg_t *geCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == geCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	geCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(geInfo, "enable")) {
		geCtrl->enable = cJSON_GetObjectItem(geInfo, "enable")->valueint;
	}
	if (!geCtrl->enable) {
		LOGI("GE feature not enable!");
	}
	else {
		LOGI("GE feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(geInfo, "useCfg")) {
		geCtrl->useCfg = cJSON_GetObjectItem(geInfo, "useCfg")->valueint;
	}
	if (geCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(geInfo, "mode")) {
			geCtrl->mode = cJSON_GetObjectItem(geInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == geCtrl->mode) {
			LOGI("GE use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == geCtrl->mode) {
			LOGI("GE use auto config");
		}
		else {
			LOGW("invalid GE mode!");
			geCtrl->useCfg = false;
			geCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchGeGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceGeConfig_t config;
	CamDeviceGeStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceGeConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceGeStatus_t));

	result = VsiCamDeviceGeGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GE config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("threshold: %f ", config.manualCfg.threshold);
	}

	result = VsiCamDeviceGeGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GE status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GE enable: %d\n", status.enable);
	}

	result = VsiCamDeviceGeGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GE version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GE version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchGeFunc
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

	result = VsiCamDeviceGeDisable(hCamDevice);
	if (0 != result) {
		LOGE("VVbench Disable GE failed!\n");
		return -1;
	}

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.ge.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.ge.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.ge.useCfg) {
			result = VsiCamDeviceGeEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable GE Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonGeSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set GE Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonGeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonGeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load GE Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonGeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonGeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load GE Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchGeGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get GE Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit", __func__);
	return result;
}
