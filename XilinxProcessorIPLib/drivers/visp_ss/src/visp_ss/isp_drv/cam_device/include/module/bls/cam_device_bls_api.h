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

#ifndef CAMDEV_BLS_API_H
#define CAMDEV_BLS_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 12_cam_device_bls VsCamDevice E01C12 Device_BLS Definitions
 * @brief Provides interfaces for controlling the black level subtraction
 * module working in the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice BLS auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsAutoConfig_s {
    uint8_t autoLevel;                                          /**< The auto configuration level */

    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];                       /**< BLS gains */
    uint32_t bls[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_RAW_CHANNEL_NUM];  /**< BLS values */
}CamDeviceBlsAutoConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice BLS manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsManualConfig_s {
    uint32_t bls[CAMDEV_RAW_CHANNEL_NUM];       /**< BLS value:
                                                 Raw : BLS[0]--red, BLS[1]--greenRed, BLS[2]--greenBlue, BLS[3]--blue. The bls order shold be corresponding with sensor bayer pattern\n
                                                 Rgbir: BLS[0]--red, BLS[1]--green, BLS[2]--blue, BLS[3]--ir. Algorithm ensure the order: 0->r, 1->g, 2->b, 3->ir, bls don't need to correspond with sensor bayer pattern */
}CamDeviceBlsManualConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice BLS configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsConfig_s {
    CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceBlsAutoConfig_t autoCfg;       /**< BLS auto configuration*/
    CamDeviceBlsManualConfig_t manualCfg;   /**< BLS manual configuration*/
}CamDeviceBlsConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice BLS status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceBlsStatus_s {
    bool_t enable;              /**< BLS enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceBlsManualConfig_t currentCfg;   /**< BLS current configuration*/
}CamDeviceBlsStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets BLS configuration parameters.
 * @startuml VsiCamDeviceBlsSetConfig
 * !include E01_External/VsiCamDeviceBlsSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pBlsCfg     Pointer to BLS configuration.
 * @details This function calls: \ref CamDeviceBlsManualSetConfig, \ref CamDeviceBlsAutoSetConfig,
 * \ref CamEngineBlsSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceBlsReset
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsSetConfig
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceBlsConfig_t  *pBlsCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS configuration parameters.
 * @startuml VsiCamDeviceBlsGetConfig
 * !include E01_External/VsiCamDeviceBlsGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pBlsCfg     Pointer to BLS configuration.
 * @details This function calls: \ref CamEngineBlsGetMode, \ref CamDeviceBlsManualGetConfig,
 * \ref CamDeviceBlsAutoGetConfig
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetConfig
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceBlsConfig_t *pBlsCfg
);

/*****************************************************************************/
/**
 * @brief   This function sets BLS bitWidth parameters.
 * @startuml VsiCamDeviceBlsSetBitWidth
 * !include E01_External/VsiCamDeviceBlsSetBitWidth.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       blsBitWidth    bitWidth value.
 * @details This function calls: \ref CamEngineBlsSetBitWidth
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsSetBitWidth
(
    CamDeviceHandle_t     hCamDevice,
    uint8_t  blsBitWidth
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS bit width parameters.
 * @startuml VsiCamDeviceBlsGetBitWidth
 * !include E01_External/VsiCamDeviceBlsGetBitWidth.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    blsBitWidth    Pointer to bitWidth.
 * @details This function calls: \ref CamEngineBlsGetBitWidth
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetBitWidth
(
    CamDeviceHandle_t     hCamDevice,
    uint8_t  *blsBitWidth
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS status.
 * @startuml VsiCamDeviceBlsGetStatus
 * !include E01_External/VsiCamDeviceBlsGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to BLS status.
 * @details This function calls: \ref CamEngineBlsGetStatus
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetStatus
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceBlsStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets BLS. It's only available in manual mode.
 * @startuml VsiCamDeviceBlsReset
 * !include E01_External/VsiCamDeviceBlsReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamCalibDbGetBlsProfileByResolution,
 * CamEngineBlsSetBitWidth, VsiCamDeviceBlsSetConfig
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets BLS version.
 * @startuml VsiCamDeviceBlsGetVersion
 * !include E01_External/VsiCamDeviceBlsGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to BLS version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceBlsGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t         *pVersion
);

/** @} 12_cam_device_bls */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_BLS_API_H
