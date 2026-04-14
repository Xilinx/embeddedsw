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


#define LOGTAG "GTMV1"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_gtm_api.h"

int VsiVvbenchParseGtmInfo
(
	cJSON *gtmInfo,
	VvbenchModuleCfg_t *gtmCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == gtmCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	gtmCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(gtmInfo, "enable")) {
		gtmCtrl->enable = cJSON_GetObjectItem(gtmInfo, "enable")->valueint;
	}
	if (!gtmCtrl->enable) {
		LOGI("GTM feature not enable!");
	}
	else {
		LOGI("GTM feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(gtmInfo, "useCfg")) {
		gtmCtrl->useCfg = cJSON_GetObjectItem(gtmInfo, "useCfg")->valueint;
	}
	if (gtmCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(gtmInfo, "mode")) {
			gtmCtrl->mode = cJSON_GetObjectItem(gtmInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == gtmCtrl->mode) {
			LOGI("GTM use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == gtmCtrl->mode) {
			LOGI("GTM use auto config");
		}
		else {
			LOGW("invalid GTM mode!");
			gtmCtrl->useCfg = false;
			gtmCtrl->status = false;
		}
	}
	return result;
}
int VsiVvbenchGtmGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceGtmConfig_t config;
	CamDeviceGtmStatus_t status;
	CamDeviceGtmHistogram_t hist;
	bool_t bwCorEnable;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceGtmConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceGtmStatus_t));
	MEMSET(&hist, 0, sizeof(CamDeviceGtmHistogram_t));

	result = VsiCamDeviceGtmGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GTM config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("gtmLightnessWeight: %d ", config.manualCfg.colorWeightCfg.gtmLightnessWeight);
		for (int i = 0; i < CAMDEV_GTM_COLOR_WEIGHT_SIZE; ++i) {
			LOGI("gtmColorWeight[%d]: %d ", i, config.manualCfg.colorWeightCfg.gtmColorWeight[i]);
		}
		LOGI("shiftBit: %d ", config.manualCfg.curveCfg.shiftBit);
		LOGI("logKneeSlope: %f\n", config.manualCfg.curveCfg.logKneeSlope);
		LOGI("bwcorMinLog: %f ", config.manualCfg.blackWhiteCorrectionCfg.bwcorMinLog);
		LOGI("bwcorDampCoef: %f\n", config.manualCfg.blackWhiteCorrectionCfg.bwcorDampCoef);
	}

	result = VsiCamDeviceGtmGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GTM status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GTM enable: %d\n", status.enable);
	}

	result = VsiCamDeviceGtmBlackWhiteCorrectionEnGetConfig(hCamDevice, &bwCorEnable);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GTM Black White Correction enable failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GTM Black White Correction enable: %d\n", bwCorEnable);
	}

	result = VsiCamDeviceGtmGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GTM version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GTM version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchGtmFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.gtm.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.gtm.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.gtm.useCfg) {
			result = VsiCamDeviceGtmEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable GTM Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonGtmSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set GTM State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonGtmRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonGtmLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load GTM Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonGtmGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get GTM Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonGtmRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonGtmLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load GTM Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchGtmGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get GTM Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
