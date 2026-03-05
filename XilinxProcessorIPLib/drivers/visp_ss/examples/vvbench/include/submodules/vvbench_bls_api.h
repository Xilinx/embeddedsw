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

#ifndef __VVDEV_BLS_H__
#define __VVDEV_BLS_H__

#include "vvdevice_common.h"
#include "cJSON.h"
#include "cam_device_bls_api.h"
#include "cam_device_cpd_api.h"
#include "cam_device_hdr_api.h"
#include "cam_device_rgbir_api.h"

int VsiVvbenchParseBlsInfo
(
	cJSON *blsInfo,
	VvbenchModuleCfg_t *blsCtrl
);

int VsiVvbenchBlsGetFunc
(
	CamDeviceHandle_t hCamDevice
);

int VsiVvbenchBlsFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
);

#endif //__VVDEV_BLS_H__
