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

#ifndef CAMDEV_WDR5_API_H
#define CAMDEV_WDR5_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 14_cam_device_wdr VsCamDevice E01C14 Device_WDR Definitions
 * @brief Provides interfaces for controlling the wide dynamic range module
 * working in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_WDR_LIGHT_THR_LOG_SIZE 4   /**< bin number of light threshold log size */
#define CAMDEV_WDR_FLAT_LEVEL_CHANNAL_SIZE 4    /**< Bin number of flat level channal size */
#define CAMDEV_WDR_FLAT_LEVEL_INC_SIZE 17    /**< Bin number of flat level size */
#define CAMDEV_WDR_COLOR_WEIGHT_SIZE 3    /**< Bin number of color weight size */
#define CAMDEV_WDR_RGB_COEF_SIZE 3    /**< Bin number of RGB coefficient size */
#define CAMDEV_WDR_LEVEL_MAX 20    /**< MAX WDR LEVEL */
#define CAMDEV_WDR_GAMMA_UP_BIN 65 /** <Bin number of gammaUp Curve */

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR strength configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrStrengthConfig_s
{
    uint8_t strength;    /**< WDR strength */
    uint8_t highStrength;    /**< WDR high strength */
    uint16_t lowStrength;    /**< WDR low strength */
} CamDeviceWdrStrengthConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR LTM weight configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrLtmWeightConfig_s
{
    int16_t contrast;    /**< WDR contrast */
    bool_t entropyEnable;    /**< WDR entropy enable */
    uint16_t entropyBase;    /**< WDR factor for all pixel based on image base */
    uint16_t entropySlope;    /**< WDR factor for all pixel based on image slope */
    uint16_t wdrLumaThr;    /**< WDR luma threshold */
} CamDeviceWdrLtmWeightConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR LTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrLtmConfig_s
{
    bool_t flatMode;    /**< WDR flat mode */
    uint8_t flatLevel;    /**< WDR flat level */
    uint8_t flatLevelInc[CAMDEV_WDR_FLAT_LEVEL_CHANNAL_SIZE][CAMDEV_WDR_FLAT_LEVEL_INC_SIZE];    /**< WDR flat level increase */
    uint8_t darkAttentionLevel;    /**< WDR dark attention level */
} CamDeviceWdrLtmConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGtmWeightConfig_s
{
    uint16_t fixedWeight;    /**< WDR fixed weight */
} CamDeviceWdrGtmWeightConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR gammaUp configurations.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGammaUpConfig_s
{
    uint32_t gammaUpCurveX[CAMDEV_WDR_GAMMA_UP_BIN];
    uint32_t gammaUpCurveY[CAMDEV_WDR_GAMMA_UP_BIN];
} CamDeviceWdrGammaUpConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGtmConfig_s
{
    uint8_t wdrCurveSelect;    /**< WDR curve select */
    uint8_t logWeight;    /**< WDR log weight */
    float32_t logAnchorXLog;    /**< WDR anchor X log */
    uint16_t logAnchorSlope;    /**< WDR anchor slope */
    float32_t ratioActual;    /**< WDR actual ratio */
    float32_t curve2Thr;    /**< WDR curve2 threshold */
    float32_t curve2Losat;    /**< WDR curve2 low saturation */
    float32_t curve2Lofactor;    /**< WDR curve2 low factor */
    float32_t curve2Hifactor;    /**< WDR curve2 high factor */
    uint8_t flatLevelGlobal;    /**< WDR global flat level */
} CamDeviceWdrGtmConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR degamma configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDegammaConfig_s
{
    float32_t degamma;    /**< WDR degamma */
} CamDeviceWdrDegammaConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR gain limitation configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGainLimitationConfig_s
{
    float64_t maxGain;   /**< WDR max gain */
    float64_t minGain;    /**< WDR min gain */
} CamDeviceWdrGainLimitationConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR defringe configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDefringeConfig_s
{
    float32_t diffHigh;    /**< WDR high difference */
    float32_t diffLow;    /**< WDR low difference */
} CamDeviceWdrDefringeConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR saturation adjustment configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrSaturationAdjustmentConfig_s
{
    float32_t satRange;    /**< WDR saturation range */
    uint16_t satThrGainDown;   /**< WDR saturation threshold gain down */
    uint16_t satThrGainUp;   /**< WDR saturation threshold gain up */
} CamDeviceWdrSaturationAdjustmentConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR halo color fading configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrHaloColorFadingConfig_s
{
    uint8_t lightSatLothr;    /**< WDR light saturation low threshold */
    uint8_t lightSatHithr;    /**< WDR light saturation high threshold */
    float32_t lightRedThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR red light threshold log */
    float32_t lightGreenThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR green light threshold log */
    float32_t lightBlueThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR blue light threshold log */
    float32_t lightYellowThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR yellow light threshold log */
    float32_t lightCyanThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR cyan light threshold log */
    float32_t lightMagentaThrLog[CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR magenta light threshold log */
} CamDeviceWdrHaloColorFadingConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR high light configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrHighLightConfig_s
{
    float32_t hlcBaseLog;    /**< WDR HLC base log */
    uint8_t hlcSlope;    /**< WDR HLC slope */
} CamDeviceWdrHighLightConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR dump configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDampConfig_s
{
    uint8_t dampMode;    /**< WDR damp mode */
    uint8_t dampCurveCoef;    /**< WDR curve coefficient */
    uint8_t dampCurveMax;    /**< WDR max damp curve */
    uint8_t dampCurveMin;    /**< WDR min damp curve */
    uint8_t dampAvgCoef;    /**< WDR average coefficient */
    uint8_t dampAvgMax;    /**< WDR max average */
    uint8_t dampAvgMin;    /**< WDR min average */
    uint8_t dampCoefDecLimit;    /**< WDR damp coefficient decrease limit */
    uint8_t dampCoefIncLimit;    /**< WDR damp coefficient increase limit */
    uint8_t dampFilterSize;    /**< WDR damp filter size */
    float32_t dampHithrLog;    /**< WDR damp high threshold log */
    float32_t dampLothrLog;    /**< WDR damp low threshold log */
} CamDeviceWdrDampConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR color weight configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrColorWeightConfig_s
{
    uint8_t wdrRgbCoef[CAMDEV_WDR_RGB_COEF_SIZE];    /**< WDR RGB coefficient */
    uint8_t wdrLightnessWeight;    /**< WDR lightness weight */
    uint8_t wdrColorWeight[CAMDEV_WDR_COLOR_WEIGHT_SIZE];    /**< WDR color weight */
} CamDeviceWdrColorWeightConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrAutoConfig_s
{
    uint8_t autoLevel;    /**< WDR auto configuration level */
    float32_t gain[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR gain */
    uint8_t highStrength[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR high strength */
    uint16_t lowStrength[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR low strength */
    uint16_t entropyBase[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR factor for all pixel based on image base */
    uint16_t wdrLumaThr[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR luma threshold */
    uint16_t entropySlope[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR factor for all pixel based on image slope */
    uint8_t flatLevel[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR flat level */
    uint8_t flatLevelGlobal[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR global flat level */
    float32_t satRange[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR saturation range */
    uint16_t satThrGaindown[CAMDEV_ISO_STRENGTH_NUM];   /**< WDR saturation threshold gain down */
    uint16_t satThrGainup[CAMDEV_ISO_STRENGTH_NUM];   /**< WDR saturation threshold gain up */
    float32_t degamma[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR degamma value */
    float32_t hlcBaseLog[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR HLC base log */
    uint8_t lightSatLothr[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR light saturation low threshold */
    uint8_t lightSatHithr[CAMDEV_ISO_STRENGTH_NUM];    /**< WDR light saturation high threshold */

    float32_t lightRedThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR red light threshold log */
    float32_t lightGreenThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR green light threshold log */
    float32_t lightBlueThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR blue light threshold log */
    float32_t lightYellowThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR yellow light threshold log */
    float32_t lightCyanThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR cyan light threshold log */
    float32_t lightMagentaThrLog[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR magenta light threshold log */

    uint8_t edrLevel;    /**< EDR level */
    float32_t edr[CAMDEV_WDR_LEVEL_MAX];    /**< EDR value */
    int16_t contrast[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];    /**< WDR contrast */
    uint16_t fixedWeight[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];    /**< WDR fixed weight */
    uint8_t logWeight[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];    /**< WDR log weight */
    float32_t logAnchorXLog[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];   /**< WDR anchor X log */
    uint16_t logAnchorSlope[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];   /**< WDR anchor slope */
    float64_t maxGain[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];  /**< WDR max gain */
    uint8_t hlcSlope[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];   /**< WDR HLC slope */
    bool_t lightEn[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM];  /**< WDR light enable configuration */
    uint8_t wdrColorWeight[CAMDEV_WDR_LEVEL_MAX][CAMDEV_ISO_STRENGTH_NUM][CAMDEV_WDR_COLOR_WEIGHT_SIZE];  /**< WDR color weight */
}CamDeviceWdrAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrManualConfig_s {
    CamDeviceWdrStrengthConfig_t strengthCfg;   /**< WDR strength configuration */
    CamDeviceWdrLtmWeightConfig_t ltmWeightCfg;   /**< WDR LTM weight configuration */
    CamDeviceWdrLtmConfig_t ltmCfg;   /**< WDR LTM configuration */
    CamDeviceWdrGtmWeightConfig_t gtmWeightCfg;   /**< WDR GTM weight configuration */
    CamDeviceWdrGtmConfig_t gtmCfg;   /**< WDR GTM configuration */
    CamDeviceWdrDegammaConfig_t degammaCfg;   /**< WDR degamma configuration */
    CamDeviceWdrGainLimitationConfig_t gainLimitationCfg;   /**< WDR gain limitation configuration */
    CamDeviceWdrDefringeConfig_t defringeCfg;   /**< WDR defringe configuration */
    CamDeviceWdrSaturationAdjustmentConfig_t saturationAdjustmentCfg;   /**< WDR saturation adjustment configuration */
    CamDeviceWdrHaloColorFadingConfig_t haloColorFadingCfg;   /**< WDR halo color configuration */
    CamDeviceWdrHighLightConfig_t highLightCfg;   /**< WDR high light configuration */
    bool_t lightEn;  /**< WDR light en configuration */
    CamDeviceWdrDampConfig_t dampCfg;   /**< WDR damp configuration */
    CamDeviceWdrColorWeightConfig_t colorWeightCfg;   /**< WDR color weight configuration */
}CamDeviceWdrManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrConfig_s {
    CamDeviceConfigMode_t configMode;      /**< The run mode: 0--manual, 1--auto */
    CamDeviceWdrAutoConfig_t autoCfg;      /**< WDR auto configuration*/
    CamDeviceWdrManualConfig_t manualCfg;   /**< WDR manual configuration*/
}CamDeviceWdrConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice WDR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrStatus_s {
    bool_t enable;    /**< WDR enable status */
    CamDeviceConfigMode_t currentMode;      /**< The run mode: 0--manual, 1--auto */
    CamDeviceWdrManualConfig_t currentCfg;   /**< WDR current configuration*/
}CamDeviceWdrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets WDR configuration parameters.
 * @startuml VsiCamDeviceWdrSetConfig
 * !include E01_External/VsiCamDeviceWdrSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to WDR Configuration
 * @details This function calls: \ref CamDeviceWdrManualSetConfig, \ref CamDeviceWdrAutoSetConfig,
 * CamEngineWdrv52SetMode
 * @details This function is called by: User application, \ref VsiCamDeviceWdrReset
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
RESULT VsiCamDeviceWdrSetConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWdrConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR configuration parameters.
 * @startuml VsiCamDeviceWdrGetConfig
 * !include E01_External/VsiCamDeviceWdrGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to WDR configuration.
 * @details This function calls: \ref CamEngineWdrv52GetMode, \ref CamDeviceWdrManualGetConfig,
 * CamDeviceWdrAutoGetConfig
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
RESULT VsiCamDeviceWdrGetConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWdrConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets WDR gammaUp configuration parameters.
 *          When wdrCurveSelect = 3, gammaUpConfig take effect.
 * @startuml VsiCamDeviceWdrSetGammaUpConfig
 * !include E01_External/VsiCamDeviceWdrSetGammaUpConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to WDR Configuration
 * @details This function calls: \ref CamEngineWdrv52GammaUpSetConfig
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
RESULT VsiCamDeviceWdrSetGammaUpConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWdrGammaUpConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR gammaUp configuration parameters.
 * @startuml VsiCamDeviceWdrGetGammaUpConfig
 * !include E01_External/VsiCamDeviceWdrGetGammaUpConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to WDR gammaUp configuration.
 * @details This function calls: \ref CamEngineWdrv52GammaUpGetConfig
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
RESULT VsiCamDeviceWdrGetGammaUpConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWdrGammaUpConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR gammaUp status parameters.
 * @startuml VsiCamDeviceWdrGetGammaUpStatus
 * !include E01_External/VsiCamDeviceWdrGetGammaUpStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to the configuration of WDR.
 * @details This function calls: \ref CamEngineWdrv52GetGammaUpStatus
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
RESULT VsiCamDeviceWdrGetGammaUpStatus
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWdrGammaUpConfig_t   *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function enables WDR.
 * @startuml VsiCamDeviceWdrEnable
 * !include E01_External/VsiCamDeviceWdrEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWdrv52Enable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrEnable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables WDR.
 * @startuml VsiCamDeviceWdrDisable
 * !include E01_External/VsiCamDeviceWdrDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWdrv52Disable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrDisable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables WDR halo color fading.
 * @startuml VsiCamDeviceWdrHaloColorFadingEnable
 * !include E01_External/VsiCamDeviceWdrHaloColorFadingEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWdrv52HaloColorFadingEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrHaloColorFadingEnable
(
    CamDeviceHandle_t                       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables WDR halo color fading.
 * @startuml VsiCamDeviceWdrHaloColorFadingDisable
 * !include E01_External/VsiCamDeviceWdrHaloColorFadingDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineWdrv52HaloColorFadingDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrHaloColorFadingDisable
(
    CamDeviceHandle_t                       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR status.
 * @startuml VsiCamDeviceWdrGetStatus
 * !include E01_External/VsiCamDeviceWdrGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to WDR status.
 * @details This function calls: \ref CamEngineWdrv52GetStatus
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
RESULT VsiCamDeviceWdrGetStatus
(
    CamDeviceHandle_t            hCamDevice,
    CamDeviceWdrStatus_t         *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR version.
 * @startuml VsiCamDeviceWdrGetVersion
 * !include E01_External/VsiCamDeviceWdrGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to GC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets WDR. It's only available in manual mode.
 * @startuml VsiCamDeviceWdrReset
 * !include E01_External/VsiCamDeviceWdrReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceWdrSetConfig
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
RESULT VsiCamDeviceWdrReset
(
    CamDeviceHandle_t hCamDevice
);

/** @} 14_cam_device_wdr */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_WDR5_API_H */
