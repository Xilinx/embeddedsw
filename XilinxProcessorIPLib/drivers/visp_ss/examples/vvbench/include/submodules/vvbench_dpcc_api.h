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

#ifndef __VVDEV_DPCC_H__
#define __VVDEV_DPCC_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_dpcc_api.h"

int VsiVvbenchParseDpccInfo
(
	cJSON *dpccInfo,
	VvbenchModuleCfg_t *dpccCtrl
);

int VsiVvbenchDpccGetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchDpccFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

#endif //__VVDEV_DPCC_H__
