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

#define LOGTAG "CPROC"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_cproc_api.h"

int VsiVvbenchParseCprocInfo
(
	cJSON *cprocInfo,
	VvbenchModuleCfg_t *cprocCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == cprocCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	cprocCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(cprocInfo, "enable")) {
		cprocCtrl->enable = cJSON_GetObjectItem(cprocInfo, "enable")->valueint;
	}
	if (!cprocCtrl->enable) {
		LOGI("CPROC feature not enable!");
	}
	else {
		LOGI("CPROC feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(cprocInfo, "useCfg")) {
		cprocCtrl->useCfg = cJSON_GetObjectItem(cprocInfo, "useCfg")->valueint;
	}
	if (cprocCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(cprocInfo, "mode")) {
			cprocCtrl->mode = cJSON_GetObjectItem(cprocInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == cprocCtrl->mode) {
			LOGI("CPROC use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == cprocCtrl->mode) {
			LOGI("CPROC use auto config");
		}
		else {
			LOGW("invalid CPROC mode!");
			cprocCtrl->useCfg = false;
			cprocCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchCprocGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceCprocConfig_t config;
	CamDeviceCprocYuvRangeS_t rangeConfig;
	CamDeviceCprocStatus_t status;
	CamDeviceCprocYuvRangeS_t yuvRange;
	CamDeviceCprocYuvRangeS_t yuvRangeStatus;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceCprocConfig_t));
	MEMSET(&rangeConfig, 0, sizeof(CamDeviceCprocYuvRangeS_t));
	MEMSET(&status, 0, sizeof(CamDeviceCprocStatus_t));
	MEMSET(&yuvRange, 0, sizeof(CamDeviceCprocYuvRangeS_t));
	MEMSET(&yuvRangeStatus, 0, sizeof(CamDeviceCprocYuvRangeS_t));

	result = VsiCamDeviceCprocGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPROC config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("contrast: %f ", config.manualCfg.contrast);
		LOGI("bright: %f ", config.manualCfg.bright);
		LOGI("saturation: %f ", config.manualCfg.saturation);
		LOGI("hue: %f ", config.manualCfg.hue);
	}

	result = VsiCamDeviceCprocGetRange(hCamDevice, &yuvRange);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPROC range failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("yuvRange: %d ", yuvRange.yuvRange);
	}

	result = VsiCamDeviceCprocGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPROC status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CPROC enable: %d\n", status.enable);
	}

	result = VsiCamDeviceCprocGetRangeStatus(hCamDevice, &yuvRangeStatus);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPROC range status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CPROC yuvRangeStatus gamut: %d\n", yuvRangeStatus.gamut);
	}

	result = VsiCamDeviceCprocGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CPROC version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CPROC version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchCprocFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.cproc.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.cproc.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.cproc.useCfg) {
			result = VsiCamDeviceCprocEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable CPROC Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonCprocSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set CPROC State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonCprocRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonCprocLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load CPROC Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonCprocGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get CPROC Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonCprocRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonCprocLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load CPROC Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchCprocGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get CPROC Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
