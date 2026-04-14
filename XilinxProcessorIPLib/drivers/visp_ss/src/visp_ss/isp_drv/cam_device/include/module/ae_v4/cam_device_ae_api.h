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

#ifndef CAMDEV_AE_API_H
#define CAMDEV_AE_API_H

#include "cam_device_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 24_cam_device_ae VsCamDevice E01C24 Device_AE Definitions
 * @brief Provides interfaces for controlling the auto exposure module working
 * in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_AE_HIST_NUM_BINS           256U       /**< Bin numbers of AE histogram */
#define CAMDEV_AE_EXP_GRID_ITEMS          (32*32)   /**< Number of grid items (see @ref CamerIcMeanLuma_t) */
#define CAMDEV_AE_EXP_TABLE_NUM           8U        /**< The number of AEV4 exposure tables */
#define AE_DEVICE_RETENTION_VALUE_MAX      500

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE status.
 *
 *****************************************************************************/
typedef enum CamDeviceAeState_e {
    CAMDEV_AE_STATE_INVALID       = 0,    /**< Invalid AE status */
    CAMDEV_AE_STATE_INITIALIZED   = 1,    /**< AE initialized */
    CAMDEV_AE_STATE_STOPPED       = 2,    /**< AE stopped */
    CAMDEV_AE_STATE_RUNNING       = 3,    /**< AE running */
    CAMDEV_AE_STATE_LOCKED        = 4,    /**< AE locked */
    CAMDEV_AE_STATE_MAX
} CamDeviceAeState_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice AE scene evaluation mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAeSemMode_t
{
    CAMDEV_AE_SCENE_EVALUATION_FIX        = 0,    /**< Fixed evaluation */
    CAMDEV_AE_SCENE_EVALUATION_ADAPTIVE   = 1,    /**< Adaptive evaluation */
    CAMDEV_AE_SCENE_METADATA              = 2,    /**< metadata mode */
    CAMDEV_AE_SCENE_EVALUATION_MAX
} CamDeviceAeSemMode_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE Flicker period.
 *
 *****************************************************************************/
typedef enum CamDeviceAeFlickerPeriod_e {
    CAMDEV_AE_FLICKER_PERIOD_OFF          = 0,    /**< Flicker period off */
    CAMDEV_AE_FLICKER_PERIOD_50Hz         = 1,    /**< Flicker period 50Hz */
    CAMDEV_AE_FLICKER_PERIOD_60Hz         = 2,    /**< Flicker period 60Hz */
    CAMDEV_AE_FLICKER_PERIOD_USER_DEFINED = 3,  /**< User-defined Flicker period */
    CAMDEV_AE_FLICKER_PERIOD_MAX
} CamDeviceAeFlickerPeriod_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE front ground type.
 *
 *****************************************************************************/
typedef enum CamDeviceAeFrontGroundType_s {
    CAMDEV_AE_FRONT_MODE            = 0,    /**< AE front mode */
    CAMDEV_AE_FACE_MODE             = 1,    /**< AE face mode */
    CAMDEV_AE_TOUCH_MODE            = 2,    /**< AE touch mode */
    CAMDEV_AE_FRONT_MODE_MAX
} CamDeviceAeFrontGroundType_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE performance optimization mode.
 *
 *****************************************************************************/
typedef enum CamDeviceAePerformanceOptiMode_s
{
    CAMDEV_AE_PERFORMANCE_NO_OPTIMIZATION         = 0,    /**< No optimization mode */
    CAMDEV_AE_PERFORMANCE_GENERAL_OPTIMIZATION    = 1,    /**< General optimization mode */
    CAMDEV_AE_PERFORMANCE_FAST_OPTIMIZATION       = 2,    /**< Fast optimization mode */
} CamDeviceAePerformanceOptiMode_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice AE Flicker Mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAeFlickerMode_s{
    CamDeviceAeFlickerPeriod_t flickerPeriod;    /**< Flicker period instance */
    float32_t userDefinedPeriodus; /**< User defined period which is only valid in user define mode. */
}CamDeviceAeFlickerMode_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE external light type.
 *
 *****************************************************************************/
typedef enum CamDeviceAeExLightType_e
{
    CAMDEV_AE_IR_LIGHT       = 0,    /**< IR lamp */
    CAMDEV_AE_MAX            = 1,
} CamDeviceAeExLightType_t;


/*****************************************************************************/
/**
 *          CamEngineAeStrategyMode_t
 *
 * @brief   AE highlighting strategy
 *
 */
 /*****************************************************************************/
typedef enum CamDeviceAeStrategyMode_e {
    CAMDEV_AE_STRATEGY_OFF               = 0,
    CAMDEV_AE_STRATEGY_HIGHLIGHT_PRIOR   = 1,
    CAMDEV_AE_STRATEGY_LOWLIGHT_PRIOR    = 2,
    CAMDEV_AE_STRATEGY_MAX
} CamDeviceAeStrategyMode_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice AE configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAeConfig_s {
    float32_t setPoint;             /**< Expected mean luma. */
    float32_t tolerance;            /**< Tolerance of stable range. */
    float32_t dampOver;             /**< Clip edge to start interpolate K. */
    float32_t dampUnder;            /**< Clip edge to start interpolate K. */

    float32_t dampUnderRatio;               /**< Clip edge to start under interpolate K. */
    float32_t dampUnderGain;                /**< Under interpolate gain of K, damp = damp^K. */
    float32_t dampOverRatio;                /**< Clip edge to start over interpolate K. */
    float32_t dampOverGain;                 /**< Over interpolate gain of K, damp = damp^K. */

    float32_t motionFilter;                 /**< Temporal average filter, resulting in a smooth exposure change. */
    float32_t motionThreshold;              /**< Motion filter threshold . */
    float32_t targetFilter;                 /**< Temporal average filter, resulting in a smoothadaptive AE target change. */

    float32_t lowlightLinearRepress[8];     /**< Repress the sensor exposure in lowlight scene and interpolate by sensor gain;
                                             Interpolate as Y axis. */
    float32_t lowlightLinearGain[8];        /**< Gain array; Interpolate as X axis. */
    uint32_t lowlightLinearLevel;       /**< Gain array number. */
    float32_t lowlightHdrRepress[8];        /**< Repress the sensor exposure in a lowlight scene and interpolate by sensor gain;
                                             Interpolate as Y axis. */
    float32_t lowlightHdrGain[8];           /**< Gain array; Interpolate as X axis. */
    uint32_t lowlightHdrLevel;          /**< Gain array number. */

    float32_t wdrContrastMin;               /**< Minimumdiff of (background - foreground), map to 0 to 1 as backlight ratio. */
    float32_t wdrContrastMax;               /**< Maximumdiff of (background - foreground), map to 0 to 1 as backlight ratio. */

    float32_t expv2WindowWeight[CAMDEV_AE_EXP_GRID_ITEMS]; /** The weight of EXPV2 every block */
    bool_t frameCalEnable;             /** Frame calculating mode selection */
    bool_t expDecomposeCustom;    /** Exposure decompose custom */
    CamDeviceAePerformanceOptiMode_t  performanceOptiMode;  /** Performance optimization mode */
    bool_t                            exLightEnable;       /** user api for controlling whether to enable enternal light source*/
    CamDeviceAeExLightType_t          exLightType;         /** the type of external light*/
    float32_t                         maxExpForExlight;    /** the total exposure threshold for turning on/off the external light*/
    float32_t                         minExpForExlight;
    CamDeviceAeStrategyMode_t         aeStrategyMode;
    float32_t                         highLightLumathr;
    float32_t                         histRatioSlope;
    float32_t                         maxHistOffset;
    float32_t                         lowLightLumathr;
    float32_t                         lumindexCalib;
} CamDeviceAeConfig_t;


/*****************************************************************************/
/**
 * @brief   VsCamDevice AE mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAeMode_s{
    CamDeviceAeSemMode_t     semMode;    /**< AE scene evaluation mode */
    CamDeviceAeFlickerMode_t antiFlickerMode;    /**< AE flicker mode */
} CamDeviceAeMode_t;


/*****************************************************************************/
/**
 * @brief   Cam Device AE result
 *
 *****************************************************************************/
typedef struct CamDeviceAeResult_s {
    float32_t    meanluma;    /**< AE mean luminance */
    float32_t    lumaIndex;
} CamDeviceAeResult_t;


/*****************************************************************************/
/**
 * @brief    Cam Device AE histogram grid.
 *
 *****************************************************************************/
typedef uint32_t CamDeviceAeHistBins_t[CAMDEV_AE_HIST_NUM_BINS];


/*****************************************************************************/
/**
 * @brief    VsCamDevice AE luminance grid.
 *
 *****************************************************************************/
typedef uint8_t CamDeviceAeMeanLuma_t[CAMDEV_AE_EXP_GRID_ITEMS];

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE object region grid.
 *
 *****************************************************************************/
typedef uint8_t CamDeviceAeObjectRegion_t[CAMDEV_AE_EXP_GRID_ITEMS];

/*****************************************************************************/
/**
 * @brief    VsCamDevice exposure statistics table.
 *
 *****************************************************************************/
typedef struct CamDeviceEsTable_s {
    float32_t exposureTime;    /**< AE exposure time: unit(us)*/
    float32_t aGain;    /**< AE simulated again */
    float32_t dGain;    /**< AE digital gain */
    float32_t ispGain;    /**< AE isp gain */
} CamDeviceEsTable_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice exposure table.
 *
 *****************************************************************************/
typedef struct CamDeviceExpTable_s {
    CamDeviceEsTable_t  expTable[CAMDEV_AE_EXP_TABLE_NUM];     /**< Exposure table */
    uint8_t             expTableNum;    /**< The number of exposure table */
} CamDeviceExpTable_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AE front ground configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAeFrontGroundConfig_s {
    CamDeviceAeFrontGroundType_t  aeFrontGroundType;    /**< AE front ground type */
    float32_t                         aeFaceWeight;     /**< AE face weight */
    float32_t                         aeTouchWeight;    /**< AE touch wieght */
} CamDeviceAeFrontGroundConfig_t;

/*****************************************************************************/
/**
 * @brief   This function registers the AE library.
 * @startuml VsiCamDeviceRegisterAeLib
 * !include E01_External/VsiCamDeviceRegisterAeLib.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       pAeLibHandle  Pointer to the AE library.
 * @details This function calls: \ref CamEngineAeRegister
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
RESULT VsiCamDeviceRegisterAeLib
(
    CamDeviceHandle_t   hCamDevice,
    void                *pAeLibHandle
);

/*****************************************************************************/
/**
 * @brief   This function unregisters the AE library.
 * @startuml VsiCamDeviceUnRegisterAeLib
 * !include E01_External/VsiCamDeviceUnRegisterAeLib.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAeUnRegister
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
RESULT VsiCamDeviceUnRegisterAeLib
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE configuration parameters.
 * @startuml VsiCamDeviceAeSetConfig
 * !include E01_External/VsiCamDeviceAeSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to the AE configuration.
 * @details This function calls: \ref CamEngineAeSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceAeReset
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
RESULT VsiCamDeviceAeSetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceAeConfig_t       *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE configuration parameters.
 * @startuml VsiCamDeviceAeGetConfig
 * !include E01_External/VsiCamDeviceAeGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to the AE configuration.
 * @details This function calls: \ref CamEngineAeGetConfig
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
RESULT VsiCamDeviceAeGetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceAeConfig_t      *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE mode parameters.
 * @startuml VsiCamDeviceAeSetMode
 * !include E01_External/VsiCamDeviceAeSetMode.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pAeMode     Pointer to the AE mode.
 * @details This function calls: \ref CamEngineAeSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceAeReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeSetMode
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMode_t       *pAeMode
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE mode parameters.
 * @startuml VsiCamDeviceAeGetMode
 * !include E01_External/VsiCamDeviceAeGetMode.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pAeMode     Pointer to the AE mode.
 * @details This function calls: \ref CamEngineAeGetMode
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
RESULT VsiCamDeviceAeGetMode
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMode_t      *pAeMode
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE ROI.
 * @startuml VsiCamDeviceAeSetRoi
 * !include E01_External/VsiCamDeviceAeSetRoi.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pRoi  Pointer to the AE ROI configuration.
 * @details This function calls: \ref CamEngineAeSetRoi
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
RESULT VsiCamDeviceAeSetRoi
(
    CamDeviceHandle_t      hCamDevice,
    const CamDeviceRoi_t         *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE ROI.
 * @startuml VsiCamDeviceAeGetRoi
 * !include E01_External/VsiCamDeviceAeGetRoi.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pRoi        Pointer to the AE ROI configuration.
 * @details This function calls: \ref CamEngineAeGetRoi
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
RESULT VsiCamDeviceAeGetRoi
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceRoi_t         *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets AE result.
 * @startuml VsiCamDeviceAeGetResult
 * !include E01_External/VsiCamDeviceAeGetResult.plantuml
 * @enduml
 * @param[in]       hCamDevice            Handle to the VsCamDevice instance.
 * @param[inout]    pCamDeviceAeResult    Pointer to AE front ground configuration.
 * @details This function calls: \ref CamEngineAeGetResult
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
RESULT VsiCamDeviceAeGetResult
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAeResult_t *pCamDeviceAeResult
);

/*****************************************************************************/
/**
 * @brief   This function enables AE.
 * @startuml VsiCamDeviceAeEnable
 * !include E01_External/VsiCamDeviceAeEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAeStart
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AE.
 * @startuml VsiCamDeviceAeDisable
 * !include E01_External/VsiCamDeviceAeDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAeStop
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets AE.
 * @startuml VsiCamDeviceAeReset
 * !include E01_External/VsiCamDeviceAeReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceAeSetConfig, \ref VsiCamDeviceAeSetExpTable,
 * \ref VsiCamDeviceAeSetMode
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
RESULT VsiCamDeviceAeReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE status.
 * @startuml VsiCamDeviceAeGetStatus
 * !include E01_External/VsiCamDeviceAeGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pAeStatus   Pointer to the AE status.
 * @details This function calls: \ref CamEngineAeGetStatus
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
RESULT VsiCamDeviceAeGetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeState_t      *pAeStatus
);

/*****************************************************************************/
/**
 * @brief   This function sets the exposure table to engine layer.
 * @startuml VsiCamDeviceAeSetExpTable
 * !include E01_External/VsiCamDeviceAeSetExpTable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pExpTable   Pointer to the exposure table.
 * @details This function calls: \ref CamEnginSetExpTable
 * @details This function is called by: User application, \ref VsiCamDeviceAeReset
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
RESULT VsiCamDeviceAeSetExpTable
(
    CamDeviceHandle_t   hCamDevice,
    const CamDeviceExpTable_t *pExpTable
);

/*****************************************************************************/
/**
 * @brief   This function gets the exposure table from engine layer.
 * @startuml VsiCamDeviceAeGetExpTable
 * !include E01_External/VsiCamDeviceAeGetExpTable.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pExpTable   Pointer to the exposure table.
 * @details This function calls: \ref CamEngineGetExpTable
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
RESULT VsiCamDeviceAeGetExpTable
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceExpTable_t *pExpTable
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE version.
 * @startuml VsiCamDeviceAeGetVersion
 * !include E01_External/VsiCamDeviceAeGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to the AE version.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 24_cam_device_ae */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_AE_API_H
