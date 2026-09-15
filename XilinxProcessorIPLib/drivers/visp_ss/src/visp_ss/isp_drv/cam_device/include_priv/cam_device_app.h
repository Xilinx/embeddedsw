/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2022 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __CAMDEV_DEVICE_H__
#define __CAMDEV_DEVICE_H__

#include "cam_device_common.h"
#include "cam_device_module_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DEBUG_FLAG

/**
*@ispcore structure define
**/
/*******************************************/
typedef struct CamDeviceIspcore_s {
	CamDeviceHandle_t hCamDevSet[CAMDEV_HARDWARE_ID_MAX][CAMDEV_VIRTUAL_ID_MAX];

} CamDeviceIspcore_t;

//typedef HalMemHandle_t CamDeviceHalMemHandle_t;

/**
*@ispcore api define
**/
/*******************************************/
void CamDeviceIspcoreInit();

RESULT CamDeviceRequestInstance
(
	uint32_t hwId,
	CamDeviceHandle_t *pCamDevhandle,
	uint32_t *pvtId
);

RESULT CamDeviceFreeInstance
(
	CamDeviceHandle_t camDevhandle,
	uint32_t hwId
);

RESULT CamDeviceInstanceIdMapping
(
	uint32_t hwId,
	uint32_t vtId,
	uint32_t *pInstanceId
);


typedef struct CamDeviceEventCb_s {
	CamDeviceEventFunc_t func;           /**< pointer to callback function */
	void *pUserContext;  /**< user context */
} CamDeviceEventCb_t;


typedef struct CamDeviceContext_s {

	uint32_t ispHwId;
	uint32_t ispVtId;
	uint32_t instanceId;
	uint32_t cookie;
	CamDeviceEventCb_t camDeviceFusaEventCb;
} CamDeviceContext_t;

#ifdef __cplusplus
}
#endif

#endif    // __CAMDEV_DEVICE_H__
