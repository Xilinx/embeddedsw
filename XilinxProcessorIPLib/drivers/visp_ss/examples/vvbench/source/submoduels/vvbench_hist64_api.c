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


#define LOGTAG "HIST64"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_hist64_api.h"

int VsiVvbenchParseHist64Info
(
	cJSON *histInfo,
	VvbenchModuleCfg_t *histCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == histCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	histCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(histInfo, "enable")) {
		histCtrl->enable = cJSON_GetObjectItem(histInfo, "enable")->valueint;
	}
	if (!histCtrl->enable) {
		LOGI("Hist64 feature not enable!");
	}
	else {
		LOGI("Hist64 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(histInfo, "useCfg")) {
		histCtrl->useCfg = cJSON_GetObjectItem(histInfo, "useCfg")->valueint;
	}
	if (histCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(histInfo, "mode")) {
			histCtrl->mode = cJSON_GetObjectItem(histInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == histCtrl->mode) {
			LOGI("Hist64 use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == histCtrl->mode) {
			LOGW("Hist64 auto config not supported!");
		}
		else {
			LOGW("invalid Hist64 mode!");
			histCtrl->useCfg = false;
			histCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchHist64GetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceHist64Config_t config;
	CamDeviceHist64Status_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceHist64Config_t));
	MEMSET(&status, 0, sizeof(CamDeviceHist64Status_t));

	result = VsiCamDeviceHist64GetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist64 config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("mode: %d channel: %d\n", config.mode, config.channel);
		LOGI("hOffset: %d vOffset: %d width: %d height: %d\n",
		     config.window.hOffset, config.window.vOffset,
		     config.window.width, config.window.height);
		LOGI("rCoeff: %f gCoeff: %f bCoeff: %f\n",
		     config.rCoeff, config.gCoeff, config.bCoeff);
	}

	result = VsiCamDeviceHist64GetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist64 status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Hist64 enable: %d\n", status.enable);
	}

	result = VsiCamDeviceHist64GetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist64 version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Hist64 version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchHist64Func
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.hist64.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.hist64.useCfg) {
			result = VsiCamDeviceHist64Enable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable Hist64 Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonHist64SetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set Hist64 Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonHist64RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonHist64LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load Hist64 Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonHist64RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonHist64LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load Hist64 Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchHist64GetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get Hist64 Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}

int VsiVvbenchHist64GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	VVbenchDeviceHist64Bin_t *pVvHistBin
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceHist64Bins_t hist64Sta;
	MEMSET(&hist64Sta, 0, sizeof(CamDeviceHist64Bins_t));

	result = VsiCamDeviceHist64GetStatistic(hCamDevice, &hist64Sta);
	if (0 != result) {
		LOGE("Vvbench Get Hist64 Statistic failed!\n");
		return -1;
	}
	else {
		for (unsigned int i = 0; i < CAMDEV_HIST64_NUM_BINS; i++) {
			LOGI("Vvbench Hist64 [%d]: %d", i, hist64Sta.bins[i]);
		}
	}
	MEMCPY(pVvHistBin, &hist64Sta, sizeof(CamDeviceHist64Bins_t));

	LOGI("%s exit \n", __func__);
	return result;
}
