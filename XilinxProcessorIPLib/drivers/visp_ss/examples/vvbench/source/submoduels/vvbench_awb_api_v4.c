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

#define LOGTAG "AWBV4"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_awb_api.h"

int VsiVvbenchParseAwbInfo
(
	cJSON *awbInfo,
	VvbenchModuleCfg_t *awbCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == awbCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	awbCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(awbInfo, "enable")) {
		awbCtrl->enable = cJSON_GetObjectItem(awbInfo, "enable")->valueint;
	}
	if (!awbCtrl->enable) {
		LOGI("AWBV4 feature not enable!");
	}
	else {
		LOGI("AWBV4 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(awbInfo, "useCfg")) {
		awbCtrl->useCfg = cJSON_GetObjectItem(awbInfo, "useCfg")->valueint;
	}

	LOGI("%s exit ", __func__);
	return result;
}

int VsiVvbenchAwbGetFunc
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
	CamDeviceAwbState_t awbState = CAMDEV_AWB_STATE_INVALID;
	CamDeviceAwbMode_t AwbMode = CAMDEV_AWB_MODE;
	CamDeviceAwbConfig_t config;
	MEMSET(&config, 0, sizeof(CamDeviceAwbConfig_t));
	CamDeviceRoi_t awbRoi;
	MEMSET(&awbRoi, 0, sizeof(CamDeviceRoi_t));

	CamDeviceAwbFrontGroundConfig_t awbFrontGroundCfg;
	MEMSET(&awbFrontGroundCfg, 0, sizeof(CamDeviceAwbFrontGroundConfig_t));

	result = VsiCamDeviceAwbGetMode(hCamDevice, &AwbMode);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AWB mode failed!\n", __func__);
		return RET_FAILURE;
	}

	//AWB Roi Cfg
	result = VsiCamDeviceAwbGetRoi(hCamDevice, &awbRoi);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AWB roi config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("roiNum: %d", awbRoi.roiNum);
	}

	result = VsiCamDeviceAwbGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AWB config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("awbEnterLockThreshold: %f ", config.awbEnterLockThreshold);
	}

	//AWB Get Status
	result = VsiCamDeviceAwbGetStatus(hCamDevice, &awbState);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AWB state failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AWB state: %d", awbState);
	}

	//AWB Get Version
	result = VsiCamDeviceAwbGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s  Get AWB Version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AWB Version: %d", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchAwbFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.awb.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.awb.useCfg) {
			result = VsiCamDeviceAwbEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable AWB Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonAwbSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set AWB Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonAwbRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonAwbLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load AWB Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonAwbRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonAwbLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load AWB Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchAwbGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get AWB Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}

int VsiVvbenchAwbGetStatistic
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceAwbTemWeight_t awbTemWeight;
	MEMSET(&awbTemWeight, 0, sizeof(CamDeviceAwbTemWeight_t));

	result = VsiCamDeviceAwbGetColorTempWeight(hCamDevice, &awbTemWeight);
	if (0 != result) {
		LOGE("Vvbench Get AWB Color Temp Weight failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench AWB awbTemWeight[%f, %f, %f, %f, %f, %f, %f]\n", awbTemWeight[0], awbTemWeight[1],
		     awbTemWeight[2],
		     awbTemWeight[3], awbTemWeight[4], awbTemWeight[5], awbTemWeight[6]);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchAwbGetResult
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceAwbResult_t camDeviceAwbResult;
	MEMSET(&camDeviceAwbResult, 0, sizeof(CamDeviceAwbResult_t));

	result = VsiCamDeviceAwbGetResult(hCamDevice, &camDeviceAwbResult);
	if (0 != result) {
		LOGE("Vvbench Get AWB Result Failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench AWB Get Result awbCCT:%d\n", camDeviceAwbResult.awbCCT);
	}

	LOGI("%s exit \n", __func__);
	return result;
}
