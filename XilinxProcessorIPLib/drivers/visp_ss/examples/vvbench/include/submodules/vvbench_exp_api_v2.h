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


#ifndef __VVDEV_EXP2_H__
#define __VVDEV_EXP2_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_exp_api_v2.h"

#define VVBENCH_AAA_EXP_GRID_SIZE     32    /**< 3A EXP grid size */
typedef uint8_t VvbenchExpV2Statistics_t[VVBENCH_AAA_EXP_GRID_SIZE * VVBENCH_AAA_EXP_GRID_SIZE *
		16];


int VsiVvbenchParseExpv2Info
(
	cJSON *expv2Info,
	VvbenchModuleCfg_t *expv2Ctrl
);

int VsiVvbenchExpv2GetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchExpv2Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchExpv2GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	VvbenchExpV2Statistics_t *ExpV2Statistics
);

#endif //__VVDEV_EXP2_H__
