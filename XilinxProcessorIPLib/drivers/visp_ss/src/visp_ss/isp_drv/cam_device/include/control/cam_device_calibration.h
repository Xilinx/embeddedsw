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
 * @defgroup cam_device_calibration CamDevice Calibration Definitions
 * @{
 *
 *
 */

#ifndef CAMDEV_CALIBRATION_H
#define CAMDEV_CALIBRATION_H

#include <types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_CALIB_LENGTH 50

/*****************************************************************************/
/**
 * @brief   CamDevice Calibration doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum CamDeviceDoorType_e {
	CAMDEV_DOOR_TYPE_OUTDOOR = 0,   /**< Scene type, outdoor */
	CAMDEV_DOOR_TYPE_INDOOR,        /**< Scene type, indoor */
	CAMDEV_DOOR_TYPE_MAX,
	CAMDEV_DUMMY_004 = 0xDEADFEED
} CamDeviceDoorType_t;

/*****************************************************************************/
/**
 * @brief   CamDevice Calibration AWB manual/auto control type of an illumination profile
 */
/*****************************************************************************/
typedef enum CamDeviceCalibAwbType_e {
	CAMDEV_CALIB_AWB_TYPE_MANUAL = 0,    /**< Manual AWB type */
	CAMDEV_CALIB_AWB_TYPE_AUTO,           /**< Auto AWB type */
	CAMDEV_CALIB_AWB_TYPE_MAX,
	CAMDEV_DUMMY_005 = 0xDEADFEED
} CamDeviceCalibAwbType_t;

/*****************************************************************************/
/**
 * @brief   CamDevice illumination type of calibration
 */
/*****************************************************************************/
typedef enum CamDeviceCalibIllumType_e {
	CAMDEV_CALIB_ILLUM_TYPE_A = 0,    /**< Illumination type: A */
	CAMDEV_CALIB_ILLUM_TYPE_D50,      /**< Illumination type: D50*/
	CAMDEV_CALIB_ILLUM_TYPE_D65,      /**< Illumination type: D65 */
	CAMDEV_CALIB_ILLUM_TYPE_D75,      /**< Illumination type: D75 */
	CAMDEV_CALIB_ILLUM_TYPE_F2,       /**< Illumination type: F2 */
	CAMDEV_CALIB_ILLUM_TYPE_F11,      /**< Illumination type: F11 */
	CAMDEV_CALIB_ILLUM_TYPE_F12,      /**< Illumination type: F12 */
	CAMDEV_CALIB_ILLUM_TYPE_H,        /**< Illumination type: H */
	CAMDEV_CALIB_ILLUM_TYPE_MAX,
	CAMDEV_DUMMY_006 = 0xDEADFEED
} CamDeviceCalibIllumType_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration file header resolution parameters: framerate configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibHeaderResolutionFramerate_s {
	char name[CAMDEV_CALIB_LENGTH];      /**< Framerate header name */
	float fps;          /**< Framerate value */
} CamDeviceCalibHeaderResolutionFramerate_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration file header parameters: resolution configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibHeaderResolution_s {
	char name[CAMDEV_CALIB_LENGTH];      /**< Resolution header name */
	char id[CAMDEV_CALIB_LENGTH];        /**< Resolution index */
	uint16_t width;     /**< Width */
	uint16_t height;    /**< Heigth */

	CamDeviceCalibHeaderResolutionFramerate_t *pFramerate;      /**< Framerate list */
	uint8_t framerateNumber;                                    /**< The number of framerate list members */
} CamDeviceCalibHeaderResolution_t;


/******************************************************************************/
/**
 * @brief   CamDevice Calibration file header configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibHeader_s {
	char date[CAMDEV_CALIB_LENGTH];                  /**< Calibration file date */
	char creator[CAMDEV_CALIB_LENGTH];               /**< Calibration file creator */
	char sensorName[CAMDEV_CALIB_LENGTH];            /**< Calibration file sensor name */
	char sampleName[CAMDEV_CALIB_LENGTH];            /**< Calibration file sample name */
	char generatorVersion[CAMDEV_CALIB_LENGTH];      /**< Calibration file generator version */

	CamDeviceCalibHeaderResolution_t
	*pResolutions; /**< Calibration file header resolution parameters */
	uint8_t resolutionNumber;                       /**< Resolution parameters number */
} CamDeviceCalibHeader_t;


/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB IIR configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIIR_s {
	float dampCoefAdd;              /**< Increment of damping, which is an infinite impulse response filter (IIR) parameter */
	float dampCoefSub;              /**< Decrement of damping, which is an IIR parameter. */
	float dampFilterThreshold;      /**< Threshold of indoor probability change, which is an IIR parameter. */
	float dampingCoefMin;           /**< Minimum value of damping, which is an IIR parameter. */
	float dampingCoefMax;           /**< Maximum value of damping, which is an IIR parameter. */
	float dampingCoefInit;          /**< Initial value of the ring-shaped buffer (multi-frame indoor probability). */
	int32_t expPriorFilterSizeMax;  /**< Ring buffer size of the AWB damping queue, which is an IIR parameter. */
	int32_t expPriorFilterSizeMin;  /**< IIR parameter, which is currently unused. */
	float expPriorMiddle;           /**< IIR parameter, which is currently unused. */
} CamDeviceCalibSensorAwbIIR_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB global configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbGlobal_s {
	char name[CAMDEV_CALIB_LENGTH];                  /**< AWB header name */
	char resolution[CAMDEV_CALIB_LENGTH];            /**< Resolution */
	float svdMeanValue[3];          /**< Mean of singular value decomposition */
	float pcaMatrix[6];             /**< Transformation matrix of the PCA domain */
	float centerLine[3];            /**< Position coordinate of first-order straight lines fitted from white points during AWB calibration. */
	float afRg2[16];                /**< Horizontal coordinate of the projection of the upper edge of the black box along the sampling point to the centerline during AWB calibration */
	float afMaxDist2[16];           /**< Distance between the horizontal coordinate of the sample point on the upper edge of the black box and afRg2 during AWB calibration. */
	float afRg1[16];                /**< Horizontal coordinate of the projection of the sampling point to the centerline along the lower edge of the black box during AWB calibration. */
	float afMaxDist1[16];           /**< Distance between the horizontal coordinate of the sampling point along the lower edge of the black box and afRg1 during AWB calibration. */
	float afGlobalFade2[16];        /**< Horizontal coordinate of the projection of the upper edge of the orange box along the sampling point to the centerline during AWB calibration. */
	float afGlobalGainDistance2[16];    /**< Distance between the horizontal coordinate of the sampling point on the upper edge of the orange box and afGlobalFade2 during AWB calibration. */
	float afGlobalFade1[16];        /**< Horizontal coordinate of the projection of the sampling point to the centerline along the lower edge of the orange box during the AWB calibration. */
	float afGlobalGainDistance1[16];    /**< Distance between the horizontal coordinate of the sampling point on the upper edge of the orange box and afGlobalFade1 during AWB calibration. */
	float fRgProjIndoorMin;         /**< Minimum Rg value in indoor scenes when the AWB clip box is calibrated. */
	float fRgProjMax;               /**< Maximum Rg boundary value of the black box when the AWB clip box is calibrated. */
	float fRgProjMaxSky;            /**< Maximum Rg boundary value of the orange box when the AWB clip box is calibrated. */
	float fRgProjOutdoorMin;        /**< Rg of indoor and outdoor dividing line when the AWB clip box is calibrated. */
	char awbClipOutDoor[CAMDEV_CALIB_LENGTH];    /**< Name of the light source which is the dividing line between indoor and outdoor */
	float kFactor;                  /**< Sensor sensitivity coefficient, which determines whether the scene is indoor or outdoor */
	float afFade2[6];               /**< Six coordinates on the Rg-Bg coordinate system. */
	int32_t afCbMinRegionMax[6];    /**< White points are located in the max range specified by Cb. */
	int32_t afCrMinRegionMax[6];    /**< White points are located in the max range specified by Cr. */
	int32_t afMaxCSumRegionMax[6];  /**< White points are located in the min range specified by Cb and Cr. */
	int32_t afCbMinRegionMin[6];    /**< White points are located in the min range specified by Cb. */
	int32_t afCrMinRegionMin[6];    /**< White points are located in the min range specified by Cr. */
	int32_t afMaxCSumRegionMin[6];  /**< White points are located in the range specified by Cb and Cr. */
	int32_t regionSize;             /**< Controls the proportional scaling of the white point area. */
	float regionSizeInc;            /**< Increment used for adjusting the RegionSize in the driver */
	float regionSizeDec;            /**< Decrement used for adjusting the RegionSize in the driver */

	CamDeviceCalibSensorAwbIIR_t awbIIR;
} CamDeviceCalibSensorAwbGlobal_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination Gaussian-mixed-model configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIlluminationGmm_s {
	float invCovMatrix[4];          /**< InvCovMatrix: 2 x 2 inverse covariance matrix. */
	float gaussianScalingFactor;    /**< GaussianScalingFactor: 1 x 1 Gaussian scaling factor */
	float tau[2];                 /**< Tau: 1 x 2 matrix, corresponding to the interpolation strategies tau1 and tau2 adjusted in the PCA domain during AWB calibration. */
	float gaussianMeanValue[2];     /**< GaussianMeanValue: 1 x 2 matrix, Gaussian mean value. */
} CamDeviceCalibSensorAwbIlluminationGmm_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination  auto LSC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIlluminationAlsc_s {
	char resolution[CAMDEV_CALIB_LENGTH];        /**< Auto LSC resolution */
	char profile[CAMDEV_CALIB_LENGTH * 4];         /**< LSC profile corresponding to the light source */
} CamDeviceCalibSensorAwbIlluminationAlsc_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination saturation configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIlluminationSatCt_s {
	int32_t gain[4];        /**< Saturation gains: a set of gains, 10 gains at most. */
	int32_t saturation[4];  /**< A set of saturation values corresponding to gain values, 10 values at most. */
} CamDeviceCalibSensorAwbIlluminationSatCt_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination LSC compensation configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIlluminationVigCt_s {
	int32_t gain[4];        /**< A set of gains, five gains at most. */
	int32_t vignetting[4];  /**< A set of LSC compensation proportional parameters corresponding to gain values, five parameters at most. */
} CamDeviceCalibSensorAwbIlluminationVigCt_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination auto CC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIlluminationAcc_s {
	char profile[CAMDEV_CALIB_LENGTH * 4]; /**< CC profile corresponding to the light source */
} CamDeviceCalibSensorAwbIlluminationAcc_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB Illumination configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwbIllumination_s {
	char name[CAMDEV_CALIB_LENGTH];                              /**< Name of the light source. */
	char doorType[CAMDEV_CALIB_LENGTH];                          /**< Scene type, which includes indoor and outdoor. */

	CamDeviceCalibSensorAwbIlluminationGmm_t gmm;   /**< Illumination Gaussian-mixed-model. */
	CamDeviceCalibSensorAwbIlluminationAlsc_t alsc; /**< Illumination  auto LSC configuration. */

	float manualWb[4];                              /**< WB gain value calibrated for the light source. */
	float manualCcMatrix[9];                        /**< CC matrix value calibrated for the light source. */
	float manualCcOffset[3];                        /**< CC offset value calibrated for the light source. */

	char awbType[CAMDEV_CALIB_LENGTH];                           /**< AWB type, including AUTO and MANUAL. */

	CamDeviceCalibSensorAwbIlluminationSatCt_t satCt;   /**< Illumination saturation configuration. */
	CamDeviceCalibSensorAwbIlluminationVigCt_t
	vigCt;   /**< Illumination LSC compensation configuration. */
	CamDeviceCalibSensorAwbIlluminationAcc_t
	acc;       /**< Illumination auto color-correction configuration. */
} CamDeviceCalibSensorAwbIllumination_t;


/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AWB configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorAwb_s {
	CamDeviceCalibSensorAwbGlobal_t
	*pGlobals;              /**< Calibration sensor AWB global configuration. */
	uint16_t globalNumber;                                  /**< Global configuration nummber. */

	CamDeviceCalibSensorAwbIllumination_t
	*pIlluminations;  /**< Calibration sensor AWB Illumination configuration */
	uint16_t illuminationNumber;                            /**< Illumination configuration number */
} CamDeviceCalibSensorAwb_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor LSC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorLsc_s {
	char name[CAMDEV_CALIB_LENGTH];              /**< LSC header name. */
	char resolution[CAMDEV_CALIB_LENGTH];        /**< resolution. */
	char illumination[CAMDEV_CALIB_LENGTH];      /**< Illumination. */
	int32_t sectors;                /**< Number of Lens sectors. */
	int32_t no;                /**< Corresponds to HW Repres in the Lens Shade Correction Tool. */
	int32_t xo;                /**< Corresponds to Horizontal multiplication-factors in the Lens Shade Correction Tool. */
	int32_t yo;                /**< Corresponds to Vertical multiplication-factors in the Lens Shade Correction Tool. */
	int32_t sectSizeX[32];     /**< Horizontal spacing between nodes. Because of symmetry, only half of the data is recorded [1x8].. */
	int32_t sectSizeY[32];     /**< Vertical spacing between nodes. Because of symmetry, only half of the data is recorded [1x8].. */
	int32_t vignetting;         /**< Proportion of compensation. */

	int32_t sampleRed[33][33];         /**< Lens compensation coefficient for the red channel, [17 x 17] matrix. */
	int32_t sampleGreenR[33][33];      /**< LSC compensation coefficient of the green red channel, [17 x 17] matrix. */
	int32_t sampleGreenB[33][33];      /**< LSC compensation coefficient of the green blue channel, [17 x 17] matrix. */
	int32_t sampleBlue[33][33];        /**< LSC compensation coefficient of the blue channel, [17 x 17] matrix. */
} CamDeviceCalibSensorLsc_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor CC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorCc_s {
	char name[CAMDEV_CALIB_LENGTH];      /**< Color-correction header name. */
	float saturation;   /**< Saturation value. */
	float ccMatrix[9];  /**< CC matrices of the light source. */
	float ccOffset[3];  /**< CC offset matrices of the light source. */
	float wb[4];        /**< WB gains of the light source. */
} CamDeviceCalibSensorCc_t;


/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor BLS configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensorBls_s {
	char name[CAMDEV_CALIB_LENGTH];          /**< BLS header name. */
	char resolution[CAMDEV_CALIB_LENGTH];    /**< Resolution. */
	int32_t data[4];            /**< BLS value of RAW Bayer channel. */
} CamDeviceCalibSensorBls_t;


/******************************************************************************/
/**
 * @brief   CamDevice Calibration sensor AEC configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSensor_s {
	CamDeviceCalibSensorAwb_t awb;      /**< Calibration sensor AWB configuration. */

	CamDeviceCalibSensorLsc_t *pLsc;    /**< Calibration sensor LSC configuration. */
	uint16_t lscNumber;                 /**< LSC configure number. */

	CamDeviceCalibSensorCc_t *pCc;      /**< Calibration sensor CC configuration. */
	uint16_t ccNumber;                  /**< CC configure number. */

	CamDeviceCalibSensorBls_t *pBls;    /**< Calibration sensor BLS configuration. */
	uint16_t blsNumber;                 /**< BLS configure number. */

} CamDeviceCalibSensor_t;

/******************************************************************************/
/**
 * @brief   CamDevice Calibration system configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibSystem_s {
	char afpsDefault[CAMDEV_CALIB_LENGTH];    /**< Auto FPS default value. */
} CamDeviceCalibSystem_t;

/*****************************************************************************/
/**
 * @brief   CamDevice Calibration configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceCalibCfg_s {
	CamDeviceCalibHeader_t header;      /**< Calibration header configuration. */
	CamDeviceCalibSensor_t sensor;      /**< Calibration sensor configuration. */
	CamDeviceCalibSystem_t system;      /**< Calibration system configuration. */
} CamDeviceCalibCfg_t;


#ifdef __cplusplus
}
#endif

/* @} cam_device_calibration */

#endif    // CAMDEV_CALIBRATION_H
