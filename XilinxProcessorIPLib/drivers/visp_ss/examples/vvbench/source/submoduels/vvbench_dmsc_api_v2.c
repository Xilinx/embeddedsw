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


#define LOGTAG "DMSCV2"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_dmsc_api.h"

int VsiVvbenchParseDmscInfo
(
	cJSON *dmscInfo,
	VvbenchModuleCfg_t *dmscCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == dmscCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	dmscCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(dmscInfo, "enable")) {
		dmscCtrl->enable = cJSON_GetObjectItem(dmscInfo, "enable")->valueint;
	}
	if (!dmscCtrl->enable) {
		LOGI("DMSC feature not enable!");
	}
	else {
		LOGI("DMSC feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(dmscInfo, "useCfg")) {
		dmscCtrl->useCfg = cJSON_GetObjectItem(dmscInfo, "useCfg")->valueint;
	}
	if (dmscCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(dmscInfo, "mode")) {
			dmscCtrl->mode = cJSON_GetObjectItem(dmscInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == dmscCtrl->mode) {
			LOGI("DMSC use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == dmscCtrl->mode) {
			LOGI("DMSC use auto config");
		}
		else {
			LOGW("invalid DMSC mode!");
			dmscCtrl->useCfg = false;
			dmscCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchDmscGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceDmscConfig_t config;
	CamDeviceDmscStatus_t status;
	uint32_t version;
	MEMSET(&status, 0, sizeof(CamDeviceDmscStatus_t));
	MEMSET(&config, 0, sizeof(CamDeviceDmscConfig_t));

	result = VsiCamDeviceDmscGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DMSC config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("demosaicThr: %d ", config.manualCfg.demosaicThr);
		LOGI("dmscDirThrMax: %d ", config.manualCfg.dmscDirThrMax);
		LOGI("dmscDemoireR1: %d ", config.manualCfg.demoireCfg.dmscDemoireR1);
		LOGI("dmscDemoireAreaThr: %d ", config.manualCfg.demoireCfg.dmscDemoireAreaThr);
		LOGI("dmscSharpenClipBlack: %d ", config.manualCfg.sharpenCfg.dmscSharpenClipBlack);
		LOGI("dmscSharpenT2Shift: %d ", config.manualCfg.sharpenCfg.dmscSharpenT2Shift);
		LOGI("dmscSharpenLineR1: %d ", config.manualCfg.sharpenLineCfg.dmscSharpenLineR1);
		LOGI("dmscSharpenLineThrShift1: %d ", config.manualCfg.sharpenLineCfg.dmscSharpenLineThrShift1);
		LOGI("skinCbThrMax: %d ", config.manualCfg.skinCfg.dmscSkinCbThrMax);
		LOGI("dmscSkinYThrMin: %d ", config.manualCfg.skinCfg.dmscSkinYThrMin);
		LOGI("dmscDepurpleCbcrMode: %d ", config.manualCfg.depurpleCfg.dmscDepurpleCbcrMode);
		LOGI("dmscDepurpleSatShrink: %d ", config.manualCfg.depurpleCfg.dmscDepurpleSatShrink);
		LOGI("aBlue: %f ", config.manualCfg.cacCfg.aBlue);
		LOGI("cRed: %f ", config.manualCfg.cacCfg.cRed);
	}

	result = VsiCamDeviceDmscGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DMSC status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DMSC enable: %d, cacEnable: %d, demoireEnable: %d, depurpleEnable: %d, \
            sharpenEnable: %d, sharpenLineEnable: %d, skinEnable: %d",
		     status.enable,
		     status.cacEnable,
		     status.dmscDemoireEnable,
		     status.dmscDepurpleEnable,
		     status.dmscSharpenEnable,
		     status.dmscSharpenLineEnable,
		     status.dmscSkinEnable
		    );
	}

	result = VsiCamDeviceDmscGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get DMSC version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("DMSC version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchDmscFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.dmsc.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.dmsc.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.dmsc.useCfg) {
			result = VsiCamDeviceDmscEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable DMSC Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommonDmscSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set DMSC State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonDmscRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonDmscLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load DMSC Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonDmscGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get DMSC Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonDmscRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonDmscLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load DMSC Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchDmscGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get DMSC Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
