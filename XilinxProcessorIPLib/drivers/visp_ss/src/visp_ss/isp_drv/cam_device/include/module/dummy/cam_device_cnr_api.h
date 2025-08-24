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
 *
 * @cond CNR_DUMMY
 * @defgroup cam_device_cnr_api CamDevice CNR Dummy API
 * @{
 *
 */

#ifndef CAMDEV_CNR_API_H
#define CAMDEV_CNR_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamDeviceCnrConfig_s {
	uint8_t nop;
} CamDeviceCnrConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CNR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCnrStatus_s {
	bool_t enable;    /**< EE enable status */
} CamDeviceCnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets the CNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pEeCfg              This pointer of CNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets the CNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pEeCfg              This pointer of CNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function enable denoise of C channel.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disable denoise of C channel.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get CNR status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             pointer to the CNR configuration
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get CNR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            pointer to the CNR version
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_cnr_api dummy*/
/* @endcond */

#endif /* CAMDEV_CNR_API_H */
