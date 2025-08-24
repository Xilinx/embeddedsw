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

/* VeriSilicon 2022 */

/**
 * @file isi.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup isi Independent Sensor Interface
 * @{
 *
 */
#ifndef __ISI_H__
#define __ISI_H__

#include <types.h>
#include <return_codes.h>
#include <buf_defs.h>
#include <isi_vvsensor.h>
#include <isi_otp.h>
#include <isi_metadata.h>

#define ISI_LINEAR_PARAS               0
#define ISI_DUAL_EXP_L_PARAS           0
#define ISI_DUAL_EXP_S_PARAS           1
#define ISI_TRI_EXP_L_PARAS            0
#define ISI_TRI_EXP_S_PARAS            1
#define ISI_TRI_EXP_VS_PARAS           2
#define ISI_QUAD_EXP_L_PARAS           0
#define ISI_QUAD_EXP_S_PARAS           1
#define ISI_QUAD_EXP_VS_PARAS          2
#define ISI_QUAD_EXP_VVS_PARAS         3

#define ISI_EXP_NUM_MAX 4

#ifdef __cplusplus
extern "C"
{
#endif

typedef void *IsiSensorHandle_t;
typedef struct IsiSensor_s IsiSensor_t;

typedef struct IsiResolution_s {
	uint16_t width;
	uint16_t height;
} IsiResolution_t;

/*****************************************************************************/
/**
 * @brief   This structure defines the sensor capabilities.
 */
/*****************************************************************************/
typedef struct IsiCaps_s {
	uint32_t bitWidth;                  /**< supported bus-width */
	uint32_t mode;                      /**< supported operating modes */
	uint32_t bayerPattern;              /**< bayer pattern */
	IsiResolution_t resolution;         /**< supported resolutions */
	uint32_t vinType;

	uint32_t mipiMode;	  /**< vinType: ISI_ITF_TYPE_MIPI  bitWidth*/
	uint32_t mipiLanes;   /**< vinType: ISI_ITF_TYPE_MIPI  */

	IsiSyncSignalPolarity_t hSyncPol;	 /**< vinType: ISI_ITF_TYPE_DVP/ISI_ITF_TYPE_BT601  */
	IsiSyncSignalPolarity_t vSyncPol;	 /**< vinType: ISI_ITF_TYPE_DVP/ISI_ITF_TYPE_BT601  */
	IsiSampleEdgePolarity_t sampleEdge;  /**< vinType: ISI_ITF_TYPE_DVP/ISI_ITF_TYPE_BT601  */
	IsiSensorCCIRSequence_t ccirSeq;     /**< vinType: ISI_ITF_TYPE_BT601*/
} IsiCaps_t;

typedef struct IsiIspStatus_s {
	bool_t useSensorAE;
	bool_t useSensorBLC;
	bool_t useSensorAWB;
} IsiIspStatus_t;

typedef struct IsiIrLightExp_s {
	bool_t irOn;
	uint8_t irStrength;
} IsiIrLightExp_t;

typedef struct IsiIrLightRange_s {
	uint8_t minIrStrength;
	uint8_t maxIrStrength;
	uint8_t irStrengthStep;
} IsiIrLightRange_t;

typedef struct IsiIrLightInfo_s {
	bool_t irSuppAeCtrl;
	IsiIrLightRange_t irStrength;
	uint8_t irDelayFrame;
} IsiIrLightInfo_t;

typedef enum IsiFocusPosMode_enum {
	ISI_FOCUS_POS_ABSOLUTE = 0,
	ISI_FOCUS_POS_RELATIVE,
	DUMMY_ISI_014 = 0xdeadfeed
} IsiFocusPosMode_e;

typedef enum IsiPdafSensorType_enum {
	ISI_PDAF_SENSOR_DUAL_PIXEL = 0,
	ISI_PDAF_SENSOR_OCL2X1 = 1,
	ISI_PDAF_SENSOR_TYPE_MAX,
	DUMMY_ISI_015 = 0xdeadfeed
} IsiPdafSensorType_e;

typedef struct IsiFocusPosInfo_s {
	uint32_t minPos;
	uint32_t maxPos;
	uint32_t minStep;
} IsiFocusPosInfo_t;

typedef struct IsiFocusPDInfo_s {
	IsiPdafSensorType_e sensorType;
	IsiBayerPattern_t bayerPattern;
	bool_t ocl2x1Shield;
	uint8_t bitWidth;
	uint32_t imageWidth;
	uint32_t imageHeight;
	uint16_t pdArea[4];
	uint32_t correctRect[4];
	uint8_t pdNumPerArea[2];
	uint8_t pdShiftL2R[2];
	uint8_t pdShiftMark[32];
	uint8_t pdFocalHeigh;
	uint8_t pdFocalWidth;
	uint8_t pdDistance;
	int pdFocal[48];
} IsiFocusPDInfo_t;

typedef struct IsiFocusCalibAttr_s {
	IsiFocusPosInfo_t posInfo;
	IsiFocusPDInfo_t pdInfo;
} IsiFocusCalibAttr_t;

typedef struct IsiFocusPos_s {
	IsiFocusPosMode_e posType;
	uint32_t pos;
} IsiFocusPos_t;

typedef struct IsiBus_s {
	uint8_t type;
	union {
		struct I2C {
			uint8_t i2cBusNum;          /**< The I2C bus the sensor is connected to. */
			uint16_t slaveAddr;          /**< The I2C slave addr the sensor is configured to. */
			uint8_t addrWidth;
			uint8_t dataWidth;
		} i2c;

		struct SPI {
			uint8_t spiNum;
		} spi;
	};
} IsiBus_t;

typedef struct IsiSensorInstanceConfig_s {
	uint32_t halDevID;           /**< HAL device ID of this sensor(reserved). */
	IsiBus_t sensorBus;          /**< BUS info of this sensor. */
	IsiBus_t motorBus;           /**< BUS info of this sensor motor. */
	uint32_t instanceID;         /**< instance ID of this sensor. */
	uint32_t cameraDriverID;     /**< camera driver ID of this sensor. */
	uint32_t cameraDevId;         /**< camera device ID. */
	IsiSensor_t *pSensor;           /**< Sensor driver interface */
} IsiSensorInstanceConfig_t;


typedef struct IsiAeBaseInfo_s {
	float32_t aecCurGain;
	float32_t aecCurIntTime;
	IsiSensorGain_t
	curGain; /*gain[0]: Long gain; gain[1]: Short gain; gain[2]: VS gain; gain[3]: VVS gain;*/
	IsiSensorIntTime_t
	curIntTime; /*IntTime[0]: Long gain; gain[1]: Short gain; gain[2]: VS gain; gain[3]: VVS gain;*/
	float32_t aecGainStep;
	float32_t aecIntTimeStep;
	IsiSensorStitchingMode_t stitchingMode;
	IsiSensorNativeMode_t nativeMode;
	float32_t nativeHdrRatio[3]; /* 0: L/S, 1: S/VS, 2:VS/VVS */
	float32_t conversionGainDCG;

	IsiRange_t longGain, shortGain, vsGain, vvsGain, gain; /* total gain*/
	IsiRange_t longIntTime, shortIntTime, vsIntTime, vvsIntTime, intTime; /* total integrationtime */

	IsiGainInfo_t aLongGain, aShortGain, aVSGain, aVVSGain, aGain; /* analog gain of each exp*/
	IsiGainInfo_t dLongGain, dShortGain, dVSGain, dVVSGain, dGain; /* digital gain of each exp*/

	IsiIrLightExp_t aecIrLightExp;
	IsiIrLightInfo_t aecIrLightInfo;

	bool_t normGainSupport;
	float32_t normGain;
} IsiAeBaseInfo_t;


typedef struct IsiExpDecomposeParam_s {
	float32_t gain;
	float32_t integrationTime;
	float32_t ispRatio[ISI_EXP_NUM_MAX - 1];
} IsiExpDecomposeParam_t;

typedef struct IsiExpDecomposeResult_s {
	float32_t sensorAgain[ISI_EXP_NUM_MAX];
	float32_t sensorDgain[ISI_EXP_NUM_MAX];
	float32_t sensorExpTime[ISI_EXP_NUM_MAX];
	float32_t sensorRatio[ISI_EXP_NUM_MAX - 1];
} IsiExpDecomposeResult_t;

typedef struct IsiSensorExpParam_s {
	float32_t sensorAgain[ISI_EXP_NUM_MAX];
	float32_t sensorDgain[ISI_EXP_NUM_MAX];
	float32_t sensorExpTime[ISI_EXP_NUM_MAX];
	float32_t sensorRatio[ISI_EXP_NUM_MAX - 1];
	float32_t ispGain;
} IsiSensorExpParam_t;

/*****************************************************************************/
/**
 *          IsiOpenIss
 *
 * @brief   Open of the image sensor considering the given configuration.
 *
 * @param   handle      Sensor instance handle
 * @param   mode        Current work mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiOpenIss
(
	IsiSensorHandle_t handle,
	uint32_t mode
);


/*****************************************************************************/
/**
 *          IsiCloseIss
 *
 * @brief   Close the image sensor considering the given configuration.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiCloseIss
(
	IsiSensorHandle_t handle
);


/*****************************************************************************/
/**
 *          IsiReadRegIss
 *
 * @brief   reads a given number of bytes from the image sensor device
 *
 * @param   handle              Handle to image sensor device
 * @param   addr                register address
 * @param   pValue              value to read
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT IsiReadRegIss
(
	IsiSensorHandle_t handle,
	const uint16_t addr,
	uint16_t *pValue
);


/*****************************************************************************/
/**
 *          IsiWriteRegIss
 *
 * @brief   writes a given number of bytes to the image sensor device by
 *          calling the corresponding sensor-function
 *
 * @param   handle              Handle to image sensor device
 * @param   addr                register address
 * @param   value               value to write
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT IsiWriteRegIss
(
	IsiSensorHandle_t handle,
	const uint16_t addr,
	const uint16_t value
);


/*****************************************************************************/
/**
 *          IsiGetModeIss
 *
 * @brief   get cuurent sensor mode info.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
RESULT IsiGetModeIss
(
	IsiSensorHandle_t handle,
	IsiSensorMode_t *pMode
);


/*****************************************************************************/
/**
 *          IsiEnumModeIss
 *
 * @brief   query sensor info.
 *
 * @param   handle                  sensor instance handle
 * @param   pEnumMode               sensor query mode
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiEnumModeIss
(
	IsiSensorHandle_t handle,
	IsiSensorEnumMode_t *pEnumMode
);


/*****************************************************************************/
/**
 *          IsiGetCapsIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   handle      Sensor instance handle
 * @param   pCaps       Sensor caps pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetCapsIss
(
	IsiSensorHandle_t handle,
	IsiCaps_t *pCaps
);


/*****************************************************************************/
/**
 *          IsiCheckConnectionIss
 *
 * @brief   Checks the connection to the camera sensor, if possible.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiCheckConnectionIss
(
	IsiSensorHandle_t handle
);


/*****************************************************************************/
/**
 *          IsiGetRevisionIss
 *
 * @brief   This function reads the sensor revision register and returns it.
 *
 * @param   handle      sensor instance handle
 * @param   pRevision   pointer to revision
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT IsiGetRevisionIss
(
	IsiSensorHandle_t handle,
	uint32_t *pRevision
);


/*****************************************************************************/
/**
 *          IsiSetStreamingIss
 *
 * @brief   Enables/disables streaming of sensor data, if possible.
 *
 * @param   handle      Sensor instance handle
 * @param   on          new streaming state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiSetStreamingIss
(
	IsiSensorHandle_t handle,
	bool_t on
);

/*****************************************************************************/
/**
 *          IsiGainExecuteIss
 *
 * @brief   complete the sensor gain execute.
 *
 * @param   totalGain      the sensor total gain need to execute
 * @param   aGain          the limit of sensor analog gain
 * @param   dGain          the limit of sensor digital gain
 * @param   splitAgain     the pointer of split analog gain
 * @param   splitDgain     the pointer of split digital gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiGainExecuteIss
(
	float32_t totalGain,
	IsiGainInfo_t aGain,
	IsiGainInfo_t dGain,
	float32_t *splitAgain,
	float32_t *splitDgain
);

/*****************************************************************************/
/**
 *          IsiSensorExecuteExposureControl
 *
 * @brief   Excute exposure divided in sensor driver.
 *
 * @param   pExpParam      the input parameters for dividing exposure.
 * @param   pExpResult     the result after dividing exposure.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiSensorExecuteExposureControl
(
	IsiSensorHandle_t handle,
	const IsiSensorExpParam_t *pExpParam,
	IsiSensorExpParam_t *pExpResult
);


/*****************************************************************************/
/**
 *          IsiExpDecomposeControl
 *
 * @brief   Excute exposure decompose in sensor driver.
 *
 * @param   IsiExpDecomposeParam_t      the input parameters for decomposing exposure.
 * @param   IsiExpDecomposeResult_t     the result after decomposing exposure.
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiExpDecomposeControl
(
	IsiSensorHandle_t handle,
	const IsiExpDecomposeParam_t *pDecParam,
	IsiExpDecomposeResult_t *pDecResult
);


/*****************************************************************************/
/**
 *          IsiGetAeBaseInfoIss
 *
 * @brief   Returns the Ae base info of a sensor
 *          instance
 *
 * @param   handle      sensor instance handle
 * @param   pAeBaseInfo Pointer to the sensor aebase info value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetAeBaseInfoIss
(
	IsiSensorHandle_t handle,
	IsiAeBaseInfo_t *pAeBaseInfo
);

/*****************************************************************************/
/**
 *          IsiGetAGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorAGain            pointer to sensor again to get
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetAGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorAGain
);

/*****************************************************************************/
/**
 *          IsiGetDGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorDGain            pointer to sensor dgain to get
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetDGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorDGain
);

/*****************************************************************************/
/**
 *          IsiSetAGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorAGain            pointer to sensor again to set
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetAGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorAGain
);

/*****************************************************************************/
/**
 *          IsiSetDGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorDGain            pointer to sensor dgain to set
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetDGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorDGain
);


/*****************************************************************************/
/**
 *          IsiGetIntTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorIntTime          pointer to integration time to get
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetIntTimeIss
(
	IsiSensorHandle_t handle,
	IsiSensorIntTime_t *pSensorIntTime
);


/*****************************************************************************/
/**
 *          IsiSetIntTimeIss
 *
 * @brief   Writes integration time values to the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorIntTime          pointer to sensor integration time to set
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetIntTimeIss
(
	IsiSensorHandle_t handle,
	const IsiSensorIntTime_t *pSensorIntTime
);


/*****************************************************************************/
/**
 *          IsiGetFpsIss
 *
 * @brief   Get Sensor Fps Config.
 *
 * @param   handle                  sensor instance handle
 * @param   pFps                    current fps
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetFpsIss
(
	IsiSensorHandle_t handle,
	uint32_t *pFps
);


/*****************************************************************************/
/**
 *          IsiSetFpsIss
 *
 * @brief   set Sensor Fps Config.
 *
 * @param   handle                  sensor instance handle
 * @param   fps                     Setfps
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetFpsIss
(
	IsiSensorHandle_t handle,
	uint32_t fps
);


/*****************************************************************************/
/**
 *          IsiGetIspStatusIss
 *
 * @brief   Get sensor isp status.
 *
 * @param   handle                  sensor instance handle
 * @param   pSensorIspStatus        sensor isp status
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetIspStatusIss
(
	IsiSensorHandle_t handle,
	IsiIspStatus_t *pIspStatus
);


/*****************************************************************************/
/**
 *          IsiSetBlcIss
 *
 * @brief   set sensor linear mode black level
 *
 *
 * @param   handle          sensor instance handle
 * @param   pBlc            blc params pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiSetBlcIss
(
	IsiSensorHandle_t handle,
	const IsiSensorBlc_t *pBlc
);

/*****************************************************************************/
/**
 *          IsiGetBlcIss
 *
 * @brief   set sensor linear mode black level
 *
 *
 * @param   handle          sensor instance handle
 * @param   pBlc            blc params point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiGetBlcIss
(
	IsiSensorHandle_t handle,
	IsiSensorBlc_t *pBlc
);


/*****************************************************************************/
/**
 *          IsiSetWBIss
 *
 * @brief   set sensor linear mode white balance
 *          or hdr mode normal exp frame white balance
 *
 * @param   handle          sensor instance handle
 * @param   pWb             wb params pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiSetWBIss
(
	IsiSensorHandle_t handle,
	const IsiSensorWb_t *pWb
);

/*****************************************************************************/
/**
 *          IsiGetWBIss
 *
 * @brief   set sensor linear mode white balance
 *          or hdr mode normal exp frame white balance
 *
 * @param   handle          sensor instance handle
 * @param   pWb             wb params point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiGetWBIss
(
	IsiSensorHandle_t handle,
	IsiSensorWb_t *pWb
);


/*****************************************************************************/
/**
 *          IsiSetTpgIss
 *
 * @brief   set sensor test pattern.
 *
 * @param   handle      Sensor instance handle
 * @param   tpg         Sensor test pattern
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiSetTpgIss
(
	IsiSensorHandle_t handle,
	IsiSensorTpg_t tpg
);

/*****************************************************************************/
/**
 *          IsiGetTpgIss
 *
 * @brief   set sensor test pattern.
 *
 * @param   handle      Sensor instance handle
 * @param   pTpg         Sensor test pattern ptr
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/

RESULT IsiGetTpgIss
(
	IsiSensorHandle_t handle,
	IsiSensorTpg_t *pTpg
);

/*****************************************************************************/
/**
 *          IsiGetExpandCurveIss
 *
 * @brief   get sensor expand curve
 *
 * @param   handle          sensor instance handle
 * @param   pCurve          expand curve pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiGetExpandCurveIss
(
	IsiSensorHandle_t handle,
	IsiSensorCompandCurve_t *pCurve
);


/*****************************************************************************/
/**
 *          IsiGetCompressCurveIss
 *
 * @brief   get sensor compress curve
 *
 * @param   handle          sensor instance handle
 * @param   pCurve          compress curve pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT IsiGetCompressCurveIss
(
	IsiSensorHandle_t handle,
	IsiSensorCompandCurve_t *pCurve
);


/*****************************************************************************/
/**
 *          IsiExtendFuncIss
 *
 * @brief   sensor extend function.
 *
 * @param   handle                  sensor instance handle
 * @param   pUserData               sensor extend info
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiExtendFuncIss
(
	IsiSensorHandle_t handle,
	void *pUserData
);


/*****************************************************************************/
/**
 *          IsiGetOtpDataIss
 *
 * @brief   get sensor otp data.
 *
 * @param   handle                  sensor instance handle
 * @param   ctx                     sensor otp data
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetOtpDataIss
(
	IsiSensorHandle_t handle,
	IsiOTP_t *pOtpData
);


/*****************************************************************************/
/**
 *          IsiFocusCreateIss
 *
 * @brief   create sensor focus
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiFocusCreateIss
(
	IsiSensorHandle_t handle
);


/*****************************************************************************/
/**
 *          IsiFocusReleaseIss
 *
 * @brief   release sensor focus.
 *
 * @param   handle          sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiFocusReleaseIss
(
	IsiSensorHandle_t handle
);


/*****************************************************************************/
/**
 *          IsiFocusGetCalibrateIss
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          sensor instance handle
 * @param   pFocusCalib     sensor focus calib pointor
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiFocusGetCalibrateIss
(
	IsiSensorHandle_t handle,
	IsiFocusCalibAttr_t *pFocusCalib
);


/*****************************************************************************/
/**
 *          IsiFocusSetIss
 *
 * @brief   Drives the lens system to a certain focus point.
 *
 * @param   handle          sensor instance handle
 * @param   pPos            focus position pointer
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiFocusSetIss
(
	IsiSensorHandle_t handle,
	const IsiFocusPos_t *pPos
);


/*****************************************************************************/
/**
 *          IsiFocusGetIss
 *
 * @brief   Retrieves the currently applied focus point.
 *
 * @param   handle          sensor instance handle
 * @param   pPos            pointer to a variable to receive the current
 *                          focus point
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiFocusGetIss
(
	IsiSensorHandle_t handle,
	IsiFocusPos_t *pPos
);

RESULT IsiSetInfraredLightExpParamIss
(
	IsiSensorHandle_t handle,
	IsiIrLightExp_t *pIrExpParam
);

RESULT IsiGetInfraredLightExpParamIss
(
	IsiSensorHandle_t handle,
	IsiIrLightExp_t *pIrExpParam
);


RESULT IsiQueryMetadataAttrIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t *pAttr
);

RESULT IsiSetMetadataAttrEnableIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t attr
);

RESULT IsiGetMetadataAttrEnableIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t *pAttr
);

RESULT IsiGetMetadataWindowIss
(
	IsiSensorHandle_t handle,
	IsiMetadataWinInfo_t *pMetaWin
);

RESULT IsiParserMetadataIss
(
	IsiSensorHandle_t handle,
	const MetadataBufInfo_t *pMetaBuf,
	IsiSensorMetadata_t *pMetaInfo
);

#ifdef __cplusplus
}
#endif


/* @} isi */


#endif /* __ISI_H__ */
