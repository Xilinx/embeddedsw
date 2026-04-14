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


#define LOGTAG "EEV2"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_ee_api.h"

int VsiVvbenchParseEeInfo
(
	cJSON *eeInfo,
	VvbenchModuleCfg_t *eeCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == eeCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	eeCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(eeInfo, "enable")) {
		eeCtrl->enable = cJSON_GetObjectItem(eeInfo, "enable")->valueint;
	}
	if (!eeCtrl->enable) {
		LOGI("EE feature not enable!");
	}
	else {
		LOGI("EE feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(eeInfo, "useCfg")) {
		eeCtrl->useCfg = cJSON_GetObjectItem(eeInfo, "useCfg")->valueint;
	}
	if (eeCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(eeInfo, "mode")) {
			eeCtrl->mode = cJSON_GetObjectItem(eeInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == eeCtrl->mode) {
			LOGI("EE use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == eeCtrl->mode) {
			LOGI("EE use auto config");
		}
		else {
			LOGW("invalid EE mode!");
			eeCtrl->useCfg = false;
			eeCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchEeGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceEeConfig_t config;
	CamDeviceEeStatus_t status;
	CamDeviceEeCurveEnConfig_t curveEnableCfg;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceEeConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceEeStatus_t));
	MEMSET(&curveEnableCfg, 0, sizeof(CamDeviceEeCurveEnConfig_t));

	result = VsiCamDeviceEeGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EE config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("eeEdgeGain: %d ", config.manualCfg.eeEdgeGain);
		LOGI("eeStrength: %d ", config.manualCfg.eeStrength);
		LOGI("eeYUpGain: %d ", config.manualCfg.eeYUpGain);
	}

	result = VsiCamDeviceEeGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EE status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("EE enable: %d\n", status.enable);
	}

	result = VsiCamDeviceEeGetCurveEnableConfig(hCamDevice, &curveEnableCfg);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EE Curve Enable Config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("EE Curve enable: %d\n", curveEnableCfg.curveEn);
	}

	result = VsiCamDeviceEeGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EE version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("EE version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchEeFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.ee.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.ee.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.ee.useCfg) {
			result = VsiCamDeviceEeEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable EE Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonEeSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set EE State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonEeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonEeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load EE Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonEeGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get EE Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonEeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonEeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load EE Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchEeGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get EE Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
