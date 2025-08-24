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
#include "oslayer.h"


CamDeviceIspcore_t gCamDevIspcore;

void CamDeviceIspcoreInit()
{
	static bool_t init = BOOL_FALSE;

	if (init == BOOL_FALSE) {
		for (uint32_t hwId = 0; hwId < CAMDEV_HARDWARE_ID_MAX; hwId++) {
			for (uint32_t vtId = 0; vtId < CAMDEV_VIRTUAL_ID_MAX; vtId++)
				gCamDevIspcore.hCamDevSet[hwId][vtId] = NULL;
		}
		init = BOOL_TRUE;
	}
	return;
}

RESULT CamDeviceRequestInstance
(
	uint32_t hwId,
	CamDeviceHandle_t *pCamDevhandle,
	uint32_t *pvtId
)
{
	uint32_t index = 0;

	if (hwId > CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
	for (; index < CAMDEV_VIRTUAL_ID_MAX; index ++) {
		if (NULL == gCamDevIspcore.hCamDevSet[hwId][index])
			break;
	}
	if (index < CAMDEV_VIRTUAL_ID_MAX) {
		gCamDevIspcore.hCamDevSet[hwId][index] = osMalloc(sizeof(CamDeviceContext_t));
		if (NULL == gCamDevIspcore.hCamDevSet[hwId][index])
			return RET_OUTOFMEM;
		*pCamDevhandle = gCamDevIspcore.hCamDevSet[hwId][index];
		*pvtId = index;
		return 0;
	} else
		return RET_NOTAVAILABLE;
}

RESULT CamDeviceFreeInstance
(
	CamDeviceHandle_t camDevhandle,
	uint32_t hwId
)
{
	uint32_t index = 0;
	if (hwId > CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
	for (; index < CAMDEV_VIRTUAL_ID_MAX; index++) {
		if (camDevhandle == gCamDevIspcore.hCamDevSet[hwId][index])
			break;
	}
	if (index < CAMDEV_VIRTUAL_ID_MAX) {
		osFree(camDevhandle);
		gCamDevIspcore.hCamDevSet[hwId][index] = NULL;
		return 0;
	} else
		return RET_NOTAVAILABLE;
}

RESULT CamDeviceInstanceIdMapping
(
	uint32_t hwId,
	uint32_t vtId,
	uint32_t *pInstanceId
)
{
	RESULT ret = RET_SUCCESS;

	/* Hardware Pipeline ID / Virtual Device ID Mapping Policy */
	/* The mapping can be modified according to system configurations */
	/*---------------------------------------------------------------------------- */
	/*    ID                        |   HW              |   VT                     */
	/*---------------------------------------------------------------------------- */
	/*    0                         |   0               |   0                      */
	/*    1                         |   0               |   1                      */
	/*    2                         |   0               |   2                      */
	/*    ..                        |   ..              |   ..                     */
	/*    CAMDEV_VIRTUAL_ID_MAX-1   |   0               |   CAMDEV_VIRTUAL_ID_MAX -1      */
	/*---------------------------------------------------------------------------- */
	/*    CAMDEV_VIRTUAL_ID_MAX     |   1               |   0                      */
	/*    CAMDEV_VIRTUAL_ID_MAX+1   |   1               |   1                      */
	/*    ..                        |   ..              |   ..                     */
	/*    CAMDEV_VIRTUAL_ID_MAX*2-1 |   1               |   CAMDEV_VIRTUAL_ID_MAX -1      */
	/*------------------------------------------------------------------------- */
	if (NULL == pInstanceId)
		return RET_NULL_POINTER;

	if (hwId >= CAMDEV_HARDWARE_ID_MAX)
		return RET_UNSUPPORT_ID;

	if (vtId >= CAMDEV_VIRTUAL_ID_MAX)
		return RET_UNSUPPORT_ID;

	*pInstanceId = hwId * CAMDEV_VIRTUAL_ID_MAX + vtId;
	return ret;
}
