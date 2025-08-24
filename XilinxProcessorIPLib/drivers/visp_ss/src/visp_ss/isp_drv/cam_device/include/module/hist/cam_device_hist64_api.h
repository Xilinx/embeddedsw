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
 * @cond HIST64
 *
 * @defgroup cam_device_hist64 CamDevice HIST64 Definitions
 * @{
 *
 */

#ifndef CAMDEV_HIST64_API_H
#define CAMDEV_HIST64_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_HIST64_GRID_ITEMS         25  /**< The number of grid sub windows */
#define CAMDEV_HIST64_NUM_BINS           32  /**< number of bins */


/******************************************************************************/
/**
 * @brief   CamDevice HIST64 mode.
 *
 *****************************************************************************/
typedef enum CamDeviceHist64Mode_e {
	CAMDEV_HIST64_MODE_DISABLE = 0,    /**< Disabled, no measurements  */
	CAMDEV_HIST64_MODE_ONE_FROM_YRGB = 1,    /**< Control Y/R/G/B histogram via coefficients coeff_r/g/b */
	CAMDEV_HIST64_MODE_R = 2,    /**< R histogram */
	CAMDEV_HIST64_MODE_GR = 3,    /**< Gr histogram */
	CAMDEV_HIST64_MODE_B = 4,    /**< B histogram */
	CAMDEV_HIST64_MODE_GB = 5,    /**< Gb histogram */
	CAMDEV_IST64_MODE_MAX,
	CAMDEV_DUMMY_072 = 0xDEADFEED
} CamDeviceHist64Mode_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST64 channel.
 *
 *****************************************************************************/
typedef enum CamDeviceHist64Channel_e {
	CAMDEV_HIST64_CHANNEL_0 = 0,    /* After De-gamma channel */
	CAMDEV_HIST64_CHANNEL_1 = 1,    /* After LSC channel */
	CAMDEV_HIST64_CHANNEL_2 = 2,    /* After AWB GAIN channel */
	CAMDEV_HIST64_CHANNEL_3 = 3,    /* RGB domain after WDR channel */
	CAMDEV_HIST64_CHANNEL_4 = 4,    /* After Demosaic channel */
	CAMDEV_HIST64_CHANNEL_5 = 5,    /* RGB domain after cross talk channel */
	CAMDEV_HIST64_CHANNEL_6 = 6,    /* Reserved */
	CAMDEV_HIST64_CHANNEL_7 = 7,    /* RGB domain after gamma out channel */
	CAMDEV_HIST64_CHANNEL_MAX,
	CAMDEV_DUMMY_073 = 0xDEADFEED
} CamDeviceHist64Channel_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST64 bins.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Bins_s {
	uint32_t bins[CAMDEV_HIST64_NUM_BINS];   /**< HIST64 bins */
} CamDeviceHist64Bins_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST64 configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Config_s {
	CamDeviceHist64Mode_t mode;    /**< HIST64 mode */
	CamDeviceHist64Channel_t channel;    /**< HIST64 channel */
	CamDeviceWindow_t window;    /**< HIST64 window */
	float32_t rCoeff;    /**< Red channel coefficient */
	float32_t gCoeff;    /**< Green channel coefficient */
	float32_t bCoeff;    /**< Blue channel coefficient */
} CamDeviceHist64Config_t;

/******************************************************************************/
/**
 * @brief   CamDevice HIST64 status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHist64Status_s {
	bool_t enable;    /**< HIST64 enable status*/
} CamDeviceHist64Status_t;


/*****************************************************************************/
/**
 * @brief   This function sets HIST64 configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST64 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64SetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST64 configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64GetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Config_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables HIST64.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64Enable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HIST64.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64Disable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 statistical data.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pHistBins           Pointer to HIST64 bins
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Bins_t *pHistBins
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to HIST64 status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64GetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Status_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST64 version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to HIST64 version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64GetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets HIST64.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist64Reset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_hist64 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_HIST64_API_H */
