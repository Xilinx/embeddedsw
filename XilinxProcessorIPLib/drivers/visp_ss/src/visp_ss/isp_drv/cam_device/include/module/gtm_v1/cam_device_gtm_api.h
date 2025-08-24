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
 * @cond GTM_V1
 *
 * @defgroup cam_device_gtm CamDevice GTM Definitions
 * @{
 *
 */
#ifndef CAMDEV_GTM_API_H
#define CAMDEV_GTM_API_H


#include "cam_device_common.h"


#define CAMDEV_GTM_RGB_COEF_SIZE       3
#define CAMDEV_GTM_COLOR_WEIGHT_SIZE   3
#define CAMDEV_GTM_HIST_LUMA_SIZE      4
#define CAMDEV_GTM_USER_CURVE_SIZE     129
#define CAMDEV_GTM_HIST_BINS           128


/******************************************************************************/
/**
 * @brief   CamDevice GTM curve mode.
 *
 *****************************************************************************/
typedef enum CamDeviceGtmCurveMode_e {
	CAMDEV_GTM_INTERPOLATION_CURVE_MODE = 0,    /**< Interpolation mode */
	CAMDEV_GTM_HISTOGRAM_CURVE_MODE = 1,    /**< Histogram mode */
	CAMDEV_GTM_INTERPOLATION_HISTOGRAM_CURVE_MODE = 2,    /**< Interpolation and histogram mode */
	CAMDEV_GTM_CURVE_MODE_MAX,
	CAMDEV_DUMMY_068 = 0xDEADFEED
} CamDeviceGtmCurveMode_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM curve select.
 *
 *****************************************************************************/
typedef enum CamDeviceGtmCurveSelect_e {
	CAMDEV_GTM_LINEAR_SHIFT = 0,    /**< Linear shift mode */
	CAMDEV_GTM_PIECE_WISE_LINEAR = 1,    /**< Piece wise linear mode*/
	CAMDEV_GTM_LOGARITHM_COMPRESSION = 2,    /**< Logarithm compression mode */
	CAMDEV_GTM_USER_CURVE = 3,    /**< User curve mode */
	CAMDEV_GTM_CURVE_MAX,
	CAMDEV_DUMMY_069 = 0xDEADFEED
} CamDeviceGtmCurveSelect_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM black white correction mode.
 *
 *****************************************************************************/
typedef enum CamDeviceBlackWhiteCorMode_e {
	CAMDEV_BW_COR_MODE0 = 0,           /**<Manual mode*/
	CAMDEV_BW_COR_MODE1 = 1,           /**<Black white correction auto mode use previous frame statistics */
	CAMDEV_BW_COR_MODE2 = 2,           /**<White correction auto mode*/
	CAMDEV_BW_COR_MODE_MAX,                     /**<Maximum border */
	CAMDEV_DUMMY_070 = 0xDEADFEED
} CamDeviceBlackWhiteCorMode_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM linear shift bit.
 *
 *****************************************************************************/
typedef enum CamDeviceLinearShiftBit_e {
	CAMDEV_SHIFT_BIT_0 = 0,           /**<Output 0-19 bit*/
	CAMDEV_SHIFT_BIT_1 = 1,           /**<Output 1-20 bit*/
	CAMDEV_SHIFT_BIT_2 = 2,           /**<Output 2-21bit*/
	CAMDEV_SHIFT_BIT_3 = 3,           /**<Output 3-22bit*/
	CAMDEV_SHIFT_BIT_4 = 4,           /**<Output 4-23bit*/
	CAMDEV_SHIFT_BIT_MAX,                     /**<Maximum border */
	CAMDEV_DUMMY_071 = 0xDEADFEED
} CamDeviceLinearShiftBit_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM histogram state structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmHistogram_s {
	uint32_t min;    /**< Minimum value */
	uint32_t max;    /**< Maximum value */
	uint32_t hist[CAMDEV_GTM_HIST_BINS];    /**< Histogram array */
} CamDeviceGtmHistogram_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM color weight structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmColorWeightConfig_s {
	uint8_t gtmRgbCoef[CAMDEV_GTM_RGB_COEF_SIZE];    /**< Rgb coefficient */
	uint8_t gtmLightnessWeight;    /**< Lightness weight */
	uint8_t gtmColorWeight[CAMDEV_GTM_COLOR_WEIGHT_SIZE];    /**< Color weight */
} CamDeviceGtmColorWeightConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM black white correction configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmBlackWhiteCorrectionConfig_s {
	CamDeviceBlackWhiteCorMode_t bwcorMode;    /**< Mode value */
	float64_t bwcorMinLog;    /**< Black white Correction min log */
	float64_t bwcorMaxLog;    /**< Black white Correction max log */
	float64_t bwcorDampCoef;    /**< Black white Correction damp coefficient */
} CamDeviceGtmBlackWhiteCorrectionConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM curve configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmCurveConfig_s {
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
 * @brief   CamDevice GTM curve histogram information.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmCurveInfoConfig_s {
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
 * @brief   CamDevice GTM current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmManualConfig_s {
	CamDeviceGtmColorWeightConfig_t colorWeightCfg;    /**< Color weight configuration */
	CamDeviceGtmBlackWhiteCorrectionConfig_t
	blackWhiteCorrectionCfg;    /**< Black white correction configuration */
	CamDeviceGtmCurveConfig_t curveCfg;    /**< Curve configuration */
} CamDeviceGtmManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmAutoConfig_s {
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
 * @brief   CamDevice GTM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmConfig_s {
	CamDeviceConfigMode_t configMode;    /**< GTM mode configuration */
	CamDeviceGtmAutoConfig_t autoCfg;    /**< GTM auto configuration */
	CamDeviceGtmManualConfig_t manualCfg;    /**< GTM manual configuration */
} CamDeviceGtmConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GTM status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGtmStatus_s {
	bool_t enable;    /**< GTM enable status */
	bool_t bwCorrectEnable;    /**< GTM black white correction status */
	CamDeviceConfigMode_t currentMode;    /**< GTM mode configuration */
	CamDeviceGtmManualConfig_t currentCfg;    /**< GTM current configuration */
} CamDeviceGtmStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets GTM configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to GTM configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGtmConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to GTM configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGtmConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables GTM
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GTM
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables GTM black white correction
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmBlackWhiteCorrectionEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GTM black white correction
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmBlackWhiteCorrectionDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get GTM black white correction enable config
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pBwCorEnable        The BwCorEnable config
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
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
 *
 * @param   hCamDevice        Handle to the CamDevice instance
 * @param   pStatus           Pointer to GTM status


 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGtmStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to GTM version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets GTM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGtmReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GTM histogram.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   phist               Pointer to GTM histogram
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT CamDeviceGtmGetHist
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGtmHistogram_t *phist
);


/* @} cam_device_gtm */
/* @endcond */

#endif
