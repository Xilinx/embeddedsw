/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include <cam_device_app.h>


RESULT VsiCamDeviceAfRegister
(
	CamDeviceHandle_t hCamDevice,
	void *pAfLibHandle
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfUnRegister
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfSetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfMode_t *pMode
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfGetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfMode_t *pMode
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfConfig_t *pConfig
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfConfig_t *pConfig
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfSetStatisticWindow
(
	CamDeviceHandle_t hCamDevice,
	uint8_t AfmWinId,
	CamDeviceWindow_t *pWindow
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfEnable
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfDisable
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfState_t *pStatus
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfReset
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}
