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


RESULT VsiCamDeviceAfmSetThreshold
(
	CamDeviceHandle_t hCamDevice,
	const uint32_t threshold

)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceAfmGetThreshold
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pThreshold
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmGetResult
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmMeasureResult_t *pResult
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmSetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmWindowId_t afmWinId,
	CamDeviceWindow_t *pWindow
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmGetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmWindowId_t afmWinId,
	CamDeviceWindow_t *pWindow
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmEnable
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmDisable
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmStatus_t *pStatus
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDeviceAfmReset
(
	CamDeviceHandle_t hCamDevice
)
{

	return RET_SUCCESS;
}
