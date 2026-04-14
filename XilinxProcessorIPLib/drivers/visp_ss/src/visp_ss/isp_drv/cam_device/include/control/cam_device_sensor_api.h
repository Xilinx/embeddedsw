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

#ifndef CAMDEV_SENSOR_API_H
#define CAMDEV_SENSOR_API_H

#include "cam_device_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup 28_cam_device_sensor VsCamDevice E01C28 Device_SensorCtrl Definitions
 * @brief The sensor control API provides a top-level interface for controlling sensors,
 * mainly encapsulating some functions of the ISI layer and combining them with the
 * current working state to control sensors.
 * @{
 */

#define SENSOR_QUERY_NUM 12U   /**< Sensor query number */
#define FILE_NAME_LEN 30      /**< Sensor file name length */
#define COMPAND_CURVE_LEN 65  /**< Sensor compand curve length */
#define SENSOR_ID_LEN 11      /**< Image sensor ID length */
#define MODULE_SN_LEN 12      /**< Barcode-Module SN length */
#define CAMDEV_SENSOR_EXP_NUM_MAX 4U  /**< Maximum exposure number of sensor*/
#define CAMDEV_SENSOR_METADATA_WIN_NUM_MAX 3  /**< Maximum sensor metedata window number */

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorType_e {
    CAMDEV_SENSOR_TYPE_LINEAR = 0,          /**< Sensor type linear*/
    CAMDEV_SENSOR_TYPE_STITCHING_HDR,       /**< Sensor type stitching HDR */
    CAMDEV_SENSOR_TYPE_NATIVE_HDR           /**< Sensor type native HDR */
}CamDeviceSensorType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor native HDR mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorNativeMode_e {
    CAMDEV_SENSOR_NATIVE_MODE_DCG = 0,          /**< HCG and LCG combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_L_AND_S,          /**< L+S combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_3DOL,             /**< 3DOL combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_4DOL,             /**< 4DOL combined in sensor*/
    CAMDEV_SENSOR_NATIVE_MODE_DCG_SPD_VS,       /**< 4DOL combined in sensor*/
    CAMDEV_SENSOR_NATIVE_MODE_MAX
}CamDeviceSensorNativeMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor AF mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorAutoFocusMode_s {
    CAMDEV_SENSOR_AF_NOTSUPP,       /**< AF not support */
    CAMDEV_SENSOR_AF_MODE_CDAF,     /**< CDAF mode */
    CAMDEV_SENSOR_AF_MODE_PDAF      /**< PDAF mode */
}CamDeviceSensorAutoFocusMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor focusing position mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorFocusPosMode_e {
    CAMDEV_SENSOR_FOCUS_POS_ABSOLUTE = 0,    /**< Absolute focusing mode */
    CAMDEV_SENSOR_FOCUS_POS_RELATIVE,        /**< Relative focusing mode */
    CAMDEV_SENSOR_FOCUS_POS_MAX
} CamDeviceSensorFocusPosMode_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor output data type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorDataType_e
{
    CAMDEV_SENSOR_DATA_TYPE_BAYER   = 0,   /**< Bayer pattern data type */
    CAMDEV_SENSOR_DATA_TYPE_YUV     = 1,   /**< YUV data type */
    CAMDEV_SENSOR_DATA_TYPE_MAX
} CamDeviceSensorDataType_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor interface type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorItfType_e
{
    CAMDEV_SENSOR_ITF_TYPE_MIPI_1LANES     = 0,   /**< 1 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_MIPI_2LANES     = 1,   /**< 2 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_MIPI_4LANES     = 2,   /**< 4 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_BT601           = 3,   /**< BT.601 type */
    CAMDEV_SENSOR_ITF_TYPE_MAX
} CamDeviceSensorItfType_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice isi sensor driver configuration.
 *
 *****************************************************************************/
typedef void* CamDeviceSensorDrvHandle_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor compand configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorCompandCurve_s {
    bool_t   enable;                          /**< Enable value */
    uint8_t  inBit;                           /**< Compand curve input bit value */
    uint8_t  outBit;                          /**< Compand curve output bit value */
    uint8_t  px[COMPAND_CURVE_LEN-1];         /**< X axis interval */
    uint32_t xData[COMPAND_CURVE_LEN];        /**< Compand curve X axis - 65 points */
    uint32_t yData[COMPAND_CURVE_LEN];        /**< Compand curve Y axis - 65 points */
}CamDeviceSensorCompandCurve_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor test pattern configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorTestPattern_s {
    uint32_t enable;                        /**< Test pattern enable value */
    uint32_t pattern;                       /**< Sensor pattern type */
}CamDeviceSensorTestPattern_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor resolution structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolution_s {
    uint32_t width;                   /**< Sensor resoulution width */
    uint32_t height;                  /**< Sensor resoulution height */
}CamDeviceSensorResolution_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor resolution collection information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolutionCollection_s {
    int index;                                    /**< Sensor mode index */
    CamDeviceSensorResolution_t resolution;       /**< Sensor resolution */
}CamDeviceSensorResolutionCollection_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor register configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRegister_s {
    uint16_t addr;                /**< Sensor register address */
    uint16_t value;               /**< Sensor register value */
}CamDeviceSensorRegister_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor size information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorSize_s {
	uint32_t boundsWidth;                 /**< Sensor bounds width */
	uint32_t boundsHeight;                /**< Sensor bounds height */
	uint32_t top;                         /**< Top position */
	uint32_t left;                        /**< Left position */
	uint32_t width;                       /**< Real image width */
	uint32_t height;                      /**< Real image height */
}CamDeviceSensorSize_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor compress information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorCompress_s {
	uint32_t enable;                  /**< Enable value */
	uint32_t xBit;                    /**< Expand curve input data bit width */
	uint32_t yBit;                    /**< Expand curve output data bit width */
}CamDeviceSensorCompress_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor mode information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorModeInfo_s {
    uint32_t index;                              /**< Sensor mode index value */
    CamDeviceSensorSize_t size;                  /**< Sensor size */
    CamDeviceSensorType_t sensorType;            /**< The sensor type is linear or HDR*/
    CamDeviceStitchingMode_t     stitchingMode;  /**< The sensor type is HDR stitching*/
    CamDeviceSensorNativeMode_t  nativeMode;     /**< The sensor type is HDR Native */
    uint32_t bitWidth;                           /**< Sensor bit width */
    CamDeviceSensorCompress_t compress;          /**< Sensor compression information */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type*/
    uint32_t maxFps;                             /**< Sensor maximum FPS value */
    CamDeviceSensorAutoFocusMode_t   afMode;     /**< Sensor auto focusing mode */
}CamDeviceSensorModeInfo_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor query information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorQuery_s {
    uint32_t number;                                              /**< Query sensor index number */
    CamDeviceSensorModeInfo_t sensorModeInfo[SENSOR_QUERY_NUM];   /**< Query sensor mode information */
}CamDeviceSensorQuery_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRange_s {
    float32_t max;             /**< Maximum value */
    float32_t min;             /**< Minimum value */
    float32_t step;            /**< Step value */
}CamDeviceSensorRange_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor integer range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorIntegerRange_s {
    uint32_t  max;         /**< Maximum value*/
    uint32_t  min;         /**< Minimum value*/
    uint32_t  step;        /**< Step value */
}CamDeviceSensorIntegerRange_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor motor position structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorFocusPos_s {
    CamDeviceSensorFocusPosMode_t  posMode;  /**< Sensor motor focusing position mode*/
    uint32_t  position;                      /**< Sensor motor focusing position, the ranging is from 0 to 1023. */
}CamDeviceSensorFocusPos_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor gain configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorGain_s {
    float32_t aGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor analog gain */
    float32_t dGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor digital gain */
}CamDeviceSensorGain_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor status information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorStatus_s {
    bool_t isConnected;          /**< Sensor connection status */
    CamDeviceSensorGain_t currGain; /**< Sensor current gain info */
    float32_t currIntTime[CAMDEV_SENSOR_EXP_NUM_MAX];              /**< Sensor current integration time info: the unit is microseconds  */
    CamDeviceSensorResolution_t  currRes;      /**< Sensor current resolution info */
    float32_t currFps;                             /**< Sensor current FPS info */
}CamDeviceSensorStatus_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorExposureControl_s {
    float32_t integrationTime[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor integration time value */
}CamDeviceSensorExposureControl_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor bls configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorBls_s {
    uint32_t bls[CAMDEV_RAW_CHANNEL_NUM];       /**< BLS value:
                                                 Raw : BLS[0]--red, BLS[1]--greenRed, BLS[2]--greenBlue, BLS[3]--blue. The bls order shold be corresponding with sensor bayer pattern\n
                                                 Rgbir: BLS[0]--red, BLS[1]--green, BLS[2]--blue, BLS[3]--ir. Algorithm ensure the order: 0->r, 1->g, 2->b, 3->ir, bls don't need to correspond with sensor bayer pattern */
}CamDeviceSensorBls_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor bls configuration structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorWb_s {
    float32_t gain[CAMDEV_RAW_CHANNEL_NUM];       /**< WB gain */
}CamDeviceSensorWb_t;


/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef union CamDeviceSensorMetadataAttr_s {
    struct {
        uint32_t support  : 1;   /**< bit 0: 0-disable 1-enable */
        uint32_t regInfo  : 1;   /**< bit 1: register information */
        uint32_t expTime  : 1;   /**< bit 2: exposure time */
        uint32_t again    : 1;   /**< bit 3: Analogue again */
        uint32_t dgain    : 1;   /**< bit 4: Digital gain */
        uint32_t bls      : 1;   /**< bit 5: BLS */
        uint32_t hist     : 1;   /**< bit 6: Histogram */
        uint32_t meanLuma : 1;   /**< bit 7: Mean luminance */
        uint32_t frameCRC : 1;   /**< bit 8: Frame CRC */
        uint32_t reservedEnable : 23;/**< bit 9:31: Reserved bit */

    }subAttr; /**< Sub-attribute */
    uint32_t mainAttr;  /**< Main attribute */
}CamDeviceSensorMetadataAttr_t;


/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor meatdata window information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorMetadataWin_s {
    uint8_t winNum;  /**< The number of windows */
    CamDeviceWindow_t metaWin[CAMDEV_SENSOR_METADATA_WIN_NUM_MAX];  /**< Metadata windows */
}CamDeviceSensorMetadataWin_t;


/*****************************************************************************/
/**
 * @brief   CamDevice sensor meatdata parser information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorMetadataParserInfo_s
{
    CamDeviceSensorMetadataAttr_t validMask;

    uint32_t regNum;
    CamDeviceSensorRegister_t *pReg;

    uint8_t expFrmNum;
    uint32_t frmCRC;
    CamDeviceSensorExposureControl_t expTime;
    CamDeviceSensorGain_t    aGain;
    CamDeviceSensorGain_t    dGain;
    CamDeviceSensorBls_t     blc;
    // IsiSensorHist_t    hist;
    // IsiSensorMeanLuma_t meanLuma;

} CamDeviceSensorMetadataParserInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor meatdata information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorMetadata_s {
    uint32_t chipId;   //sensor version id
    uint32_t frmCount;

    CamDeviceSensorMetadataParserInfo_t data;
}CamDeviceSensorMetadata_t;


/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorInfo_s {

    char sensorName[CAMDEV_INPUT_DEV_NAME_LEN];  /**< Sensor name */
    uint32_t sensorRevId;                        /**< Sensor revision ID register */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type */
    CamDeviceSensorTestPattern_t testPattern;    /**< Sensor testPattern info*/

    CamDeviceSensorRange_t  aGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor analog gain range*/
    CamDeviceSensorRange_t  dGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor digital gain range*/
    CamDeviceSensorRange_t  intTimeRange[CAMDEV_SENSOR_EXP_NUM_MAX];      /**< Sensor integration time range: the unit is microseconds */

    CamDeviceSensorMetadataAttr_t metaSupp;   /**< Sensor metadata support information */
}CamDeviceSensorInfo_t;

/*****************************************************************************/
/**
 * @brief   VsCamDevice sensor one time program information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorOtpModuleInfo_s {
    uint16_t hwVersion;                        /**< Module HW version */
    uint16_t SensorEEPROMRevision;             /**< EEPROM revision */
    uint16_t sensorRevision;                   /**< Image sensor revision */
    uint16_t tLensRevision;                    /**< Tlens revision */
    uint16_t ircfRevision;                     /**< Ircf revision */
    uint16_t lensRevision;                     /**< Lens revision */
    uint16_t caRevision;                       /**< Contact assembly revision */
    uint16_t moduleInteID;                     /**< Module integrator ID */
    uint16_t factoryID;                        /**< Factory ID */
    uint16_t mirrorFlip;                       /**< Image mirror and flip */
    uint16_t tLensSlaveID;                     /**< Tlens slave ID */
    uint16_t SensorEEPROMSlaveID;              /**< EEPROM slave ID */
    uint16_t sensorSlaveID;                    /**< Image sensor slave ID */
    uint8_t  sensorID[SENSOR_ID_LEN];          /**< Image sensor ID */
    uint16_t manuDateYear;                     /**< Manufacture Date (Year) */
    uint16_t manuDateMonth;                    /**< Manufacture Date (Month) */
    uint16_t manuDateDay;                      /**< Manufacture Date (Date) */
    uint8_t  barcodeModuleSN[MODULE_SN_LEN];   /**< Barcode-Module SN */
    uint16_t mapTotalSize;                     /**< Total size of EEPROM map */
}CamDeviceSensorOtpModuleInfo_t;

/*****************************************************************************/
/**
 * @brief   Gets the sensor information, e.g., sensor name, calibration file, etc.
 * @startuml VsiCamDeviceSensorGetInfo
 * !include E01_External/VsiCamDeviceSensorGetInfo.plantuml
 * @enduml
 * @param[in]       hCamDevice     Handle to the VsCamDevice instance.
 * @param[inout]    pSensorInfo  The pointer to sensor information structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetInfo
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorInfo_t *pSensorInfo
);

/*****************************************************************************/
/**
 * @brief   Opens the sensor from the sensor driver.
 * @startuml VsiCamDeviceSensorOpen
 * !include E01_External/VsiCamDeviceSensorOpen.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       modeIndex   sensor drv mode index.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiOpen
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorOpen
(
    CamDeviceHandle_t hCamDevice,
    uint32_t          modeIndex
);

/*****************************************************************************/
/**
 * @brief   Closes the sensor from the sensor driver.
 * @startuml VsiCamDeviceSensorClose
 * !include E01_External/VsiCamDeviceSensorClose.plantuml
 * @enduml
 * @param[inout]    hCamDevice     Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiClose
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorClose
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   Registers sensor driver handle.
 * @startuml VsiCamDeviceSensorDrvHandleRegister
 * !include E01_External/VsiCamDeviceSensorDrvHandleRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice       Handle to the VsCamDevice instance.
 * @param[inout]    sensorDrvHandle  Handle to sensor driver.
 * @details This function calls: \ref CamDeviceSensorDrvHandleRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorDrvHandleRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorDrvHandle_t sensorDrvHandle
);

/*****************************************************************************/
/**
 * @brief   Unregisters sensor driver handle.
 * @startuml VsiCamDeviceSensorDrvHandleUnRegister
 * !include E01_External/VsiCamDeviceSensorDrvHandleUnRegister.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @details This function calls: \ref CamDeviceSensorDrvHandleUnRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 * @retval  RET_OUTOFMEM        Operation failed due to out of memory
 * @retval  RET_WRONG_CONFIG    Operation failed due to given
 *                              configuration is invalid
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorDrvHandleUnRegister
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   Changes the sensor output from a live image to a test pattern.
 * @startuml VsiCamDeviceSensorSetTestPattern
 * !include E01_External/VsiCamDeviceSensorSetTestPattern.plantuml
 * @enduml
 * @param[inout]    hCamDevice     Handle to the VsCamDevice instance.
 * @param[in]       pTestPattern   The pointer to sensor test pattern structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetTestPattern
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetTestPattern
(
    CamDeviceHandle_t hCamDevice,
    const CamDeviceSensorTestPattern_t *pTestPattern
);

/*****************************************************************************/
/**
 * @brief   Configures the sensor registers via I2C.
 * @startuml VsiCamDeviceSensorSetRegister
 * !include E01_External/VsiCamDeviceSensorSetRegister.plantuml
 * @enduml
 * @param[in]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]    pRegister   The pointer to sensor register structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiWriteRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
);

/*****************************************************************************/
/**
 * @brief   Gets the sensor registers via I2C.
 * @startuml VsiCamDeviceSensorGetRegister
 * !include E01_External/VsiCamDeviceSensorGetRegister.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pRegister   The pointer to sensor register structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiReadRegister
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
);

/*****************************************************************************/
/**
 * @brief   Sets the sensor frame rate.
 * @startuml VsiCamDeviceSensorSetFrameRate
 * !include E01_External/VsiCamDeviceSensorSetFrameRate.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pFps        The pointer to sensor frame per second.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetFrameRate
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetFrameRate
(
    CamDeviceHandle_t hCamDevice,
    float32_t *pFps
);

/*****************************************************************************/
/**
 * @brief   Gets the current frame rate of the sensor.
 * @startuml VsiCamDeviceSensorGetFrameRate
 * !include E01_External/VsiCamDeviceSensorGetFrameRate.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pFps        The pointer to sensor frame per second.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetFrameRate
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetFrameRate
(
    CamDeviceHandle_t hCamDevice,
    float32_t *pFps
);

/*****************************************************************************/
/**
 * @brief   Gets the current working sensor mode, including the sensor working
            status, (HDR/Linear, image width/height, etc.).
 * @startuml VsiCamDeviceSensorGetModeInfo
 * !include E01_External/VsiCamDeviceSensorGetModeInfo.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pMode       The pointer to sensor mode structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetModeInfo
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetModeInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorModeInfo_t *pMode
);

/*****************************************************************************/
/**
 * @brief   Gets all supported modes for the sensor to the application, including
            sensor driver mode(width, height, bit width, etc).
 * @startuml VsiCamDeviceSensorQuery
 * !include E01_External/VsiCamDeviceSensorQuery.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pQuery      The pointer to sensor query structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetQuery
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_INVALID_PARM    Operation failed due to invalid configuration
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorQuery
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorQuery_t *pQuery
);

/*****************************************************************************/
/**
 * @brief   Gets the current sensor exposure time, gain value, etc.
 * @startuml VsiCamDeviceSensorGetStatus
 * !include E01_External/VsiCamDeviceSensorGetStatus.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pStatus     The pointer to sensor status structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetStatus
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   Sets gain value to the sensor driver.
 * @startuml VsiCamDeviceSensorSetGain
 * !include E01_External/VsiCamDeviceSensorSetGain.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pGain       The pointer to sensor gain structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetGain
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
);

/*****************************************************************************/
/**
 * @brief   Gets gain value from sensor driver.
 * @startuml VsiCamDeviceSensorGetGain
 * !include E01_External/VsiCamDeviceSensorGetGain.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pGain       The pointer to sensor gain structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetGain
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
);

/*****************************************************************************/
/**
 * @brief   Sets exposure time value to the sensor driver.
 * @startuml VsiCamDeviceSensorSetExposureControl
 * !include E01_External/VsiCamDeviceSensorSetExposureControl.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pExpCtrl    The pointer to sensor exposure structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetExpControl
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetExposureControl
(
    CamDeviceHandle_t                  hCamDevice,
    CamDeviceSensorExposureControl_t  *pExpCtrl
);

/*****************************************************************************/
/**
 * @brief   Gets exposure time value from the sensor driver.
 * @startuml VsiCamDeviceSensorGetExposureControl
 * !include E01_External/VsiCamDeviceSensorGetExposureControl.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pExpCtrl    The pointer to sensor exposure structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetExpControl
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetExposureControl
(
    CamDeviceHandle_t                  hCamDevice,
    CamDeviceSensorExposureControl_t  *pExpCtrl

);

/*****************************************************************************/
/**
 * @brief   Sets sensor motor focusing position.
 * @startuml VsiCamDeviceSensorSetFocusPositon
 * !include E01_External/VsiCamDeviceSensorSetFocusPositon.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pFocusPos   The pointer to the focusing position.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetFocusPos
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetFocusPositon
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorFocusPos_t  *pFocusPos
);

/*****************************************************************************/
/**
 * @brief   Gets sensor motor focusing position.
 * @startuml VsiCamDeviceSensorGetFocusPositon
 * !include E01_External/VsiCamDeviceSensorGetFocusPositon.plantuml
 * @enduml
 * @param[in]       hCamDevice   Handle to the VsCamDevice instance.
 * @param[inout]    pFocusPos    The pointer to the sensor focusing position.
 * @param[inout]    pRangeLimit  The pointer to sensor integer range information.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetFocusPos,
 * CamDeviceSensorIsiGetFocusCalibData
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_OUTOFRANGE      Operation failed due to
 *                              parameter/variable out of range
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetFocusPositon
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorFocusPos_t  *pFocusPos,
    CamDeviceSensorIntegerRange_t *pRangeLimit
);

/*****************************************************************************/
/**
 * @brief   Sets streaming on or off to the sensor driver.
 * @startuml VsiCamDeviceSensorSetStreaming
 * !include E01_External/VsiCamDeviceSensorSetStreaming.plantuml
 * @enduml
 * @param[inout]    hCamDevice    Handle to the VsCamDevice instance.
 * @param[in]       isEnable      Enable or Disable the streaming.
 * @details This function calls: \ref CamDeviceSensorIsiSetStreamEnable
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetStreaming
(
    CamDeviceHandle_t hCamDevice,
    bool_t isEnable
);

/*****************************************************************************/
/**
 * @brief   Gets the one-time program information from sensor driver.
 * @startuml VsiCamDeviceSensorGetOtpInfo
 * !include E01_External/VsiCamDeviceSensorGetOtpInfo.plantuml
 * @enduml
 * @param[in]       hCamDevice      Handle to the VsCamDevice instance.
 * @param[inout]    pOtpModuleInfo  The pointer to sensor OTP module information structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetOtpInfo
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetOtpInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorOtpModuleInfo_t *pOtpModuleInfo
);

/*****************************************************************************/
/**
 * @brief   Gets sensor metadata support attribute.
 * @startuml VsiCamDeviceSensorGetMetadataAttr
 * !include E01_External/VsiCamDeviceSensorGetMetadataAttr.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pMetaAttr   The pointer to metadata attribute parameters.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetMetadataAttr
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t *pMetaAttr
);

/*****************************************************************************/
/**
 * @brief   Sets sensor metadata enable attribute.
 * @startuml VsiCamDeviceSensorSetMetadataAttr
 * !include E01_External/VsiCamDeviceSensorSetMetadataAttr.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       metaAttr    Metadata attribute parameters.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetMetadataAttr
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t metaAttr
);

/*****************************************************************************/
/**
 * @brief   Gets sensor metadata window information.
 * @startuml VsiCamDeviceSensorGetMetadataWin
 * !include E01_External/VsiCamDeviceSensorGetMetadataWin.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pMetaWin    The pointer to sensor metadata window info.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetMetadataWindow
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetMetadataWin
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataWin_t *pMetaWin
);

/*****************************************************************************/
/**
 * @brief   Parse sensor metadata information.
 * @startuml VsiCamDeviceSensorParaserMetadata
 * !include E01_External/VsiCamDeviceSensorParaserMetadata.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pMetaBuf    The pointer to sensor metadata buffer.
 * @param[inout]    pMetaInfo   The pointer to sensor metadata information.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetMetadataWindow
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorParaserMetadata
(
    CamDeviceHandle_t hCamDevice,
    const MetadataBufInfo_t *pMetaBuf,
    CamDeviceSensorMetadata_t *pMetaInfo
);

/*****************************************************************************/
/**
 * @brief   Sets bls value to the sensor driver.
 * @startuml VsiCamDeviceSensorSetBls
 * !include E01_External/VsiCamDeviceSensorSetBls.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pBls        The pointer to sensor bls structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetBls
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetBls
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorBls_t *pBls
);

/*****************************************************************************/
/**
 * @brief   Gets bls value from the sensor driver.
 * @startuml VsiCamDeviceSensorGetBls
 * !include E01_External/VsiCamDeviceSensorGetBls.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pBls        The pointer to sensor bls structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetBls
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetBls
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorBls_t *pBls
);

/*****************************************************************************/
/**
 * @brief   Sets wb value to the sensor driver.
 * @startuml VsiCamDeviceSensorSetWb
 * !include E01_External/VsiCamDeviceSensorSetWb.plantuml
 * @enduml
 * @param[inout]    hCamDevice  Handle to the VsCamDevice instance.
 * @param[in]       pWb         The pointer to sensor wb structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiSetWb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetWb
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorWb_t *pWb
);

/*****************************************************************************/
/**
 * @brief   Gets wb value from the sensor driver.
 * @startuml VsiCamDeviceSensorGetWb
 * !include E01_External/VsiCamDeviceSensorGetWb.plantuml
 * @enduml
 * @param[in]       hCamDevice  Handle to the VsCamDevice instance.
 * @param[inout]    pWb         The pointer to sensor wb structure.
 * @details This function calls: \ref CamDeviceSensorCheckValid,
 * \ref CamDeviceSensorIsiGetWb
 * @details This function is called by: User application
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_NULL_POINTER    Operation failed due to invalid pointer(s)
 * @retval  RET_WRONG_HANDLE    Operation failed due to wrong handle
 * @retval  RET_NOTSUPP         Operation aborted due to feature not supported
 * @retval  RET_WRONG_STATE     Operation failed due to wrong state
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetWb
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorWb_t *pWb
);

/** @} 28_cam_device_sensor */

#ifdef __cplusplus
}
#endif

#endif    // CAMDEV_SENSOR_API_H
