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

#ifndef CAMDEV_CPD_API_H
#define CAMDEV_CPD_API_H

#include "cam_device_common.h"

/**
 * @cond CPD
 *
 * @defgroup cam_device_cpd CamDevice CPD Definitions
 * @{
 *
 */

#define CAMDEV_CPD_CURVE_X_SIZE  64 /**< The number of coordinates CPD curve on the X-axis 64*/
#define CAMDEV_CPD_CURVE_Y_SIZE  64 /**< The number of coordinates CPD curve on the Y-axis 64*/

#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice CPD type enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceCpdType_e {
	CAMDEV_CPD_TYPE_INAVLID,    /**< CPD module mode is invalid*/
	CAMDEV_CPD_TYPE_EXPAND,     /**< CPD module working on expand mode */
	CAMDEV_DUMMY_054 = 0xDEADFEED
} CamDeviceCpdType_t;


/******************************************************************************/
/**
 * @brief   CamDevice expand configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpandConfig_s {
	uint32_t expandCurveX[CAMDEV_CPD_CURVE_X_SIZE];      /**< The X-coordinates of expand curve */
	uint32_t expandCurveY[CAMDEV_CPD_CURVE_Y_SIZE];     /**< The Y-coordinates of expand curve */
	bool_t expandUseOutYcurve;                          /**< If set to 0, use default expand curve */
} CamDeviceExpandConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CPD configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCpdConfig_s {
	CamDeviceExpandConfig_t expandCfg;        /**< The expand curve configuration */
} CamDeviceCpdConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CPD status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCpdStatus_s {
	bool_t enable;              /**< CPD enable status*/
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceExpandConfig_t currentCfg;        /**< The expand curve current configuration */
} CamDeviceCpdStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets CPD configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   cpdType             CPD type
 * @param   pCpdCfg             Pointer to CPD configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCpdType_t cpdType,
	const CamDeviceCpdConfig_t *pCpdCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CPD configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   cpdType             CPD type
 * @param   pCpdCfg             Pointer to CPD configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCpdType_t cpdType,
	CamDeviceCpdConfig_t *pCpdCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables CPDExpand.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdExpandEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CPDExpand.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdExpandDisable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets CPDExpand status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus          Pointer to CPD status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdExpandGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCpdStatus_t *pStatus
);


/*****************************************************************************/
/**
 * @brief   This function gets CPDCompress status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pCpdStatus          Pointer to CPD status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdCompressGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCpdStatus_t *pCpdStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CPD. It's only available in manual mode.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPD version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to CPD version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCpdGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);
/* @} cam_device_cpd */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_CPD_API_H
