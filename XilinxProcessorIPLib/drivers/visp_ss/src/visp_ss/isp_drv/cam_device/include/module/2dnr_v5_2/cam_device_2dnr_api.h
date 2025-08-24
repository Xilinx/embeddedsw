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
 * @cond DNR2_V5_2
 *
 * @defgroup cam_device_2dnr_v5_2 CamDevice 2DNR V5.2 Definitions
 * @{
 *
 */

#ifndef CAMDEV_2DNR_API_H
#define CAMDEV_2DNR_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_DNR_LUMA_CURVE_SIZE 12   //bin number of dnr_luma_curve

/******************************************************************************/
/**
 * @brief   CamDevice 2DNR curve configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrCurveConfig_s {
	uint16_t lumaCurveX[CAMDEV_DNR_LUMA_CURVE_SIZE];    /**< Luma curve on X axis */
	uint16_t lumaCurveY[CAMDEV_DNR_LUMA_CURVE_SIZE];    /**< Luma curve on Y axis */
} CamDevice2DnrCurveConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice 2DNR current configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrManualConfig_s {

	uint8_t pregammaStrength;   /**< Pregamma strength value*/
	uint8_t strength;   /**< Strength value*/
	float32_t sigma;    /**< Sigma value */
	uint8_t sigmaRange;    /**< Sigma range value */
	uint16_t sigmaOffset;   /**< Sigma square */
	CamDevice2DnrCurveConfig_t curveCfg;
} CamDevice2DnrManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice 2DNR auto configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrAutoConfig_s {
	uint8_t autoLevel;  /**< The auto level */
	float32_t gain[CAMDEV_ISO_STRENGTH_NUM];    /**< 2DNR gain */
	float32_t sigma[CAMDEV_ISO_STRENGTH_NUM];   /**< 2DNR sigma */
	uint8_t strength[CAMDEV_ISO_STRENGTH_NUM];  /**< 2DNR strength */
	uint8_t pregammaStrength[CAMDEV_ISO_STRENGTH_NUM];  /**< 2DNR pregamma strength */
	uint16_t lumaCurveX[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_DNR_LUMA_CURVE_SIZE];   /**< 2DNR luma curve on X axis */
	uint16_t lumaCurveY[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_DNR_LUMA_CURVE_SIZE];   /**< 2DNR luma curve on Y axis */
} CamDevice2DnrAutoConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice 2DNR configuration.
 *
 *****************************************************************************/
typedef struct CamDevice2DnrConfig_s {
	CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
	CamDevice2DnrAutoConfig_t autoCfg;      /**< 2DNR auto configuration*/
	CamDevice2DnrManualConfig_t manualCfg;  /**< 2DNR manual configuration*/
} CamDevice2DnrConfig_t;

typedef struct CamDevice2DnrStatus_s {
	bool_t enable;  /**< 2DNR enable status */
	bool_t lumaEnable;  /**< Luma enable status */
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDevice2DnrManualConfig_t currentCfg;  /**< 2DNR current configuration*/
} CamDevice2DnrStatus_t;


/*****************************************************************************/
/**
 * @brief   This function enables 2DNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 2DNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets 2DNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to 2DNR Configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice2DnrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             The pointer of 2DNR configuration.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 * @retval  RET_NOTAVAILABLE    Module is not available for driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDevice2DnrConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables  2DNR luma curve.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrLumaEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 2DNR luma curve.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrLumaDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             The pointer of 2DNR status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDevice2DnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets 2DNR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            The pointer of 2DNR version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets 2DNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevice2DnrReset
(
	CamDeviceHandle_t hCamDevice
);


#ifdef __cplusplus
}
#endif

/* @} cam_device_2dnr_v5_2 */
/* @endcond */

#endif /* CAMDEV_2DNR_API_H */
