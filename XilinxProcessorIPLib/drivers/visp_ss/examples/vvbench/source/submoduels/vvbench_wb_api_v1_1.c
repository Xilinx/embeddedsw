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


#define LOGTAG "WBV11"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_wb_api.h"

int VsiVvbenchParseWbInfo
(
	cJSON *wbInfo,
	VvbenchModuleCfg_t *wbCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == wbCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	wbCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(wbInfo, "enable")) {
		wbCtrl->enable = cJSON_GetObjectItem(wbInfo, "enable")->valueint;
	}
	if (!wbCtrl->enable) {
		LOGI("WB feature not enable!");
	}
	else {
		LOGI("WB feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(wbInfo, "useCfg")) {
		wbCtrl->useCfg = cJSON_GetObjectItem(wbInfo, "useCfg")->valueint;
	}
	if (wbCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(wbInfo, "mode")) {
			wbCtrl->mode = cJSON_GetObjectItem(wbInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == wbCtrl->mode) {
			LOGI("WB use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == wbCtrl->mode) {
			LOGW("WB auto config not supported!");
		}
		else {
			LOGW("invalid WB mode!");
			wbCtrl->useCfg = false;
			wbCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchWbGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceWbConfig_t config;
	CamDeviceWbStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceWbConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceWbStatus_t));

	result = VsiCamDeviceWbGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WB config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0; i < sizeof(config.manualCfg.gain) / sizeof(config.manualCfg.gain[0]); ++i) {
			LOGI("gain[%d]: %f ", i, config.manualCfg.gain[i]);
		}
	}

	result = VsiCamDeviceWbGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WB status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("WB enable: %d\n", status.enable);
	}

	result = VsiCamDeviceWbGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WB version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("WB version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchWbFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.wb.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.wb.useCfg) {
			result = VsiCamDeviceWbEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable WB Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonWbSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set WB State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonWbRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonWbLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load WB Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonWbGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get WB Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonWbRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonWbLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load WB Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchWbGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get WB Func Failed!\n");
			return -1;
		}
	}


	LOGI("%s exit ", __func__);
	return result;
}
