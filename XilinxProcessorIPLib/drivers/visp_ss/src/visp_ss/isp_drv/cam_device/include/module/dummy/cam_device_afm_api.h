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
 * @cond AFM_DUMMY
 *
 * @defgroup cam_device_afm_dummy CamDevice AFM Dummy Definitions
 * @{
 *
 */

#ifndef CAMDEV_AFM_API_H
#define CAMDEV_AFM_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cam_device_common.h"


/******************************************************************************/
/**
 *
 * @brief   Enumeration type to identify the Autofocus measuring window.
 *
 *****************************************************************************/
typedef enum CamDeviceAfmWindowId_e {
	CAMDEV_ISP_AFM_WINDOW_INVALID = 0,    /**< lower border (only for an internal evaluation) */
	DUMMY_CAMDEV_0058 = 0xdeadfeed
} CamDeviceAfmWindowId_t;


/******************************************************************************/
/**
 * @brief   Enumeration type to identify the Autofocus measuring instance.
 *
 *****************************************************************************/
typedef enum CamDeviceIspAfmInstanceId_e {
	CAMDEV_ISP_AFM_INSTANCE_INVALID = 0,    /**< Lower border (only for an internal evaluation) */
	DUMMY_CAMDEV_0059 = 0xdeadfeed
} CamDeviceIspAfmInstanceId_t;


/*****************************************************************************/
/**
 *
 * @brief   CamDevice AFM measure results structure.
 *
 */
/*****************************************************************************/
typedef struct CamDeviceAfmMeasureResult_s {
	uint8_t nop;
} CamDeviceAfmMeasureResult_t;


/*****************************************************************************/
/**
 * @brief   CamDevice AF status structure
 *
 *****************************************************************************/
typedef struct CamDeviceAfmStatus_s {
	bool_t enable;  /**< AFM enable*/
} CamDeviceAfmStatus_t;


/*****************************************************************************/
/**
 *
 * @brief   This function sets the threshold in the AFM module.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   threshold      Threshold value
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmSetThreshold
(
	CamDeviceHandle_t hCamDevice,
	const uint32_t threshold

);


/*****************************************************************************/
/**
 *
 * @brief   This function gets the threshold in the AFM module.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   threshold      Threshold value pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetThreshold
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pThreshold
);


/*****************************************************************************/
/**
 *
 * @brief   This function get the AFM statistic result.
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pResult          Measure results pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetResult
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmMeasureResult_t *pResult
);


/*****************************************************************************/
/**
 *
 * @brief   This function sets the AFM statistics window
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   AfmWinId     AFM window item index
 * @param   pWindow      Pointer to window size
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmSetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmWindowId_t afmWinId,
	CamDeviceWindow_t *pWindow
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets the AFM statistics window
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   AfmWinId     AFM window item index
 * @param   pWindow      Pointer to window size
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmWindowId_t afmWinId,
	CamDeviceWindow_t *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function enables AFM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AFM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AFM status.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pStatus        Pointer to AFM status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfmStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets AFM version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to AFM version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);


/*****************************************************************************/
/**
 * @brief   This function resets AFM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfmReset
(
	CamDeviceHandle_t hCamDevice
);


#ifdef __cplusplus
}
#endif

/* @} cam_device_afm_dummy */
/* @endcond */

#endif /* CAMDEV_AFM_DUMMY_API_H */
