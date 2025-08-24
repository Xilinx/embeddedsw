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

/**
 * @cond AF_V4
 *
 * @defgroup cam_device_af_v4 CamDevice AF V4 Definitions
 * @{
 *
 */

#ifndef CAMDEV_AF_API_H
#define CAMDEV_AF_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define CAMDEV_AF_WINDOWNUM     9    /**< The number of AF window */
#define CAMDEV_AF_FILTERNUM     5    /**< The number of AF filter */
#define CAMDEV_PD_FOCAL_NUM_MAX 48   /**< The number of AF focal PD */
#define CAMDEV_PD_ROI_INDEX_MAX 48   /**< Maximum index of PDAF ROI */
#define CAMDEV_AFMV3_BLOCK_NUM  225  /**< The number of AFM V3 blocks */
#define AF_DEVICE_RETENTION_VALUE_MAX      500

/*****************************************************************************/
/**
 * @brief   CamDevice AF status.
 *
 *****************************************************************************/
typedef enum CamDeviceAfCtrlState_s {
	CAMDEV_AF_STATE_INVALID = 0,    /**< Invalid AF status */
	CAMDEV_AF_STATE_INITIALIZED = 1,    /**< AF initialized */
	CAMDEV_AF_STATE_STOPPED = 2,    /**< AF stopped */
	CAMDEV_AF_STATE_RUNNING = 3,    /**< AF running */
	CAMDEV_AF_STATE_TRACKING = 4,    /**< AF tracking */
	CAMDEV_AF_STATE_LOCKED = 5,    /**< AF locked */
	CAMDEV_AF_STATE_MAX,
	DUMMY_CAMDEV_0056 = 0xdeadfeed
} CamDeviceAfState_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AF mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAfMode_e {
	CAMDEV_CDAF_INDIVIDUAL_MODE = 0,    /**< CDAF focus */
	CAMDEV_PDAF_INDIVIDUAL_MODE = 1,    /**< PDAF focus */
	CAMDEV_PDAF_CDAF_HYBRID_MODE = 2,    /**< PDAF and CDAF hybrid */
	CAMDEV_AF_MODE_MAX,
	DUMMY_CAMDEV_0057 = 0xdeadfeed
} CamDeviceAfMode_t;

/*****************************************************************************/
/**
 * @brief   CamDevice PDAF and CDAF hybrid focus parameters
 *
 *****************************************************************************/
typedef struct CamDeviceAfPcdfHybridConfig_s {
	uint8_t defocusFrameNum;    /**< The number of defocus frames */
	uint8_t lossConfidenceFrameNum;    /**< The number of loss confidence frames */
	uint8_t accurateFocusStep;    /**< Accurate focus step */
	bool_t accurateFocusEnable;    /**< Accurate focus enable */
} CamDeviceAfPcdfHybridConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AF library configuration, which is required by the AF library calculation.
 *
 *****************************************************************************/
typedef struct CamDeviceAfConfig_s {
	// CDAF params
	float32_t weightWindow[CAMDEV_AF_WINDOWNUM];    /**< Weight window */
	float32_t cStableTolerance;      /**< Range: [0, 1] */
	uint8_t cPointsOfCurve;         /**< Range: [5, 100] */
	uint16_t maxFocal;    /**< Maximum focal point */
	uint16_t minFocal;    /**< Minimum focal point*/
	float32_t cMotionThreshold;    /**< Motion threshold */
	uint8_t uphillAllowance;         /**< The range of top hill when CDAF in searching  */
	uint8_t downhillAllowance;         /**< The range of bottom hill when CDAF in searching */

	// PDAF params
	float32_t cPdConfThreshold;    /**< PDAF configuration threshold */
	float32_t pdShiftThreshold;     /**< Phase Detection shift stable threshold */
	uint8_t pdStableCountMax;    /**< Maximum focusing to which PDAF locks */
	float32_t pdafUnlockThreshold;    /**< PDAF unlock threshold */

	// PDAF and CDAF hybrid params
	CamDeviceAfPcdfHybridConfig_t pcdafHybridConfig;    /**< PDAF and CDAF hybrid configuration */
	float32_t retResultFloat[AF_DEVICE_RETENTION_VALUE_MAX];
	uint32_t retResultUnit[AF_DEVICE_RETENTION_VALUE_MAX];
} CamDeviceAfConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AF status.
 *
 *****************************************************************************/
typedef struct CamDeviceAfStatus_s {
	CamDeviceAfState_t state;  /**< AF state*/
} CamDeviceAfStatus_t;


/*****************************************************************************/
/**
 *
 * @brief   This function registers AF.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pAfLibHandle   Pointer to AF library handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfRegister
(
	CamDeviceHandle_t hCamDevice,
	void *pAfLibHandle
);

/*****************************************************************************/
/**
 *
 * @brief   This function unregisters AF.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfUnRegister
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets AF calculation mode parameters.
 *
 * @param   hCamDevice      Handle to the CamDevice instance
 * @param   pMode           Pointer to AF calculation mode type
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfMode_t *pMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AF calculation mode parameters.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pMode          Pointer to AF calculation mode type
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_INVALID_PARM    Invalid configuration
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfMode_t *pMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets AF calculation parameters
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pConfig          Pointer to AF calculation parameters

 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfConfig_t *pConfig
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AF calculation parameters
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pConfig          Pointer to AF calculation configuration parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfConfig_t *pConfig
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets AF statistics window
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   AfmWinId     AF window item index
 * @param   pWindow      Pointer to window size
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetStatisticWindow
(
	CamDeviceHandle_t hCamDevice,
	uint8_t AfmWinId,
	CamDeviceWindow_t *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function enables AF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AF status.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pStatus        Pointer to AF status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAfState_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets AF version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to AF version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);


/*****************************************************************************/
/**
 * @brief   This function resets AF version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfReset
(
	CamDeviceHandle_t hCamDevice
);

/* @} cam_device_af_v4 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif
