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

#ifndef __VVDEV_VI200_D_H__
#define __VVDEV_VI200_D_H__

#include "vvdevice_common.h"
#include "cJSON.h"

int VsiVvbenchParseVi200Info
(
	cJSON *vi200Info,
	VvbenchInstanceVi200Cfg_t *vi200Ctrl
);

int VsiVvbenchVi200Func
(
	CamDeviceVi200Handle_t hVi200,
	VvbenchVvdev_t *caseCtx,
	int index
);

int VsiVvbenchVi200MetadataGetBufferSize
(
	CamDeviceVi200Handle_t hVi200,
	uint32_t *pBufSize
);


int VsiVvbenchVi200TpgFunc
(
	CamDeviceVi200Handle_t hVi200,
	VvbenchVvdev_t *caseCtx,
	int index
);
#endif //__VVDEV_VI200_D_H__
