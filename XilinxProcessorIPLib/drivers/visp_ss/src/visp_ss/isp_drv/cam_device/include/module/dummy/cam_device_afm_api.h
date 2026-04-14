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

#ifndef CAMDEV_AFM_V1_API_H
#define CAMDEV_AFM_V1_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 01_cam_device_afm VsCamDevice E01C01_AFM Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Enumeration type to identify the AF measuring window.
 *
 *****************************************************************************/
typedef enum CamDeviceAfmWindowId_e
{
    CAMDEV_AFM_WINDOW_INVALID  = 0,    /**< Lower border (only for an internal evaluation) */
    CAMDEV_AFM_WINDOW_A        = 1,    /**< Window A (1st window) */
    CAMDEV_AFM_WINDOW_B        = 2,    /**< Window B (2nd window) */
    CAMDEV_AFM_WINDOW_C        = 3,    /**< Window C (3rd window) */
    CAMDEV_AFM_WINDOW_MAX,             /**< Upper border (only for an internal evaluation) */
} CamDeviceAfmWindowId_t;


/*****************************************************************************/
/**
 * @brief   VsCamDevice AFM measure results structure.
 *
*****************************************************************************/
typedef struct CamDeviceAfmMeasureResult_s
{
    uint32_t    sharpnessA;         /**< Sharpness of window A */
    uint32_t    sharpnessB;         /**< Sharpness of window B */
    uint32_t    sharpnessC;         /**< Sharpness of window C */

    uint32_t    luminanceA;         /**< Luminance of window A */
    uint32_t    luminanceB;         /**< Luminance of window B */
    uint32_t    luminanceC;         /**< Luminance of window C */

    uint32_t    pixelCntA;         /**< Pixel counts of window A */
    uint32_t    pixelCntB;         /**< Pixel counts of window B */
    uint32_t    pixelCntC;         /**< Pixel counts of window C */
} CamDeviceAfmMeasureResult_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AFM window configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAfmWindowConfig_s
{
    CamDeviceWindow_t  window;    /**< AFM window */
    CamDeviceAfmWindowId_t id;     /**< AFM window index */
} CamDeviceAfmWindowConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice AFM status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceAfmStatus_s {
    bool_t enable;    /**< AFM enable status */
} CamDeviceAfmStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets the threshold in the AFM module.
 * @startuml VsiCamDeviceAfmSetThreshold
 * !include E01_External/VsiCamDeviceAfmSetThreshold.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       threshold   Threshold value.
 * @details this function calls: CamEngineAfmSetThreshold
 * @details this function is called by: User application, VsiCamDeviceAfmReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmSetThreshold
(
    CamDeviceHandle_t  hCamDevice,
    const uint32_t     threshold

);

/*****************************************************************************/
/**
 * @brief   This function gets the threshold in the AFM module.
 * @startuml VsiCamDeviceAfmGetThreshold
 * !include E01_External/VsiCamDeviceAfmGetThreshold.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pThreshold  Threshold value pointer.
 * @details this function calls: CamEngineAfmGetThreshold
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetThreshold
(
    CamDeviceHandle_t  hCamDevice,
    uint32_t          *pThreshold
);

/*****************************************************************************/
/**
 * @brief   This function gets the AFM statistic result.
 * @startuml VsiCamDeviceAfmGetStatistics
 * !include E01_External/VsiCamDeviceAfmGetStatistics.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pResult     Measure results pointer.
 * @details this function calls: CamEngineAfmGetStatistic
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetStatistics
(
    CamDeviceHandle_t              hCamDevice,
    CamDeviceAfmMeasureResult_t   *pResult
);

/*****************************************************************************/
/**
 * @brief   This function sets the AFM statistics window.
 * @startuml VsiCamDeviceAfmSetMeasureWindow
 * !include E01_External/VsiCamDeviceAfmSetMeasureWindow.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pWindow     Pointer to window configuration.
 * @details this function calls: CamEngineAfmSetMeasureWindow, VsiCamDeviceAfmReset
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmSetMeasureWindow
(
    CamDeviceHandle_t            hCamDevice,
    CamDeviceAfmWindowConfig_t  *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets the AFM statistics window.
 * @startuml VsiCamDeviceAfmGetMeasureWindow
 * !include E01_External/VsiCamDeviceAfmGetMeasureWindow.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pWindow     Pointer to window configuration.
 * @details this function calls: CamEngineAfmGetMeasureWindow
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetMeasureWindow
(
    CamDeviceHandle_t                 hCamDevice,
    CamDeviceAfmWindowConfig_t       *pWindow
);


/*****************************************************************************/
/**
 * @brief   This function enables AFM.
 * @startuml VsiCamDeviceAfmEnable
 * !include E01_External/VsiCamDeviceAfmEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngineAfmEnable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AFM.
 * @startuml VsiCamDeviceAfmDisable
 * !include E01_External/VsiCamDeviceAfmDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngineAfmDisable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets AFM status.
 * @startuml VsiCamDeviceAfmGetStatus
 * !include E01_External/VsiCamDeviceAfmGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to the AFM status.
 * @details this function calls: CamEngineAfmIsEnable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetStatus
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfmStatus_t     *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets the AFM version.
 * @startuml VsiCamDeviceAfmGetVersion
 * !include E01_External/VsiCamDeviceAfmGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to the AFM version.
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets AFM.
 * @startuml VsiCamDeviceAfmReset
 * !include E01_External/VsiCamDeviceAfmReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamDeviceSetIspLowPower, VsiCamDeviceAfmSetThreshold,
 * VsiCamDeviceAfmSetMeasureWindow
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmReset
(
    CamDeviceHandle_t             hCamDevice
);

/** @} 01_cam_device_afm */

#ifdef __cplusplus
}
#endif

#endif /*CAMDEV_AFM_V1_API_H*/
