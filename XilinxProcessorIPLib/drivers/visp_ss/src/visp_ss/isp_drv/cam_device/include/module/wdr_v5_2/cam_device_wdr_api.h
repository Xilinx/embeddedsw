/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @cond WDR_V5_2
 *
 * @defgroup cam_device_wdr_v5_2 CamDevice WDR V5.2 Definitions
 * @{
 *
 */

#ifndef CAMDEV_WDR5_API_H
#define CAMDEV_WDR5_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define CAMDEV_WDR_LIGHT_THR_LOG_SIZE 4   /**< bin number of light threshold log size */
#define CAMDEV_WDR_FLAT_LEVEL_CHANNAL_SIZE 4    /**< Bin number of flat level channal size */
#define CAMDEV_WDR_FLAT_LEVEL_INC_SIZE 17    /**< Bin number of flat level size */
#define CAMDEV_WDR_COLOR_WEIGHT_SIZE 3    /**< Bin number of color weight size */
#define CAMDEV_WDR_RGB_COEF_SIZE 3    /**< Bin number of RGB coefficient size */
#define CAMDEV_WDR_LEVEL_MAX 20    /**< MAX WDR LEVEL */
#define CAMDEV_WDR_GAMMA_UP_BIN 65 /** <Bin number of gammaUp Curve */

/******************************************************************************/
/**
 * @brief   CamDevice WDR strength configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrStrengthConfig_s {
	uint8_t strength;    /**< WDR strength */
	uint8_t highStrength;    /**< WDR high strength */
	uint16_t lowStrength;    /**< WDR low strength */
} CamDeviceWdrStrengthConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR LTM weight configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrLtmWeightConfig_s {
	int16_t contrast;    /**< WDR contrast */
	bool_t entropyEnable;    /**< WDR entropy enable */
	uint16_t entropyBase;    /**< WDR factor for all pixel based on image base */
	uint16_t entropySlope;    /**< WDR factor for all pixel based on image slope */
	uint16_t wdrLumaThr;    /**< WDR luma threshold */
} CamDeviceWdrLtmWeightConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR LTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrLtmConfig_s {
	bool_t flatMode;    /**< WDR flat mode */
	uint8_t flatLevel;    /**< WDR flat level */
	uint8_t flatLevelInc[CAMDEV_WDR_FLAT_LEVEL_CHANNAL_SIZE][CAMDEV_WDR_FLAT_LEVEL_INC_SIZE];    /**< WDR flat level increase */
	uint8_t darkAttentionLevel;    /**< WDR dark attention level */
} CamDeviceWdrLtmConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGtmWeightConfig_s {
	uint16_t fixedWeight;    /**< WDR fixed weight */
} CamDeviceWdrGtmWeightConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR gammaUp configurations.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGammaUpConfig_s {
	uint32_t gammaUpCurveX[CAMDEV_WDR_GAMMA_UP_BIN];
	uint32_t gammaUpCurveY[CAMDEV_WDR_GAMMA_UP_BIN];
} CamDeviceWdrGammaUpConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGtmConfig_s {
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
 * @brief   CamDevice WDR degamma configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDegammaConfig_s {
	float32_t degamma;    /**< WDR degamma */
} CamDeviceWdrDegammaConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR gain limitation configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrGainLimitationConfig_s {
	float64_t maxGain;   /**< WDR max gain */
	float64_t minGain;    /**< WDR min gain */
} CamDeviceWdrGainLimitationConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR defringe configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDefringeConfig_s {
	float32_t diffHigh;    /**< WDR high difference */
	float32_t diffLow;    /**< WDR low difference */
} CamDeviceWdrDefringeConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR saturation adjustment configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrSaturationAdjustmentConfig_s {
	float32_t satRange;    /**< WDR saturation range */
	uint16_t satThrGainDown;   /**< WDR saturation threshold gain down */
	uint16_t satThrGainUp;   /**< WDR saturation threshold gain up */
} CamDeviceWdrSaturationAdjustmentConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR halo color fading configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrHaloColorFadingConfig_s {
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
 * @brief   CamDevice WDR high light configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrHighLightConfig_s {
	float32_t hlcBaseLog;    /**< WDR HLC base log */
	uint8_t hlcSlope;    /**< WDR HLC slope */
} CamDeviceWdrHighLightConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR dump configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrDampConfig_s {
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
 * @brief   CamDevice WDR color weight configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrColorWeightConfig_s {
	uint8_t wdrRgbCoef[CAMDEV_WDR_RGB_COEF_SIZE];    /**< WDR RGB coefficient */
	uint8_t wdrLightnessWeight;    /**< WDR lightness weight */
	uint8_t wdrColorWeight[CAMDEV_WDR_COLOR_WEIGHT_SIZE];    /**< WDR color weight */
} CamDeviceWdrColorWeightConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrAutoConfig_s {
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
} CamDeviceWdrAutoConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR current configuration.
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
	CamDeviceWdrSaturationAdjustmentConfig_t
	saturationAdjustmentCfg;   /**< WDR saturation adjustment configuration */
	CamDeviceWdrHaloColorFadingConfig_t haloColorFadingCfg;   /**< WDR halo color configuration */
	CamDeviceWdrHighLightConfig_t highLightCfg;   /**< WDR high light configuration */
	bool_t lightEn;  /**< WDR light en configuration */
	CamDeviceWdrDampConfig_t dampCfg;   /**< WDR damp configuration */
	CamDeviceWdrColorWeightConfig_t colorWeightCfg;   /**< WDR color weight configuration */
} CamDeviceWdrManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrConfig_s {
	CamDeviceConfigMode_t configMode;      /**< The run mode: 0--manual, 1--auto */
	CamDeviceWdrAutoConfig_t autoCfg;      /**< WDR auto configuration*/
	CamDeviceWdrManualConfig_t manualCfg;   /**< WDR manual configuration*/
} CamDeviceWdrConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice WDR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceWdrStatus_s {
	bool_t enable;    /**< WDR enable status */
	CamDeviceConfigMode_t currentMode;      /**< The run mode: 0--manual, 1--auto */
	CamDeviceWdrManualConfig_t currentCfg;   /**< WDR current configuration*/
} CamDeviceWdrStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets WDR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the configuration of WDR
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the configuration of WDR
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets WDR gammaUp configuration parameters.
 *          When wdrCurveSelect = 3, gammaUpConfig take effect.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the configuration of WDR
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrSetGammaUpConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrGammaUpConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR gammaUp configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the configuration of WDR
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetGammaUpConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrGammaUpConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR gammaUp status parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to the configuration of WDR
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetGammaUpStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrGammaUpConfig_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function enables WDR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables the WDR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables WDR halo color fading
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrHaloColorFadingEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables WDR halo color fading
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrHaloColorFadingDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR status
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to WDR status

 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWdrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets WDR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to WDR version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets WDR. It's only available in manual mode.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWdrReset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_wdr_v5_2 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_WDR5_API_H */
