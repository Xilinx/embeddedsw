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

#define LOGTAG "CCM"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_ccm_api.h"

int VsiVvbenchParseCcmInfo
(
	cJSON *ccmInfo,
	VvbenchModuleCfg_t *ccmCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == ccmCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	ccmCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(ccmInfo, "enable")) {
		ccmCtrl->enable = cJSON_GetObjectItem(ccmInfo, "enable")->valueint;
	}
	if (!ccmCtrl->enable) {
		LOGI("CCM feature not enable!");
	}
	else {
		LOGI("CCM feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(ccmInfo, "useCfg")) {
		ccmCtrl->useCfg = cJSON_GetObjectItem(ccmInfo, "useCfg")->valueint;
	}
	if (ccmCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(ccmInfo, "mode")) {
			ccmCtrl->mode = cJSON_GetObjectItem(ccmInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == ccmCtrl->mode) {
			LOGI("CCM use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == ccmCtrl->mode) {
			LOGI("CCM use auto config");
		}
		else {
			LOGW("invalid CCM mode!");
			ccmCtrl->useCfg = false;
			ccmCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchCcmGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceCcmConfig_t config;
	CamDeviceCcmStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceCcmConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceCcmStatus_t));

	result = VsiCamDeviceCcmGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CCM config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		for (int i = 0; i < sizeof(config.manualCfg.ccOffset) / sizeof(config.manualCfg.ccOffset[0]); ++i) {
			LOGI("ccOffset[%d]: %f ", i, config.manualCfg.ccOffset[i]);
		}
	}

	result = VsiCamDeviceCcmGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CCM status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CCM enable: %d", status.enable);
	}

	result = VsiCamDeviceCcmGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get CCM version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("CCM version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchCcmFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.ccm.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.ccm.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.ccm.useCfg) {
			result = VsiCamDeviceCcmEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable CCM Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonCcmSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set CCM State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonCcmRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonCcmLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load CCM Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonCcmGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get CCM Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonCcmRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonCcmLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load CCM Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchCcmGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get CCM Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}
