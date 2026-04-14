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

#ifndef CAMDEV_EE_V2_API_H
#define CAMDEV_EE_V2_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 19_cam_device_ee VsCamDevice E01C19 Device_EE Definitions
 * @brief Provides interfaces for controlling the edge enhance module working
 * in the ISP pipeline.
 * @{
 *
 */

#define CAMDEV_EE_CA_CURVE_SIZE 65   /**< Bin number of CA curve */
#define CAMDEV_EE_DCI_CURVE_SIZE 65  /**< Bin number of DCI curve */

/******************************************************************************/
/**
 * @brief   VsCamDevice EE CA mode enumeration.
 *
 *****************************************************************************/
typedef enum CamDeviceEeCaMode_e
{
    CAMDEV_EE_CA_MODE0  = 0,    /**<Y mode */
    CAMDEV_EE_CA_MODE1  = 1,    /**<Saturation mode */
    CAMDEV_EE_CA_MODE2  = 2,    /**< Y & Saturation mode */
    CAMDEV_EE_CA_MODE_MAX
} CamDeviceEeCaMode_t;

/*****************************************************************************/
/**
 * @brief   EE DCI curve mode.
 *
 *****************************************************************************/
typedef enum CamDeviceEeDciMode_s
{
    CAMDEV_EE_DCI_DCI_CURVE_MODE = 0,    /**< DCI curve mode */
    CAMDEV_EE_DCI_CURVE_2SEC_MODE = 1,    /**< DCI 2-second mode */
    CAMDEV_EE_DCI_CURVE_3SEC_MODE = 2,    /**< DCI 3-second mode */
    CAMDEV_EE_DCI_CURVE_MODE_MAX
} CamDeviceEeDciMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeAutoConfig_s {
    uint8_t autoLevel;                                      /**< EE auto configuration level */

    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];                          /**< EE gain */
    uint8_t eeStrength[CAMDEV_ISO_STRENGTH_NUM];                   /**< Weight to merge EE result back to input image */
    uint8_t eeSrcStrength[CAMDEV_ISO_STRENGTH_NUM];                /**< Y channel input image weight */
    uint32_t eeYUpGain[CAMDEV_ISO_STRENGTH_NUM];                   /**< Selected at edge/texture area */
    uint32_t eeYDownGain[CAMDEV_ISO_STRENGTH_NUM];                 /**< Selected at flatten area */
    uint16_t eeUVGain[CAMDEV_ISO_STRENGTH_NUM];                    /**< EE UV gain */
    uint32_t eeEdgeGain[CAMDEV_ISO_STRENGTH_NUM];                  /**< Gain for detected edge */

    bool_t caEnable[CAMDEV_ISO_STRENGTH_NUM];                      /**< EE CA enable value */
    uint16_t caCurve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_EE_CA_CURVE_SIZE];   /**< EE color adjust curve  */
    bool_t dciEnable[CAMDEV_ISO_STRENGTH_NUM];                     /**< EE DCI enable value */
    uint16_t dciCurve[CAMDEV_ISO_STRENGTH_NUM][CAMDEV_EE_DCI_CURVE_SIZE]; /**< EE dynamic contrast improve curve */
}CamDeviceEeAutoConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE DCI manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeDciManualConfig_s {
    uint16_t dciCurve[CAMDEV_EE_DCI_CURVE_SIZE];       /**< EE DCI curve */
}CamDeviceEeDciManualConfig_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice EE DCI curve configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeDciCurveConfig_s
{
    float32_t degamma;    /**< EE DCI degamma */
    CamDeviceEeDciMode_t dciMode;    /**< DCI mode */
    float32_t lowlumaIndex;    /**< Low luminance index */
    float32_t middlelumaIndex;    /**< Middle luminance index */
    float32_t highlumaIndex;    /**< High luminance index */
    uint16_t inflctStartY;    /**< Inflection start point on Y axis */
    uint16_t inflct1X;    /**< Inflection point 1 on X axis */
    uint16_t inflct1Y;    /**< Inflection point 1 on Y axis */
    uint16_t inflct2X;    /**< Inflection point 2 on X axis */
    uint16_t inflct2Y;    /**< Inflection point 2 on Y axis */
    uint16_t inflctStopY;    /**< Inflection stop point on Y axis */
} CamDeviceEeDciCurveConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE CA manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeCaManualConfig_s {
    CamDeviceEeCaMode_t caMode;                        /**< EE CA mode */
    uint16_t caCurve[CAMDEV_EE_CA_CURVE_SIZE];         /**< EE CA curve */
}CamDeviceEeCaManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeManualConfig_s {
    uint32_t eeEdgeGain;  /**< Gain for detected edge */
    uint8_t eeSrcStrength;/**< Y channel input image weight */
    uint8_t eeStrength;   /**< Weight to merge EE result back to input image */
    uint16_t eeUvGain;    /**< EE UV gain */
    uint32_t eeYDownGain; /**< Selected at flatten area */
    uint32_t eeYUpGain;   /**< Selected at edge/texture area */
    CamDeviceEeCaManualConfig_t caCfg;  /**< EE CA manual configuration */
    CamDeviceEeDciManualConfig_t dciCfg;  /**< EE DCI manual configuration */
    CamDeviceEeDciCurveConfig_t dciCurveCfg;  /**< EE DCI curve configuration */
}CamDeviceEeManualConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEeConfig_s {
    CamDeviceConfigMode_t mode;           /**< EE mode configuration */
    CamDeviceEeAutoConfig_t autoCfg;      /**< EE auto configuration */
    CamDeviceEeManualConfig_t manualCfg;  /**< EE manual configuration */
}CamDeviceEeConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceEeStatus_s {
    bool_t enable;    /**< EE enable status */
    bool_t caEnable;  /**< EE CA enable status */
    bool_t dciEnable; /**< EE DCI enable status */
    bool_t curveEnable; /**< EE curve enable status */
    CamDeviceConfigMode_t currentMode;           /**< EE mode configuration */
    CamDeviceEeManualConfig_t currentCfg;  /**< EE current configuration */
}CamDeviceEeStatus_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice EE curveEn configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceEeCurveEnConfig_s
{
    bool_t  curveEn;
    bool_t  caEn;
    bool_t  dciEn;
} CamDeviceEeCurveEnConfig_t;

/*****************************************************************************/
/**
 * @brief   This function sets EE configuration parameters.
 * @startuml VsiCamDeviceEeSetConfig
 * !include E01_External/VsiCamDeviceEeSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pEeCfg      Pointer to EE Configuration
 * @details This function calls: \ref CamDeviceEeManualSetConfig, \ref CamDeviceEeAutoSetConfig,
 * \ref CamEngineEeSetMode
 * @details This function is called by: User application, \ref VsiCamDeviceEeReset
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
RESULT VsiCamDeviceEeSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceEeConfig_t *pEeCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets EE configuration parameters.
 * @startuml VsiCamDeviceEeGetConfig
 * !include E01_External/VsiCamDeviceEeGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pEeCfg      Pointer to EE configuration.
 * @details This function calls: \ref CamEngineEeGetMode, \ref CamDeviceEeManualGetConfig,
 * \ref CamDeviceEeAutoGetConfig
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
RESULT VsiCamDeviceEeGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceEeConfig_t *pEeCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets EE cureve configuration parameters.
 * @startuml VsiCamDeviceEeGetCurveEnableConfig
 * !include E01_External/VsiCamDeviceEeGetCurveEnableConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice     Handle to the VsCamDevice instance.
 * @param[inout]    pEeCurveEnCfg  Pointer to EE curve enable configuration.
 * @details This function calls: \ref CamEngineEeGetCurveConfig
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
RESULT VsiCamDeviceEeGetCurveEnableConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceEeCurveEnConfig_t *pEeCurveEnCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables EE.
 * @startuml VsiCamDeviceEeEnable
 * !include E01_External/VsiCamDeviceEeEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeEnable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EE.
 * @startuml VsiCamDeviceEeDisable
 * !include E01_External/VsiCamDeviceEeDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeDisable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables EE curve.
 * @startuml VsiCamDeviceEeCurveEnable
 * !include E01_External/VsiCamDeviceEeCurveEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeCurveEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeCurveEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EE curve.
 * @startuml VsiCamDeviceEeCurveDisable
 * !include E01_External/VsiCamDeviceEeCurveDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeCurveDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeCurveDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables EE CA.
 * @startuml VsiCamDeviceEeCaEnable
 * !include E01_External/VsiCamDeviceEeCaEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeCaEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeCaEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EE CA.
 * @startuml VsiCamDeviceEeCaDisable
 * !include E01_External/VsiCamDeviceEeCaDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeCaDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeCaDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables EE DCI.
 * @startuml VsiCamDeviceEeDciEnable
 * !include E01_External/VsiCamDeviceEeDciEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeDciEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeDciEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables EE DCI.
 * @startuml VsiCamDeviceEeDciDisable
 * !include E01_External/VsiCamDeviceEeDciDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineEeDciDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeDciDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets EE status.
 * @startuml VsiCamDeviceEeGetStatus
 * !include E01_External/VsiCamDeviceEeGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to EE status.
 * @details This function calls: \ref CamEngineEeGetStatus
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
RESULT VsiCamDeviceEeGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceEeStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets EE. It's only available in manual mode.
 * @startuml VsiCamDeviceEeReset
 * !include E01_External/VsiCamDeviceEeReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceEeSetConfig
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
RESULT VsiCamDeviceEeReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets EE version.
 * @startuml VsiCamDeviceEeGetVersion
 * !include E01_External/VsiCamDeviceEeGetVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersion    Pointer to EE version
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceEeGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/** @} 19_cam_device_ee */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_EE_V2_API_H
