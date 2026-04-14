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


#define LOGTAG "HIST256"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_hist256_api.h"

int VsiVvbenchParseHist256Info
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
		LOGI("Hist256 feature not enable!");
	}
	else {
		LOGI("Hist256 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(histInfo, "useCfg")) {
		histCtrl->useCfg = cJSON_GetObjectItem(histInfo, "useCfg")->valueint;
	}
	if (histCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(histInfo, "mode")) {
			histCtrl->mode = cJSON_GetObjectItem(histInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == histCtrl->mode) {
			LOGI("Hist256 use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == histCtrl->mode) {
			LOGW("Hist256 auto config not supported!");
		}
		else {
			LOGW("invalid Hist256 mode!");
			histCtrl->useCfg = false;
			histCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchHist256GetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceHist256Config_t config;
	CamDeviceHist256Status_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceHist256Config_t));
	MEMSET(&status, 0, sizeof(CamDeviceHist256Status_t));

	result = VsiCamDeviceHist256GetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist256 config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("mode: %d\n", config.mode);
		LOGI("hOffset: %d vOffset: %d width: %d height: %d\n",
		     config.window.hOffset, config.window.vOffset,
		     config.window.width, config.window.height);
	}

	result = VsiCamDeviceHist256GetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist256 status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Hist256 enable: %d\n", status.enable);
	}

	result = VsiCamDeviceHist256GetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Hist256 version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("Hist256 version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchHist256Func
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.hist256.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.hist256.useCfg) {
			result = VsiCamDeviceHist256Enable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable Hist256 Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonHist256SetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set Hist256 Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonHist256RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonHist256LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load Hist256 Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonHist256RangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonHist256LoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load Hist256 Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchHist256GetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get Hist256 Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}

int VsiVvbenchHist256GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	VVbenchDeviceHist256Bin_t *pVvHistBin
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceHist256Bins_t hist256Sta;
	MEMSET(&hist256Sta, 0, sizeof(CamDeviceHist256Bins_t));

	result = VsiCamDeviceHist256GetStatistic(hCamDevice, &hist256Sta);
	if (0 != result) {
		LOGE("Vvbench Get Hist256 Statistic failed!\n");
		return -1;
	}
	else {
		for (unsigned int i = 0; i < CAMDEV_HIST256_NUM_BINS; i++) {
			LOGI("Vvbench Hist256 [%d]: %d", i, hist256Sta.bins[i]);
		}
	}

	MEMCPY(pVvHistBin, &hist256Sta, sizeof(CamDeviceHist256Bins_t));

	LOGI("%s exit \n", __func__);
	return result;
}
