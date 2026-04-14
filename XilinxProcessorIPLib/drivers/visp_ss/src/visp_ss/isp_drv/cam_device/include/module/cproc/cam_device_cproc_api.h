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

#ifndef CAMDEV_CPROC_API_H
#define CAMDEV_CPROC_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 06_cam_device_cproc VsCamDevice E01C06 Device_CPROC Definitions
 * @brief Provides interfaces for controlling the color process module working
 * in the ISP pipeline.
 * @{
 *
 */

#define CPROC_RGBTOYUV_MATRIX_NUM 9

/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC pixel clipping range type enumeration for chroma and luma.
 *
 *****************************************************************************/
typedef enum CamDeviceCprocYuvRangeType_e {
    CAMDEV_CPROC_YUV_RANGE_INVALID       = 0,    /**< Invalid range tpye */
    CAMDEV_CPROC_YUV_RANGE_LIMIT_RANGE   = 1,    /**< Limit Y/U/V clipping range 16..240 */
    CAMDEV_CPROC_YUV_RANGE_FULL_RANGE    = 2,    /**< Full Y/U/V clipping range 0..255 */
    CAMDEV_CPROC_YUV_RANGE_MAX                   /**< Range type max */
}CamDeviceCprocYuvRangeType_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC color gamut type enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceCprocColorGamutType_e {
    CAMDEV_CPROC_COLOR_GAMUT_BT601       = 0,    /**< ITU-R BT.601 standard */
    CAMDEV_CPROC_COLOR_GAMUT_BT709       = 1,    /**< ITU-R BT.709 standard */
    CAMDEV_CPROC_COLOR_GAMUT_BT2020      = 2,    /**< ITU-R BT.2020 standard */
    CAMDEV_CPROC_COLOR_GAMUT_MAX                   /**< gamut type max */
}CamDeviceCprocColorGamutType_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC YUV range configuration structure.
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
 * @brief   VsCamDevice CPROC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocAutoConfig_s {
    uint8_t autoLevel;                          /**< The auto configuration level */

    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];       /**< CPROC gains */
    float32_t contrast[CAMDEV_ISO_STRENGTH_NUM];    /**< CPROC contrast adjust value */
    float32_t bright[CAMDEV_ISO_STRENGTH_NUM];      /**< CPROC brightness adjust value */
    float32_t saturation[CAMDEV_ISO_STRENGTH_NUM];  /**< CPROC saturation adjust value */
    float32_t hue[CAMDEV_ISO_STRENGTH_NUM];         /**< CPROC rotation in HSV domain */
}CamDeviceCprocAutoConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocManualConfig_s {
    float32_t contrast;     /**< CPROC contrast adjust value [0, 1.999]*/
    float32_t bright;		/**< CPROC brightness adjust value [-127, 127]*/
    float32_t saturation;	/**< CPROC saturation adjust value [0, 1.999]*/
    float32_t hue;			/**< CPROC rotation in HSV domain [-90, 90]*/
}CamDeviceCprocManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocConfig_s {
    CamDeviceConfigMode_t configMode;           /**< The run mode: 0--manual, 1--auto */
    CamDeviceCprocAutoConfig_t autoCfg;         /**< CPROC auto configuration*/
    CamDeviceCprocManualConfig_t manualCfg;     /**< CPROC auto configuration*/
}CamDeviceCprocConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice CPROC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCprocStatus_s {
    bool_t enable;              /**< CPROC enable status*/
    CamDeviceConfigMode_t currentMode;           /**< The run mode: 0--manual, 1--auto */
    CamDeviceCprocManualConfig_t currentCfg;     /**< CPROC current configuration*/
}CamDeviceCprocStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets CPROC configuration parameters.
 * @startuml VsiCamDeviceCprocSetConfig
 * !include E01_External/VsiCamDeviceCprocSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pCprocCfg   Pointer to CPROC Configuration
 * @details This function calls: \ref CamEngineCprocSetConfig, \ref CamEngineCprocSetAutoConfig,
 * \ref CamEngineCprocSetMode
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
RESULT VsiCamDeviceCprocSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceCprocConfig_t *pCprocCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC configuration parameters.
 * @startuml VsiCamDeviceCprocGetConfig
 * !include E01_External/VsiCamDeviceCprocGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pCprocCfg   Pointer to CPROC configuration.
 * @details This function calls: \ref CamEngineCprocGetMode, \ref CamEngineCprocGetConfig,
 * \ref CamEngineCprocGetAutoConfig
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
RESULT VsiCamDeviceCprocGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceCprocConfig_t *pCprocCfg
);

/*****************************************************************************/
/**
 * @brief   This function sets CPROC pixel clipping range.
 * @startuml VsiCamDeviceCprocSetRange
 * !include E01_External/VsiCamDeviceCprocSetRange.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pYuvRange   Pointer to CPROC pixel clipping range
 * @details This function calls: \ref CamEngineSetYuvRangeConfig, \ref CamEngineCsmSetConfig
 * @details This function is called by: User application, \ref VsiCamDeviceCprocReset
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
RESULT VsiCamDeviceCprocSetRange
(
    CamDeviceHandle_t          hCamDevice,
    CamDeviceCprocYuvRangeS_t *pYuvRange
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC pixel clipping range.
 * @startuml VsiCamDeviceCprocGetRange
 * !include E01_External/VsiCamDeviceCprocGetRange.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pYuvRange   Pointer to CPROC pixel clipping range.
 * @details This function calls: \ref CamEngineGetYuvRangeConfig
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
RESULT VsiCamDeviceCprocGetRange
(
    CamDeviceHandle_t          hCamDevice,
	CamDeviceCprocYuvRangeS_t *pYuvRange
);

/*****************************************************************************/
/**
 * @brief   This function enables CPROC.
 * @startuml VsiCamDeviceCprocEnable
 * !include E01_External/VsiCamDeviceCprocEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineCprocEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables CPROC.
 * @startuml VsiCamDeviceCprocDisable
 * !include E01_External/VsiCamDeviceCprocDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceSetIspLowPower, \ref CamEngineCprocDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC status.
 * @startuml VsiCamDeviceCprocGetStatus
 * !include E01_External/VsiCamDeviceCprocGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to CPROC status.
 * @details This function calls: \ref CamEngineCprocGetStatus
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
RESULT VsiCamDeviceCprocGetStatus
(
	CamDeviceHandle_t       hCamDevice,
	CamDeviceCprocStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CPROC.
 * @startuml VsiCamDeviceCprocReset
 * !include E01_External/VsiCamDeviceCprocReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceCprocSetConfig, \ref VsiCamDeviceCprocSetRange
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
RESULT VsiCamDeviceCprocReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC version.
 * @startuml VsiCamDeviceCprocGetVersion
 * !include E01_External/VsiCamDeviceCprocGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to CPROC version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCprocGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function gets CPROC pixel clipping range status.
 * @startuml VsiCamDeviceCprocGetRangeStatus
 * !include E01_External/VsiCamDeviceCprocGetRangeStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pYuvRange   Pointer to CPROC pixel clipping range.
 * @details This function calls: \ref CamEngineGetYuvRangeConfig
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
RESULT VsiCamDeviceCprocGetRangeStatus
(
    CamDeviceHandle_t          hCamDevice,
	CamDeviceCprocYuvRangeS_t *pYuvRange
);

/** @} 06_cam_device_cproc */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_CPROC_API_H
