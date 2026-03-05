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

#ifndef __VVDEV_EXP3_D_H__
#define __VVDEV_EXP3_D_H__

#include "vvdevice_common.h"
#include "cJSON.h"

int VsiVvbenchParseExpv3Info
(
	cJSON *expv3Info,
	VvbenchModuleCfg_t *expv3Ctrl
);
int VsiVvbenchExpv3Func
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);
int VsiVvbenchExpv3GetStatistic
(
	CamDeviceHandle_t hCamDevice
);
#endif //__VVDEV_EXP3_D_H__