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

#ifndef CAMDEV_PDAF_API_H
#define CAMDEV_PDAF_API_H

#include "cam_device_common.h"

/**
 * @cond PDAF_V1
 *
 * @defgroup cam_device_pdaf_v1 CamDevice PDAF Definitions
 * @{
 *
 */

#define CAMDEV_PDAF_RECT_ARRAY_SIZE   4     /**< Maximum size of PDAF rectangle array */
#define CAMDEV_PDAF_NUM_PER_AERA      2     /**< The number of area per frame */
#define CAMDEV_PDAF_PD_AERA_NUM       4     /**< The number of PD area */
#define CAMDEV_PDAF_SHIFT_LR_NUM      2     /**< The number of LR shift */
#define CAMDEV_PDAF_SHIFT_MARK_NUM    32    /**< The number of shift mark */


#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice PDAF Bayer pattern enumeration.
 *
 *****************************************************************************/
typedef enum CamDevicePdafBayerPattern_e {
	CAMDEV_PDAF_BAYER_PATTERN_RGGB = 0,      /**< RGGB PDAF Bayer pattern */
	CAMDEV_PDAF_BAYER_PATTERN_GRBG,          /**< GRBG PDAF Bayer pattern */
	CAMDEV_PDAF_BAYER_PATTERN_GBRG,          /**< GBRG PDAF Bayer pattern */
	CAMDEV_PDAF_BAYER_PATTERN_BGGR,          /**< BGGR PDAF Bayer pattern */
	CAMDEV_PDAF_BAYER_PATTERN_MAX,
	DUMMY_CAMDEV_0072 = 0xdeadfeed
} CamDevicePdafBayerPattern_t;

/******************************************************************************/
/**
 * @brief   CamDevice PDAF sensor type enumeration.
 *
 *****************************************************************************/
typedef enum CamDevicePdafSensorType_e {
	CAMDEV_PDAF_SENSOR_TYPE_DUAL_PIXEL = 0,    /**< Dual pixel sensor type */
	CAMDEV_PDAF_SENSOR_TYPE_OCL2X1,            /**< OCL2x1 sensor type */
	CAMDEV_PDAF_SENSOR_TYPE_MAX,
	DUMMY_CAMDEV_0073 = 0xdeadfeed
} CamDevicePdafSensorType_t;

/******************************************************************************/
/**
 * @brief   CamDevice PDAF internal configuration.
 *
 *****************************************************************************/
typedef struct CamDevicePdafInternalConfig_s {
	CamDevicePdafBayerPattern_t bayerPattern;    /**< Bayer pattern */
	uint32_t
	correctRect[CAMDEV_PDAF_RECT_ARRAY_SIZE];    /**< Rectangle correction */
	uint32_t imageHeight;    /**< Image height */
	uint32_t imageWidth;    /**< Image width */
	uint8_t
	numPerArea[CAMDEV_PDAF_NUM_PER_AERA];    /**< The number of per area */
	uint16_t pdArea[CAMDEV_PDAF_PD_AERA_NUM];    /**< PD area */
	CamDevicePdafSensorType_t sensorType;    /**< Sensor type */
	bool_t ocl2x1Shield;    /**< OCL2X1 shield */
	uint8_t shiftLr[CAMDEV_PDAF_SHIFT_LR_NUM];    /**< LR shift */
	uint8_t shiftMark[CAMDEV_PDAF_SHIFT_MARK_NUM];    /**< Shift mark */
} CamDevicePdafInternalConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice PDAF current configuration.
 *
 *****************************************************************************/
typedef struct CamDevicePdafManualConfig_s {
	uint8_t correctLChangeDown;   /**< The max change of down limit for corrected L */
	uint8_t correctLChangeUp;     /**< The max change of up limit for corrected L */
	uint8_t correctLChannel;      /**< Color filter channel for corrected L */
	uint8_t correctLLimitBase;    /**< The base ratio of change limit for corrected L */
	uint8_t correctRChangeDown;   /**< The max change of down limit for corrected R */
	uint8_t correctRChangeUp;     /**< The max change of up limit for corrected R */
	uint8_t correctRChannel;      /**< Color filter channel for corrected R */
	uint8_t correctRLimitBase;    /**< The base ratio of change limit for corrected R */
	uint8_t correctThreshold;     /**< This threshold to judge iso/aniso */
	uint32_t dummyLineHw;         /**< Hardware dummy lines */
	uint16_t roi[CAMDEV_PDAF_RECT_ARRAY_SIZE];      /**< ROI(only PD data in ROI is written to memory): ROI start width, ROI start height, ROI width, ROI height */

	CamDevicePdafInternalConfig_t internalPdaf; /**< Only use for load image */
} CamDevicePdafManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice PDAF configuration.
 *
 *****************************************************************************/
typedef struct CamDevicePdafConfig_s {
	CamDevicePdafManualConfig_t manualCfg;  /**< PDAF current configuration */
} CamDevicePdafConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice PDAF status structure.
 *
 *****************************************************************************/
typedef struct CamDevicePdafStatus_s {
	bool_t enable;          /**< PDAF enable status */
	bool_t correctEnable;   /**< PDAF correct enable status */
	CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
	CamDevicePdafManualConfig_t currentCfg;  /**< PDAF current configuration */
} CamDevicePdafStatus_t;


//*****************************************************************************/
/**
 * @brief   This function sets PDAF configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pPdafCfg            Pointer to PDAF configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePdafConfig_t *pPdafCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets PDAF configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pPdafCfg            Pointer to PDAF configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePdafConfig_t *pPdafCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets PDAF status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pPdafStatus         Pointer to PDAF status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePdafStatus_t *pPdafStatus
);

/*****************************************************************************/
/**
 * @brief   This function enables PDAF correction.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafCorrectEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables PDAF correction.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafCorrectDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets PDAF version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to PDAF version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_pdaf_v1 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_PDAF_API_H
