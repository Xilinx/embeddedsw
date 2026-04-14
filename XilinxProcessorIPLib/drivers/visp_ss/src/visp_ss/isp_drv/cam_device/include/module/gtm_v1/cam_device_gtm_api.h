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

#ifndef CAMDEV_GTM_API_H
#define CAMDEV_GTM_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 17_cam_device_gtm VsCamDevice E01C17 Device_GTM Definitions
 * @brief Provides interfaces for controlling the global tone mapping module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_GTM_RGB_COEF_SIZE       3
#define CAMDEV_GTM_COLOR_WEIGHT_SIZE   3
#define CAMDEV_GTM_HIST_LUMA_SIZE      4
#define CAMDEV_GTM_USER_CURVE_SIZE     129
#define CAMDEV_GTM_HIST_BINS           128

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM curve mode.
 *
 *****************************************************************************/
typedef enum CamDeviceGtmCurveMode_e
{
    CAMDEV_GTM_INTERPOLATION_CURVE_MODE           = 0,    /**< Interpolation mode */
    CAMDEV_GTM_HISTOGRAM_CURVE_MODE               = 1,    /**< Histogram mode */
    CAMDEV_GTM_INTERPOLATION_HISTOGRAM_CURVE_MODE = 2,    /**< Interpolation and histogram mode */
    CAMDEV_GTM_CURVE_MODE_MAX
} CamDeviceGtmCurveMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM curve select.
 *
 *****************************************************************************/
typedef enum CamDeviceGtmCurveSelect_e
{
    CAMDEV_GTM_LINEAR_SHIFT           = 0,    /**< Linear shift mode */
    CAMDEV_GTM_PIECE_WISE_LINEAR      = 1,    /**< Piece wise linear mode*/
    CAMDEV_GTM_LOGARITHM_COMPRESSION  = 2,    /**< Logarithm compression mode */
    CAMDEV_GTM_USER_CURVE             = 3,    /**< User curve mode */
    CAMDEV_GTM_CURVE_MAX
} CamDeviceGtmCurveSelect_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM black white correction mode.
 *
 *****************************************************************************/
typedef enum CamDeviceBlackWhiteCorMode_e {
    CAMDEV_BW_COR_MODE0         = 0,           /**<Manual mode*/
    CAMDEV_BW_COR_MODE1         = 1,           /**<Black white correction auto mode use previous frame statistics */
    CAMDEV_BW_COR_MODE2         = 2,           /**<White correction auto mode*/
    CAMDEV_BW_COR_MODE_MAX                     /**<Maximum border */
} CamDeviceBlackWhiteCorMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM linear shift bit.
 *
 *****************************************************************************/
typedef enum CamDeviceLinearShiftBit_e {
    CAMDEV_SHIFT_BIT_0         = 0,           /**<Output 0-19 bit*/
    CAMDEV_SHIFT_BIT_1         = 1,           /**<Output 1-20 bit*/
    CAMDEV_SHIFT_BIT_2         = 2,           /**<Output 2-21bit*/
    CAMDEV_SHIFT_BIT_3         = 3,           /**<Output 3-22bit*/
    CAMDEV_SHIFT_BIT_4         = 4,           /**<Output 4-23bit*/
    CAMDEV_SHIFT_BIT_MAX                     /**<Maximum border */
} CamDeviceLinearShiftBit_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM histogram state structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmHistogram_s
{
    uint32_t min;    /**< Minimum value */
    uint32_t max;    /**< Maximum value */
    uint32_t hist[CAMDEV_GTM_HIST_BINS];    /**< Histogram array */
} CamDeviceGtmHistogram_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM color weight structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmColorWeightConfig_s
{
    uint8_t gtmRgbCoef[CAMDEV_GTM_RGB_COEF_SIZE];    /**< Rgb coefficient */
    uint8_t gtmLightnessWeight;    /**< Lightness weight */
    uint8_t gtmColorWeight[CAMDEV_GTM_COLOR_WEIGHT_SIZE];    /**< Color weight */
} CamDeviceGtmColorWeightConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM black white correction configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmBlackWhiteCorrectionConfig_s
{
    CamDeviceBlackWhiteCorMode_t bwcorMode;    /**< Mode value */
    float64_t                       bwcorMinLog;    /**< Black white Correction min log */
    float64_t                       bwcorMaxLog;    /**< Black white Correction max log */
    float64_t                       bwcorDampCoef;    /**< Black white Correction damp coefficient */
} CamDeviceGtmBlackWhiteCorrectionConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM curve configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmCurveConfig_s
{
    CamDeviceGtmCurveSelect_t gtmCurveSelect;    /**< Curve select */
    CamDeviceLinearShiftBit_t shiftBit;    /**< Shift bit */
    float64_t pwlKneeXLog;    /**< PWL knee X log */
    float64_t pwlKneeYLog;    /**< PWL knee Y log */
    float64_t pwlMaxLog;    /**< PWL max log */
    float64_t pwlMinLog;    /**< PWL min log */
    float32_t logKneeXLog;    /**< Log knee X log */
    float32_t logKneeSlope;    /**< Log knee slope */
    uint32_t userCurve[CAMDEV_GTM_USER_CURVE_SIZE];    /**< User curve */
} CamDeviceGtmCurveConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM curve histogram information.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmCurveInfoConfig_s
{
    CamDeviceGtmCurveMode_t gtmCurveMode;    /**< Curve mode */
    bool_t userCurveEn;    /**< Whether to enable user curve */

    uint8_t histContrastLimitLevel;    /**< Histogram contrast limit level */
    uint8_t histLumaThrLog[CAMDEV_GTM_HIST_LUMA_SIZE];    /**< The log of histogram luma threshold */
    uint16_t histLumaWeight[CAMDEV_GTM_HIST_LUMA_SIZE];    /**< Weights of different luminance intervals in histogram curve */
    float32_t histHlcFactor;    /**< Highlight compensation factor in histogram curve */
    float32_t histLumaPresWeight;    /**< Luminance preservation weight in histogram curve */
    float32_t histCompressWeight;    /**< Compress preservation weight in histogram curve */
    float32_t histMaxGain;    /**< Maximum histogram gain */
    float32_t histMinGain;    /**< Minimum histogram gain */
    float32_t histStrength;    /**< Histogram strength */
    float32_t histDampCoef;    /**< Histogram damp coefficient */
} CamDeviceGtmCurveInfoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmManualConfig_s
{
    CamDeviceGtmColorWeightConfig_t colorWeightCfg;    /**< Color weight configuration */
    CamDeviceGtmBlackWhiteCorrectionConfig_t blackWhiteCorrectionCfg;    /**< Black white correction configuration */
    CamDeviceGtmCurveConfig_t curveCfg;    /**< Curve configuration */
} CamDeviceGtmManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmAutoConfig_s
{
    uint8_t autoLevel;    /**< Auto level */
    float32_t gain[CAMDEV_ISO_STRENGTH_NUM];    /**< GTM gain */
    uint8_t bwcorMinLog[CAMDEV_ISO_STRENGTH_NUM];    /**< Black white Correction log */
    uint8_t bwcorMaxLog[CAMDEV_ISO_STRENGTH_NUM];    /**< Black white Correction max log */
    uint8_t histContrastLimitLevel[CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram contrast limit level */
    float32_t histHlcFactor[CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram HLC factor */
    float32_t histLumaPreserveWeight[CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram luma preserve weight */
    float32_t histMaxGain[CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram max gain */
    float32_t histMinGain[CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram min gain */
    float32_t histStrength[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_ISO_STRENGTH_NUM];    /**< Histogram strength */

    uint8_t gtmColorWeight[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_GTM_COLOR_WEIGHT_SIZE];    /**< Color weight */
    uint16_t histLumaWeight[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_GTM_HIST_LUMA_SIZE];    /**< Histogram luma weight */
    uint32_t userCurve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_GTM_USER_CURVE_SIZE];    /**< User curve */
    CamDeviceGtmCurveInfoConfig_t gtmCurveInfo;    /**< Curve histogram information */

    uint8_t edrLevel;    /**< EDR level */
    float32_t edr[CAMDEV_ISO_STRENGTH_NUM];    /**< EDR array */
} CamDeviceGtmAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmConfig_s
{
    CamDeviceConfigMode_t configMode;    /**< GTM mode configuration */
    CamDeviceGtmAutoConfig_t autoCfg;    /**< GTM auto configuration */
    CamDeviceGtmManualConfig_t manualCfg;    /**< GTM manual configuration */
} CamDeviceGtmConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice GTM status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmStatus_s
{
    bool_t  enable;    /**< GTM enable status */
    bool_t  bwCorrectEnable;    /**< GTM black white correction status */
    CamDeviceConfigMode_t currentMode;    /**< GTM mode configuration */
    CamDeviceGtmManualConfig_t currentCfg;    /**< GTM current configuration */
} CamDeviceGtmStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets GTM configuration parameters.
 * @startuml VsiCamDeviceGtmSetConfig
 * !include E01_External/VsiCamDeviceGtmSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to GTM Configuration
 * @details This function calls: \ref CamDeviceGtmManualSetConfig, \ref CamDeviceGtmAutoSetConfig,
 * \ref CamEngineGtmSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceGtmReset
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
RESULT VsiCamDeviceGtmSetConfig
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceGtmConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM configuration parameters.
 * @startuml VsiCamDeviceGtmGetConfig
 * !include E01_External/VsiCamDeviceGtmGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to GTM configuration.
 * @details This function calls: \ref CamEngineGtmGetMode, \ref CamDeviceGtmManualGetConfig,
 * \ref CamDeviceGtmAutoGetConfig
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
RESULT VsiCamDeviceGtmGetConfig
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceGtmConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables GTM.
 * @startuml VsiCamDeviceGtmEnable
 * !include E01_External/VsiCamDeviceGtmEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGtmEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GTM.
 * @startuml VsiCamDeviceGtmDisable
 * !include E01_External/VsiCamDeviceGtmDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGtmDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables GTM black white correction.
 * @startuml VsiCamDeviceGtmBlackWhiteCorrectionEnable
 * !include E01_External/VsiCamDeviceGtmBlackWhiteCorrectionEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGtmBlackWhiteCorrectionEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmBlackWhiteCorrectionEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GTM black white correction.
 * @startuml VsiCamDeviceGtmBlackWhiteCorrectionDisable
 * !include E01_External/VsiCamDeviceGtmBlackWhiteCorrectionDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineGtmBlackWhiteCorrectionDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmBlackWhiteCorrectionDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM black white correction enable config.
 * @startuml VsiCamDeviceGtmBlackWhiteCorrectionEnGetConfig
 * !include E01_External/VsiCamDeviceGtmBlackWhiteCorrectionEnGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice    Handle to the VsCamDevice instance.
 * @param[inout]    pBwCorEnable  Pointer to BwCorEnable config.
 * @details This function calls: \ref CamEngineGtmBlackWhiteCorrectionEnGetConfig
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
RESULT VsiCamDeviceGtmBlackWhiteCorrectionEnGetConfig
(
    CamDeviceHandle_t hCamDevice,
    bool_t *pBwCorEnable
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM status.
 * @startuml VsiCamDeviceGtmGetStatus
 * !include E01_External/VsiCamDeviceGtmGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to GTM status.
 * @details This function calls: \ref CamEngineGtmGetStatus
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
RESULT VsiCamDeviceGtmGetStatus
(
    CamDeviceHandle_t    hCamDevice,
    CamDeviceGtmStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM version.
 * @startuml VsiCamDeviceGtmGetVersion
 * !include E01_External/VsiCamDeviceGtmGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to GTM version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets GTM.
 * @startuml VsiCamDeviceGtmReset
 * !include E01_External/VsiCamDeviceGtmReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceGtmSetConfig
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
RESULT VsiCamDeviceGtmReset
(
    CamDeviceHandle_t             hCamDevice
);

/** @} 17_cam_device_gtm */

#ifdef __cplusplus
}
#endif

#endif
