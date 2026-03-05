/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#define LOGTAG "AFV4"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_af_api.h"
#include "vvbench_afm_api.h"
#include "vvbench_afm_api_v3.h"

int VsiVvbenchParseAfInfo
(
	cJSON *afInfo,
	VvbenchModuleCfg_t *afCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == afCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	afCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(afInfo, "enable")) {
		afCtrl->enable = cJSON_GetObjectItem(afInfo, "enable")->valueint;
	}
	if (!afCtrl->enable) {
		LOGI("AFV4 feature not enable!");
	}
	else {
		LOGI("AFV4 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(afInfo, "useCfg")) {
		afCtrl->useCfg = cJSON_GetObjectItem(afInfo, "useCfg")->valueint;
	}
	if (afCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(afInfo, "mode")) {
			afCtrl->mode = cJSON_GetObjectItem(afInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == afCtrl->mode) {
			LOGI("AFV4 use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == afCtrl->mode) {
			LOGI("AFV4 auto config not supported!");
		}
		else {
			LOGW("invalid AFV4 mode!");
			afCtrl->useCfg = false;
			afCtrl->status = false;
		}
	}

	LOGI("%s exit ", __func__);
	return result;

}

int VsiVvbenchAfGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	uint32_t version = 0;
	CamDeviceAfStatus_t afState;
	CamDeviceAfMode_t AfMode;
	CamDeviceAfConfig_t config;
	MEMSET(&config, 0, sizeof(CamDeviceAfConfig_t));

	result = VsiCamDeviceAfGetMode(hCamDevice, &AfMode);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AF mode failed!\n", __func__);
		return RET_FAILURE;
	}

	//AF Roi Cfg
	result = VsiCamDeviceAfmGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get Afmv11 Version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AFM Version: %d", version);
	}

	result = VsiCamDeviceAfGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AF config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("cStableTolerance: %f ", config.cStableTolerance);
		LOGI("cPointsOfCurve: %d ", config.cPointsOfCurve);
		LOGI("cPdConfThreshold: %f ", config.cPdConfThreshold);
	}

	result = VsiCamDeviceAfGetStatus(hCamDevice, &afState);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AF state failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AF state: %d", afState.state);
	}

	//AF Get Version
	result = VsiCamDeviceAfGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AF Version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AF Version: %d", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchAfFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.af.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.af.useCfg) {
			result = VsiCamDeviceAfEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable AF Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonAfSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set AF Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonAfRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonAfLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load AF Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonAfRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonAfLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load AF Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchAfGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get AF Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
