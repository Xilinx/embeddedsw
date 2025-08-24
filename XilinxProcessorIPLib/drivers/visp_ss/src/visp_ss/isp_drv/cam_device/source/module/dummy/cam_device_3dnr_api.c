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


RESULT VsiCamDevice3DnrSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrConfig_t *pConfig
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDevice3DnrGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrConfig_t *pConfig
)
{
	return RET_SUCCESS;
}


RESULT VsiCamDevice3DnrEnable
(
	CamDeviceHandle_t hCamDevice
)
{
	//not implement in cam_engine
	return RET_SUCCESS;
}


RESULT VsiCamDevice3DnrDisable
(
	CamDeviceHandle_t hCamDevice
)
{
	//not implement in cam_engine
	return RET_SUCCESS;
}

RESULT VsiCamDevice3DnrGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrStatus_t *pStatus
)
{

	return RET_SUCCESS;
}

RESULT VsiCamDevice3DnrGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
)
{
	return RET_SUCCESS;
}

RESULT VsiCamDevice3DnrReset
(
	CamDeviceHandle_t hCamDevice
)
{
	return RET_SUCCESS;
}
