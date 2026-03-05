/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __VVPATH_H__
#define __VVPATH_H__

#include "vvdevice.h"
#include "vvbparser.h"

int VsiVvdeviceInputPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdeviceOutPutPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdevicePathEnable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *bufIo,
	int bufCount
);

int VsiVvdevicePathDisable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *bufIo,
	int bufCount
);

int VsiVvdevicePathRelease
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdevicePathStart
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *pBufIo,
	int bufCount

);

#endif //__VVPATH_H__