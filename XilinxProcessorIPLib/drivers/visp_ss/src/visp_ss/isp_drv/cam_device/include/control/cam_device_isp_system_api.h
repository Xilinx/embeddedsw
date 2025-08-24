/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2024 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#ifndef CAMDEV_ISP_SYSTEM_API_H
#define CAMDEV_ISP_SYSTEM_API_H

#include <types.h>
#include <return_codes.h>
#include "cam_device_common.h"
#include <system_module.h>

/**
 * @defgroup cam_device_isp_system CamDevice ISP System Definitions
 * @{
 *
 *
 */

/*****************************************************************************/
/**
 * @brief   This function sets enqueue buffer callback.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   func                Enqueue buffer callback
 * @param   pUserContext        Pointer to user context
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceSetEnqueueBufCb
(
	CamDeviceHandle_t hCamDevice,
	IspSystemEnqueueBuf_t func,
	void *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function sets dequeue buffer callback.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   func                Dequeue buffer callback
 * @param   pUserContext        Pointer to user context
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceSetDequeueBufCb
(
	CamDeviceHandle_t hCamDevice,
	IspSystemDequeueBuf_t func,
	void *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function starts ISP streaming.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to start streaming configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceStartStreaming
(
	CamDeviceHandle_t hCamDevice,
	void *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function stops ISP streaming.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to stop streaming configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceStopStreaming
(
	CamDeviceHandle_t hCamDevice,
	void *pConfig
);


/* @} cam_device_isp_system */

#endif    // __ISP_CAMDEV_ISP_SYSTEM_H__
