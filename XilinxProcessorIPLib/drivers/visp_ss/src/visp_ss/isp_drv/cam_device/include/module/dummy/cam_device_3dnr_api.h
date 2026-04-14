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

#ifndef CAMDEV_3DNRV2_1_API_H
#define CAMDEV_3DNRV2_1_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 09_cam_device_3dnr VsCamDevice E01C09_3DNR Definitions
 * @{
 *
 */

#define CAMDEV_3DNR_TEMPORAL_CURVE_NUM 17       /**< The number of 3DNR temporal curves */
#define CAMDEV_3DNR_BLS_EXP_NUM 4  /**< The number of 3DNR gammaFeBe bls*/

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR temporal manual configuration.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrTemporalManualConfig_s
{
    uint16_t updateTemporal;    /**< Temporal update */
    uint16_t strengthCurveTemporal;    /**< Temporal strength curve */
    uint16_t temporalCurve[CAMDEV_3DNR_TEMPORAL_CURVE_NUM];    /**< Temporal curve */
    uint8_t  rangeT;    /**< Temporal range */
}CamDevice3DnrTemporalManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR temporal auto configuration.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrTemporalAutoConfig_s
{
    uint8_t motionDilateEn[CAMDEV_ISO_STRENGTH_NUM];    /**< Motion dilation enable */
    uint16_t updateTemporal[CAMDEV_ISO_STRENGTH_NUM];    /**< Temporal update */
    uint16_t strengthCurveTemporal[CAMDEV_ISO_STRENGTH_NUM];    /**< Temporal strength curve */
    uint16_t temporalCurve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_3DNR_TEMPORAL_CURVE_NUM];    /**< Temporal curve */
    uint8_t  rangeT[CAMDEV_ISO_STRENGTH_NUM];    /**< Temporal range */
}CamDevice3DnrTemporalAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR V2.1 configuration.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrAutoConfig_s
{
    uint8_t autoLevel;  /**< The auto level */
    float32_t   gains[CAMDEV_ISO_STRENGTH_NUM]; /**< 3DNR gains */
    uint8_t strength[CAMDEV_ISO_STRENGTH_NUM];  /**< General strength */
    int32_t motionInv[CAMDEV_ISO_STRENGTH_NUM]; /**< Motion factor. Smaller values mean larger motion image weight */
    uint16_t delta[CAMDEV_ISO_STRENGTH_NUM];    /**< Temporal delta. Smaller values mean more merged ref image */

    CamDevice3DnrTemporalAutoConfig_t temporalCfg; /**< Auto temporal configuration */
}CamDevice3DnrAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR V2.1 current configuration.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrManualConfig_s
{
    uint8_t strength;   /**< General strength */
    int32_t motionInv;  /**< Motion factor. Smaller values mean larger motion image weight */
    uint16_t delta; /**< Temporal delta. Smaller values mean more merged ref image */

    CamDevice3DnrTemporalManualConfig_t temporalCfg;  /**< Manual temporal configuration */
}CamDevice3DnrManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR V2.1 configuration.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrConfig_s {
    CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
    CamDevice3DnrAutoConfig_t autoCfg;      /**< 3DNR auto configuration*/
    CamDevice3DnrManualConfig_t manualCfg;  /**< 3DNR current configuration*/
}CamDevice3DnrConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice 3DNR status structure.
 *
 *****************************************************************************/
typedef struct CamDevice3DnrStatus_s {
    bool_t enable;                  /**< 3DNR enable status*/
    bool_t motionDilateEn;      /**< 3DNR motion dilation enable status */
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDevice3DnrManualConfig_t currentCfg;  /**< 3DNR current configuration*/
}CamDevice3DnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets 3DNR configuration parameters.
 * @startuml VsiCamDevice3DnrSetConfig
 * !include E01_External/VsiCamDevice3DnrSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to 3DNR Configuration
 * @details this function calls: CamDevice3DnrManualSetConfig, CamDevice3DnrAutoSetConfig,
 * CamEngine3DnrSetMode
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrSetConfig
(
    CamDeviceHandle_t           hCamDevice,
    CamDevice3DnrConfig_t       *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets 3DNR configuration parameters.
 * @startuml VsiCamDevice3DnrGetConfig
 * !include E01_External/VsiCamDevice3DnrGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to 3DNR configuration.
 * @details this function calls: CamEngine3DnrGetMode, CamDevice3DnrManualGetConfig,
 * CamDevice3DnrAutoGetConfig
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDevice3DnrConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables 3DNR.
 * @startuml VsiCamDevice3DnrEnable
 * !include E01_External/VsiCamDevice3DnrEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngine3DnrEnable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 3DNR.
 * @startuml VsiCamDevice3DnrDisable
 * !include E01_External/VsiCamDevice3DnrDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngine3DnrDisable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables 3DNR motion dilation.
 * @startuml VsiCamDevice3DnrMotionDilationEnable
 * !include E01_External/VsiCamDevice3DnrMotionDilationEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngine3DnrMotionDilationEnable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrMotionDilationEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables 3DNR motion dilation.
 * @startuml VsiCamDevice3DnrMotionDilationDisable
 * !include E01_External/VsiCamDevice3DnrMotionDilationDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: CamEngine3DnrMotionDilationDisable
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrMotionDilationDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets 3DNR status.
 * @startuml VsiCamDevice3DnrGetStatus
 * !include E01_External/VsiCamDevice3DnrGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to 3DNR status.
 * @details this function calls: CamEngine3DnrGetStatus
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetStatus
(
    CamDeviceHandle_t      hCamDevice,
    CamDevice3DnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets 3DNR.
 * @startuml VsiCamDevice3DnrReset
 * !include E01_External/VsiCamDevice3DnrReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details this function calls: VsiCamDevice3DnrSetConfig
 * @details this function is called by: User application, CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets 3DNR version.
 * @startuml VsiCamDevice3DnrGetVersion
 * !include E01_External/VsiCamDevice3DnrGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to 3DNR version
 * @details this function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDevice3DnrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/** @} 09_cam_device_3dnr */

#ifdef __cplusplus
}
#endif

#endif /* CAMDEV_3DNRV2_1_API_H */
