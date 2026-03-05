/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#define LOGTAG "CPDV1"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_cpd_api.h"

int VsiVvbenchParseCpdInfo
(
	cJSON *cpdInfo,
	VvbenchModuleCfg_t *cpdCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == cpdCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	cpdCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(cpdInfo, "enable")) {
		cpdCtrl->enable = cJSON_GetObjectItem(cpdInfo, "enable")->valueint;
	}
	if (!cpdCtrl->enable) {
		LOGI("CPD feature not enable!");
	}
	else {
		LOGI("CPD feature enable!");
	}

	if (NULL != cJSON_GetObjectItem(cpdInfo, "useCfg")) {
		cpdCtrl->useCfg = cJSON_GetObjectItem(cpdInfo, "useCfg")->valueint;
	}
	if (cpdCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(cpdInfo, "mode")) {
			cpdCtrl->mode = cJSON_GetObjectItem(cpdInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == cpdCtrl->mode) {
			LOGI("CPD use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == cpdCtrl->mode) {
			LOGW("CPD auto config not supported!");
		}
		else {
			LOGW("invalid CPD mode!");
			cpdCtrl->useCfg = false;
			cpdCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchCpdGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceCpdConfig_t config;
	CamDeviceCpdStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceCpdConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceCpdStatus_t));

	result = VsiCamDeviceCpdGetConfig(hCamDevice, CAMDEV_CPD_TYPE_EXPAND, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPD expand config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0; i < 5; ++i) {
			LOGI("expandCurveY[%d]: %d ", i, config.expandCfg.expandCurveY[i]);
		}
	}

	result = VsiCamDeviceCpdExpandGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPD status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CPD enable: %d", status.enable);
	}

	result = VsiCamDeviceCpdGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPD version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CPD version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchCpdFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.cpd.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.cpd.useCfg) {
			result = VsiCamDeviceCpdExpandDisable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable CPD Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonCpdSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set CPD State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonCpdRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonCpdLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load CPD Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonCpdGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get CPD Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonCpdRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonCpdLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load CPD Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchCpdGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get CPD Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
