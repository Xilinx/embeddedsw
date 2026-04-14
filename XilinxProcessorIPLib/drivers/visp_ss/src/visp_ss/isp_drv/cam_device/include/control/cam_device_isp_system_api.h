// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
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

#include "types.h"
#include "return_codes.h"
#include "cam_device_common.h"
#include <system_module.h>

/**
 * @defgroup 27_cam_device_general VsCamDevice E01C27 Device_GeneralCtrl Definitions
 * @{
 *
 *
 */

/*****************************************************************************/
/**
 * @brief   This function sets enqueue buffer callback.
 * @startuml VsiCamDeviceSetEnqueueBufCb
 * !include E01_External/VsiCamDeviceSetEnqueueBufCb.plantuml
 * @enduml
 * @param[inout]    hCamDevice     Handle to the VsCamDevice instance.
 * @param[in]       func  Enqueue buffer callback.
 * @param[in]       pUserContext      Pointer to user context.
 * @details This function calls: \ref CamEngineSetEnqueueBufCb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetEnqueueBufCb
(
    CamDeviceHandle_t     hCamDevice,
    IspSystemEnqueueBuf_t func,
    void                  *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function sets dequeue buffer callback.
 * @startuml VsiCamDeviceSetDequeueBufCb
 * !include E01_External/VsiCamDeviceSetDequeueBufCb.plantuml
 * @enduml
 * @param[inout]    hCamDevice     Handle to the VsCamDevice instance.
 * @param[in]       func  Enqueue buffer callback.
 * @param[in]       pUserContext      Pointer to user context.
 * @details This function calls: \ref CamEngineSetDequeueBufCb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetDequeueBufCb
(
    CamDeviceHandle_t     hCamDevice,
    IspSystemDequeueBuf_t func,
    void                  *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function starts ISP streaming.
 * @startuml VsiCamDeviceStartStreaming
 * !include E01_External/VsiCamDeviceStartStreaming.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to start streaming configuration.
 * @details This function calls: \ref VsiCamDeviceSetPathStreaming
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
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
 * @startuml VsiCamDeviceStopStreaming
 * !include E01_External/VsiCamDeviceStopStreaming.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to start streaming configuration.
 * @details This function calls: \ref VsiCamDeviceSetPathStreaming
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceStopStreaming
(
    CamDeviceHandle_t hCamDevice,
    void *pConfig
);

 /** @} cam_device_isp_system */

#endif    // __ISP_CAMDEV_ISP_SYSTEM_H__
