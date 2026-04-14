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

#ifndef CAMDEV_GC_V2_API_H
#define CAMDEV_GC_V2_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 15_cam_device_gc VsCamDevice E01C15 Device_GC Definitions
 * @brief Provides interfaces for controlling the gamma correction module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_GC_CURVE_SIZE 64   /**< Curve size */

/******************************************************************************/
/**
 * @brief   VsCamDevice GC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcAutoConfig_s {
    uint8_t autoLevel;                                               /**< GC auto configuration level */

    float32_t gain[CAMDEV_ISO_STRENGTH_NUM];                             /**< GC gain */
    uint16_t curve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve */
}CamDeviceGcAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GC current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcManualConfig_s {
    bool_t standard;                        /**< True/false: true for standard_val, false for curve */
    float32_t standardVal;                      /**< Generate uniform curve with gamma formula */
    uint16_t curve[CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve */
    bool_t userCurveX;   /**< User curve on X axis */
    uint32_t curvePx[CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve Px */
}CamDeviceGcManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcConfig_s {
    CamDeviceConfigMode_t mode;           /**< GC mode configuration */
    CamDeviceGcAutoConfig_t autoCfg;      /**< GC auto configuration */
    CamDeviceGcManualConfig_t manualCfg;  /**< GC manual configuration */
}CamDeviceGcConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGcStatus_s {
    bool_t enable;            /**< GC enable status */
    CamDeviceConfigMode_t currentMode;           /**< The run mode: 0--manual, 1--auto */
    CamDeviceGcManualConfig_t currentCfg;  /**< GC current configuration */
}CamDeviceGcStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets GC configuration parameters.
 * @startuml VsiCamDeviceGcSetConfig
 * !include E01_External/VsiCamDeviceGcSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pGcCfg      Pointer to GC Configuration
 * @details This function calls: \ref CamDeviceGcManualSetConfig, \ref CamDeviceGcAutoSetConfig,
 * \ref CamEngineGcSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceGcReset
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
RESULT VsiCamDeviceGcSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGcConfig_t *pGcCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets GC configuration parameters.
 * @startuml VsiCamDeviceGcGetConfig
 * !include E01_External/VsiCamDeviceGcGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pGcCfg      Pointer to GC configuration.
 * @details This function calls: \ref CamEngineGcGetMode, \ref CamDeviceGcManualGetConfig,
 * \ref CamDeviceGcAutoGetConfig
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
RESULT VsiCamDeviceGcGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGcConfig_t *pGcCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables GC.
 * @startuml VsiCamDeviceGcEnable
 * !include E01_External/VsiCamDeviceGcEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGcEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GC.
 * @startuml VsiCamDeviceGcDisable
 * !include E01_External/VsiCamDeviceGcDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGcDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GC status.
 * @startuml VsiCamDeviceGcGetStatus
 * !include E01_External/VsiCamDeviceGcGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to GC status.
 * @details This function calls: \ref CamEngineGcGetStatus
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
RESULT VsiCamDeviceGcGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGcStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets GC. It's only available in manual mode.
 * @startuml VsiCamDeviceGcReset
 * !include E01_External/VsiCamDeviceGcReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceGcSetConfig
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
RESULT VsiCamDeviceGcReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GC version.
 * @startuml VsiCamDeviceGcGetVersion
 * !include E01_External/VsiCamDeviceGcGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to GC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/** @} 15_cam_device_gc */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_GC_V2_API_H
