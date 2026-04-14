// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2023 Vivante Corporation
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

#ifndef CAMDEV_TPG_API_H
#define CAMDEV_TPG_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 10_cam_device_tpg VsCamDevice E01C10 Device_TPG Definitions
 * @brief Provides interfaces for controlling the test pattern generator module
 * working in the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Enumeration of TPG picture type.
 *
 *****************************************************************************/
typedef enum CamDeviceISPTpgImageType_e
{
    CAMDEV_ISP_TPG_IMAGE_TYPE_3X3COLORBLOCK    = 0,    /**< 3 x 3 color block */
    CAMDEV_ISP_TPG_IMAGE_TYPE_COLORBAR         = 1,    /**< Color bar */
    CAMDEV_ISP_TPG_IMAGE_TYPE_GRAYBAR          = 2,    /**< Gray bar */
    CAMDEV_ISP_TPG_IMAGE_TYPE_GRID             = 3,    /**< Highlighted grid */
    CAMDEV_ISP_TPG_IMAGE_TYPE_RANDOM           = 4,    /**< Random data */
    CAMDEV_ISP_TPG_IMAGE_TYPE_MAX
} CamDeviceISPTpgImageType_t;

/******************************************************************************/
/**
* @brief   Enumeration of TPG resolution.
*
*****************************************************************************/
typedef enum CamDeviceIspTpgResolution_e
{
    CAMDEV_ISP_TPG_1080P           = 0,        /**< 1920 x 1080 resolution */
    CAMDEV_ISP_TPG_720P            = 1,        /**< 1280 x 720 resolution */
    CAMDEV_ISP_TPG_4K              = 2,        /**< 3840 x 2160 resolution */
} CamDeviceIspTpgResolution_t;

 /******************************************************************************/
/**
 * @brief   VsCamDevice TPG configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceTpgConfig_s
{
    CamDeviceISPTpgImageType_t        imageType;    /**< Image type */
    CamDeviceRawPattern_t             bayerPattern;  /**< Bayer pattern */
    CamDeviceBitDepth_t               colorDepth;    /**< Color depth */
    CamDeviceIspTpgResolution_t       resolution;    /**< TPG resolution */

    uint16_t                        pixleGap;    /**< The width of sub-picture */
    uint16_t                        lineGap;    /**< The height of sub-picture */
    uint16_t                        gapStandard;    /**< The gap of sub-picture */
    uint32_t                        randomSeed;    /**< Random seed */
    uint32_t                        frameNum;    /**< The number of frames */
} CamDeviceTpgConfig_t;

/*****************************************************************************/
/**
 * @brief   This function sets TPG configuration.
 * @startuml VsiCamDeviceTpgSetConfig
 * !include E01_External/VsiCamDeviceTpgSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pTpgCfg     Pointer to TPG configuration.
 * @details This function calls: \ref CamEngineTpgConfig, \ref CamDeviceConfigEngineAcq
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
RESULT VsiCamDeviceTpgSetConfig
(
    CamDeviceHandle_t hCamDevice,
    const CamDeviceTpgConfig_t *pTpgCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables TPG.
 * @startuml VsiCamDeviceTpgEnable
 * !include E01_External/VsiCamDeviceTpgEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEnableTpg
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables TPG.
 * @startuml VsiCamDeviceTpgDisable
 * !include E01_External/VsiCamDeviceTpgDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDisableTpg
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets TPG version.
 * @startuml VsiCamDeviceTpgGetVersion
 * !include E01_External/VsiCamDeviceTpgGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to TPG version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 10_cam_device_tpg */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_TPG_API_H
