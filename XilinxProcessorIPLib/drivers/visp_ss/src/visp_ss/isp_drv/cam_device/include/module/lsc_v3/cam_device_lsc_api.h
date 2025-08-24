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
 * @cond LSC_V3
 *
 * @defgroup cam_device_lsc_v3 CamDevice LSC V3 Definitions
 * @{
 *
 */

#ifndef CAMDEV_LSC_V3_API_H
#define CAMDEV_LSC_V3_API_H

#include "cam_device_common.h"

#define CAMDEV_LSC_H_BLOCK_NUM 32U  /**< Maximum horizontal block size of LSC map */
#define CAMDEV_LSC_V_BLOCK_NUM 32U  /**< Maximum vertical block size of LSC map */
#define CAMDEV_LSC_H_POINT_NUM (CAMDEV_LSC_H_BLOCK_NUM + 1U)  /**< Maximum horizontal size of LSC map */
#define CAMDEV_LSC_V_POINT_NUM (CAMDEV_LSC_H_BLOCK_NUM + 1U)  /**< Maximum vertical size of LSC map */

/******************************************************************************/
/**
 * @brief   CamDevice LSC internal mode.
 *
 *****************************************************************************/
typedef enum CamDeviceLscInterMode_e {
	CAMDEV_LSC_GAIN_INTER = 0,    /**< LSC internal gain */
	CAMDEV_LSC_COLOR_TEMP_WEIGHT_INTER = 1,    /**< LSC internal temperature weight */
	CAMDEV_LSC_GAIN_COLOR_TEMP_WEIGHT_INTER = 2,    /**< LSC internal color temperature weight gain */
	CAMDEV_LSC_INTER_MAX,
	CAMDEV_DUMMY_075 = 0xDEADFEED
} CamDeviceLscInterMode_t;

/******************************************************************************/
/**
 * @brief   CamDevice LSC V3 current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscManualConfig_s {
	uint16_t matrix[CAMDEV_RAW_CHANNEL_NUM][CAMDEV_LSC_V_POINT_NUM][CAMDEV_LSC_H_POINT_NUM];  /**< 4 channel(r, gr, gb, b) 33*33 grids */
	uint16_t xSize[CAMDEV_LSC_H_BLOCK_NUM];                                                   /**< LSC V3 int array for x axis size */
	uint16_t ySize[CAMDEV_LSC_V_BLOCK_NUM /
		       2U];                                                 /**< LSC V3 int array for y axis size*/
} CamDeviceLscManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice LSC V3 auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscAutoConfig_s {
	uint8_t autoLevel;    /**< Auto configuration level */
	float32_t gains[CAMDEV_ISO_STRENGTH_NUM];    /**< LSC gains */
	float32_t damping;    /**< LSC damping value */
	float32_t strength[CAMDEV_ISO_STRENGTH_NUM];    /**< LSC strength */
	CamDeviceLscInterMode_t interMode;    /**< LSC internal mode */
} CamDeviceLscAutoConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice LSC V3 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLscConfig_s {
	CamDeviceConfigMode_t configMode;        /**< The run mode: 0--manual, 1--auto */
	CamDeviceLscManualConfig_t manualCfg;    /**< LSC V3 manual configuration */
	CamDeviceLscAutoConfig_t autoCfg;      /**< LSC V3 auto configuration */
} CamDeviceLscConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice LSC V3 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceLscStatus_s {
	bool_t enable;                          /**< LSC V3 enable status */
	CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
	CamDeviceLscManualConfig_t currentCfg;    /**< LSC V3 current configuration */
} CamDeviceLscStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets LSC V3 configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pLscCfg             Pointer to LSC V3 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceLscConfig_t *pLscCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets the LSC V3 configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pLscCfg             Pointer to LSC V3 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceLscConfig_t *pLscCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables LSC V3.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables LSC V3.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets LSC V3.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets LSC V3 status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to LSC V3 status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceLscStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets LSC V3 version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to LSC V3 version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLscGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_lsc_v3 */
/* @endcond */

#endif   // CAMDEV_LSC_V3_API_H
