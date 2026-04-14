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


#define LOGTAG "RGBIRV21"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_rgbir_api.h"

int VsiVvbenchParseRgbirInfo
(
	cJSON *rgbirInfo,
	VvbenchModuleCfg_t *rgbirCtrl
)
{
	int result = 0;

	LOGI("%s enter \n", __func__);
	if (NULL == rgbirCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	rgbirCtrl->isSupport = true;

	if (NULL != cJSON_GetObjectItem(rgbirInfo, "enable")) {
		rgbirCtrl->enable = cJSON_GetObjectItem(rgbirInfo, "enable")->valueint;
	}
	if (!rgbirCtrl->enable) {
		LOGI("RGBIR v2_1 feature not enable!");
	}
	else {
		LOGI("RGBIR v2_1 feature enable!");
	}
	if (NULL != cJSON_GetObjectItem(rgbirInfo, "useCfg")) {
		rgbirCtrl->useCfg = cJSON_GetObjectItem(rgbirInfo, "useCfg")->valueint;
	}
	if (rgbirCtrl->useCfg) {
		if (NULL != cJSON_GetObjectItem(rgbirInfo, "mode")) {
			rgbirCtrl->mode = cJSON_GetObjectItem(rgbirInfo, "mode")->valueint;
		}
		if (CAMDEV_CFG_MODE_MANUAL == rgbirCtrl->mode) {
			LOGI("RGBIR v2_1 use manual config");
		}
		else if (CAMDEV_CFG_MODE_AUTO == rgbirCtrl->mode) {
			LOGI("RGBIR v2_1 use auto config");
		}
		else {
			LOGW("invalid RGBIR v2_1 mode!");
			rgbirCtrl->useCfg = false;
			rgbirCtrl->status = false;
		}
	}
	return result;
}

int VsiVvbenchRgbirGetFunc
(
	CamDeviceHandle_t hCamDevice
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);

	if (NULL == hCamDevice) {
		return -1;
	}

	CamDeviceRgbirConfig_t config;
	CamDeviceRgbirStatus_t status;
	CamDeviceRgbirIrPathSel_t path = CAMDEV_RGBIR_IR_RAW_SELECT_MP;
	CamDeviceRgbirOutPat_t pattern = CAMDEV_RGBIR_OUT_PAT_RGGB;
	uint32_t version;
	MEMSET(&config, 0, sizeof(CamDeviceRgbirConfig_t));
	MEMSET(&status, 0, sizeof(CamDeviceRgbirStatus_t));

	result = VsiCamDeviceRgbirGetConfig(hCamDevice, &config);
	if (RET_SUCCESS != result) {
		LOGE("%s Get RGBIR config failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("irThreshold: %d ", config.irThreshold);
		LOGI("lThreshold: %d ", config.lThreshold);
		LOGI("dpccMidTh:[%d %d %d %d]", config.dpccMidTh[0], config.dpccMidTh[1], config.dpccMidTh[2],
		     config.dpccMidTh[3]);
	}

	result = VsiCamDeviceRgbirGetStatus(hCamDevice, &status);
	if (RET_SUCCESS != result) {
		LOGE("%s Get RGBIR status failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("RGBIR enable: %d\n", status.enable);
	}

	result = VsiCamDeviceRgbirGetIrPathSelect(hCamDevice, &path);
	if (RET_SUCCESS != result) {
		LOGE("%s Get RGBIR IR path select failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("RGBIR IR path select: %d\n", path);
	}

	result = VsiCamDeviceRgbirGetOutPattern(hCamDevice, &pattern);
	if (RET_SUCCESS != result) {
		LOGE("%s Get RGBIR output pattern failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("RGBIR output pattern: %d\n", pattern);
	}

	result = VsiCamDeviceRgbirGetVersion(hCamDevice, &version);
	if (RET_SUCCESS != result) {
		LOGE("%s Get RGBIR version failed!\n", __func__);
		return RET_FAILURE;
	}
	else {
		LOGI("RGBIR version: %x\n", version);
	}

	LOGI("%s exit \n", __func__);
	return result;
}

int VsiVvbenchRgbirFunc
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

	bool_t enable = caseCtx->instanceCfgCtx[index].moduleCfg.rgbir.enable;
	bool_t irRawOutEnable = caseCtx->instanceCfgCtx[index].funcCtrl.irRawOutEnable;
	bool_t sp1IrSelect = caseCtx->instanceCfgCtx[index].funcCtrl.sp1IrSelect;

	if (!caseCtx->swSimuCfg.enable) {
		if (enable && !caseCtx->instanceCfgCtx[index].moduleCfg.rgbir.useCfg) {
			result = VsiCamDeviceRgbirEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable RGBIR Failed!");
				return -1;
			}
			if (irRawOutEnable) {
				result = VsiCamDeviceRgbirIrRawOutEnable(hCamDevice);
				if (0 != result) {
					LOGE("Vvbench Set RGBIR output pattern failed!\n");
					return -1;
				}
				if (sp1IrSelect) {
					result = VsiCamDeviceRgbirSetIrPathSelect(hCamDevice, CAMDEV_RGBIR_IR_RAW_SELECT_SELF1);
					if (0 != result) {
						LOGE("Vvbench Set RGBIR output pattern failed!\n");
						return -1;
					}
				}
				else {
					result = VsiCamDeviceRgbirSetIrPathSelect(hCamDevice, CAMDEV_RGBIR_IR_RAW_SELECT_MP);
					if (0 != result) {
						LOGE("Vvbench Set RGBIR output pattern failed!\n");
						return -1;
					}
				}
			}
			result = VsiCamDeviceWbEnable(hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Enable WB failed!\n");
				return -1;
			}
		}
		else {
			result = VsiCamCommonRgbirSetState(caseCtx->instanceCfgCtx[index].hCamCommon, enable,
							   irRawOutEnable, sp1IrSelect);
			if (0 != result) {
				LOGE("Vvbench Set RGBIR State to Database Failed!\n");
				return -1;
			}
			result = VsiCamCommonRgbirRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
			result |= VsiCamCommonRgbirLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
			if (0 != result) {
				LOGE("Vvbench Load RGBIR Configuration Failed!\n");
				return -1;
			}
		}
	}
	else {
		result = VsiCamCommonRgbirGetState(caseCtx->instanceCfgCtx[index].hCamCommon, &enable);
		if (0 != result) {
			LOGE("Vvbench Get RGBIR Status from Database Failed!\n");
			return -1;
		}
		result = VsiCamCommonRgbirRangeCheck(caseCtx->instanceCfgCtx[index].hCamCommon);
		result |= VsiCamCommonRgbirLoadConfig(caseCtx->instanceCfgCtx[index].hCamCommon, hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Load RGBIR Configuration Failed!\n");
			return -1;
		}
	}

	if (enable) {
		result = VsiVvbenchRgbirGetFunc(hCamDevice);
		if (0 != result) {
			LOGE("Vvbench Get RGBIR Func Failed!\n");
			return -1;
		}
	}

	LOGI("%s exit \n", __func__);
	return result;
}
