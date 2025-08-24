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
 * @cond HIST256
 *
 * @defgroup cam_device_hist256 CamDevice HIST256 Definitions
 * @{
 *
 */

#ifndef CAMDEV_HIST256_API_H
#define CAMDEV_HIST256_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_HIST256_GRID_ITEMS         25  /**< The number of grid sub windows */
#define CAMDEV_HIST256_NUM_BINS           256  /**< The number of bins */


/******************************************************************************/
/**
 * @brief   CamDevice HIST256 mode.
 *
 *****************************************************************************/
typedef enum CamDeviceHist256Mode_e {
	CAMDEV_HIST256_MODE_DISABLE = 0,    /**< Disabled, no measurements  */
	CAMDEV_HIST256_MODE_RGB_COMBINED = 1,    /**< RGB combined histogram */
	CAMDEV_HIST256_MODE_R = 2,    /**< R histogram */
	CAMDEV_HIST256_MODE_G = 3,    /**< G histogram */
	CAMDEV_HIST256_MODE_B = 4,    /**< B histogram */
	CAMDEV_HIST256_MODE_Y = 5,    /**< Luminance histogram */
	CAMDEV_HIST64_MODE_MAX,
	CAMDEV_DUMMY_074 = 0xDEADFEED
} CamDeviceHist256Mode_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST256 bins.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Bins_s {
	uint32_t bins[CAMDEV_HIST256_NUM_BINS];   /**< HIST256 bins */
} CamDeviceHist256Bins_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST256 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHIST256Config_s {
	CamDeviceHist256Mode_t mode;    /**< HIST256 mode */
	CamDeviceWindow_t window;    /**< HIST256 window */
} CamDeviceHist256Config_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST256 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Status_s {
	bool_t enable;    /**< HIST256 enable status*/
} CamDeviceHist256Status_t;


/*****************************************************************************/
/**
 * @brief   This function sets HIST256 configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST256 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256SetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist256Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST256 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist256Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables HIST256.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Enable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HIST256.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Disable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 statistical data.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pHistBins           Pointer to HIST256 bins
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist256Bins_t *pHistBins
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to HIST256 status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist256Status_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to HIST256 version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets HIST256.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Reset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_hist256 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_HIST256_API_H */
