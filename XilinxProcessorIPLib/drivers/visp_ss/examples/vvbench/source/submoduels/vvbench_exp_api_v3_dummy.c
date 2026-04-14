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


#define LOGTAG "EXPV3DUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_exp_api_v3.h"

int VsiVvbenchParseExpv3Info
(
	cJSON *expv3Info,
	VvbenchModuleCfg_t *expv3Ctrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == expv3Ctrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s EXP v3 feature not supported!!", __func__);
	expv3Ctrl->isSupport = false;
	return result;
}

int VsiVvbenchExpv3Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}

int VsiVvbenchExpv3GetStatistic
(
	CamDeviceHandle_t hCamDevice
)
{
	return 0;
}
