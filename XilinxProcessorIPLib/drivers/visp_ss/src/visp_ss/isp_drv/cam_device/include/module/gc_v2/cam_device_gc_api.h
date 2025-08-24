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

#ifndef CAMDEV_GC_V2_API_H
#define CAMDEV_GC_V2_API_H

#include "cam_device_common.h"

/**
 * @cond GC_V2
 *
 * @defgroup cam_device_gc_v2 CamDevice GC V2 Definitions
 * @{
 *
 */

#define CAMDEV_GC_CURVE_SIZE 64   /**< Curve size */


#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice GC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcAutoConfig_s {
	uint8_t autoLevel;                                               /**< GC auto configuration level */

	float32_t gain[CAMDEV_ISO_STRENGTH_NUM];                             /**< GC gain */
	uint16_t curve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve */
} CamDeviceGcAutoConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GC current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcManualConfig_s {
	bool_t standard;                        /**< True/false: true for standard_val, false for curve */
	float32_t standardVal;                      /**< Generate uniform curve with gamma formula */
	uint16_t curve[CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve */
	bool_t userCurveX;   /**< User curve on X axis */
	uint32_t curvePx[CAMDEV_GC_CURVE_SIZE];   /**< Gamma curve Px */
} CamDeviceGcManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGcConfig_s {
	CamDeviceConfigMode_t mode;           /**< GC mode configuration */
	CamDeviceGcAutoConfig_t autoCfg;      /**< GC auto configuration */
	CamDeviceGcManualConfig_t manualCfg;  /**< GC manual configuration */
} CamDeviceGcConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice GC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGcStatus_s {
	bool_t enable;            /**< GC enable status */
	CamDeviceConfigMode_t currentMode;           /**< The run mode: 0--manual, 1--auto */
	CamDeviceGcManualConfig_t currentCfg;  /**< GC current configuration */
} CamDeviceGcStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets GC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pGcCfg              Pointer to GC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGcConfig_t *pGcCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets GC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pGcCfg              Pointer to GC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGcConfig_t *pGcCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables GC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets GC status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to GC status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceGcStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets GC. It's only available in manual mode.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the GC version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to GC version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGcGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_gc_v2 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_GC_V2_API_H
