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


#define LOGTAG "WDRV52"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_wdr_api.h"

int VsiVvbenchParseWdrInfo
(
	cJSON *wdrInfo,
	VvbenchModuleCfg_t *wdrCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == wdrCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	wdrCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(wdrInfo, "enable")) {
		wdrCtrl->enable = cJSON_GetObjectItem(wdrInfo, "enable")->valueint;
	}
	if (!wdrCtrl->enable) {
		LOGI("WDR feature not enable!");
	}
	else {
		LOGI("WDR feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(wdrInfo, "useCfg")) {
		wdrCtrl->useCfg = cJSON_GetObjectItem(wdrInfo, "useCfg")->valueint;
	}
	if (wdrCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(wdrInfo, "mode")) {
			wdrCtrl->mode = cJSON_GetObjectItem(wdrInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == wdrCtrl->mode) {
			LOGI("WDR use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == wdrCtrl->mode) {
			LOGI("WDR use auto config");
		}
		else {
			LOGW("invalid WDR mode!");
			wdrCtrl->useCfg = false;
			wdrCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchWdrGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceWdrConfig_t config;
	CamDeviceWdrStatus_t status;
	CamDeviceWdrGammaUpConfig_t gammauUpConfig;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceWdrConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceWdrStatus_t));
	MEMSET(&gammauUpConfig, 0, sizeof(CamDeviceWdrGammaUpConfig_t));

	result = VsiCamDeviceWdrGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WDR config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("strength: %d ", config.manualCfg.strengthCfg.strength);
		LOGI("lowStrength: %d ", config.manualCfg.strengthCfg.lowStrength);
		LOGI("flatLevel: %d ", config.manualCfg.ltmCfg.flatLevel);
		LOGI("fixedWeight: %d ", config.manualCfg.gtmWeightCfg.fixedWeight);
		LOGI("curve2Thr: %f ", config.manualCfg.gtmCfg.curve2Thr);
		LOGI("satRange: %f ", config.manualCfg.saturationAdjustmentCfg.satRange);
		LOGI("dampCurveMax: %d ", config.manualCfg.dampCfg.dampCurveMax);
		LOGI("dampFilterSize: %d ", config.manualCfg.dampCfg.dampFilterSize);
		for (int i = 0; i < CAMDEV_WDR_RGB_COEF_SIZE; i++) {
			LOGI("wdrRgbCoef[%d]: %d ", i, config.manualCfg.colorWeightCfg.wdrRgbCoef[i]);
		}
		for (int i = 0; i < CAMDEV_WDR_LIGHT_THR_LOG_SIZE; i++) {
			LOGI("lightRedThrLog[%d]: %f ", i, config.manualCfg.haloColorFadingCfg.lightRedThrLog[i]);
		}
	}

	result = VsiCamDeviceWdrGetGammaUpConfig(hCamDevice, &gammauUpConfig);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WDR Gamma Up config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0;
		     i < (sizeof(gammauUpConfig.gammaUpCurveX) / sizeof(gammauUpConfig.gammaUpCurveX[0])); i++) {
			// LOGI("gammaUpCurveX[%d]: %d ", i, gammauUpConfig.gammaUpCurveX[i]);
			// LOGI("gammaUpCurveY[%d]: %d ", i, gammauUpConfig.gammaUpCurveY[i]);
		}
	}

	result = VsiCamDeviceWdrGetGammaUpStatus(hCamDevice, &gammauUpConfig);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WDR Gamma Up status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0;
		     i < (sizeof(gammauUpConfig.gammaUpCurveX) / sizeof(gammauUpConfig.gammaUpCurveX[0])); i++) {
			// LOGI("gammaUpCurveX[%d]: %d ", i, gammauUpConfig.gammaUpCurveX[i]);
			// LOGI("gammaUpCurveY[%d]: %d ", i, gammauUpConfig.gammaUpCurveY[i]);
		}
	}

	result = VsiCamDeviceWdrGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WDR status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("WDR enable: %d\n", status.enable);
	}

	result = VsiCamDeviceWdrGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get WDR version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("WDR version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchWdrFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.wdr.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.wdr.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.wdr.useCfg) {
			result = VsiCamDeviceWdrEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable WDR Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonWdrSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set WDR State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonWdrRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonWdrLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load WDR Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonWdrGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get WDR Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonWdrRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonWdrLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load WDR Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchWdrGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get WDR Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
