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


#define LOGTAG "BLS"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_bls_api.h"

int VsiVvbenchParseBlsInfo
(
	cJSON *blsInfo,
	VvbenchModuleCfg_t *blsCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == blsCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	blsCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(blsInfo, "enable")) {
		blsCtrl->enable = cJSON_GetObjectItem(blsInfo, "enable")->valueint;
	}
	if (!blsCtrl->enable) {
		LOGI("BLS feature not enable!");
	}
	else {
		LOGI("BLS feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(blsInfo, "useCfg")) {
		blsCtrl->useCfg = cJSON_GetObjectItem(blsInfo, "useCfg")->valueint;
	}
	if (blsCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(blsInfo, "mode")) {
			blsCtrl->mode = cJSON_GetObjectItem(blsInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == blsCtrl->mode) {
			LOGI("cpd BLS manual config!");
		}
		else if (CAMDEV_CFG_MODE_AUTO == blsCtrl->mode) {
			LOGI("BLS use auto config");
		}
		else {
			LOGW("invalid BLS mode!");
			blsCtrl->useCfg = false;
			blsCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchBlsGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceBlsConfig_t config;
	CamDeviceBlsStatus_t status;
	uint32_t version = 0;
	uint8_t bitWidth = 0;
	MEMSET(&config, 0, sizeof(CamDeviceBlsConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceBlsStatus_t));
	result = VsiCamDeviceBlsGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get BLS config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("bls:[%d, %d, %d %d]",
		     config.manualCfg.bls[0], config.manualCfg.bls[1],
		     config.manualCfg.bls[2], config.manualCfg.bls[3]);
	}

	result = VsiCamDeviceBlsGetBitWidth(hCamDevice, &bitWidth);
	if (RET_SUCCESS != result) {
		LOGE("%s Get BLS bitWidth failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("BLS bitWidth: %d\n", bitWidth);
	}

	result = VsiCamDeviceBlsGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get BLS status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("BLS enable: %d\n", status.enable);
	}

	result = VsiCamDeviceBlsGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get BLS version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("BLS version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchBlsFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.bls.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.bls.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (!enable) {
			LOGI("Vvbench BLS not enabled\n");
			return 0;
		}
		else {
			result = VsiCamCommonBlsSetState(caseCtx->instanceCfgCtx[index].hCamCommon, mode);
			if (0 != result) {
				LOGE("Vvbench Set BLS State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonBlsRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonBlsLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load BLS Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonBlsGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get BLS Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonBlsRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonBlsLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load BLS Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchBlsGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get BLS Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
