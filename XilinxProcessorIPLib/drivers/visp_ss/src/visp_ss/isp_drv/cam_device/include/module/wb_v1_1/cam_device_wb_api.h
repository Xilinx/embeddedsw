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

#ifndef CAMDEV_WB_API_H
#define CAMDEV_WB_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 05_cam_device_wb VsCamDevice E01C05 Device_WBG Definitions
 * @brief Provides interfaces for controlling the white balance module working
 * in the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice WB manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceManualWbConfig_s {
    float32_t gain[CAMDEV_RAW_CHANNEL_NUM];       /**< WB gain */
}CamDeviceManualWbConfig_t;

 /******************************************************************************/
/**
 * @brief   VsCamDevice WB configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWbConfig_s {
    CamDeviceManualWbConfig_t manualCfg;  /**< WB manual configuration */
}CamDeviceWbConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WB status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceWbStatus_s {
    bool_t enable;              /**< WB enable status */
    CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
    CamDeviceManualWbConfig_t currentCfg;  /**< WB current configuration */
}CamDeviceWbStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets WB configuration parameters.
 * @startuml VsiCamDeviceWbSetConfig
 * !include E01_External/VsiCamDeviceWbSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pWbCfg      Pointer to WB configuration.
 * @details This function calls: \ref CamEngineWbSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceWbReset
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
RESULT VsiCamDeviceWbSetConfig
(
    CamDeviceHandle_t hCamDevice,
    const CamDeviceWbConfig_t *pWbCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets WB configuration.
 * @startuml VsiCamDeviceWbGetConfig
 * !include E01_External/VsiCamDeviceWbGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pWbCfg      Pointer to WB configuration.
 * @details This function calls: \ref CamEngineWbGetConfig
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
RESULT VsiCamDeviceWbGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceWbConfig_t *pWbCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables WB.
 * @startuml VsiCamDeviceWbEnable
 * !include E01_External/VsiCamDeviceWbEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWbEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables WB.
 * @startuml VsiCamDeviceWbDisable
 * !include E01_External/VsiCamDeviceWbDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWbDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WB status.
 * @startuml VsiCamDeviceWbGetStatus
 * !include E01_External/VsiCamDeviceWbGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to WB status.
 * @details This function calls: \ref CamEngineWbGetStatus
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
RESULT VsiCamDeviceWbGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceWbStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets WB.
 * @startuml VsiCamDeviceWbReset
 * !include E01_External/VsiCamDeviceWbReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamCalibDbGetCcProfileByName, \ref VsiCamDeviceWbSetConfig
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
RESULT VsiCamDeviceWbReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WB version.
 * @startuml VsiCamDeviceWbGetVersion
 * !include E01_External/VsiCamDeviceWbGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to WB version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 05_cam_device_wb */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_WB_API_H
