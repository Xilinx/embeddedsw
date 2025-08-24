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
 * @cond DNR3_DUMMY
 * @defgroup cam_device_3dnr_v3_1_api CamDevice ISP API
 * @{
 *
 */

#ifndef CAMDEV_3DNR_API_H
#define CAMDEV_3DNR_API_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamDevice3DnrConfig_s {
	uint8_t nop;
} CamDevice3DnrConfig_t;

typedef struct CamDevice3DnrStatus_s {
	uint8_t nop;
} CamDevice3DnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets 3DNR V3.1 configuration parameters
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Configuration of 3DNR V3.1
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function returns the 3DNR V3.1 status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Configuration of 3DNR V3.1
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrConfig_t *pConfig
);


/*****************************************************************************/
/**
 * @brief   This function enable the 3DNR V3.1
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disable the 3DNR V3.1
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get the status of 3DNR V3.1
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pSensorMode         sensor mode
 * @param   pMode               running mode
 * @param   pCfg                3DNR V3.1 status

 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDevice3DnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets 3DNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets 3DNR v31 version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            This pointer of 3DNR v31 version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_3dnr_api dummy*/
/* @endcond */

#endif /* CAMDEV_3DNR_API_H */
