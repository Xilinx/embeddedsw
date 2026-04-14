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

#ifndef CAMDEV_CCM_V1_1_API_H
#define CAMDEV_CCM_V1_1_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 03_cam_device_ccm VsCamDevice E01C03 Device_CCM Definitions
 * @brief Provides interfaces for controlling the color correction matrix
 * module working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_CC_MATRIX_SIZE       9U      /**< Color correction matrix size 3x3*/
#define CAMDEV_CC_COLOR_CHANNEL_NUM 3U      /**< Color channel number 3*/

/******************************************************************************/
/**
 * @brief   VsCamDevice CCM manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmAutoConfig_s {
    uint8_t   autoLevel;                              /**< Auto configuration level > */
    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];     /**< CCM gains > */
    float32_t damping;                                /**< damping function >*/
    float32_t strength[CAMDEV_ISO_STRENGTH_NUM];  /**< CCM strength >*/
} CamDeviceCcmAutoConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice CCM manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmManualConfig_s {
    float32_t ccMatrix[CAMDEV_CC_MATRIX_SIZE];          /**< Color correction matrix coefficient*/
    float32_t ccOffset[CAMDEV_CC_COLOR_CHANNEL_NUM];    /**< Color offset coefficient*/
}CamDeviceCcmManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CCM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmConfig_s {
    CamDeviceCcmAutoConfig_t   autoCfg;     /**< CCM auto configuration*/
    CamDeviceCcmManualConfig_t manualCfg;   /**< CCM manual configuration*/
    CamDeviceConfigMode_t      configMode;
}CamDeviceCcmConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice CCM status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmStatus_s {
    bool_t enable;              /**< CCM enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceCcmManualConfig_t currentCfg;   /**< CCM current configuration*/
}CamDeviceCcmStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets CCM configuration parameters.
 * @startuml VsiCamDeviceCcmSetConfig
 * !include E01_External/VsiCamDeviceCcmSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pCcmCfg     Pointer to CCM Configuration
 * @details This function calls: \ref CamDeviceCcmManualSetConfig, \ref CamDeviceCcmAutoSetConfig,
 * \ref CamEngineCcmSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceCcmReset
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
RESULT VsiCamDeviceCcmSetConfig
(
    CamDeviceHandle_t     hCamDevice,
    const CamDeviceCcmConfig_t *pCcmCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CCM configuration parameters.
 * @startuml VsiCamDeviceCcmGetConfig
 * !include E01_External/VsiCamDeviceCcmGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pCcmCfg     Pointer to CCM configuration.
 * @details This function calls: \ref CamDeviceCcmManualGetConfig, \ref CamDeviceCcmAutoGetConfig,
 * \ref CamEngineCcmGetMode
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
RESULT VsiCamDeviceCcmGetConfig
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceCcmConfig_t *pCcmCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables CCM.
 * @startuml VsiCamDeviceCcmEnable
 * !include E01_External/VsiCamDeviceCcmEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineCcmEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CCM.
 * @startuml VsiCamDeviceCcmDisable
 * !include E01_External/VsiCamDeviceCcmDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineCcmDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CCM status.
 * @startuml VsiCamDeviceCcmGetStatus
 * !include E01_External/VsiCamDeviceCcmGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to CCM status.
 * @details This function calls: \ref CamEngineCcmGetStatus
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
RESULT VsiCamDeviceCcmGetStatus
(
	CamDeviceHandle_t     hCamDevice,
	CamDeviceCcmStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CCM.
 * @startuml VsiCamDeviceCcmReset
 * !include E01_External/VsiCamDeviceCcmReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamCalibDbGetCcProfileByName, \ref VsiCamDeviceCcmSetConfig
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
RESULT VsiCamDeviceCcmReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CCM version.
 * @startuml VsiCamDeviceCcmGetVersion
 * !include E01_External/VsiCamDeviceCcmGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to CCM version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t         *pVersion
);

/** @} 03_cam_device_ccm */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_CCM_V1_1_API_H
