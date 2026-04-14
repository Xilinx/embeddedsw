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


#define LOGTAG "EXPV2"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_exp_api_v2.h"

int VsiVvbenchParseExpv2Info
(
	cJSON *expv2Info,
	VvbenchModuleCfg_t *expv2Ctrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == expv2Ctrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	expv2Ctrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(expv2Info, "enable")) {
		expv2Ctrl->enable = cJSON_GetObjectItem(expv2Info, "enable")->valueint;
	}
	if (!expv2Ctrl->enable) {
		LOGI("EXP feature not enable!");
	}
	else {
		LOGI("EXP feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(expv2Info, "useCfg")) {
		expv2Ctrl->useCfg = cJSON_GetObjectItem(expv2Info, "useCfg")->valueint;
	}

	return result;
}

int VsiVvbenchExpv2GetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceExpV2Config_t config;
	CamDeviceWindow_t window;
	CamDeviceExpV2Status_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceExpV2Config_t));
	MEMSET(&window, 0, sizeof(CamDeviceWindow_t));
	MEMSET(&status, 0, sizeof(CamDeviceExpV2Status_t));

	result = VsiCamDeviceExpV2GetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EXP V2 config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Get Expv2 inputSelect: %d", config.inputSelect);
		LOGI("Get Expv2 windowSetCustomEnable: %d ", config.windowSetCustomEnable);
	}

	result = VsiCamDeviceExpV2GetMeasureWindow(hCamDevice, &window);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EXP V2 Measure Window failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Get EXP V2 Measure Window hOffset: %d\n", window.hOffset);
		LOGI("Get EXP V2 Measure Window vOffset: %d\n", window.vOffset);
		LOGI("Get EXP V2 Measure Window width: %d\n", window.width);
		LOGI("Get EXP V2 Measure Window height: %d\n", window.height);
	}

	result = VsiCamDeviceExpV2GetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EXP V2 status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("EXP V2 enable: %d\n", status.enable);
	}

	result = VsiCamDeviceExpV2GetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get EXP V2 version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("EXP V2 version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchExpv2Func
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.expv2.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.expv2.useCfg) {
			result = VsiCamDeviceExpV2Enable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable EXP V2 Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonExpV2SetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set EXP V2 Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonExpV2RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonExpV2LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load EXP V2 Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonExpV2RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonExpV2LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load EXP V2 Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchExpv2GetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get EXP V2 Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchExpv2GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	VvbenchExpV2Statistics_t *ExpV2Statistics
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceExpV2Status_t expStatus;
	CamDeviceExpV2Statistics_t expStatic;
	MEMSET(&expStatus, 0, sizeof(CamDeviceExpV2Status_t));
	MEMSET(&expStatic, 0, sizeof(CamDeviceExpV2Statistics_t));

	result = VsiCamDeviceExpV2GetStatistics(hCamDevice, &expStatic);
	if (0 != result) {
		LOGE("Vvbench Get EXP static failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench EXP expStatic[0]: %d", expStatic.statistics[0]);
	}

	MEMCPY(ExpV2Statistics, &expStatic, sizeof(VvbenchExpV2Statistics_t));

	result = VsiCamDeviceExpV2GetStatus(hCamDevice, &expStatus);
	if (0 != result) {
		LOGE("Vvbench Get EXP status failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench EXP enable: %d", expStatus.enable);
	}

	LOGI("%s exit \n", __func__);
	return result;
}
