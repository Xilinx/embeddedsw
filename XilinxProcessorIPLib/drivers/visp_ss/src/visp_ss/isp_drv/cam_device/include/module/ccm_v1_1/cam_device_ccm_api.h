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

#ifndef CAMDEV_CCM_V1_1_API_H
#define CAMDEV_CCM_V1_1_API_H

#include "cam_device_common.h"

/**
 * @cond CCM_V1_1
 *
 * @defgroup cam_device_ccm_v1_1 CamDevice CCM V1.1 Definitions
 * @{
 *
 */

#define CAMDEV_CC_MATRIX_SIZE       9U      /**< Color correction matrix size 3x3*/
#define CAMDEV_CC_COLOR_CHANNEL_NUM 3U      /**< Color channel number 3*/


#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice CCM manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmManualConfig_s {
	float32_t ccMatrix[CAMDEV_CC_MATRIX_SIZE];          /**< Color correction matrix coefficient*/
	float32_t ccOffset[CAMDEV_CC_COLOR_CHANNEL_NUM];    /**< Color offset coefficient*/
} CamDeviceCcmManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CCM configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmConfig_s {
	CamDeviceCcmManualConfig_t manualCfg;   /**< CCM manual configuration*/
	CamDeviceConfigMode_t configMode;
} CamDeviceCcmConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice CCM status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCcmStatus_s {
	bool_t enable;              /**< CCM enable status*/
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceCcmManualConfig_t currentCfg;   /**< CCM current configuration*/
} CamDeviceCcmStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets CCM configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pCcmCfg             Pointer to CCM configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceCcmConfig_t *pCcmCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CCM configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pCcmCfg             Pointer to CCM configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCcmConfig_t *pCcmCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables CCM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CCM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmDisable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets CCM status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus          Pointer to CCM status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCcmStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CCM.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CCM version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to CCM version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCcmGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_ccm_v1_1 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_CCM_V1_1_API_H
