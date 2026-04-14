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

#ifndef CAMDEV_LSC_V3_API_H
#define CAMDEV_LSC_V3_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 16_cam_device_lsc VsCamDevice E01C16 Device_LSC Definitions
 * @brief Provides interfaces for controlling the lens shading correction module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_LSC_H_BLOCK_NUM 32U  /**< Maximum horizonta block size of LSC map */
#define CAMDEV_LSC_V_BLOCK_NUM 32U  /**< Maximum vertical block size of LSC map */
#define CAMDEV_LSC_H_POINT_NUM (CAMDEV_LSC_H_BLOCK_NUM + 1U)  /**< Maximum horizontal size of LSC map */
#define CAMDEV_LSC_V_POINT_NUM (CAMDEV_LSC_H_BLOCK_NUM + 1U)  /**< Maximum vertical size of LSC map */

/******************************************************************************/
/**
 * @brief   VsCamDevice LSC internal mode.
 *
 *****************************************************************************/
typedef enum CamDeviceLscInterMode_e
{
    CAMDEV_LSC_GAIN_INTER                    = 0,    /**< LSC internal gain */
    CAMDEV_LSC_COLOR_TEMP_WEIGHT_INTER       = 1,    /**< LSC internal temperature weight */
    CAMDEV_LSC_GAIN_COLOR_TEMP_WEIGHT_INTER  = 2,    /**< LSC internal color temperature weight gain */
    CAMDEV_LSC_INTER_MAX
} CamDeviceLscInterMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice LSC V3 current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscManualConfig_s {
    uint16_t matrix[CAMDEV_RAW_CHANNEL_NUM][CAMDEV_LSC_V_POINT_NUM][CAMDEV_LSC_H_POINT_NUM];  /**< 4 channel(r, gr, gb, b) 33*33 grids */
    uint16_t xSize[CAMDEV_LSC_H_BLOCK_NUM];                                                   /**< LSC V3 int array for x axis size */
    uint16_t ySize[CAMDEV_LSC_V_BLOCK_NUM/2U];                                                   /**< LSC V3 int array for y axis size*/
}CamDeviceLscManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice LSC V3 auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscAutoConfig_s
{
    uint8_t autoLevel;    /**< Auto configuration level */
    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];    /**< LSC gains */
    float32_t damping;    /**< LSC damping value */
    float32_t strength[CAMDEV_ISO_STRENGTH_NUM];    /**< LSC strength */
    CamDeviceLscInterMode_t interMode;    /**< LSC internal mode */
} CamDeviceLscAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice LSC V3 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscConfig_s {
    CamDeviceConfigMode_t configMode;        /**< The run mode: 0--manual, 1--auto */
    CamDeviceLscManualConfig_t manualCfg;    /**< LSC V3 manual configuration */
    CamDeviceLscAutoConfig_t   autoCfg;      /**< LSC V3 auto configuration */
}CamDeviceLscConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice LSC V3 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceLscStatus_s {
    bool_t enable;                          /**< LSC V3 enable status */
    CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
    CamDeviceLscManualConfig_t currentCfg;    /**< LSC V3 current configuration */
}CamDeviceLscStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets LSC configuration parameters.
 * @startuml VsiCamDeviceLscSetConfig
 * !include E01_External/VsiCamDeviceLscSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pLscCfg     Pointer to LSC Configuration
 * @details This function calls: \ref CamDeviceLscManualSetConfig, \ref CamDeviceLscAutoSetConfig,
 * \ref CamEngineLscSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceLscReset
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
RESULT VsiCamDeviceLscSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscConfig_t *pLscCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets LSC configuration parameters.
 * @startuml VsiCamDeviceLscGetConfig
 * !include E01_External/VsiCamDeviceLscGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pLscCfg     Pointer to LSC configuration.
 * @details This function calls: \ref CamEngineLscGetMode, \ref CamDeviceLscManualGetConfig,
 * \ref CamDeviceLscAutoGetConfig
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
RESULT VsiCamDeviceLscGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscConfig_t *pLscCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables LSC.
 * @startuml VsiCamDeviceLscEnable
 * !include E01_External/VsiCamDeviceLscEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineLscEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables LSC.
 * @startuml VsiCamDeviceLscDisable
 * !include E01_External/VsiCamDeviceLscDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineLscDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets LSC.
 * @startuml VsiCamDeviceLscReset
 * !include E01_External/VsiCamDeviceLscReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamCalibDbGetLscProfileByName, \ref VsiCamDeviceLscSetConfig
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
RESULT VsiCamDeviceLscReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets LSC status.
 * @startuml VsiCamDeviceLscGetStatus
 * !include E01_External/VsiCamDeviceLscGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to LSC status.
 * @details This function calls: \ref CamEngineLscGetStatus
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
RESULT VsiCamDeviceLscGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets LSC version.
 * @startuml VsiCamDeviceLscGetVersion
 * !include E01_External/VsiCamDeviceLscGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to LSC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 16_cam_device_lsc */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_LSC_V3_API_H
