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

#ifndef CAMDEV_CPD_API_H
#define CAMDEV_CPD_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 21_cam_device_cpd VsCamDevice E01C21 Device_CPD Definitions
 * @brief Provides interfaces for controlling the compand module working in
 * the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_CPD_CURVE_X_SIZE  64 /**< The number of coordinates CPD curve on the X-axis 64*/
#define CAMDEV_CPD_CURVE_Y_SIZE  64 /**< The number of coordinates CPD curve on the Y-axis 64*/

/******************************************************************************/
/**
 * @brief   VsCamDevice CPD type enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceCpdType_e {
    CAMDEV_CPD_TYPE_INAVLID,    /**< CPD module mode is invalid*/
    CAMDEV_CPD_TYPE_EXPAND     /**< CPD module working on expand mode */
}CamDeviceCpdType_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice expand configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpandConfig_s {
    uint32_t expandCurveX[CAMDEV_CPD_CURVE_X_SIZE];      /**< The X-coordinates of expand curve */
    uint32_t expandCurveY[CAMDEV_CPD_CURVE_Y_SIZE];     /**< The Y-coordinates of expand curve */
    bool_t expandUseOutYcurve;                          /**< not used */
}CamDeviceExpandConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CPD configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCpdConfig_s {
    CamDeviceExpandConfig_t   expandCfg;        /**< The expand curve configuration */
}CamDeviceCpdConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CPD status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCpdStatus_s {
    bool_t enable;              /**< CPD enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceExpandConfig_t   currentCfg;        /**< The expand curve current configuration */
}CamDeviceCpdStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets CPD configuration parameters.
 * @startuml VsiCamDeviceCpdSetConfig
 * !include E01_External/VsiCamDeviceCpdSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       cpdType     CPD type.
 * @param[in]       pCpdCfg     Pointer to CPD configuration.
 * @details This function calls: \ref CamEngineCmpdExpandSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceCpdReset
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
RESULT VsiCamDeviceCpdSetConfig
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceCpdType_t    cpdType,
    const CamDeviceCpdConfig_t  *pCpdCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CPD configuration parameters.
 * @startuml VsiCamDeviceCpdGetConfig
 * !include E01_External/VsiCamDeviceCpdGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       cpdType     CPD type.
 * @param[inout]    pCpdCfg     Pointer to CPD configuration.
 * @details This function calls: \ref CamEngineCmpdExpandGetConfig
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
RESULT VsiCamDeviceCpdGetConfig
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceCpdType_t    cpdType,
    CamDeviceCpdConfig_t *pCpdCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables CPDExpand.
 * @startuml VsiCamDeviceCpdExpandEnable
 * !include E01_External/VsiCamDeviceCpdExpandEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineCmpdExpandEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdExpandEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CPDExpand.
 * @startuml VsiCamDeviceCpdExpandDisable
 * !include E01_External/VsiCamDeviceCpdExpandDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineCmpdExpandDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdExpandDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPDExpand status.
 * @startuml VsiCamDeviceCpdExpandGetStatus
 * !include E01_External/VsiCamDeviceCpdExpandGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to CPD status.
 * @details This function calls: \ref CamEngineCpdExpandGetStatus
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
RESULT VsiCamDeviceCpdExpandGetStatus
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceCpdStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CPD. It's only available in manual mode.
 * @startuml VsiCamDeviceCpdReset
 * !include E01_External/VsiCamDeviceCpdReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceCpdSetConfig
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
RESULT VsiCamDeviceCpdReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPD version.
 * @startuml VsiCamDeviceCpdGetVersion
 * !include E01_External/VsiCamDeviceCpdGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to CPD version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);
/** @} 21_cam_device_cpd */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_CPD_API_H
