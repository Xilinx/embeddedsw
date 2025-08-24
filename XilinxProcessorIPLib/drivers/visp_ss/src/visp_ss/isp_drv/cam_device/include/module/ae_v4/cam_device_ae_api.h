/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @cond AE_V4
 *
 * @defgroup cam_device_ae_v4 CamDevice AE V4 Definitions
 * @{
 *
 */

#define CAMDEV_AE_HIST_NUM_BINS           256       /**< Bin numbers of AE histogram */
#define CAMDEV_AE_EXP_GRID_ITEMS          (32*32)   /**< Number of grid items (see @ref CamerIcMeanLuma_t) */
#define CAMDEV_AE_EXP_TABLE_NUM           8        /**< The number of AEV4 exposure tables */
#define AE_DEVICE_RETENTION_VALUE_MAX      500

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/**
 * @brief    CamDevice AE status.
 *
 *****************************************************************************/
typedef enum CamDeviceAeState_e {
	CAMDEV_AE_STATE_INVALID = 0,    /**< Invalid AE status */
	CAMDEV_AE_STATE_INITIALIZED = 1,    /**< AE initialized */
	CAMDEV_AE_STATE_STOPPED = 2,    /**< AE stopped */
	CAMDEV_AE_STATE_RUNNING = 3,    /**< AE running */
	CAMDEV_AE_STATE_LOCKED = 4,    /**< AE locked */
	CAMDEV_AE_STATE_MAX,
	CAMDEV_DUMMY_045 = 0xDEADFEED
} CamDeviceAeState_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AE scene evaluation mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAeSemMode_t {
	CAMDEV_AE_SCENE_EVALUATION_FIX = 0,    /**< Fixed evaluation */
	CAMDEV_AE_SCENE_EVALUATION_ADAPTIVE = 1,    /**< Adaptive evaluation */
	CAMDEV_AE_SCENE_METADATA = 2,    /**< metadata mode */
	CAMDEV_AE_SCENE_EVALUATION_MAX,
	CAMDEV_DUMMY_046 = 0xDEADFEED
} CamDeviceAeSemMode_t;

/*****************************************************************************/
/**
 * @brief    CamDevice AE Flicker period.
 *
 *****************************************************************************/
typedef enum CamDeviceAeFlickerPeriod_e {
	CAMDEV_AE_FLICKER_PERIOD_OFF = 0,    /**< Flicker period off */
	CAMDEV_AE_FLICKER_PERIOD_50Hz = 1,    /**< Flicker period 50Hz */
	CAMDEV_AE_FLICKER_PERIOD_60Hz = 2,    /**< Flicker period 60Hz */
	CAMDEV_AE_FLICKER_PERIOD_USER_DEFINED = 3,  /**< User-defined Flicker period */
	CAMDEV_AE_FLICKER_PERIOD_MAX,
	CAMDEV_DUMMY_047 = 0xDEADFEED
} CamDeviceAeFlickerPeriod_t;

/*****************************************************************************/
/**
 * @brief    CamDevice AE front ground type.
 *
 *****************************************************************************/
typedef enum CamDeviceAeFrontGroundType_s {
	CAMDEV_AE_FRONT_MODE = 0,    /**< AE front mode */
	CAMDEV_AE_FACE_MODE = 1,    /**< AE face mode */
	CAMDEV_AE_TOUCH_MODE = 2,    /**< AE touch mode */
	CAMDEV_AE_FRONT_MODE_MAX,
	CAMDEV_DUMMY_048 = 0xDEADFEED
} CamDeviceAeFrontGroundType_t;

/*****************************************************************************/
/**
 * @brief    CamDevice AE performance optimization mode.
 *
 *****************************************************************************/
typedef enum CamDeviceAePerformanceOptiMode_s {
	CAMDEV_AE_PERFORMANCE_NO_OPTIMIZATION = 0,    /**< No optimization mode */
	CAMDEV_AE_PERFORMANCE_GENERAL_OPTIMIZATION = 1,    /**< General optimization mode */
	CAMDEV_AE_PERFORMANCE_FAST_OPTIMIZATION = 2,    /**< Fast optimization mode */
	CAMDEV_DUMMY_049 = 0xDEADFEED
} CamDeviceAePerformanceOptiMode_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AE Flicker Mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAeFlickerMode_s {
	CamDeviceAeFlickerPeriod_t flickerPeriod;    /**< Flicker period instance */
	float32_t userDefinedPeriodus; /**< User defined period which is only valid in user define mode. */
} CamDeviceAeFlickerMode_t;

/*****************************************************************************/
/**
 * @brief    CamDevice AE external light type.
 *
 *****************************************************************************/
typedef enum CamDeviceAeExLightType_e {
	CAMDEV_AE_IR_LIGHT = 0,    /**< IR lamp */
	CAMDEV_AE_MAX = 1,
	CAMDEV_DUMMY_050 = 0xDEADFEED
} CamDeviceAeExLightType_t;

/******************************************************************************/
/**
 * @brief   CamDevice AE configuration.
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
	CamDeviceAePerformanceOptiMode_t performanceOptiMode;  /** Performance optimization mode */
	bool_t
	exLightEnable;       /** user api for controlling whether to enable enternal light source*/
	CamDeviceAeExLightType_t exLightType;         /** the type of external light*/
	float32_t
	maxExpForExlight;    /** the total exposure threshold for turning on/off the external light*/
	float32_t minExpForExlight;
} CamDeviceAeConfig_t;


/*****************************************************************************/
/**
 * @brief   CamDevice AE mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAeMode_s {
	CamDeviceAeSemMode_t semMode;    /**< AE scene evaluation mode */
	CamDeviceAeFlickerMode_t antiFlickerMode;    /**< AE flicker mode */
} CamDeviceAeMode_t;


/*****************************************************************************/
/**
 * @brief   Cam Device AE result
 *
 *****************************************************************************/
typedef struct CamDeviceAeResult_s {
	float32_t meanluma;    /**< AE mean luminance */
	float32_t lumaIndex;
} CamDeviceAeResult_t;


/*****************************************************************************/
/**
 * @brief    Cam Device AE histogram grid.
 *
 *****************************************************************************/
typedef uint32_t CamDeviceAeHistBins_t[CAMDEV_AE_HIST_NUM_BINS];


/*****************************************************************************/
/**
 * @brief    CamDevice AE luminance grid.
 *
 *****************************************************************************/
typedef uint8_t CamDeviceAeMeanLuma_t[CAMDEV_AE_EXP_GRID_ITEMS];

/*****************************************************************************/
/**
 * @brief    CamDevice AE object region grid.
 *
 *****************************************************************************/
typedef uint8_t CamDeviceAeObjectRegion_t[CAMDEV_AE_EXP_GRID_ITEMS];

/*****************************************************************************/
/**
 * @brief    CamDevice exposure statistics table.
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
 * @brief    CamDevice exposure table.
 *
 *****************************************************************************/
typedef struct CamDeviceExpTable_s {
	CamDeviceEsTable_t expTable[CAMDEV_AE_EXP_TABLE_NUM];     /**< Exposure table */
	uint8_t expTableNum;    /**< The number of exposure table */
} CamDeviceExpTable_t;

/*****************************************************************************/
/**
 * @brief    CamDevice AE front ground configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAeFrontGroundConfig_s {
	CamDeviceAeFrontGroundType_t aeFrontGroundType;    /**< AE front ground type */
	float32_t aeFaceWeight;     /**< AE face weight */
	float32_t aeTouchWeight;    /**< AE touch wieght */
} CamDeviceAeFrontGroundConfig_t;

/*****************************************************************************/
/**
 * @brief   This function registers the AE library.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pAeLibHandle        Pointer to the AE library
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceRegisterAeLib
(
	CamDeviceHandle_t hCamDevice,
	void *pAeLibHandle
);

/*****************************************************************************/
/**
 * @brief   This function unregisters the AE library.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceUnRegisterAeLib
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the AE configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the AE configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeConfig_t *pConfig
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets the AE mode parameters.
 *
 * @param   hCamDevice            Handle to the CamDevice instance
 * @param   pAeMode               Pointer to the AE mode
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeSetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMode_t *pAeMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets the AE mode parameters.
 *
 * @param   hCamDevice            Handle to the CamDevice instance
 * @param   pAeMode               Pointer to the AE mode
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMode_t *pAeMode
);

/*****************************************************************************/
/**
 * @brief   This function sets the AE ROI.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pRoi                Pointer to the AE ROI configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeSetRoi
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceRoi_t *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE ROI.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pRoi                Pointer to the AE ROI configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetRoi
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRoi_t *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets AE result
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pCamDeviceAeResult  The pointer to AE front ground configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_INVALID_PARM    Invalid configuration
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetResult
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeResult_t *pCamDeviceAeResult
);


/*****************************************************************************/
/**
 * @brief   This function enables AE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_OUTOFRANGE      A configuration parameter is out of range
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeEnable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function disables AE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeDisable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function resets AE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeReset
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets the AE status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pAeStatus           Pointer to the AE status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeState_t *pAeStatus
);


/*****************************************************************************/
/**
 * @brief   This function gets the current AE histogram.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pHistogram          Pointer to the histogram bins
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetHistogram
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeHistBins_t *pHistogram
);

/*****************************************************************************/
/**
 * @brief   This function returns the current AE luminance grid.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pLuma               Pointer to the luminance grid
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetLuminance
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMeanLuma_t *pLuma
);

/*****************************************************************************/
/**
 * @brief   This function gets the current AE control object region.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pObjectRegion       Pointer to the object region
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetObjectRegion
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeObjectRegion_t *pObjectRegion
);

/*****************************************************************************/
/**
 * @brief   This function sets the exposure table to engine layer.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pExpTable           Pointer to the exposure table.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeSetExpTable
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceExpTable_t *pExpTable
);

/*****************************************************************************/
/**
 * @brief   This function gets the exposure table from engine layer.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pExpTable           Pointer to the exposure table.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetExpTable
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpTable_t *pExpTable
);

/*****************************************************************************/
/**
 * @brief   This function gets the AE version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to the AE version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAeGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);


/* @} cam_device_ae_v4 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_AE_API_H
