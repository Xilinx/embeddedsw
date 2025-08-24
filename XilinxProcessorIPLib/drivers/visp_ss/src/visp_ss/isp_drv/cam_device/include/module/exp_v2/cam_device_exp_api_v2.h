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
 * @cond EXP_V2
 *
 * @defgroup cam_device_exp_v2 CamDevice EXP V2 Definitions
 * @{
 *
 */

#ifndef CAMDEV_EXP_API_H
#define CAMDEV_EXP_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_AAA_EXP_GRID_SIZE     32    /**< 3A EXP grid size */
#define CAMDEV_AAA_PIXEL_CHANNEL     4


/******************************************************************************/
/**
 * @brief   CamDevice ISP EXP input select.
 *
 *****************************************************************************/
typedef enum CamDeviceExpV2Sel_e {
	CAMDEV_EXP_INPUT_SEL_DEGAMMA = 0,    /**< Input select of degamma */
	CAMDEV_EXP_INPUT_SEL_AWBGAIN = 1,    /**< Input select of AWB gain */
	CAMDEV_EXP_INPUT_SEL_WDR3 = 2,    /**< Input select of WDR3 module */
	CAMDEV_EXP_INPUT_SEL_LSC = 3,    /**< Input select of LSC module */
	CAMDEV_EXP_STATE_MAX,
	CAMDEV_DUMMY_063 = 0xDEADFEED
} CamDeviceExpV2Sel_t;

/******************************************************************************/
/**
 * @brief   CamDevice ISP EXP statistics data bit type.
 *
 *****************************************************************************/
typedef enum CamDeviceExpV2StatisticsType_e {
	CAMDEV_EXP_8BIT_DATA = 0,    /**< EXP 8 bit data */
	CAMDEV_EXP_16BIT_DATA = 1,    /**< EXP 16 bit data */
	CAMDEV_EXP_24BIT_DATA = 2,    /**< EXP 24 bit data */
	CAMDEV_EXP_BIT_DATA_MAX,
	CAMDEV_DUMMY_064 = 0xDEADFEED
} CamDeviceExpV2StatisticsType_t;


/******************************************************************************/
/**
 * @brief   CamDevice EXP statistics configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Statistics_s {
	uint32_t statistics[CAMDEV_AAA_EXP_GRID_SIZE * CAMDEV_AAA_EXP_GRID_SIZE *
			    CAMDEV_AAA_PIXEL_CHANNEL];  /**< EXP statistics data */
	CamDeviceExpV2StatisticsType_t type;    /**< Statistics bit data type */
} CamDeviceExpV2Statistics_t;

/******************************************************************************/
/**
 * @brief   CamDevice EXP configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Config_s {
	CamDeviceExpV2Sel_t inputSelect;   /**< EXP input select */
	bool_t windowSetCustomEnable;
} CamDeviceExpV2Config_t;

/******************************************************************************/
/**
 * @brief   CamDevice EXP status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceExpV2Status_s {
	bool_t enable;    /**< EXP enable status*/
} CamDeviceExpV2Status_t;


/*****************************************************************************/
/**
 * @brief   This function sets EXP configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to EXP configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2SetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpV2Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to EXP configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpV2Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables EXP.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2Enable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EXP.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2Disable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets the EXP statistics window
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   pWindow      Pointer to window configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2SetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceWindow_t *pWindow
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets the EXP statistics window
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   pWindow      Pointer to window configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetMeasureWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWindow_t *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP statistic parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pExpStatic        Pointer to the EXP statistics
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetStatistics
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpV2Statistics_t *pExpStatic
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to EXP status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpV2Status_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets EXP version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to EXP version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2GetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets EXP.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceExpV2Reset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_exp_v2 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_EXP_API_H */
