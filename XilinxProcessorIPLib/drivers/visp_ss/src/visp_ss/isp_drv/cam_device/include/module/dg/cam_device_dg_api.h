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
 * @cond DG
 *
 * @defgroup cam_device_dg CamDevice DG Definitions
 * @{
 *
 */

#ifndef CAMDEV_DG_API_H
#define CAMDEV_DG_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice DG manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDgManualConfig_s {
	float32_t digitalGainB;         /**< Blue DG */
	float32_t digitalGainGb;        /**< Green blue DG */
	float32_t digitalGainGr;        /**< Green red DG */
	float32_t digitalGainR;         /**< Red DG */
} CamDeviceDgManualConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DG configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDgConfig_s {
	CamDeviceDgManualConfig_t manualCfg;        /**< DG manual configuration*/
} CamDeviceDgConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DG status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDgStatus_s {
	bool_t enable;              /**< DG enable status*/
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceDgManualConfig_t currentCfg;        /**< DG current configuration*/
} CamDeviceDgStatus_t;

/***************************************************/


/*****************************************************************************/
/**
 * @brief   This function sets DG configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDgCfg              Pointer to DG configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceDgConfig_t *pDgCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets DG configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDgCfg              Pointer to DG configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDgConfig_t *pDgCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables DG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgDisable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets DG status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus           Pointer to DG status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDgStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DG version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to DG version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDgGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_dg */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_DG_API_H
