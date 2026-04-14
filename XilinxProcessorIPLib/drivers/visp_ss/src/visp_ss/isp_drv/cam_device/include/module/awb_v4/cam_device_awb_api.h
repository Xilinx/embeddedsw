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

#ifndef CAMDEV_AWB_API_H
#define CAMDEV_AWB_API_H

/* VIV: TODO: #include "vsi_3alib_interface.h" */
#include "cam_device_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 05_cam_device_awb VsCamDevice E01C05 Device_WBG Definitions
 * @brief Provides interfaces for controlling the auto white balance module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_AWB_COLOR_TEM_WEIGHT_NUM     10U    /**< The number of AWB color temperature weight */
#define CAMDEV_AWB_LIGHT_LEVEL              18U    /**< AWB light level */
#define CAMDEV_ILLUPROFILE_NUM              10U    /**< The number of illumination profiles */
#define CAMDEV_AWB_CONFOUND_POINTS_NUM      25U    /**< The AWB new confound function parameters */
#define AWB_DEVICE_RETENTION_VALUE_MAX      500U   /**< The AWB maximum device retention value */

#define CAMDEV_AWB_FADE_NUM                 16U
#define CAMDEV_AWB_GAINDIST_NUM             16U

/*****************************************************************************/
/**
 * @brief    AWB color temperature weight of every lighting
 *
 *****************************************************************************/
typedef float32_t CamDeviceAwbTemWeight_t[CAMDEV_AWB_COLOR_TEM_WEIGHT_NUM];

/*****************************************************************************/
/**
 *
 * @brief   VsCamDevice AWB status.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbState_e {
    CAMDEV_AWB_STATE_INVALID       = 0,    /**< Invalid status */
    CAMDEV_AWB_STATE_INITIALIZED   = 1,    /**< AWB initialized */
    CAMDEV_AWB_STATE_STOPPED       = 2,    /**< AWB stopped */
    CAMDEV_AWB_STATE_RUNNING       = 3,    /**< AWB running */
    CAMDEV_AWB_STATE_LOCKED        = 4,    /**< AWB locked */
    CAMDEV_AWB_STATE_MAX
} CamDeviceAwbState_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbMode_e {
    CAMDEV_AWB_MODE             = 0,    /**< AWB mode */
    CAMDEV_AWB_METEDATA_MODE    = 1,    /**< AWB METADATA MODE */
} CamDeviceAwbMode_t;

/*****************************************************************************/
/**
 * @brief    VsCamDevice AWB performance optimizationon mode.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbPerformanceOptiMode_s
{
    CAMDEV_AWB_PERFORMANCE_NO_OPTIMIZATION         = 0,    /**< No optimization mode */
    CAMDEV_AWB_PERFORMANCE_GENERAL_OPTIMIZATION    = 1,    /**< General optimization mode */
    CAMDEV_AWB_PERFORMANCE_FAST_OPTIMIZATION       = 2,    /**< Fast optimization mode */
} CamDeviceAwbPerformanceOptiMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB temperature preference weight parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbTemPreferenceWeight_s {
    bool_t     temptureEnbale;    /**< Temperature weight enable */
    uint16_t preferenceD65;     /**< High temperature preference parameters */
    uint16_t preferenceCwf;     /**< Median temperature preference parameters */
    uint16_t preferenceA;       /**< Low temperature preference parameters */
} CamDeviceAwbTemPreferenceWeight_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB confidence found position parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbConfoundPosition_s {
    bool_t  customPositionEnable;    /**< Position enable */
    float32_t rg;                      /**< Red green value */
    float32_t bg;                      /**< Blue green value */
    float32_t threshold;               /**< Position threshold */
} CamDeviceAwbConfoundPosition_t;


/*****************************************************************************/
/**
 *          CamDeviceAwbConfoundPositionCustom_t
 *
 * @brief   awb button function cutomer function parameters
 */
/*****************************************************************************/
typedef struct CamDeviceAwbConfoundPositionCustom_s
{
    bool      customPositionEnable;   // button function enable
    float32_t rg;                     // The rg button
    float32_t bg;                     // The bg button
    float32_t threshold;              // The threshold
    float32_t weight;                 // Misleading color point's weight. when weight=1, the effect equals to customPositionEnable=0.
                                      // Setting weight to 0 means totally remove this misleading color region.
} CamDeviceAwbConfoundPositionCustom_t;

/*****************************************************************************/
/**
 *          CamDeviceAwbCalibPoint_t
 *
 * @brief   awb new calibration area
 */
/*****************************************************************************/
typedef struct CamDeviceAwbCalibPoint_s
{
	bool_t    customPositionEnable;  // button function enable
	float32_t rg;                    // The rg button
	float32_t bg;                    // The bg button
	float32_t threshold;             // The threshold
	float32_t weight;
} CamDeviceAwbCalibPoint_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB light weight level parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbLightWeightLevel_s {
    bool_t  lightWeightLevelEnable;                       /**< Light weight level enable */
    float32_t brightnessLevel[CAMDEV_AWB_LIGHT_LEVEL];      /**< Brightness level */
    float32_t weight[CAMDEV_AWB_LIGHT_LEVEL];               /**< Light weight level */
} CamDeviceAwbLightWeightLevel_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB preference gain parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbPreferenceGain_s {
    bool_t     preferenceGainEnable;                       /**< Grayscale preference enable */
    float32_t    brightnessLevel[CAMDEV_AWB_LIGHT_LEVEL];    /**< Brightness level */
    uint16_t grayRgain[CAMDEV_AWB_LIGHT_LEVEL];          /**< The grayscale of red channel */
    uint16_t grayBgain[CAMDEV_AWB_LIGHT_LEVEL];          /**< The grayscale of green channel */
} CamDeviceAwbPreferenceGain_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB damping parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbDamping_s
{
    bool_t      useDamping;    /**< Damping mode open */
    bool_t      useManualDampCoff;    /**< Use manual damping coefficient */
    float32_t   manualDampCoff;    /**< Manual damping coefficient */
} CamDeviceAwbDamping_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice AWB control configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbConfig_s {
    float32_t                             confidenceThreshold[CAMDEV_ILLUPROFILE_NUM];               /**< Light source confidence */
    CamDeviceAwbTemPreferenceWeight_t     temPreference;                                             /**< The temperature preference */
    CamDeviceAwbConfoundPosition_t        confoundPointCwf;                                          /**< Button function in outdorr with green grass */
    CamDeviceAwbConfoundPosition_t        confoundPointTl84;                                         /**< Button function in outdorr with yellow grass */
    CamDeviceAwbConfoundPosition_t        confoundPointD65;                                          /**< Button function in outdorr with blue sky */
    bool_t                                confoundPointCustomEnable;                                 // button function enable
    uint8_t                               confoundPointNum;                                          /**< confound point number */
    CamDeviceAwbConfoundPositionCustom_t  confoundPointCustom[CAMDEV_AWB_CONFOUND_POINTS_NUM];       // button function in customer mode
    CamDeviceAwbLightWeightLevel_t        lightWeightLevel[CAMDEV_ILLUPROFILE_NUM];                  /**< The weight of different light source */
    CamDeviceAwbPreferenceGain_t          grayPreference[CAMDEV_ILLUPROFILE_NUM];                    /**< Gray scale preference */

    CamDeviceAwbPerformanceOptiMode_t     performanceOptiMode;        /** Performance optimization mode */
    float32_t                             awbEnterLockThreshold;      /** Enter lock threshold */
    float32_t                             awbEnterUnlockThreshold;    /** Enter unlock threshold */
    CamDeviceAwbDamping_t                 awbDampingCfg;              /** Damping parameters */
    bool_t                                awbFrameCal;                /**< Whether to enable frame calculation */
    float32_t                             nonLinearThd;               /**< The threshold for nonlinear statistics */

    bool_t                                calibPointCustomEnable;                                     /**< calib point enable */
    uint8_t                               calibPointNum;                                              /**< calib point number */
    CamDeviceAwbCalibPoint_t              calibPoint[CAMDEV_AWB_CONFOUND_POINTS_NUM];                 /**< calibPoint */
} CamDeviceAwbConfig_t;

/*****************************************************************************/
/**
 * @brief   Cam Device AWB result
 *
 *****************************************************************************/
typedef struct CamDeviceAwbResult_s {
    uint16_t    awbCCT;
} CamDeviceAwbResult_t;

/*****************************************************************************/
/**
 *          CamDeviceAwbCalibData_t
 *
 * @brief   VsCamDevice AWB calibration configuration which lib calculation need
 *
 */
/*****************************************************************************/
typedef struct CamDeviceAwbCalibData_s {
    bool_t       useManualCalib;
    float32_t    rgProjIndoorMin;
    float32_t    rgProjMax;
    float32_t    kFactor;

    float32_t    centerLineRg;
    float32_t    centerLineBg;
    float32_t    centerLineD;

    float32_t    fade1[CAMDEV_AWB_FADE_NUM];
    float32_t    fade1GainDist[CAMDEV_AWB_GAINDIST_NUM];
    float32_t    fade2[CAMDEV_AWB_FADE_NUM];
    float32_t    fade2GainDist[CAMDEV_AWB_GAINDIST_NUM];

    float32_t    fIIRDampCoefAdd;
    float32_t    fIIRDampCoefSub;
    float32_t    fIIRDampFilterThreshold;
    float32_t    fIIRDampingCoefMin;
    float32_t    fIIRDampingCoefMax;
    float32_t    fIIRDampingCoefInit;
    int32_t      expPriorFilterSizeMax;
} CamDeviceAwbCalibData_t;

/*****************************************************************************/
/**
 * @brief    Cam Device AWB front ground configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbFrontGroundConfig_s {
    bool_t       faceAwbEnable;    /**< Enable AWB face function */
    uint8_t      roiNum;           /**< The number of ROI */
    float32_t    faceWeight;       /**< AWB face weight */
    float32_t    rg;               /**< Red green value */
    float32_t    bg;               /**< Blue green value */
} CamDeviceAwbFrontGroundConfig_t;

/*****************************************************************************/
/**
 * @brief   This function registers the AWB library.
 * @startuml VsiCamDeviceAwbRegister
 * !include E01_External/VsiCamDeviceAwbRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       pAwbLibHandle  Pointer to the AWB library handle.
 * @details This function calls: \ref CamEngineAwbRegister
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
RESULT VsiCamDeviceRegisterAwbLib
(
    CamDeviceHandle_t   hCamDevice,
    void                *pAwbLibHandle
);

/*****************************************************************************/
/**
 * @brief   This function unregisters the AWB library.
 * @startuml VsiCamDeviceAwbUnRegister
 * !include E01_External/VsiCamDeviceAwbUnRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAwbUnRegister
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
RESULT VsiCamDeviceUnRegisterAwbLib
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets AWB calculation parameters.
 * @startuml VsiCamDeviceAwbSetConfig
 * !include E01_External/VsiCamDeviceAwbSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to the AWB calculation parameters.
 * @details This function calls: \ref CamEngineAwbSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceAwbReset
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
RESULT VsiCamDeviceAwbSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbConfig_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets AWB calculation parameters.
 * @startuml VsiCamDeviceAwbGetConfig
 * !include E01_External/VsiCamDeviceAwbGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to the AWB calculation configuration parameters.
 * @details This function calls: \ref CamEngineAwbGetConfig
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
RESULT VsiCamDeviceAwbGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets AWB calculation mode parameters.
 * @startuml VsiCamDeviceAwbSetMode
 * !include E01_External/VsiCamDeviceAwbSetMode.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pAwbMode       Pointer to the AWB calculation mode.
 * @details This function calls: \ref CamEngineAwbSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceAwbReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbSetMode
(
    CamDeviceHandle_t            hCamDevice,
    const CamDeviceAwbMode_t     *pAwbMode
);

/*****************************************************************************/
/**
 * @brief   This function gets AWB calculation mode parameters.
 * @startuml VsiCamDeviceAwbGetMode
 * !include E01_External/VsiCamDeviceAwbGetMode.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pAwbMode       Pointer to the AWB calculation mode.
 * @details This function calls: \ref CamEngineAwbGetMode
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
RESULT VsiCamDeviceAwbGetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAwbMode_t  *pAwbMode
);

/*****************************************************************************/
/**
 * @brief   This function sets the AWB calibration.
 * @startuml VsiCamDeviceAwbSetCalibData
 * !include E01_External/VsiCamDeviceAwbSetCalibData.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pAwbCalib   Pointer to the AWB calibration parameters.
 * @details This function calls: \ref CamEngineAwbSetCalibData
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
RESULT VsiCamDeviceAwbSetCalibData
(
    CamDeviceHandle_t               hCamDevice,
    const CamDeviceAwbCalibData_t   *pAwbCalib
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB calibration.
 * @startuml VsiCamDeviceAwbGetCalibData
 * !include E01_External/VsiCamDeviceAwbGetCalibData.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pAwbCalib   Pointer to the AWB calibration parameters.
 * @details This function calls: \ref CamEngineAwbGetCalibData
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
RESULT VsiCamDeviceAwbGetCalibData
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceAwbCalibData_t   *pAwbCalib
);

/*****************************************************************************/
/**
 * @brief   This function sets the AWB ROI.
 * @startuml VsiCamDeviceAwbSetRoi
 * !include E01_External/VsiCamDeviceAwbSetRoi.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pRoi  Pointer to the AWB ROI configuration.
 * @details This function calls: \ref CamEngineAwbSetRoi
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
RESULT VsiCamDeviceAwbSetRoi
(
    CamDeviceHandle_t       hCamDevice,
    const CamDeviceRoi_t   *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB ROI.
 * @startuml VsiCamDeviceAwbGetRoi
 * !include E01_External/VsiCamDeviceAwbGetRoi.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pRoi        Pointer to the AWB ROI configuration.
 * @details This function calls: \ref CamEngineAwbGetRoi
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
RESULT VsiCamDeviceAwbGetRoi
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceRoi_t          *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB temperature weight of every lighting.
 * @startuml VsiCamDeviceAwbGetColorTempWeight
 * !include E01_External/VsiCamDeviceAwbGetColorTempWeight.plantuml
 * @enduml
 * @param[in]       hCamDevice     Handle to the VsCamDevice instance.
 * @param[inout]    pAwbTemWeight  Pointer to the AWB temperature weight.
 * @details This function calls: \ref CamEngineAwbGetColorTempWeight
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
RESULT VsiCamDeviceAwbGetColorTempWeight
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAwbTemWeight_t  *pAwbTemWeight
);

/*****************************************************************************/
/**
 * @brief   This function enables AWB.
 * @startuml VsiCamDeviceAwbEnable
 * !include E01_External/VsiCamDeviceAwbEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAwbStart
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbEnable
(
    CamDeviceHandle_t           hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AWB.
 * @startuml VsiCamDeviceAwbDisable
 * !include E01_External/VsiCamDeviceAwbDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineAwbStop
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets AWB result.
 * @startuml VsiCamDeviceAwbGetResult
 * !include E01_External/VsiCamDeviceAwbGetResult.plantuml
 * @enduml
 * @param[in]       hCamDevice            Handle to the VsCamDevice instance.
 * @param[inout]    pCamDeviceAwbResult    Pointer to AWB front ground configuration.
 * @details This function calls: \ref CamEngineAwbGetResult
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
RESULT VsiCamDeviceAwbGetResult
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceAwbResult_t  *pCamDeviceAwbResult
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB status.
 * @startuml VsiCamDeviceAwbGetStatus
 * !include E01_External/VsiCamDeviceAwbGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pAwbStatus   Pointer to the AWB status.
 * @details This function calls: \ref CamEngineAwbGetStatus
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
RESULT VsiCamDeviceAwbGetStatus
(
    CamDeviceHandle_t           hCamDevice,
    CamDeviceAwbState_t         *pAwbStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets AWB.
 * @startuml VsiCamDeviceAwbReset
 * !include E01_External/VsiCamDeviceAwbReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceAwbSetConfig, \ref VsiCamDeviceAwbSetMode
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
RESULT VsiCamDeviceAwbReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB version.
 * @startuml VsiCamDeviceAwbGetVersion
 * !include E01_External/VsiCamDeviceAwbGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to the AWB version.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 26_cam_device_awb */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_AWB_API_H
