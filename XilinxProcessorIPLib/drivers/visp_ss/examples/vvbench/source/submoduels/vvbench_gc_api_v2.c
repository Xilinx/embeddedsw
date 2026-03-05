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

#define LOGTAG "GCV2"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_gc_api.h"

int VsiVvbenchParseGcInfo
(
	cJSON *gcInfo,
	VvbenchModuleCfg_t *gcCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == gcCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	gcCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(gcInfo, "enable")) {
		gcCtrl->enable = cJSON_GetObjectItem(gcInfo, "enable")->valueint;
	}
	if (!gcCtrl->enable) {
		LOGI("GC feature not enable!");
	}
	else {
		LOGI("GC feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(gcInfo, "useCfg")) {
		gcCtrl->useCfg = cJSON_GetObjectItem(gcInfo, "useCfg")->valueint;
	}
	if (gcCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(gcInfo, "mode")) {
			gcCtrl->mode = cJSON_GetObjectItem(gcInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == gcCtrl->mode) {
			LOGI("GC use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == gcCtrl->mode) {
			LOGI("GC use auto config");
		}
		else {
			LOGW("invalid GC mode!");
			gcCtrl->useCfg = false;
			gcCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchGcGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceGcConfig_t config;
	CamDeviceGcStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceGcConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceGcStatus_t));

	result = VsiCamDeviceGcGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GC config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("standardVal: %f ", config.manualCfg.standardVal);
		for (int i = 0; i < sizeof(config.manualCfg.curve) / sizeof(config.manualCfg.curve[0]); ++i) {
			LOGI("curve[%d]: %d ", i, config.manualCfg.curve[i]);
		}
		LOGI("userCurveX: %d ", config.manualCfg.userCurveX);
		LOGI("curvePx[0]: %d, curvePx[63]: %d", config.manualCfg.curvePx[0], config.manualCfg.curvePx[63]);
	}

	result = VsiCamDeviceGcGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GC status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GC enable: %d\n", status.enable);
	}

	result = VsiCamDeviceGcGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get GC version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("GC version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchGcFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.gc.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.gc.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.gc.useCfg) {
			result = VsiCamDeviceGcEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable GC Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonGcSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set GC State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonGcRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonGcLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load GC Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonGcGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get GC Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonGcRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonGcLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load GC Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchGcGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get GC Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
