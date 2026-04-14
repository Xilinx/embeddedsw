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


#define LOGTAG "DG"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_dg_api.h"

int VsiVvbenchParseDgInfo
(
	cJSON *dgInfo,
	VvbenchModuleCfg_t *dgCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == dgCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	dgCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(dgInfo, "enable")) {
		dgCtrl->enable = cJSON_GetObjectItem(dgInfo, "enable")->valueint;
	}
	if (!dgCtrl->enable) {
		LOGI("DG feature not enable!");
	}
	else {
		LOGI("DG feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(dgInfo, "useCfg")) {
		dgCtrl->useCfg = cJSON_GetObjectItem(dgInfo, "useCfg")->valueint;
	}
	if (dgCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(dgInfo, "mode")) {
			dgCtrl->mode = cJSON_GetObjectItem(dgInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == dgCtrl->mode) {
			LOGI("DG use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == dgCtrl->mode) {
			LOGW("DG auto config not supported!");
		}
		else {
			LOGW("invalid DG mode!");
			dgCtrl->useCfg = false;
			dgCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchDgGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceDgConfig_t config;
	CamDeviceDgStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceDgConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceDgStatus_t));

	result = VsiCamDeviceDgGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DG config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("digitalGainB: %f ", config.manualCfg.digitalGainB);
		LOGI("digitalGainGb: %f ", config.manualCfg.digitalGainGb);
		LOGI("digitalGainGr: %f ", config.manualCfg.digitalGainGr);
		LOGI("digitalGainR: %f ", config.manualCfg.digitalGainR);
	}

	result = VsiCamDeviceDgGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DG status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DG enable: %d\n", status.enable);
	}

	result = VsiCamDeviceDgGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DG version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DG version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchDgFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.dg.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.dg.useCfg) {
			result = VsiCamDeviceDgEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable DG Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonDgSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set DG State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonDgRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonDgLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load DG Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonDgGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get DG Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonDgRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonDgLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load DG Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchDgGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get DG Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
