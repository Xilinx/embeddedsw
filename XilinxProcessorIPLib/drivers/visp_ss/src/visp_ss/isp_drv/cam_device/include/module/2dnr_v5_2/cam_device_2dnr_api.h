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

#ifndef CAMDEV_2DNR_API_H
#define CAMDEV_2DNR_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 08_cam_device_2dnr VsCamDevice E01C08 Device_2DNR Definitions
 * @brief Provides interfaces for controlling the 2D noise reduction module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_DNR_LUMA_CURVE_SIZE 12   //bin number of dnr_luma_curve

/******************************************************************************/
/**
 * @brief   VsCamDevice 2DNR curve configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrCurveConfig_s
{
    uint16_t lumaCurveX[CAMDEV_DNR_LUMA_CURVE_SIZE];    /**< Luma curve on X axis */
    uint16_t lumaCurveY[CAMDEV_DNR_LUMA_CURVE_SIZE];    /**< Luma curve on Y axis */
} CamDevice2DnrCurveConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 2DNR current configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrManualConfig_s
{

    uint8_t pregammaStrength;   /**< Pregamma strength value*/
    uint8_t strength;   /**< Strength value*/
    float32_t sigma;    /**< Sigma value */
    uint8_t sigmaRange;    /**< Sigma range value */
    uint16_t sigmaOffset;   /**< Sigma square */
    CamDevice2DnrCurveConfig_t curveCfg;
} CamDevice2DnrManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 2DNR auto configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrAutoConfig_s
{
    uint8_t autoLevel;  /**< The auto level */
    float32_t gain[CAMDEV_ISO_STRENGTH_NUM];    /**< 2DNR gain */
    float32_t sigma[CAMDEV_ISO_STRENGTH_NUM];   /**< 2DNR sigma */
    uint8_t strength[CAMDEV_ISO_STRENGTH_NUM];  /**< 2DNR strength */
    uint8_t pregammaStrength[CAMDEV_ISO_STRENGTH_NUM];  /**< 2DNR pregamma strength */
    uint16_t lumaCurveX[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_DNR_LUMA_CURVE_SIZE];   /**< 2DNR luma curve on X axis */
    uint16_t lumaCurveY[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_DNR_LUMA_CURVE_SIZE];   /**< 2DNR luma curve on Y axis */
} CamDevice2DnrAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 2DNR configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrConfig_s {
    CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
    CamDevice2DnrAutoConfig_t autoCfg;      /**< 2DNR auto configuration*/
    CamDevice2DnrManualConfig_t manualCfg;  /**< 2DNR manual configuration*/
}CamDevice2DnrConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 2DNR status structure.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrStatus_s
{
    bool_t     enable;  /**< 2DNR enable status */
    bool_t     lumaEnable;  /**< Luma enable status */
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDevice2DnrManualConfig_t currentCfg;  /**< 2DNR current configuration*/
} CamDevice2DnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function enables 2DNR.
 * @startuml VsiCamDevice2DnrEnable
 * !include E01_External/VsiCamDevice2DnrEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngine2DnrEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 2DNR.
 * @startuml VsiCamDevice2DnrDisable
 * !include E01_External/VsiCamDevice2DnrDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngine2DnrDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets 2DNR configuration parameters.
 * @startuml VsiCamDevice2DnrSetConfig
 * !include E01_External/VsiCamDevice2DnrSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to 2DNR Configuration
 * @details This function calls: \ref CamDevice2DnrManualSetConfig,
 * \ref CamDevice2DnrAutoSetConfig, \ref CamEngine2DnrSetMode
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDevice2DnrConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR configuration.
 * @startuml VsiCamDevice2DnrGetConfig
 * !include E01_External/VsiCamDevice2DnrGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to 2DNR configuration.
 * @details This function calls: \ref CamEngine2DnrGetMode,
 * \ref CamDevice2DnrManualGetConfig, \ref CamDevice2DnrAutoGetConfig
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
RESULT VsiCamDevice2DnrGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDevice2DnrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables 2DNR luma curve.
 * @startuml VsiCamDevice2DnrLumaEnable
 * !include E01_External/VsiCamDevice2DnrLumaEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngine2DnrLumaEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrLumaEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 2DNR luma curve.
 * @startuml VsiCamDevice2DnrLumaDisable
 * !include E01_External/VsiCamDevice2DnrLumaDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngine2DnrLumaDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrLumaDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR status.
 * @startuml VsiCamDevice2DnrGetStatus
 * !include E01_External/VsiCamDevice2DnrGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to 2DNR status
 * @details This function calls: \ref CamEngine2DnrGetStatus
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
RESULT VsiCamDevice2DnrGetStatus
(
    CamDeviceHandle_t      hCamDevice,
    CamDevice2DnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR version.
 * @startuml VsiCamDevice2DnrGetVersion
 * !include E01_External/VsiCamDevice2DnrGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to 2DNR version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets 2DNR.
 * @startuml VsiCamDevice2DnrReset
 * !include E01_External/VsiCamDevice2DnrReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDevice2DnrSetConfig
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
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrReset
(
    CamDeviceHandle_t             hCamDevice
);

/** @} 08_cam_device_2dnr */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_2DNR_API_H */
