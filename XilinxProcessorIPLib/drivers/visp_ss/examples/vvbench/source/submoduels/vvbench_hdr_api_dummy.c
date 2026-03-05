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

#define LOGTAG "HDRDUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_hdr_api.h"

int VsiVvbenchParseHdrInfo
(
	cJSON *hdrInfo,
	VvbenchModuleCfg_t *hdrCtrl
)
{
	int result = 0;
	LOGI("%s enter \n", __func__);
	if (NULL == hdrCtrl) {
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}
	LOGW("%s HDR feature not supported!!", __func__);
	hdrCtrl->isSupport = false;
	return result;
}

int VsiVvbenchHdrFunc
(
	CamDeviceHandle_t hCamDevice,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}
