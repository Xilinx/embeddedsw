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
 * @cond DMSC_V2
 *
 * @defgroup cam_device_dmsc_v2 CamDevice DMSC V2 Definitions
 * @{
 *
 */

#ifndef CAMDEV_DMSC_V2_API_H
#define CAMDEV_DMSC_V2_API_H

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   CamDevice DMSC auto configuration.
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
} CamDeviceDmscAutoConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DMSC CAC configuration.
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
} CamDeviceDmscCacConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DMSC demoire configuration.
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

} CamDeviceDmscDemoireConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DMSC depurple configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscDepurpleConfig_s {

	uint8_t dmscDepurpleCbcrMode;       /**< Purple fringe and red fringe remove enable */
	uint8_t dmscDepurpleSatShrink;      /**< Forced saturation compression in the purple fringing area*/
	uint8_t dmscDepurpleThr;            /**< Judgment threshold of purple fringe area*/
} CamDeviceDmscDepurpleConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DMSC sharpen configuration.
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
} CamDeviceDmscSharpenConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DMSC sharpenLine configuration.
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
} CamDeviceDmscSharpenLineConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DMSC skin configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSkinConfig_s {
	int16_t dmscSkinCbThrMax;      /**< Maximum threshold of skin color area in Cb color space */
	int16_t dmscSkinCbThrMin;      /**< Minimum threshold of skin color area in Cb color space */
	int16_t dmscSkinCrThrMax;      /**< Maximum threshold of skin color area in Cr color space */
	int16_t dmscSkinCrThrMin;      /**< Minimum threshold of skin color area in Cr color space */
	uint16_t dmscSkinYThrMax;       /**< Maximum threshold of skin color area in Y color space */
	uint16_t dmscSkinYThrMin;       /**< Minimum threshold of skin color area in Y color space */
} CamDeviceDmscSkinConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DMSC current configuration.
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
} CamDeviceDmscManualConfig_t;


/******************************************************************************/
/**
 * @brief   CamDevice DMSC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscConfig_s {
	CamDeviceConfigMode_t configMode;       /**< The run mode: 0--manual, 1--auto */
	CamDeviceDmscAutoConfig_t autoCfg;      /**< DMSC auto configuration*/
	CamDeviceDmscManualConfig_t manualCfg;  /**< DMSC manual configuration*/
} CamDeviceDmscConfig_t;

/*****************************************************************************/
/**
 * @brief  DMSC submodule enable configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDmscSubEnableConfig_s {
	bool_t cacEn;
	bool_t demoireEn;
	bool_t sharpenEn;
	bool_t sharpenLineEn;
	bool_t skinEn;
	bool_t depurpleEn;
} CamDeviceDmscSubEnableConfig_t;

/******************************************************************************/
/**
 * @brief   CamDevice DMSC status structure.
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
} CamDeviceDmscStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets DMSC configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to DMSC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDmscConfig_t *pConfig
);


/*****************************************************************************/
/**
 * @brief   This function gets DMSC configuration parameters.
 *
 * @param   hCamDevice          Handle to the DMSC instance
 * @param   pConfig             Pointer to DMSC configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDmscConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC submdoule enable configuration parameters.
 *
 * @param   hCamDevice          Handle to the DMSC instance
 * @param   pConfig             Pointer to DMSC submdoule enable configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSubEnableGetConfigure
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDmscSubEnableConfig_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscEnable
(
	CamDeviceHandle_t hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function disables DMSC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC CAC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscCacEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC CAC.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscCacDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC demoire.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDemoireEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC demoire.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDemoireDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC depurple.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDepurpleEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC depurple.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscDepurpleDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC sharpen.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC sharpen.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC sharpenLine.
 *
 * @param   hCamDevice              Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenLineEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC sharpenLine.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSharpenLineDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DMSC skin.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSkinEnable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DMSC skin.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscSkinDisable
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus         Pointer to DMSC status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceDmscStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DMSC. It's only available in manual mode.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscReset
(
	CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DMSC version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to DMSC version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDmscGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
);
/*******************************************/

#ifdef __cplusplus
}
#endif

/* @} cam_device_dmsc_v2 */
/* @endcond */

#endif   // CAMDEV_DMSC_V2_API_H
