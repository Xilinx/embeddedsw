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

#ifndef CAMDEV_DPCC_API_H
#define CAMDEV_DPCC_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 11_cam_device_dpcc VsCamDevice E01C11 Device_DPCC Definitions
 * @brief Provides interfaces for controlling the defect pixel cluster correction
 * module working in the ISP pipeline.
 * @{
 *
 */

/*******************************************/
#define CAMDEV_DPCC_DEFECT_PIXEL_NUM          2048  /**< Maximum number of defected pixels */
#define CAMDEV_DPCC_CHANNEL_NUM               2     /**< Channel number of defected pixels */
#define CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM 3     /**< Method parameter type number of defected pixels */
#define CAMDEV_DPCC_MODE_NUM                  6     /**< Maximum number of auto mode */
/*******************************************/

/******************************************************************************/
/**
 * @brief   VsCamDevice DPCC manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccManualConfig_s {
    bool_t bptEnable;                                 /**< Bad pixel table enable */
    uint16_t bptNum;                                  /**< Bad pixel table number */
    uint8_t bptOutMode;                              /**< Bad pixel table output mode */
    uint16_t bptPosX[CAMDEV_DPCC_DEFECT_PIXEL_NUM];   /**< Bad pixel table X position */
    uint16_t bptPosY[CAMDEV_DPCC_DEFECT_PIXEL_NUM];   /**< Bad pixel table Y position */
    uint8_t lineMadFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Line_mad method used by the RB channel\n */
                                                                                        /**< [1][0~2]Represents the three thresholds of Line_mad method used by the G channel */
    uint8_t lineThresh[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Line_thresh method used by the RB channel\n */
                                                                                        /**< [1][0~2]Represents the three thresholds of Line_thresh method used by the G channel */
    uint16_t methodsSet[CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0] Represents the switch whether the first set of thresholds of the five bad point determination methods are enabled\n */
                                                                /**< [1] Represents the switch whether the second set of thresholds of the five bad point determination methods are enabled\n */
                                                                /**< [2] Represents the switch whether the third set of thresholds of the five bad point determination methods are enabled */
    uint8_t outMode;  /**<Interpolation mode for correction unit. range:[0, 15] */
    uint8_t pgFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of peak Gradient check method used by the RB channel\n */
                                                                                   /**< [1][0~2]Represents the three thresholds of peak Gradient check method used by the G channel */
    uint8_t rgFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rank neighbor difference method used by the RB channel\n */
                                                                                   /**< [1][0~2]Represents the three thresholds of Rank neighbor difference method used by the G channel */
    uint8_t rndOffs[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three offsets of Rand Neighbor Difference method used by the RB channel\n */
                                                                                     /**< [1][0~2]Represents the three offsets of Rand Neighbor Difference method used by the G channel */
    uint8_t rndThresh[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the RB channel\n */
                                                                                       /**< [1][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the G channel */
    uint8_t roLimits[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rand Order method used by the RB channel\n */
                                                                                      /**< [1][0~2]Represents the three thresholds of Rand Order method used by the G channel */
    uint8_t setUse; /**< DPCC methods set usage for detection */
}CamDeviceDpccManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DPCC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccAutoConfig_s {
    uint8_t autoModeSelect;    /**< The auto config level */
} CamDeviceDpccAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DPCC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccConfig_s {
    CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceDpccManualConfig_t manualCfg;  /**< DPCC manual configuration*/
    CamDeviceDpccAutoConfig_t autoCfg;      /**< DPCC auto configuration*/
}CamDeviceDpccConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DPCC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccStatus_s {
    bool_t enable;                          /**< DPCC enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceDpccManualConfig_t currentCfg;  /**< DPCC current configuration*/
}CamDeviceDpccStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets DPCC configuration parameters.
 * @startuml VsiCamDeviceDpccSetConfig
 * !include E01_External/VsiCamDeviceDpccSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pDpccCfg    Pointer to DPCC Configuration
 * @details This function calls: \ref CamDeviceDpccManualSetConfig, \ref CamDeviceDpccAutoSetConfig,
 * \ref CamEngineDpccSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceDpccReset
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
RESULT VsiCamDeviceDpccSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpccConfig_t *pDpccCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets DPCC configuration parameters.
 * @startuml VsiCamDeviceDpccGetConfig
 * !include E01_External/VsiCamDeviceDpccGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pDpccCfg    Pointer to DPCC configuration.
 * @details This function calls: \ref CamEngineDpccGetMode, \ref CamDeviceDpccManualGetConfig,
 * \ref CamDeviceDpccAutoGetConfig
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
RESULT VsiCamDeviceDpccGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpccConfig_t *pDpccCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables DPCC.
 * @startuml VsiCamDeviceDpccEnable
 * !include E01_External/VsiCamDeviceDpccEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDpccEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DPCC.
 * @startuml VsiCamDeviceDpccDisable
 * !include E01_External/VsiCamDeviceDpccDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDpccDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DPCC status.
 * @startuml VsiCamDeviceDpccGetStatus
 * !include E01_External/VsiCamDeviceDpccGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to DPCC status.
 * @details This function calls: \ref CamEngineDpccGetStatus
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
RESULT VsiCamDeviceDpccGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpccStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DPCC.
 * @startuml VsiCamDeviceDpccReset
 * !include E01_External/VsiCamDeviceDpccReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceDpccSetConfig
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
RESULT VsiCamDeviceDpccReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DPCC version.
 * @startuml VsiCamDeviceDpccGetVersion
 * !include E01_External/VsiCamDeviceDpccGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to DPCC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 11_cam_device_dpcc */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_DPCC_API_H
