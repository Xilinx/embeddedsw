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

#ifndef CAMDEV_AF_API_H
#define CAMDEV_AF_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 25_cam_device_af VsCamDevice E01C25 Device_AF Definitions
 * @brief Provides interfaces for controlling the auto focus module working
 * in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_AF_WINDOWNUM     9    /**< The number of AF window */
#define CAMDEV_AF_FILTERNUM     5    /**< The number of AF filter */
#define CAMDEV_PD_FOCAL_NUM_MAX 48   /**< The number of AF focal PD */
#define CAMDEV_PD_ROI_INDEX_MAX 48   /**< Maximum index of PDAF ROI */
#define CAMDEV_AFMV3_BLOCK_NUM  225  /**< The number of AFM V3 blocks */
#define AF_DEVICE_RETENTION_VALUE_MAX      500

/*****************************************************************************/
/**
 * @brief   VsCamDevice AF status.
 *
 *****************************************************************************/
typedef enum CamDeviceAfCtrlState_s{
    CAMDEV_AF_STATE_INVALID       = 0,    /**< Invalid AF status */
    CAMDEV_AF_STATE_INITIALIZED   = 1,    /**< AF initialized */
    CAMDEV_AF_STATE_STOPPED       = 2,    /**< AF stopped */
    CAMDEV_AF_STATE_RUNNING       = 3,    /**< AF running */
    CAMDEV_AF_STATE_TRACKING      = 4,    /**< AF tracking */
    CAMDEV_AF_STATE_LOCKED        = 5,    /**< AF locked */
    CAMDEV_AF_STATE_MAX
} CamDeviceAfState_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice AF mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAfMode_e{
    CAMDEV_CDAF_INDIVIDUAL_MODE   = 0,    /**< CDAF focus */
    CAMDEV_PDAF_INDIVIDUAL_MODE   = 1,    /**< PDAF focus */
    CAMDEV_PDAF_CDAF_HYBRID_MODE  = 2,    /**< PDAF and CDAF hybrid */
    CAMDEV_AF_MODE_MAX
} CamDeviceAfMode_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice PDAF and CDAF hybrid focus parameters
 *
 *****************************************************************************/
typedef struct CamDeviceAfPcdfHybridConfig_s
{
    uint8_t defocusFrameNum;    /**< The number of defocus frames */
    uint8_t lossConfidenceFrameNum;    /**< The number of loss confidence frames */
    uint8_t accurateFocusStep;    /**< Accurate focus step */
    bool_t    accurateFocusEnable;    /**< Accurate focus enable */
} CamDeviceAfPcdfHybridConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice AF library configuration, which is required by the AF library calculation.
 *
 *****************************************************************************/
typedef struct CamDeviceAfConfig_s
{
    // CDAF params
    float32_t   weightWindow[CAMDEV_AF_WINDOWNUM];    /**< Weight window */
    float32_t   cStableTolerance;       /**< Range: [0, 1] */
    uint8_t     cPointsOfCurve;         /**< Range: [5, 100] */
    uint16_t    maxFocal;               /**< Maximum focal point */
    uint16_t    minFocal;               /**< Minimum focal point*/
    float32_t   cMotionThreshold;       /**< Motion threshold */
    uint8_t     uphillAllowance;        /**< The range of top hill when CDAF in searching  */
    uint8_t     downhillAllowance;      /**< The range of bottom hill when CDAF in searching */

    // PDAF params
    float32_t   cPdConfThreshold;       /**< PDAF configuration threshold */
    float32_t   pdShiftThreshold;       /**< Phase Detection shift stable threshold */
    uint8_t     pdStableCountMax;       /**< Maximum focusing to which PDAF locks */
    float32_t   pdafUnlockThreshold;    /**< PDAF unlock threshold */

    // PDAF and CDAF hybrid params
    CamDeviceAfPcdfHybridConfig_t pcdafHybridConfig;    /**< PDAF and CDAF hybrid configuration */
} CamDeviceAfConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice AF status.
 *
 *****************************************************************************/
typedef struct CamDeviceAfStatus_s {
    CamDeviceAfState_t state;  /**< AF state*/
} CamDeviceAfStatus_t;

/*****************************************************************************/
/**
 * @brief   This function registers the AF library.
 * @startuml VsiCamDeviceAfRegister
 * !include E01_External/VsiCamDeviceAfRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       pAfLibHandle  Pointer to the AF library handle.
 * @details This function calls: \ref CamEngineAfRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfRegister
(
    CamDeviceHandle_t  hCamDevice,
    void              *pAfLibHandle
);

/*****************************************************************************/
/**
 * @brief   This function unregisters the AF library.
 * @startuml VsiCamDeviceAfUnRegister
 * !include E01_External/VsiCamDeviceAfUnRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAfUnRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_BUSY            Operation failed due to system occupied
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfUnRegister
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets AF calculation mode parameters.
 * @startuml VsiCamDeviceAfSetMode
 * !include E01_External/VsiCamDeviceAfSetMode.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pMode       Pointer to the AF calculation mode.
 * @details This function calls: \ref CamEngineAfSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceAfReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAfMode_t   *pMode
);

/*****************************************************************************/
/**
 * @brief   This function gets AF calculation mode parameters.
 * @startuml VsiCamDeviceAfGetMode
 * !include E01_External/VsiCamDeviceAfGetMode.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pMode       Pointer to the AF calculation mode.
 * @details This function calls: \ref CamEngineAfGetMode
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetMode
(
    CamDeviceHandle_t    hCamDevice,
    CamDeviceAfMode_t    *pMode
);

/*****************************************************************************/
/**
 * @brief   This function sets AF calculation parameters.
 * @startuml VsiCamDeviceAfSetConfig
 * !include E01_External/VsiCamDeviceAfSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to the AF calculation parameters
 * @details This function calls: \ref CamEngineAfSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceAfReset
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
RESULT VsiCamDeviceAfSetConfig
(
    CamDeviceHandle_t        hCamDevice,
    const CamDeviceAfConfig_t      *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets AF calculation parameters.
 * @startuml VsiCamDeviceAfGetConfig
 * !include E01_External/VsiCamDeviceAfGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to the AF calculation configuration parameters
 * @details This function calls: \ref CamEngineAfGetConfig
 * @details This function is called by: User application
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
RESULT VsiCamDeviceAfGetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfConfig_t      *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables AF.
 * @startuml VsiCamDeviceAfEnable
 * !include E01_External/VsiCamDeviceAfEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAfStart
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AF.
 * @startuml VsiCamDeviceAfDisable
 * !include E01_External/VsiCamDeviceAfDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAfStop
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the AF status.
 * @startuml VsiCamDeviceAfGetStatus
 * !include E01_External/VsiCamDeviceAfGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to the AF status.
 * @details This function calls: \ref CamEngineAfGetStatus
 * @details This function is called by: User application
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
RESULT VsiCamDeviceAfGetStatus
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfStatus_t       *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets the AF version.
 * @startuml VsiCamDeviceAfGetVersion
 * !include E01_External/VsiCamDeviceAfGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to the AF version.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets AF.
 * @startuml VsiCamDeviceAfReset
 * !include E01_External/VsiCamDeviceAfReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceAfSetConfig, \ref VsiCamDeviceAfSetMode
 * @details This function is called by: User application
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
RESULT VsiCamDeviceAfReset
(
    CamDeviceHandle_t     hCamDevice
);

/** @} 25_cam_device_af */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_AF_API_H
