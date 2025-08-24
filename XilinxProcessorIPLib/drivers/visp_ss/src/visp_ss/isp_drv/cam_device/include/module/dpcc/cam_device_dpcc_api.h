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

#ifndef CAMDEV_DPCC_API_H
#define CAMDEV_DPCC_API_H

#include "cam_device_common.h"

/**
 * @cond DPCC
 *
 * @defgroup cam_device_dpcc CamDevice DPCC Definitions
 * @{
 *
 */

/*******************************************/
#define CAMDEV_DPCC_DEFECT_PIXEL_NUM          2048  /**< Maximum number of defected pixels */
#define CAMDEV_DPCC_CHANNEL_NUM               2     /**< Channel number of defected pixels */
#define CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM 3     /**< Method parameter type number of defected pixels */
#define CAMDEV_DPCC_MODE_NUM                  6     /**< Maximum number of auto mode */
/*******************************************/


#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice DPCC manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccManualConfig_s {
	bool_t bptEnable;                                 /**< Bad pixel table enable */
	uint16_t bptNum;                                  /**< Bad pixel table number */
	uint8_t bptOutMode;                              /**< Bad pixel table output mode */
	uint16_t bptPosX[CAMDEV_DPCC_DEFECT_PIXEL_NUM];   /**< Bad pixel table X position */
	uint16_t bptPosY[CAMDEV_DPCC_DEFECT_PIXEL_NUM];   /**< Bad pixel table Y position */
	uint8_t lineMadFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Line_mad method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of Line_mad method used by the G channel */
	uint8_t lineThresh[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Line_thresh method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of Line_thresh method used by the G channel */
	uint16_t methodsSet[CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0] Represents the switch whether the first set of thresholds of the five bad point determination methods are enabled\n */
	/**< [1] Represents the switch whether the second set of thresholds of the five bad point determination methods are enabled\n */
	/**< [2] Represents the switch whether the third set of thresholds of the five bad point determination methods are enabled */
	uint8_t outMode;  /**<Interpolation mode for correction unit. range:[0, 15] */
	uint8_t pgFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of peak Gradient check method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of peak Gradient check method used by the G channel */
	uint8_t rgFac[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rank neighbor difference method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of Rank neighbor difference method used by the G channel */
	uint8_t rndOffs[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three offsets of Rand Neighbor Difference method used by the RB channel\n */
	/**< [1][0~2]Represents the three offsets of Rand Neighbor Difference method used by the G channel */
	uint8_t rndThresh[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the G channel */
	uint8_t roLimits[CAMDEV_DPCC_CHANNEL_NUM][CAMDEV_DPCC_METHOD_PARAMETER_TYPE_NUM]; /**< [0][0~2]Represents the three thresholds of Rand Order method used by the RB channel\n */
	/**< [1][0~2]Represents the three thresholds of Rand Order method used by the G channel */
	uint8_t setUse; /**< DPCC methods set usage for detection */
} CamDeviceDpccManualConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DPCC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccAutoConfig_s {
	uint8_t autoModeSelect;    /**< The auto config level */
} CamDeviceDpccAutoConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DPCC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccConfig_s {
	CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceDpccManualConfig_t manualCfg;  /**< DPCC manual configuration*/
	CamDeviceDpccAutoConfig_t autoCfg;      /**< DPCC auto configuration*/
} CamDeviceDpccConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DPCC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDpccStatus_s {
	bool_t enable;                          /**< DPCC enable status*/
	CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceDpccManualConfig_t currentCfg;  /**< DPCC current configuration*/
} CamDeviceDpccStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets DPCC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDpccCfg            Pointer to DPCC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDpccConfig_t *pDpccCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets DPCC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDpccCfg            Pointer to DPCC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDpccConfig_t *pDpccCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables DPCC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DPCC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DPCC status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus         Pointer to DPCC status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDpccStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DPCC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the DPCC version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to DPCC version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpccGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/* @} cam_device_dpcc */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_DPCC_API_H
