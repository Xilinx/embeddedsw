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


#define LOGTAG "2DNRV52"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_2dnr_api.h"

int VsiVvbenchParse2dnrInfo
(
	cJSON *dnr2Info,
	VvbenchModuleCfg_t *dnr2Ctrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == dnr2Ctrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	dnr2Ctrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(dnr2Info, "enable")) {
		dnr2Ctrl->enable = cJSON_GetObjectItem(dnr2Info, "enable")->valueint;
	}
	if (!dnr2Ctrl->enable) {
		LOGI("2DNR feature not enable!");
	}
	else {
		LOGI("2DNR feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(dnr2Info, "useCfg")) {
		dnr2Ctrl->useCfg = cJSON_GetObjectItem(dnr2Info, "useCfg")->valueint;
	}
	if (dnr2Ctrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(dnr2Info, "mode")) {
			dnr2Ctrl->mode = cJSON_GetObjectItem(dnr2Info, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == dnr2Ctrl->mode) {
			LOGI("2DNR use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == dnr2Ctrl->mode) {
			LOGI("2DNR use auto config");
		}
		else {
			LOGW("invalid 2DNR mode!");
			dnr2Ctrl->useCfg = false;
			dnr2Ctrl->status = false;
		}
	}
	return result;
}

int VsiVvbench2DnrGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDevice2DnrConfig_t config;
	CamDevice2DnrStatus_t status;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDevice2DnrConfig_t));
	MEMSET(&status, 0, sizeof(CamDevice2DnrStatus_t));

	result = VsiCamDevice2DnrGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get 2DNR config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("pregammaStrength: %d\n", config.manualCfg.pregammaStrength);
		LOGI("sigma: %f\n", config.manualCfg.sigma);
		LOGI("strength: %d\n", config.manualCfg.strength);
		LOGI("sigmaOffset: %d\n", config.manualCfg.sigmaOffset);
		for (int i = 0; i < CAMDEV_DNR_LUMA_CURVE_SIZE; ++i) {
			LOGI("lumaCurveX[%d]: %d\n", i, config.manualCfg.curveCfg.lumaCurveX[i]);
		}
	}

	result = VsiCamDevice2DnrGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get 2DNR status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("2DNR enable: %d\n", status.enable);
	}

	result = VsiCamDevice2DnrGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get 2DNR version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("2DNR version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbench2DnrFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.dnr2.enable;
	CamDeviceConfigMode_t mode = caseCtx->instanceCfgCtx[index].moduleCfg.dnr2.mode;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.dnr2.useCfg) {
			result = VsiCamDevice2DnrEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable 2DNR Failed!");
				return -1;
			}
		}
		else {
			result = VsiCamCommon2DnrSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable, mode);
			if (0 != result) {
				LOGE("Vvbench Set 2DNR State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommon2DnrRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommon2DnrLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load 2DNR Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommon2DnrGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get 2DNR Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommon2DnrRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommon2DnrLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load 2DNR Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbench2DnrGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get 2DNR Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
