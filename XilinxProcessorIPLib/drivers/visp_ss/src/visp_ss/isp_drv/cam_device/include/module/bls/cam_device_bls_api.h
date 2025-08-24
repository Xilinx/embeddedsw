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

#ifndef CAMDEV_BLS_API_H
#define CAMDEV_BLS_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @cond BLS
 *
 * @defgroup cam_device_bls CamDevice BLS Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   CamDevice BLS auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsAutoConfig_s {
	uint8_t autoLevel;                                          /**< The auto configuration level */

	float32_t gains[CAMDEV_ISO_STRENGTH_NUM];                       /**< BLS gains */
	uint32_t bls[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_RAW_CHANNEL_NUM];  /**< BLS values */
} CamDeviceBlsAutoConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice BLS manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsManualConfig_s {
	uint32_t bls[CAMDEV_RAW_CHANNEL_NUM];       /**< BLS value:
                                                 Raw : BLS[0]--red, BLS[1]--greenRed, BLS[2]--greenBlue, BLS[3]--blue. The bls order shold be corresponding with sensor bayer pattern\n
                                                 Rgbir: BLS[0]--red, BLS[1]--green, BLS[2]--blue, BLS[3]--ir. Algorithm ensure the order: 0->r, 1->g, 2->b, 3->ir, bls don't need to correspond with sensor bayer pattern */
} CamDeviceBlsManualConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice BLS configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsConfig_s {
	CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceBlsAutoConfig_t autoCfg;       /**< BLS auto configuration*/
	CamDeviceBlsManualConfig_t manualCfg;   /**< BLS manual configuration*/
} CamDeviceBlsConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice BLS status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsStatus_s {
	bool_t enable;              /**< BLS enable status*/
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceBlsManualConfig_t currentCfg;   /**< BLS current configuration*/
} CamDeviceBlsStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets BLS configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pBlsCfg             Pointer to BLS configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsConfig_t *pBlsCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pBlsCfg             Pointer to BLS configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsConfig_t *pBlsCfg
);

/*****************************************************************************/
/**
 * @brief   This function sets BLS bitWidth parameters.
 * When the api is not called, 12 bits are used by default
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   bitWidth             bitWidth value
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsSetBitWidth
(
	CamDeviceHandle_t hCamDevice,
	uint8_t blsBitWidth
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   bitWidth             Pointer to bitWidth
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetBitWidth
(
	CamDeviceHandle_t hCamDevice,
	uint8_t *blsBitWidth
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to BLS status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets BLS. It's only available in manual mode.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to BLS version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_bls */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_BLS_API_H
