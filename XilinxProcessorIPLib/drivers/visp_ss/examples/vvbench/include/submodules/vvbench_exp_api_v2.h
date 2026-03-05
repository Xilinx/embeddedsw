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
