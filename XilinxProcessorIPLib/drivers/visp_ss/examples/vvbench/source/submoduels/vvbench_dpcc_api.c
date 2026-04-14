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


#define LOGTAG "DPCC"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_dpcc_api.h"

int VsiVvbenchParseDpccInfo
(
	cJSON *dpccInfo,
	VvbenchModuleCfg_t *dpccCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == dpccCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	dpccCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(dpccInfo, "enable")) {
		dpccCtrl->enable = cJSON_GetObjectItem(dpccInfo, "enable")->valueint;
	}
	if (!dpccCtrl->enable) {
		LOGI("DPCC feature not enable!");
	}
	else {
		LOGI("DPCC feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(dpccInfo, "useCfg")) {
		dpccCtrl->useCfg = cJSON_GetObjectItem(dpccInfo, "useCfg")->valueint;
	}
	if (dpccCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(dpccInfo, "mode")) {
			dpccCtrl->mode = cJSON_GetObjectItem(dpccInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == dpccCtrl->mode) {
			LOGI("DPCC use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == dpccCtrl->mode) {
			LOGI("DPCC use auto config");
		}
		else {
			LOGW("invalid DPCC mode!");
			dpccCtrl->useCfg = false;
			dpccCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchDpccGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceDpccConfig_t config;
	CamDeviceDpccStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceDpccConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceDpccStatus_t));

	result = VsiCamDeviceDpccGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DPCC config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("bptNum: %d ", config.manualCfg.bptNum);
		LOGI("outMode: %d ", config.manualCfg.outMode);
		LOGI("setUse: %d ", config.manualCfg.setUse);
		for (size_t iFac = 0;
		     iFac < sizeof(config.manualCfg.lineMadFac) / sizeof(config.manualCfg.lineMadFac[0]); ++iFac) {
			for (size_t jFac = 0;
			     jFac < sizeof(config.manualCfg.lineMadFac[0]) / sizeof(config.manualCfg.lineMadFac[0][0]); ++jFac) {
				LOGI("lineMadFac[%lu][%lu]: %u ", iFac, jFac, config.manualCfg.lineMadFac[iFac][jFac]);
			}
		}
	}

	result = VsiCamDeviceDpccGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DPCC status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DPCC enable: %d\n", status.enable);
	}

	result = VsiCamDeviceDpccGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DPCC version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DPCC version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchDpccFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.dpcc.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.dpcc.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.dpcc.useCfg) {
			result = VsiCamDeviceDpccEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable DPCC Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonDpccSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set DPCC State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonDpccLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load DPCC Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonDpccGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get DPCC Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonDpccRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonDpccLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load DPCC Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchDpccGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get DPCC Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
