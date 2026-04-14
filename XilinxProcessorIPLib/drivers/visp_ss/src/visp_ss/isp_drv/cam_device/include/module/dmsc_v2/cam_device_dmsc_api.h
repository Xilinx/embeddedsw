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

#ifndef CAMDEV_DMSC_V2_API_H
#define CAMDEV_DMSC_V2_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 13_cam_device_dmsc VsCamDevice E01C13 Device_DMSC Definitions
 * @brief Provides interfaces for controlling the demosaic module working in
 * the ISP pipeline.
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC auto configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscAutoConfig_s {
    uint8_t autoLevel;                                          /**< The auto configuration level */

    float32_t gains[CAMDEV_ISO_STRENGTH_NUM];                       /**< DMSC gains */
    bool_t dmscSharpenEnable[CAMDEV_ISO_STRENGTH_NUM];          /**< DMSC sharpen enable mask */
    bool_t dmscDepurpleEnable[CAMDEV_ISO_STRENGTH_NUM];         /**< DMSC depurple enable mask */
    uint16_t dmscSharpenFactorBlack[CAMDEV_ISO_STRENGTH_NUM];   /**< Sharpen factor of black edge */
    uint16_t dmscSharpenFactorWhite[CAMDEV_ISO_STRENGTH_NUM];   /**< Sharpen factor of white edge */
    uint16_t dmscSharpenClipBlack[CAMDEV_ISO_STRENGTH_NUM];     /**< Maximum sharpen level of black edge */
    uint16_t dmscSharpenClipWhite[CAMDEV_ISO_STRENGTH_NUM];     /**< Maximum sharpen level of white edge */
    uint16_t dmscSharpenT1[CAMDEV_ISO_STRENGTH_NUM];            /**< Curve of sharpen level in different texture areas. It's the first point on X axis. */
    uint8_t dmscSharpenT2Shift[CAMDEV_ISO_STRENGTH_NUM];        /**< Curve of sharpen level in different texture areas. It's the second point on X axis,
                                                                     and represents the left shift value*/
    uint16_t dmscSharpenT3[CAMDEV_ISO_STRENGTH_NUM];            /**< Curve of sharpen level in different texture areas. It's the third point on X axis. */
    uint8_t dmscSharpenT4Shift[CAMDEV_ISO_STRENGTH_NUM];        /**< Curve of sharpen level in different texture areas. It's the fourth point on X axis,
                                                                     and represents the left shift value*/
    uint8_t dmscDenoiseStrength[CAMDEV_ISO_STRENGTH_NUM];       /**< Green channel denoise. Larger values mean more blur */
	uint8_t dmscSharpenSize[CAMDEV_ISO_STRENGTH_NUM];           /**< Green channel sharpen. Larger values mean more sharpening */
    uint16_t dmscSharpenR1[CAMDEV_ISO_STRENGTH_NUM];            /**< Curve of sharpen level in different texture areas. It's the first point on Y axis. */
    uint16_t dmscSharpenR2[CAMDEV_ISO_STRENGTH_NUM];            /**< Curve of sharpen level in different texture areas. It's the second point on Y axis. */
    uint16_t dmscSharpenR3[CAMDEV_ISO_STRENGTH_NUM];            /**< Curve of sharpen level in different texture areas. It's the third point on Y axis. */
    uint8_t dmscDepurpleSatShrink[CAMDEV_ISO_STRENGTH_NUM];     /**< Forced saturation compression in the purple fringing area*/
	uint8_t dmscDepurpleCbcrMode[CAMDEV_ISO_STRENGTH_NUM];      /**< Purple fringe and red fringe remove enable */
    uint8_t dmscDepurpleThr[CAMDEV_ISO_STRENGTH_NUM];           /**< Judgment threshold of purple fringe area*/
}CamDeviceDmscAutoConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC CAC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscCacConfig_s {

    float32_t aBlue;        /**< Parameter A_Blue for radial blue shift calculation. */
    float32_t aRed;         /**< Parameter A_Red for radial blue shift calculation. */
    float32_t bBlue;        /**< Parameter B_Blue for radial blue shift calculation. */
    float32_t bRed;         /**< Parameter B_Red for radial blue shift calculation. */
    float32_t cBlue;        /**< Parameter C_Blue for radial blue shift calculation. */
    float32_t cRed;         /**< Parameter C_Red for radial blue shift calculation. */
    int16_t centerHoffs;  /**< Defines the red/blue pixel shift in horizontal direction. */
    int16_t centerVoffs;  /**< Defines the red/blue pixel shift in vertical direction. */
}CamDeviceDmscCacConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC demoire configuration.
 *
 ****************************************************************************/
typedef struct CamDeviceDmscDemoireConfig_s {

    uint8_t dmscDemoireT1;          /**< Weighting curve of demoire interpolation results in output result.
                                        It's the first point on X axis. */
    uint8_t dmscDemoireT2Shift;     /**< Weighting curve of demoire interpolation results in output result.
                                         It's the second point on X axis, and represents the left shift value. */
    uint16_t dmscDemoireR1;         /**< Weighting curve of demoire interpolation results in output result.
                                         It's the first point on Y axis. */
    uint16_t dmscDemoireR2;         /**< Weighting curve of demoire interpolation results in output result.
                                         It's the second point on Y axis. */
    uint16_t demoireEdgeR1;         /**< Weight curve of prefer to find the direction interpolation in the
                                         process of demoire interpolation. It's the first point on Y axis. */
    uint16_t demoireEdgeR2;         /**< Weight curve of prefer to find the direction interpolation in the
                                         Process of demoire interpolation. It's the second point on Y axis. */
    uint16_t demoireEdgeT1;         /**< weight curve of prefer to find the direction interpolation in the
                                         Process of demoire interpolation. It's the first point on X axis. */
    uint8_t demoireEdgeT2Shift;     /**< Weight curve of prefer to find the direction interpolation in the
                                         process of demoire interpolation. It's the second point on X axis,
                                         and represents the left shift value. */
    uint8_t dmscDemoireAreaThr;     /**< Judgment threshold of moire area. */
    uint8_t dmscDemoireSatShrink;   /**< Moire pixel forced saturation. */

}CamDeviceDmscDemoireConfig_t;

/*****************************************************************************/
/**
 * @brief  VsCamDevice DMSC depurple CbCr mode.
 *
 *****************************************************************************/
typedef enum CamDeviceDepurCbCrMode_e {
     CAM_DEVICE_DMSC_DEPURPLE_CB_CR_MODE_DISABLE = 0,  /**close depurple*/
     CAM_DEVICE_DMSC_DEPURPLE_CR_CHANNEL,              /**do cr channel depurple*/
     CAM_DEVICE_DMSC_DEPURPLE_CB_CHANNEL,              /**do cb channel depurple*/
     CAM_DEVICE_DMSC_DEPURPLE_CB_CR_CHANNLE,           /**do both cb and cr channel depurple*/
     CAM_DEVICE_DMSC_DEPURPLE_MAX,
}CamDeviceDepurCbCrMode_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC depurple configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscDepurpleConfig_s {

    CamDeviceDepurCbCrMode_t dmscDepurpleCbcrMode;       /**< Purple fringe and red fringe remove enable */
    uint8_t dmscDepurpleSatShrink;                       /**< Forced saturation compression in the purple fringing area*/
    uint8_t dmscDepurpleThr;                             /**< Judgment threshold of purple fringe area*/
}CamDeviceDmscDepurpleConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC sharpen configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSharpenConfig_s {

    uint16_t dmscSharpenClipBlack;      /**< Maximum sharpen level of black edge */
    uint16_t dmscSharpenClipWhite;      /**< Maximum sharpen level of white edge */
    uint16_t dmscSharpenFactorBlack;    /**< Sharpen factor of black edge */
    uint16_t dmscSharpenFactorWhite;    /**< Sharpen factor of white edge */
    uint16_t dmscSharpenR1;             /**< Curve of sharpen level in different texture areas. It's the first point on Y axis. */
    uint16_t dmscSharpenR2;             /**< Curve of sharpen level in different texture areas. It's the second point on Y axis. */
    uint16_t dmscSharpenR3;             /**< Curve of sharpen level in different texture areas. It's the third point on Y axis. */
    uint16_t dmscSharpenT1;             /**< Curve of sharpen level in different texture areas. It's the first point on X axis. */
    uint16_t dmscSharpenT3;             /**< Curve of sharpen level in different texture areas. It's the third point on X axis. */
    uint8_t dmscSharpenSize;            /**< Green channel sharpen. Larger values mean more blur */
    uint8_t dmscSharpenT2Shift;         /**< Curve of sharpen level in different texture areas. It's the second point on X axis,
                                             and represents the left shift value */
    uint8_t dmscSharpenT4Shift;         /**< Curve of sharpen level in different texture areas. It's the fourth point on X axis,
                                             and represents the left shift value */
	uint8_t dmscDenoiseStrength;        /**< Green channel denoise. Larger values mean more blur */
}CamDeviceDmscSharpenConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC sharpenLine configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSharpenLineConfig_s {

    uint16_t dmscSharpenLineR1;         /**< Curve of the overall enhancement of the line stroke. The larger the curve, the more
                                             texture will be considered as a line and enhanced, first point Y axis */
    uint16_t dmscSharpenLineR2;         /**< Curve of the overall enhancement of the line stroke. The larger the curve, the more
                                             texture will be considered as a line and enhanced, second point Y axis */
    uint16_t dmscSharpenLineStrength;   /**< The strength of line sharpen. */
    uint16_t dmscSharpenLineThr;        /**< The line detection, get line rate, to control the line sharpen strength. */
    uint8_t dmscSharpenLineThrShift1;   /**< Curve of the overall enhancement of the line stroke. The larger the curve,
                                             the more texture will be considered as a line and enhanced, first point X axis
                                              left shift value */
}CamDeviceDmscSharpenLineConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC skin configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSkinConfig_s {
    int16_t dmscSkinCbThrMax;      /**< Maximum threshold of skin color area in Cb color space */
    int16_t dmscSkinCbThrMin;      /**< Minimum threshold of skin color area in Cb color space */
    int16_t dmscSkinCrThrMax;      /**< Maximum threshold of skin color area in Cr color space */
    int16_t dmscSkinCrThrMin;      /**< Minimum threshold of skin color area in Cr color space */
    uint16_t dmscSkinYThrMax;       /**< Maximum threshold of skin color area in Y color space */
    uint16_t dmscSkinYThrMin;       /**< Minimum threshold of skin color area in Y color space */
}CamDeviceDmscSkinConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC current configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscManualConfig_s {

    uint8_t demosaicThr;        /**< Threshold for Bayer demosaicing texture detection. */
    uint16_t dmscDirThrMax;     /**< Maximum threshold for Bayer demosaicing texture detection. */
    uint16_t dmscDirThrMin;     /**< Minimum threshold for Bayer demosaicing texture detection. */
    CamDeviceDmscDemoireConfig_t demoireCfg;    /**< DMSC demoire configuration */
    CamDeviceDmscSharpenConfig_t sharpenCfg;    /**< DMSC sharpen configuration */
    CamDeviceDmscSharpenLineConfig_t sharpenLineCfg;    /**< DMSC sharpen line configuration */
    CamDeviceDmscSkinConfig_t skinCfg;    /**< DMSC skin configuration */
    CamDeviceDmscDepurpleConfig_t depurpleCfg;    /**< DMSC depurple configuration */
    CamDeviceDmscCacConfig_t cacCfg;    /**< DMSC CAC configuration */
}CamDeviceDmscManualConfig_t;


/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscConfig_s {
    CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceDmscAutoConfig_t autoCfg;      /**< DMSC auto configuration*/
    CamDeviceDmscManualConfig_t manualCfg;  /**< DMSC manual configuration*/
}CamDeviceDmscConfig_t;

/*****************************************************************************/
/**
 * @brief  DMSC submodule enable configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSubEnableConfig_s
{
    bool_t cacEn;
    bool_t demoireEn;
    bool_t sharpenEn;
    bool_t sharpenLineEn;
    bool_t skinEn;
    bool_t depurpleEn;
} CamDeviceDmscSubEnableConfig_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice DMSC status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscStatus_s {
    bool_t enable;                      /**< DMSC enable status*/
    bool_t cacEnable;                   /**< DMSC CAC enable status*/
    bool_t dmscDemoireEnable;           /**< DMSC demoire enable status*/
    bool_t dmscDepurpleEnable;          /**< DMSC depurple enable status*/
    bool_t dmscSharpenEnable;           /**< DMSC sharpen enable status*/
    bool_t dmscSharpenLineEnable;       /**< DMSC sharpenLine enable status*/
    bool_t dmscSkinEnable;              /**< DMSC skin enable status*/
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceDmscManualConfig_t currentCfg;  /**< DMSC current configuration*/
}CamDeviceDmscStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets DMSC configuration parameters.
 * @startuml VsiCamDeviceDmscSetConfig
 * !include E01_External/VsiCamDeviceDmscSetConfig.plantuml
 * @enduml
 * @param[inout]    hCamDevice        Handle to the VsCamDevice instance.
 * @param[in]       pConfig  Pointer to DMSC Configuration
 * @details This function calls: \ref CamDeviceDmscManualSetConfig, \ref CamDeviceDmscAutoSetConfig,
 * \ref CamEngineDmscv2SetMode
 * @details This function is called by: User application, \ref VsiCamDeviceDmscReset
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
RESULT VsiCamDeviceDmscSetConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceDmscConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC configuration parameters.
 * @startuml VsiCamDeviceDmscGetConfig
 * !include E01_External/VsiCamDeviceDmscGetConfig.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to DMSC configuration.
 * @details This function calls: \ref CamEngineDmscv2GetMode, \ref CamDeviceDmscManualGetConfig,
 * \ref CamDeviceDmscAutoGetConfig
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
RESULT VsiCamDeviceDmscGetConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceDmscConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC submdoule enable configuration parameters.
 * @startuml VsiCamDeviceSubEnableGetConfigure
 * !include E01_External/VsiCamDeviceSubEnableGetConfigure.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to DMSC submdoule enable configuration.
 * @details This function calls: \ref CamEngineDmsc2SubEnableGetConfigure
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
RESULT VsiCamDeviceSubEnableGetConfigure
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceDmscSubEnableConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC.
 * @startuml VsiCamDeviceDmscEnable
 * !include E01_External/VsiCamDeviceDmscEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2Enable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC.
 * @startuml VsiCamDeviceDmscDisable
 * !include E01_External/VsiCamDeviceDmscDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2Disable
 * @details This function is called by: User application,
 * \ref CamDeviceEnginePipelineEnable
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC CAC.
 * @startuml VsiCamDeviceDmscCacEnable
 * !include E01_External/VsiCamDeviceDmscCacEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2CacEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscCacEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC CAC.
 * @startuml VsiCamDeviceDmscCacDisable
 * !include E01_External/VsiCamDeviceDmscCacDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2CacDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscCacDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC demoire.
 * @startuml VsiCamDeviceDmscDemoireEnable
 * !include E01_External/VsiCamDeviceDmscDemoireEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2DemoireEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDemoireEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC demoire.
 * @startuml VsiCamDeviceDmscDemoireDisable
 * !include E01_External/VsiCamDeviceDmscDemoireDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2DemoireDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDemoireDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC depurple.
 * @startuml VsiCamDeviceDmscDepurpleEnable
 * !include E01_External/VsiCamDeviceDmscDepurpleEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2DepurpleEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDepurpleEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC depurple.
 * @startuml VsiCamDeviceDmscDepurpleDisable
 * !include E01_External/VsiCamDeviceDmscDepurpleDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2DepurpleDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDepurpleDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC sharpen.
 * @startuml VsiCamDeviceDmscSharpenEnable
 * !include E01_External/VsiCamDeviceDmscSharpenEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SharpenEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC sharpen.
 * @startuml VsiCamDeviceDmscSharpenDisable
 * !include E01_External/VsiCamDeviceDmscSharpenDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SharpenDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC sharpenLine.
 * @startuml VsiCamDeviceDmscSharpenLineEnable
 * !include E01_External/VsiCamDeviceDmscSharpenLineEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SharpenLineEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenLineEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC sharpenLine.
 * @startuml VsiCamDeviceDmscSharpenLineDisable
 * !include E01_External/VsiCamDeviceDmscSharpenLineDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SharpenLineDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenLineDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC skin.
 * @startuml VsiCamDeviceDmscSkinEnable
 * !include E01_External/VsiCamDeviceDmscSkinEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SkinEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSkinEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC skin.
 * @startuml VsiCamDeviceDmscSkinDisable
 * !include E01_External/VsiCamDeviceDmscSkinDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineDmscv2SkinDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSkinDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC status.
 * @startuml VsiCamDeviceDmscGetStatus
 * !include E01_External/VsiCamDeviceDmscGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     Pointer to DMSC status.
 * @details This function calls: \ref CamEngineDmsc2GetStatus
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
RESULT VsiCamDeviceDmscGetStatus
(
	CamDeviceHandle_t      hCamDevice,
	CamDeviceDmscStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DMSC. It's only available in manual mode.
 * @startuml VsiCamDeviceDmscReset
 * !include E01_External/VsiCamDeviceDmscReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref VsiCamDeviceDmscSetConfig
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
RESULT VsiCamDeviceDmscReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC version.
 * @startuml VsiCamDeviceDmscGetVersion
 * !include E01_External/VsiCamDeviceDmscGetVersion.plantuml
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
RESULT VsiCamDeviceDmscGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t         *pVersion
);
/*******************************************/

/** @} 13_cam_device_dmsc */

#ifdef __cplusplus
}
#endif

#endif   // CAMDEV_DMSC_V2_API_H
