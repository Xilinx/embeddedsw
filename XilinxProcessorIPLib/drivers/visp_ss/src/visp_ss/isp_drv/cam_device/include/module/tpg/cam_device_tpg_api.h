/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2023 Vivante Corporation
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

#ifndef CAMDEV_TPG_API_H
#define CAMDEV_TPG_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @cond TPG
 *
 * @defgroup cam_device_tpg CamDevice TPG Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Enumeration of TPG picture type.
 *
 *****************************************************************************/
typedef enum CamDeviceISPTpgImageType_e {
	CAMDEV_ISP_TPG_IMAGE_TYPE_3X3COLORBLOCK = 0,    /**< 3 x 3 color block */
	CAMDEV_ISP_TPG_IMAGE_TYPE_COLORBAR = 1,    /**< Color bar */
	CAMDEV_ISP_TPG_IMAGE_TYPE_GRAYBAR = 2,    /**< Gray bar */
	CAMDEV_ISP_TPG_IMAGE_TYPE_GRID = 3,    /**< Highlighted grid */
	CAMDEV_ISP_TPG_IMAGE_TYPE_RANDOM = 4,    /**< Random data */
	CAMDEV_ISP_TPG_IMAGE_TYPE_MAX,
	CAMDEV_DUMMY_078 = 0xDEADFEED
} CamDeviceISPTpgImageType_t;

/******************************************************************************/
/**
* @brief   Enumeration of TPG resolution.
*
*****************************************************************************/
typedef enum CamDeviceIspTpgResolution_e {
	CAMDEV_ISP_TPG_1080P = 0,        /**< 1920 x 1080 resolution */
	CAMDEV_ISP_TPG_720P = 1,        /**< 1280 x 720 resolution */
	CAMDEV_ISP_TPG_4K = 2,        /**< 3840 x 2160 resolution */
	CAMDEV_DUMMY_079 = 0xDEADFEED
} CamDeviceIspTpgResolution_t;

/******************************************************************************/
/**
 * @brief   CamDevice TPG configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceTpgConfig_s {
	CamDeviceISPTpgImageType_t imageType;    /**< Image type */
	CamDeviceRawPattern_t bayerPattern;  /**< Bayer pattern */
	CamDeviceBitDepth_t colorDepth;    /**< Color depth */
	CamDeviceIspTpgResolution_t resolution;    /**< TPG resolution */

	uint16_t pixleGap;    /**< The width of sub-picture */
	uint16_t lineGap;    /**< The height of sub-picture */
	uint16_t gapStandard;    /**< The gap of sub-picture */
	uint32_t randomSeed;    /**< Random seed */
	uint32_t frameNum;    /**< The number of frames */
} CamDeviceTpgConfig_t;


/*****************************************************************************/
/**
 * @brief   This function sets TPG configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pTpgCfg             Pointer to TPG configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceTpgConfig_t *pTpgCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables TPG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables TPG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets TPG version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to TPG version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_tpg */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_TPG_API_H
