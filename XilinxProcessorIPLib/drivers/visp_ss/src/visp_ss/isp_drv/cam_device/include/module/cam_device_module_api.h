// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
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

#ifndef CAMDEV_MODULE_API_H
#define CAMDEV_MODULE_API_H

#include "cam_device_dg_api.h"
#include "cam_device_wb_api.h"
#include "cam_device_2dnr_api.h"
#include "cam_device_ge_api.h"
#include "cam_device_cproc_api.h"
#include "cam_device_lsc_api.h"
#include "cam_device_rgbir_api.h"
#include "cam_device_cpd_api.h"
#include "cam_device_bls_api.h"
#include "cam_device_hdr_api.h"
#include "cam_device_ccm_api.h"
#include "cam_device_3dnr_api.h"
#include "cam_device_ee_api.h"
#include "cam_device_gc_api.h"
#include "cam_device_dpcc_api.h"
#include "cam_device_wdr_api.h"
#include "cam_device_af_api.h"
#include "cam_device_afm_api.h"
#include "cam_device_awb_api.h"
#include "cam_device_dmsc_api.h"
#include "cam_device_ae_api.h"
#include "cam_device_gtm_api.h"
#include "cam_device_cnr_api.h"
#include "cam_device_exp_api_v2.h"
#include "cam_device_hist64_api.h"
#include "cam_device_hist256_api.h"
#include "cam_device_tpg_api.h"
#include "cam_device_fusa_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 27_cam_device_general VsCamDevice E01C27 Device_GeneralCtrl Definitions
 * @brief Provides interfaces for controlling the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice module  enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceModuleAutoWorkMode_e {
    CAMDEV_MODULE_AUTO_GAIN_MODE = 0,      /**< Auto gain interpolation*/
    CAMDEV_MODULE_AUTO_LUX_INDEX_MODE,     /**< Auto lux index interpolation */
    CAMDEV_MODULE_AUTO_MODE_MAX            /**< Auto ctrl mode max index */
}CamDeviceModuleAutoWorkMode_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice module configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceModuleAutoCtrlConfig_s {
    CamDeviceModuleAutoWorkMode_t workMode;  /**< Submodule auto ctrl work mode configuration */
}CamDeviceModuleAutoCtrlConfig_t;

/*****************************************************************************/
/**
 * @brief   This function sets submodule auto control work mode.
 * @startuml VsiCamDeviceModuleAutoCtrlSetConfig
 * !include E01_External/VsiCamDeviceModuleAutoCtrlSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to auto control Configuration
 * @details This function calls: \ref CamEngineSetAutoWorkMode
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceModuleAutoCtrlSetConfig
(
	CamDeviceHandle_t		         hCamDevice,
	const CamDeviceModuleAutoCtrlConfig_t	*pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets submodule auto control work mode.
 * @startuml VsiCamDeviceModuleAutoCtrlGetConfig
 * !include E01_External/VsiCamDeviceModuleAutoCtrlGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to auto control Configuration
 * @details This function calls: \ref CamEngineGetAutoWorkMode
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceModuleAutoCtrlGetConfig
(
	CamDeviceHandle_t		         hCamDevice,
	CamDeviceModuleAutoCtrlConfig_t	*pConfig
);


/** @} 27_cam_device_general */

#ifdef __cplusplus
}
#endif


#endif    // CAMDEV_MODULE_API_H
