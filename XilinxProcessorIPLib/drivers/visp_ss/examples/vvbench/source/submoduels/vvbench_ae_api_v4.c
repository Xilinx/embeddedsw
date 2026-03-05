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

#define LOGTAG "AEV4"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_ae_api.h"
#include "vvbench_exp_api_v3.h"

int VsiVvbenchParseAeInfo
(
	cJSON *aeInfo,
	VvbenchModuleCfg_t *aeCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == aeCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	aeCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(aeInfo, "enable")) {
		aeCtrl->enable = cJSON_GetObjectItem(aeInfo, "enable")->valueint;
	}
	if (!aeCtrl->enable) {
		LOGI("AEV4 feature not enable!");
	}
	else {
		LOGI("AEV4 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(aeInfo, "useCfg")) {
		aeCtrl->useCfg = cJSON_GetObjectItem(aeInfo, "useCfg")->valueint;
	}

	LOGI("%s exit ", __func__);
	return result;

}

int VsiVvbenchAeGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceAeState_t aeState;
	CamDeviceAeConfig_t config;
	MEMSET(&config, 0, sizeof(CamDeviceAeConfig_t));
	uint32_t version = 0;

	//AE Config
	result = VsiCamDeviceAeGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("dampOver: %f ", config.dampOver);
		LOGI("dampUnderRatio: %f ", config.dampUnderRatio);
		LOGI("tolerance: %f ", config.tolerance);
	}

	//AE exptable
	CamDeviceExpTable_t aeExptable;
	MEMSET(&aeExptable, 0, sizeof(CamDeviceExpTable_t));
	result = VsiCamDeviceAeGetExpTable(hCamDevice, &aeExptable);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE exptable failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("expTableNum: %d ", aeExptable.expTableNum);
	}

	//AE Roi
	CamDeviceRoi_t aeRoi;
	MEMSET(&aeRoi, 0, sizeof(CamDeviceRoi_t));
	result = VsiCamDeviceAeGetRoi(hCamDevice, &aeRoi);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE Roi failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("roiNum: %d ", aeRoi.roiNum);
		LOGI("roiWeight: %f ", aeRoi.roiWeight);
	}

	//AE Mode
	CamDeviceAeMode_t aeMode;
	MEMSET(&aeMode, 0, sizeof(CamDeviceAeMode_t));
	result = VsiCamDeviceAeGetMode(hCamDevice, &aeMode);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE mode failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("semMode: %d ", aeMode.semMode);
		LOGI("antiFlickerMode: %d ", aeMode.antiFlickerMode.flickerPeriod);
		LOGI("userDefinedPeriodus: %f ", aeMode.antiFlickerMode.userDefinedPeriodus);
	}

	//AE Histogram
	CamDeviceAeHistBins_t getAeHistogram;
	MEMSET(&getAeHistogram, 0, sizeof(CamDeviceAeHistBins_t));
	result = VsiCamDeviceAeGetHistogram(hCamDevice, &getAeHistogram);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE Histogram failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		// for (uint16_t i = 0; i < CAMDEV_AE_HIST_NUM_BINS; ++i) {
		// LOGI("AeHistBin[%d]: %d", i, getAeHistogram[i]);
		// }
	}

	//AE Luminance
	CamDeviceAeMeanLuma_t getAeLuminance;
	MEMSET(&getAeLuminance, 0, sizeof(CamDeviceAeMeanLuma_t));
	result = VsiCamDeviceAeGetLuminance(hCamDevice, &getAeLuminance);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE Luminance failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		// for (uint16_t i = 0; i < CAMDEV_AE_EXP_GRID_ITEMS; ++i) {
		// LOGI("AeLuminance[%d]: %d", i, getAeLuminance[i]);
		// }
	}

	//AE Object Region
	CamDeviceAeObjectRegion_t getAeObjectRegion;
	MEMSET(&getAeObjectRegion, 0, sizeof(CamDeviceAeObjectRegion_t));
	result = VsiCamDeviceAeGetObjectRegion(hCamDevice, &getAeObjectRegion);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE Object Region failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		// for (uint16_t i = 0; i < CAMDEV_AE_EXP_GRID_ITEMS; ++i) {
		// LOGI("AeObjectRegion[%d]: %d", i, getAeObjectRegion[i]);
		// }
	}

	//AE Get Status
	result = VsiCamDeviceAeGetStatus(hCamDevice, &aeState);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("AE state: %d", aeState);
	}

	//AE Get Version
	result = VsiCamDeviceAeGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get AE Version failed!\n", __func__);
		return -1;
	}
	else {
		LOGI("AEV4 Version: %d", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchAeFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.ae.enable;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.ae.useCfg) {
			result = VsiCamDeviceAeEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable AE Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonAeSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable);
			if (0 != result) {
				LOGE("Vvbench Set AE Configuration Failed!\n");
				return -1;
			}
			result = VsiCamCommonAeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result = VsiCamCommonAeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load AE Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonAeRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result = VsiCamCommonAeLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load AE Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchAeGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get AE Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit ", __func__);
	return result;
}

int VsiVvbenchAeGetResult
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceAeResult_t camDeviceAeResult;
	MEMSET(&camDeviceAeResult, 0, sizeof(CamDeviceAeResult_t));

	result = VsiCamDeviceAeGetResult(hCamDevice, &camDeviceAeResult);
	if (0 != result) {
		LOGE("Vvbench Get AE Result Failed!\n");
		return -1;
	}
	else {
		LOGI("Vvbench Get AE Result meanluma:%f\n", camDeviceAeResult.meanluma);
	}

	LOGI("%s exit \n", __func__);
	return result;
}
