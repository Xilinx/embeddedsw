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


#ifndef CAMDEV_API_H
#define CAMDEV_API_H

#include "cam_device_common.h"
#include "cam_device_calibration.h"
#include <offline_trace.h>
#include <cam_device_ccm_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 27_cam_device_general VsCamDevice E01C27 Device_GeneralCtrl Definitions
 * @brief The general control API mainly provides a universal ISP pipeline flow control
 * interface.
 * @{
 */

/*****************************************************************************/
/**
 * @brief   Structure to configure MCM mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceMcmModeConfig_s {
    CamDeviceMcmPortId_t     portId;    /**< Port index of MCM which indicates the sensor hardware connect position*/
    CamDeviceMcmOperation_t  mcmOp;     /**< MCM operation mode */
    CamDeviceMcmSelection_t  mcmSel;    /**< MCM selection mode */
    CamDeviceMcmRegCfgMode_t mcmRegCfg;/**< MCM register configuration mode */
}CamDeviceMcmModeConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure stream mode parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceStreamModeConfig_s {
     CamDeviceStreamPortId_t     portId;    /**< Port index of MCM which indicates the sensor hardware connect position*/
}CamDeviceStreamModeConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure tile mode parameters. Reserved.
 *
 *****************************************************************************/
typedef struct CamDeviceTileModeConfig_s {
    CamDeviceTileOperation_t tileOp;    /**< Tile operation mode */
    CamDeviceTileJoint_t     tileNum;   /**< Tile number */
    CamDeviceTileXAxis_t     xPices;    /**< Tile joint mode set user, x direction tile number */
    CamDeviceTileYAxis_t     yPices;    /**< Tile joint mode set user, y direction tile number */
}CamDeviceTileModeConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP work mode.
 *
 *****************************************************************************/
typedef union CamDeviceModeConfig_s {
    CamDeviceStreamModeConfig_t stream;   /**< Stream mode configuration parameters */
    CamDeviceMcmModeConfig_t    mcm;     /**< MCM mode configuration parameters */
}CamDeviceModeConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP input.
 *
 *****************************************************************************/
typedef struct CamDeviceInputConfig_s {
    CamDeviceInputType_t  inputType;    /**< ISP input type */
    char                  inputDevName[CAMDEV_INPUT_DEV_NAME_LEN];    /**< ISP input device name */
}CamDeviceInputConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP output.
 *
 *****************************************************************************/
typedef struct CamDeviceOutputConfig_s {
    CamDeviceOutputType_t  outputType;    /**< ISP output type */
}CamDeviceOutputConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure work parameters for camera device.
 *
 *****************************************************************************/
typedef struct CamDeviceWorkConfig_s {
    CamDeviceWorkMode_t        workMode;         /**< ISP software work mode */
    CamDeviceModeConfig_t      modeCfg;          /**< Work mode configurations */
    CamDeviceTileModeConfig_t  tileCfg;          /**< Tile mode configurations. It can be used together with other modes*/
}CamDeviceWorkConfig_t;

/*****************************************************************************/
/**
 * @brief    Structure to create camera device instance.
 *
 *****************************************************************************/
typedef struct CamDeviceConfig_s {
    uint32_t                      ispHwId;         /**< Hardware Pipeline ID. */
    CamDeviceInputConfig_t        inputCfg;        /**< Input device configuration parameters */
    CamDeviceWorkConfig_t         workCfg;         /**< ISP Work configuration parameters */
    CamDeviceOutputConfig_t       outputCfg;       /**< ISP output configuration parameters */
    CamDeviceSwitchSeqPriority_t  priority;        /**< Input device priority in switch control */
    void *                        hCascade;        /**< Cascade ctx handle */
}CamDeviceConfig_t;

/*****************************************************************************/
/**
 * @brief   Structure to set ISP pipeline output format.
 *
 *****************************************************************************/
typedef struct CamDevicePipeOutFmt_s {
    uint32_t outWidth;                     /**< Width of output image */
    uint32_t outHeight;                    /**< Height of output image */
    uint32_t pathOutType;                   /**< path out type */
    CamDevicePipePixOutFmt_t outFormat;    /**< Format of output pixel */
    uint32_t dataBits;                     /**< Data width of each color component [8~14] */
    uint8_t  alpha;                        /**< Alpha value of ARGB format , range 0~255 for ARGB/yuv8bit, range 0~3 for yuv10bit */
    CamDeviceMiYuvOrder_t  yuvOrder;       /**< The difference order of the three yuv channal save in ddr */
    CamDeviceMiSwap_u swap;    /**< Output format swap control information */
    CamDeviceMiBufOffset_t bufOffset;      /**< Output format buffer offset */
    CamDeviceMiBufStride_t bufStride;      /**< Output format buffer stride */
} CamDevicePipeOutFmt_t;

/*****************************************************************************/
/**
 * @brief   Structure to set ISP output streaming.
 *
 *****************************************************************************/
typedef struct CamDevicePathStreamingCfg_s {
    uint8_t outPathEnable;    /**< Path streaming configuration */
}CamDevicePathStreamingCfg_t;

/*****************************************************************************/
/**
 * @brief   Structure to set ISP DMA read image input format.
 *
 *****************************************************************************/
typedef struct CamDevicePipeInFmt_s {
    uint32_t inWidth;                     /**< Width of input image */
    uint32_t inHeight;                    /**< Height of input image */
    CamDeviceInputRawFmt_t   inFormat;     /**< Format of input image */
	CamDeviceBitDepth_t      inBit;        /**< Bit depth of input device: TPG */
    CamDeviceRawPattern_t    inPattern;    /**< Bayer pattern of input RAW RGB/RGBIR image */
    CamDeviceStitchingMode_t  stitchMode;  /**< The stitch mode. It should be configured when loading HDR image*/
} CamDevicePipeInFmt_t;


/*****************************************************************************/
/**
 * @brief   Structure to configure windows in ISP pipeline.
 *
 *****************************************************************************/
typedef struct CamDevicePipeIspWindow_s {
    CamDeviceWindow_t   cropWindow;         /**< Crop window for output path */
} CamDevicePipeIspWindow_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure submodules of pipeline when connecting camera.
 *
 *****************************************************************************/
typedef union CamDevicePipeSubmoduleCtrl_s {
    struct {
        uint32_t aeEnable : 1;      /**< bit 0: 0-disable 1-enable */
        uint32_t afEnable : 1;      /**< bit 1 */
        uint32_t hdrEnable : 1;     /**< bit 2 */
        uint32_t awbEnable : 1;     /**< bit 3 */
        uint32_t ccmEnable : 1;     /**< bit 4 */
        uint32_t compressEnable : 1; /**< bit 5 */
        uint32_t expandEnable : 1;  /**< bit 6 */
        uint32_t cnrEnable : 1;     /**< bit 7 */
        uint32_t ynrEnable : 1;     /**< bit 8 */
        uint32_t cprocEnable : 1;   /**< bit 9 */
        uint32_t dciEnable : 1;     /**< bit 10 */
        uint32_t demosaicEnable : 1; /**< bit 11 */
        uint32_t dgEnable : 1;      /**< bit 12 */
        uint32_t dpccEnable : 1;    /**< bit 13 */
        uint32_t dpfEnable : 1;     /**< bit 14 */
        uint32_t eeEnable : 1;      /**< bit 15 */
        uint32_t gcEnable : 1;      /**< bit 16 */
        uint32_t geEnable : 1;      /**< bit 17 */
        uint32_t gtmEnable : 1;     /**< bit 18 */
        uint32_t lscEnable : 1;     /**< bit 19 */
        uint32_t lut3dEnable : 1;   /**< bit 20 */
        uint32_t pdafEnable : 1;    /**< bit 21 */
        uint32_t rgbirEnable : 1;   /**< bit 22 */
        uint32_t wbEnable : 1;      /**< bit 23 */
        uint32_t wdrEnable : 1;     /**< bit 24 */
        uint32_t dnr3Enable : 1;    /**< bit 25 */
        uint32_t dnr2Enable : 1;    /**< bit 26 */
        uint32_t reservedEnable : 5;/**< bit 27:31 */
    } subCtrl;
    uint32_t allCtrl;
} CamDevicePipeSubmoduleCtrl_u;

#define CAMDEV_IMAGE_EXP_NUM_MAX 4 /**< Maximum exposure number of image*/

/*****************************************************************************/
/**
 * @brief   Structure to configure image exposure control information.
 *
 *****************************************************************************/
typedef struct CamDeviceImageExposureControl_s {
    uint32_t frameIndex;
    float32_t imageGain[CAMDEV_IMAGE_EXP_NUM_MAX];
        /**< In linear mode or native HDR mode:\n imageGain[0] is image gain\n
             In stitch HDR mode:\n
             imageGain[0]: L image gain\n
             imageGain[1]: S image gain\n
             imageGain[2]: VS image gain\n
             imageGain[3]: ES image gain */
    float32_t imageIntTime[CAMDEV_IMAGE_EXP_NUM_MAX];
        /**< In linear mode or native HDR mode:\n imageIntTime[0] is image integration time\n
         In stitch HDR mode:\n
         imageIntTime[0]: L image integration time\n
         imageIntTime[1]: S image integration time\n
         imageIntTime[2]: VS image integration time\n
         imageIntTime[3]: ES image integration time */
} CamDeviceImageExposureControl_t;


/*****************************************************************************/
/**
 * @brief   Histogram metadata information.
 *
 *****************************************************************************/
typedef struct CamDeviceMetadataHist_s {

    uint8_t histWinNum;    /**< The number of histogram windows */
    CamDeviceWindow_t histRoiWin[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Histogram ROI window */
    CamDeviceHistBins_t redChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Red channel statistics */
    CamDeviceHistBins_t grChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Green red channel statistics */
    CamDeviceHistBins_t gbChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Green blue channel statistics */
    CamDeviceHistBins_t blueChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Blue channel statistics */
    CamDeviceHistBins_t totalBins[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Total statistics */
}CamDeviceMetadataHist_t;

/*****************************************************************************/
/**
 * @brief   Mean luminance metadata information.
 *
 *****************************************************************************/
typedef struct CamDeviceMetadataMeanLuma_s {

    uint8_t meanWinNum;    /**< The number of mean windows */
    CamDeviceWindow_t meanRoiWin[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Mean ROI window */
    uint32_t redChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Red channel mean value */
    uint32_t grChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Green red channel mean value */
    uint32_t gbChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Green blue channel mean value */
    uint32_t blueChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< Blue channel mean value */
    uint32_t totalMean[CAM_DEVICE_ROI_WINDOWS_MAX];    /**< The total mean value of the four channels */

}CamDeviceMetadataMeanLuma_t;

/*****************************************************************************/
/**
 * @brief   Mean luminance metadata information.
 *
 *****************************************************************************/
typedef struct CamDeviceRawChannelFloat_s {
	float32_t redChannel;
	float32_t grChannel;
	float32_t gbChannel;
	float32_t blueChannel;
}CamDeviceRawChannelFloat_t;

/*****************************************************************************/
/**
 * @brief   Metadata exposure information.
 *
 *****************************************************************************/
typedef struct CamDeviceMetadataInfo_s {
    uint32_t chipId;    /**< ISP platform chip ID */
    uint64_t frameCount;    /**< Frame count */
    uint32_t apertureSize;  /**< Aperture size */
    uint32_t isoStrength;   /**< ISO strength */

    float32_t junctionTemperature;    /**< Junction temperature */
    uint32_t blackLevelPedestal;    /**< Black level pedestal */

    uint8_t exposureNum;    /**< The number of exposures */
    uint32_t integrationTime[CAMDEV_EXPOSURE_FRAME_MAX];  /**< Exposure time(unit: us) */
    CamDeviceIntegerRange_t  integrationTimeRange[CAMDEV_EXPOSURE_FRAME_MAX]; /**< Exposure time(unit: us) */

    float32_t analogGain[CAMDEV_EXPOSURE_FRAME_MAX];    /**< Analog gain */
    CamDeviceFloatRange_t analogGainRange[CAMDEV_EXPOSURE_FRAME_MAX];
    float32_t digitalGain[CAMDEV_EXPOSURE_FRAME_MAX];    /**< Digital gain */
    CamDeviceFloatRange_t digitalGainRange[CAMDEV_EXPOSURE_FRAME_MAX];
    CamDeviceRawChannelFloat_t wbGain[CAMDEV_EXPOSURE_FRAME_MAX]; /**< White balance gain */
    float32_t dualConvGain[CAMDEV_EXPOSURE_FRAME_MAX];    /**< Dual conversion gain */

    CamDeviceRawChannelFloat_t ispDgain;
    CamDeviceRawChannelFloat_t ispWbGain;

    float32_t luxIndex;
    float32_t totalGain;
    CamDeviceCcmStatus_t ccmConfig;

    CamDeviceMetadataHist_t histBins[CAMDEV_EXPOSURE_FRAME_MAX];    /**< (Reserved) Histogram statistics */
    CamDeviceMetadataMeanLuma_t meanLuma[CAMDEV_EXPOSURE_FRAME_MAX];    /**< Reserved) Mean luminance array */

}CamDeviceMetadataInfo_t;

/*****************************************************************************/
/**
 * @brief   Metadata configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceMetadataConfig_s {
    CamDeviceAwbWorkMode_t  modeAwb;     /**< Choose awb mode */
	CamDeviceAeWorkMode_t   modeAe;      /**< Choose ae mode */
    CamDeviceMetadataInfo_t metaInfo;    /**< Metadata information */
}CamDeviceMetadataConfig_t;

/*****************************************************************************/
/**
 * @brief   This function creates and initializes a VsCamDevice instance.
 * @startuml VsiCamDeviceCreate
 * !include E01_External/VsiCamDeviceCreate.plantuml
 * @enduml
 * @param[in]       pCamConfig   Instance configuration.
 * @param[inout]    phCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceIspcoreInit, \ref CamDeviceRequestInstance,
 * \ref CamDeviceInstanceIdMapping, \ref CamDeviceHalHolderCreate, \ref CamDeviceSensorInit,
 * \ref CamDevicePlatformInit, \ref CamDeviceBufCtrlInit, \ref CamDevicePipeInit, \ref CamDeviceEngineInit,
 * CamDeviceSubModuleInit
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL pointer(s)
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceCreate
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t *phCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function destroys a VsCamDevice instance.
 * @startuml VsiCamDeviceDestroy
 * !include E01_External/VsiCamDeviceDestroy.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamCalibDbRelease, \ref CamDeviceSubModuleDeinit,
 * \ref CamDeviceEngineDeinit, \ref CamDevicePipeDeinit, \ref CamDeviceBufCtrlDeInit,
 * \ref CamDevicePlatformDeinit, \ref CamDeviceSensorDeInit, \ref CamDeviceHalHolderRelease,
 * \ref CamDeviceFreeInstance
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL pointer(s)
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceDestroy
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets the output format of main or self path.
 * @startuml VsiCamDeviceSetOutFormat
 * !include E01_External/VsiCamDeviceSetOutFormat.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Select the output path ID.
 * @param[inout]    pFmt        Configuration structure for output format.
 * @details This function calls: \ref CamDeviceEngineSetOutPathFmt
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetOutFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeOutFmt_t *pFmt
);

/*****************************************************************************/
/**
 * @brief   This function sets the Input format.
 * @startuml VsiCamDeviceSetInFormat
 * !include E01_External/VsiCamDeviceSetInFormat.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Select the input path ID.
 * @param[in]       pFmt        Configuration structure for input format.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetInFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeInPathType_t path,
    CamDevicePipeInFmt_t *pFmt
);

/*****************************************************************************/
/**
 * @brief   This function gets the output format of main or self path.
 * @startuml VsiCamDeviceGetOutFormat
 * !include E01_External/VsiCamDeviceGetOutFormat.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Select the output path ID.
 * @param[inout]    pFmt        Configuration structure for output format.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetOutFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeOutFmt_t *pFmt
);

/*****************************************************************************/
/**
 * @brief   This function gets the input format of main or self path.
 * @startuml VsiCamDeviceGetInFormat
 * !include E01_External/VsiCamDeviceGetInFormat.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Select the input path ID.
 * @param[inout]    pFmt        Configuration structure for input format.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetInFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeInPathType_t path,
    CamDevicePipeInFmt_t *pFmt
);

/*****************************************************************************/
/**
 * @brief   This function sets windows in the ISP pipeline.
 * @startuml VsiCamDeviceSetIspWindow
 * !include E01_External/VsiCamDeviceSetIspWindow.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Config the output path ID.
 * @param[in]       pIspWindow  Pointer to ISP windows in the pipeline.
 * @details This function calls: \ref CamDeviceEngineSetIspWindow
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetIspWindow
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    const CamDevicePipeIspWindow_t *pIspWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets windows in the ISP pipeline.
 * @startuml VsiCamDeviceGetIspWindow
 * !include E01_External/VsiCamDeviceGetIspWindow.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       path        Output path ID.
 * @param[inout]    pIspWindow  Pointer to ISP windows in the pipeline.
 * @details This function calls: \ref CamDeviceEngineGetIspWindow
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetIspWindow
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeIspWindow_t *pIspWindow
);

/*****************************************************************************/
/**
 * @brief   This function sets up the whole camera system. First, initialize the
 *          CamEngine and set the pipeline path. Then, connect the input
 *          (sensor or image) with the CamEngine to process the image data.
 * @startuml VsiCamDeviceConnectCamera
 * !include E01_External/VsiCamDeviceConnectCamera.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pSubCtrl    Control the submodules when setting up the
 *                           camera system.
 * @details This function calls: \ref CamDevicePipeConnectInput
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceConnectCamera
(
    CamDeviceHandle_t hCamDevice,
    const CamDevicePipeSubmoduleCtrl_u *pSubCtrl
);

/*****************************************************************************/
/**
 * @brief   This function disconnects the whole camera system.
 * @startuml VsiCamDeviceDisconnectCamera
 * !include E01_External/VsiCamDeviceDisconnectCamera.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDevicePipeDisconnectInput
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceDisconnectCamera
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the version ID.
 * @startuml VsiCamDeviceGetSoftwareVersion
 * !include E01_External/VsiCamDeviceGetSoftwareVersion.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pVersionId  Version ID which is related to the ISP
 *                              hardware version.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetSoftwareVersion
(
    CamDeviceHandle_t   hCamDevice,
    char                *pVersionId
);

/*****************************************************************************/
/**
 * @brief   This function sets output path streaming.
 * @startuml VsiCamDeviceSetPathStreaming
 * !include E01_External/VsiCamDeviceSetPathStreaming.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pConfig     Pointer to the output path streaming configuration.
 * @details This function calls: \ref CamDeviceSetPathStreaming, \ref CamDevicePipeStartStreaming,
 * \ref CamDevicePipeStopStreaming, \ref CamDeviceSetPathStreaming
 * @details This function is called by: User application, VsiCamDeviceStartStreaming
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetPathStreaming
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets output path streaming.
 * @startuml VsiCamDeviceGetPathStreaming
 * !include E01_External/VsiCamDeviceGetPathStreaming.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pConfig     Pointer to the output path streaming configuration.
 * @details This function calls: \ref CamDeviceGetPathStreaming
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetPathStreaming
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets the instance ID.
 * @startuml VsiCamDeviceGetHardwareId
 * !include E01_External/VsiCamDeviceGetHardwareId.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pHwId       Pointer to hardware pipeline ID.
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceGetHardwareId
(
    CamDeviceHandle_t hCamDevice,
    uint32_t          *pHwId
);


//uint32_t CamDeviceGetVirtualId
//(
//    uint32_t instanceId
//);

/*****************************************************************************/
/**
 * @brief   This function allocates the memory from internal reserved memory.
 * @startuml VsiCamDeviceAllocResMemory
 * !include E01_External/VsiCamDeviceAllocResMemory.plantuml
 * @enduml
 * @param[in]       hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       byteSize      The total byte size of allocate memory.
 * @param[inout]    pBaseAddress  The pointer of physical base address of
 *                                allocated memory.
 * @param[inout]    pIplAddress   The pointer of the mapped virtual base
 *                                address.
 * @details This function calls: \ref CamDeviceAllocResMemory
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_CONFIG    Operation failed due to invalid parameter
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAllocResMemory
(
    CamDeviceHandle_t   hCamDevice,
    uint32_t            byteSize,
    uint32_t           *pBaseAddress,
    void              **pIplAddress
);

/*****************************************************************************/
/**
 * @brief   This function frees the memory of internal reserved memory.
 * @startuml VsiCamDeviceFreeResMemory
 * !include E01_External/VsiCamDeviceFreeResMemory.plantuml
 * @enduml
 * @param[in]    hCamDevice   Handle to the VsCamDevice instance.
 * @param[in]    baseAddress  The physical base address of free memory.
 * @details This function calls: \ref CamDeviceFreeResMemory
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceFreeResMemory
(
    CamDeviceHandle_t   hCamDevice,
    uint32_t            baseAddress
);


#ifdef ISP_OFFLINE_TEST
/*****************************************************************************/
/**
 * @brief   This function sets the name of offline case.
 * @startuml VsiCamDeviceSetCaseName
 * !include E01_External/VsiCamDeviceSetCaseName.plantuml
 * @enduml
 * @param[in]    pCaseName  The name of offline case.
 * @param[in]    len        The string length for pCaseName.
 * @details This function is called by: User application
 *
 * @return  void.
 *
 *****************************************************************************/
void VsiCamDeviceSetCaseName
(
    char *pCaseName,
    uint32_t len
);
#endif

/*****************************************************************************/
/**
 * @brief   This function writes data to the specified register.
 * @startuml VsiCamDeviceWriteRegister
 * !include E01_External/VsiCamDeviceWriteRegister.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]    address     Data address.
 * @param[in]    value       Data value.
 * @details This function calls: \ref CamDeviceWriteRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceWriteRegister
(
    CamDeviceHandle_t     hCamDevice,
    uint32_t              address,
    uint32_t              value
);

/*****************************************************************************/
/**
 * @brief   This function reads data from the specified register.
 * @startuml VsiCamDeviceReadRegister
 * !include E01_External/VsiCamDeviceReadRegister.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       address     Data address.
 * @param[inout]    pValue      Pointer to data value.
 * @details This function calls: \ref CamDeviceReadRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceReadRegister
(
    CamDeviceHandle_t     hCamDevice,
    uint32_t              address,
    uint32_t              *pValue
);

/*****************************************************************************/
/**
 * @brief   This function sets metadata configuration parameters.
 * @startuml VsiCamDeviceConfigMetadata
 * !include E01_External/VsiCamDeviceConfigMetadata.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pMetadata   Pointer to metadata value.
 * @details This function calls: \ref CamDeviceImageSetMetadata
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceConfigMetadata
(
 CamDeviceHandle_t          hCamDevice,
 CamDeviceMetadataConfig_t *pMetadata
);

/*****************************************************************************/
/**
 * @brief   This function enables noise removal relocation.
 * @startuml VsiCamDeviceNrRelocEnable
 * !include E01_External/VsiCamDeviceNrRelocEnable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceNrRelocEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceNrRelocEnable
(
 CamDeviceHandle_t     hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables noise removal relocation.
 * @startuml VsiCamDeviceNrRelocDisable
 * !include E01_External/VsiCamDeviceNrRelocDisable.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceNrRelocDisable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceNrRelocDisable
(
 CamDeviceHandle_t     hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function implements ISP software fast stop.
 * @startuml VsiCamDeviceSwFastStop
 * !include E01_External/VsiCamDeviceSwFastStop.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceSensorIsiSetStreamEnable,
 * \ref CamEngineFastStop
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_UNSUPPORT_ID    Operation failed due to
 *                              unsupported index number
 *
 *****************************************************************************/
RESULT VsiCamDeviceSwFastStop
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function implements hardware system reset.
 * @startuml VsiCamDeviceHwSystemReset
 * !include E01_External/VsiCamDeviceHwSystemReset.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineHwReset
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHwSystemReset
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function implements ISP software fast start.
 * @startuml VsiCamDeviceSwFastStart
 * !include E01_External/VsiCamDeviceSwFastStart.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamEngineFastStart,
 * \ref CamDeviceSensorIsiSetStreamEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_UNSUPPORT_ID    Operation failed due to
 *                              unsupported index number
 *
 *****************************************************************************/
RESULT VsiCamDeviceSwFastStart
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets low power mode.
 * @startuml VsiCamDeviceSetIspLowPower
 * !include E01_External/VsiCamDeviceSetIspLowPower.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       lowPower    Low power bool variable.
 * @details This function calls: \ref CamDeviceSetIspLowPower
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetIspLowPower
(
    CamDeviceHandle_t hCamDevice,
    bool_t            lowPower
);

RESULT SendLoadBinStart();

 /** @} 27_cam_device_general */

#ifdef __cplusplus
}
#endif

#endif    // CAMDEV_API_H
