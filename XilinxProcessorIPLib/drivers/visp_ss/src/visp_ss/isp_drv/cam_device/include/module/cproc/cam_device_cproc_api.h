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
 * @cond CPROC
 *
 * @defgroup cam_device_cproc CamDevice CPROC Definitions
 * @{
 *
 */

#ifndef CAMDEV_CPROC_API_H
#define CAMDEV_CPROC_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CPROC_RGBTOYUV_MATRIX_NUM 9

/******************************************************************************/
/**
 * @brief   CamDevice CPROC pixel clipping range type enumeration for chroma and luma.
 *
 *****************************************************************************/
typedef enum CamDeviceCprocYuvRangeType_e {
	CAMDEV_CPROC_YUV_RANGE_INVALID = 0,    /**< Invalid range tpye */
	CAMDEV_CPROC_YUV_RANGE_LIMIT_RANGE = 1,    /**< Limit Y/U/V clipping range 16..240 */
	CAMDEV_CPROC_YUV_RANGE_FULL_RANGE = 2,    /**< Full Y/U/V clipping range 0..255 */
	CAMDEV_CPROC_YUV_RANGE_MAX,                   /**< Range type max */
	CAMDEV_DUMMY_055 = 0xDEADFEED
} CamDeviceCprocYuvRangeType_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   CamDevice CPROC color gamut type enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceCprocColorGamutType_e {
	CAMDEV_CPROC_COLOR_GAMUT_BT601 = 0,    /**< ITU-R BT.601 standard */
	CAMDEV_CPROC_COLOR_GAMUT_BT709 = 1,    /**< ITU-R BT.709 standard */
	CAMDEV_CPROC_COLOR_GAMUT_BT2020 = 2,    /**< ITU-R BT.2020 standard */
	CAMDEV_CPROC_COLOR_GAMUT_MAX,                   /**< gamut type max */
	CAMDEV_DUMMY_080 = 0xDEADFEED
} CamDeviceCprocColorGamutType_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   CamDevice CPROC YUV range configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocYuvRange_s {
	bool_t useOutConvMatrix;       /**< The flag for use outConvMatrix or not*/
	float32_t rgbToyuvMatrix[CPROC_RGBTOYUV_MATRIX_NUM];  /**< The rgb to yuv Matrix*/
	CamDeviceCprocYuvRangeType_t yuvRange;        /**< The clipping range for chrominance pixel output*/
	CamDeviceCprocColorGamutType_t gamut;       /** < The color gamut select*/
} CamDeviceCprocYuvRangeS_t;

/******************************************************************************/
/**
 * @brief   CamDevice CPROC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocAutoConfig_s {
	uint8_t autoLevel;                          /**< The auto configuration level */

	float32_t gains[CAMDEV_ISO_STRENGTH_NUM];       /**< CPROC gains */
	float32_t contrast[CAMDEV_ISO_STRENGTH_NUM];    /**< CPROC contrast adjust value */
	float32_t bright[CAMDEV_ISO_STRENGTH_NUM];      /**< CPROC brightness adjust value */
	float32_t saturation[CAMDEV_ISO_STRENGTH_NUM];  /**< CPROC saturation adjust value */
	float32_t hue[CAMDEV_ISO_STRENGTH_NUM];         /**< CPROC rotation in HSV domain */
} CamDeviceCprocAutoConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice CPROC current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocManualConfig_s {
	float32_t contrast;     /**< CPROC contrast adjust value [0, 1.999]*/
	float32_t bright;		/**< CPROC brightness adjust value [-127, 127]*/
	float32_t saturation;	/**< CPROC saturation adjust value [0, 1.999]*/
	float32_t hue;			/**< CPROC rotation in HSV domain [-90, 90]*/
} CamDeviceCprocManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice CPROC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocConfig_s {
	CamDeviceConfigMode_t configMode;           /**< The run mode: 0--manual, 1--auto */
	CamDeviceCprocAutoConfig_t autoCfg;         /**< CPROC auto configuration*/
	CamDeviceCprocManualConfig_t manualCfg;     /**< CPROC auto configuration*/
} CamDeviceCprocConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice CPROC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocStatus_s {
	bool_t enable;              /**< CPROC enable status*/
	CamDeviceConfigMode_t currentMode;           /**< The run mode: 0--manual, 1--auto */
	CamDeviceCprocManualConfig_t currentCfg;     /**< CPROC current configuration*/
} CamDeviceCprocStatus_t;

/*******************************************/


/*****************************************************************************/
/**
 * @brief   This function sets CPROC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pCprocCfg           Pointer to CPROC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocConfig_t *pCprocCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pCprocCfg           Pointer to CPROC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocConfig_t *pCprocCfg
);

/*****************************************************************************/
/**
 * @brief   This function sets CPROC pixel clipping range.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pYuvRange           Pointer to CPROC pixel clipping range
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocSetRange
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocYuvRangeS_t *pYuvRange
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC pixel clipping range.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pYuvRange           Pointer to CPROC pixel clipping range
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetRange
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocYuvRangeS_t *pYuvRange
);


/*****************************************************************************/
/**
 * @brief   This function enables CPROC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CPROC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocDisable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets CPROC status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus        Pointer to CPROC status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CPROC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to CPROC version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC pixel clipping range status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pYuvRange           Pointer to CPROC pixel clipping range
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetRangeStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCprocYuvRangeS_t *pYuvRange
);

#ifdef __cplusplus
}
#endif

/* @} cam_device_cproc */
/* @endcond */

#endif   // CAMDEV_CPROC_API_H
