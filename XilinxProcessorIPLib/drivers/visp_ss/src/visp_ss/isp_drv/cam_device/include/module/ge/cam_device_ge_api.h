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

#ifndef CAMDEV_GE_API_H
#define CAMDEV_GE_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 18_cam_device_ge VsCamDevice E01C18 Device_GE Definitions
 * @brief Provides interfaces for controlling the green equilibration module
 * working in the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice GE manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGeManualConfig_s {
    float32_t threshold;                     /**< GE threshold */
}CamDeviceGeManualConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device GE auto configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGeAutoConfig_s
{
    uint8_t autoLevel;  /**< GE auto configuration level */
    float32_t gain[CAMDEV_ISO_STRENGTH_NUM];  /**< GE gain */
    float32_t threshold[CAMDEV_ISO_STRENGTH_NUM]; /**< GE threshold */
} CamDeviceGeAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GE configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGeConfig_s {
    CamDeviceConfigMode_t mode;           /**< GE mode configuration */
    CamDeviceGeAutoConfig_t autoCfg;      /**< GE auto configuration */
    CamDeviceGeManualConfig_t manualCfg; /**< GE manual configuration */
}CamDeviceGeConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GE status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGeStatus_s {
    bool_t enable;                      /**< GE enable status */
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceGeManualConfig_t currentCfg; /**< GE current configuration */
}CamDeviceGeStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets GE configuration parameters.
 * @startuml VsiCamDeviceGeSetConfig
 * !include E01_External/VsiCamDeviceGeSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pGeCfg      Pointer to GE Configuration
 * @details This function calls: \ref CamEngineGeSetConfig, \ref CamEngineGeSetAutoConfig,
 * \ref CamEngineGeSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceGeReset
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
RESULT VsiCamDeviceGeSetConfig
(
    CamDeviceHandle_t hCamDevice,
    const CamDeviceGeConfig_t *pGeCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets GE configuration parameters.
 * @startuml VsiCamDeviceGeGetConfig
 * !include E01_External/VsiCamDeviceGeGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pGeCfg      Pointer to GE configuration.
 * @details This function calls: \ref CamEngineGeGetMode, \ref CamEngineGeGetConfig,
 * \ref CamEngineGeGetAutoConfig
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
RESULT VsiCamDeviceGeGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGeConfig_t *pGeCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables GE.
 * @startuml VsiCamDeviceGeEnable
 * !include E01_External/VsiCamDeviceGeEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGeEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GE.
 * @startuml VsiCamDeviceGeDisable
 * !include E01_External/VsiCamDeviceGeDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGeDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GE status.
 * @startuml VsiCamDeviceGeGetStatus
 * !include E01_External/VsiCamDeviceGeGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to GE status.
 * @details This function calls: \ref CamEngineGeGetStatus
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
RESULT VsiCamDeviceGeGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGeStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets GE.
 * @startuml VsiCamDeviceGeReset
 * !include E01_External/VsiCamDeviceGeReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceGeSetConfig
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
RESULT VsiCamDeviceGeReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GE version.
 * @startuml VsiCamDeviceGeGetVersion
 * !include E01_External/VsiCamDeviceGeGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to CPROC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 18_cam_device_ge */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_GE_API_H
