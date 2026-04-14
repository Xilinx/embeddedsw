// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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

#ifndef CAMDEV_DG_API_H
#define CAMDEV_DG_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 02_cam_device_dg VsCamDevice E01C02 Device_DG Definitions
 * @brief Provides interfaces for controlling the digital gain module working
 * in the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice DG manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDgManualConfig_s {
    float32_t digitalGainB;         /**< Blue DG */
    float32_t digitalGainGb;        /**< Green blue DG */
    float32_t digitalGainGr;        /**< Green red DG */
    float32_t digitalGainR;         /**< Red DG */
}CamDeviceDgManualConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DG configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDgConfig_s {
    CamDeviceDgManualConfig_t manualCfg;        /**< DG manual configuration*/
}CamDeviceDgConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DG status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDgStatus_s {
    bool_t enable;              /**< DG enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceDgManualConfig_t currentCfg;        /**< DG current configuration*/
}CamDeviceDgStatus_t;

/***************************************************/

/*****************************************************************************/
/**
 * @brief   This function sets DG configuration parameters.
 * @startuml VsiCamDeviceDgSetConfig
 * !include E01_External/VsiCamDeviceDgSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pDgCfg      Pointer to DG configuration.
 * @details This function calls: \ref CamEngineDgSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceDgReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgSetConfig
(
    CamDeviceHandle_t    hCamDevice,
    const CamDeviceDgConfig_t *pDgCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets DG configuration parameters.
 * @startuml VsiCamDeviceDgGetConfig
 * !include E01_External/VsiCamDeviceDgGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pDgCfg      Pointer to DG configuration.
 * @details This function calls: \ref CamEngineDgGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetConfig
(
    CamDeviceHandle_t    hCamDevice,
    CamDeviceDgConfig_t *pDgCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables DG.
 * @startuml VsiCamDeviceDgEnable
 * !include E01_External/VsiCamDeviceDgEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDgEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DG.
 * @startuml VsiCamDeviceDgDisable
 * !include E01_External/VsiCamDeviceDgDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDgDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DG status.
 * @startuml VsiCamDeviceDgGetStatus
 * !include E01_External/VsiCamDeviceDgGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to DG status.
 * @details This function calls: \ref CamEngineDgGetStatus
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetStatus
(
    CamDeviceHandle_t    hCamDevice,
    CamDeviceDgStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DG.
 * @startuml VsiCamDeviceDgReset
 * !include E01_External/VsiCamDeviceDgReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceDgSetConfig
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DG version.
 * @startuml VsiCamDeviceDgGetVersion
 * !include E01_External/VsiCamDeviceDgGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to DG version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/** @} 02_cam_device_dg */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_DG_API_H
