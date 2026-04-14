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


#ifndef __SENSOR_DRV_H__
#define __SENSOR_DRV_H__

#include "isi_iss.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SENSOR_DRV_NUM 12   /**< Sensor query number */
#define SENSOR_MODULE_NAME  50

typedef void* CamDeviceSensorDrvHandle_t;
typedef struct SensorDrvConfig_s
{
    const char *pSensorName;
    IsiCamDrvConfig_t *pSensorConfig;
} SensorDrvConfig_t;

typedef struct SensorDrvList_s
{
    char name[20];
    IsiSensorMode_t *pSensorMode;
} SensorDrvList_t;

typedef struct sensorPortInfo_s
{
    uint32_t chipId;
    char name[20];
} sensorPortInfo_t;

typedef struct SensorDrvConfigList_s
{
    char *pSensorName;
    uint32_t slaveAddr;
    uint8_t regWidth;
    uint8_t dataWidth;
    uint16_t regAddr[3];
    uint32_t sensorId;
} SensorDrvConfigList_t;

typedef enum SensorDrvId_e {
    SENSOR_DRV_ID_0,   /**< Video input sensor index 0.*/
    SENSOR_DRV_ID_1,   /**< Video input sensor index 1.*/
    SENSOR_DRV_ID_2,   /**< Video input sensor index 2.*/
    SENSOR_DRV_ID_3,   /**< Video input sensor index 3.*/
    SENSOR_DRV_ID_MAX,
}SensorDrvId_t;

typedef struct CamDeviceSensorModuleMapCfg_s {
	char moduleName[SENSOR_MODULE_NAME];
    uint32_t sensorDevId;
}CamDeviceSensorModuleMapCfg_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor driver mode information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorDrvModeInfo_s {
    uint32_t index;                              /**< Sensor mode index */
    uint32_t width;                       /**< Real image width */
    uint32_t height;                      /**< Real image height */
    IsiSensorHdrMode_t sensorType;            /**< The sensor type is linear or HDR*/
    IsiSensorStitchingMode_t  stitchingMode;  /**< The sensor type is HDR stitching*/
    IsiSensorNativeMode_t  nativeMode;        /**< The sensor type is HDR Native */
    uint32_t bitWidth;                    /**< Sensor bit width */
    IsiBayerPattern_t bayerPattern;       /**< Sensor Bayer pattern type*/
    uint32_t maxFps;                      /**< Sensor maximum FPS value */
    IsiSensorAfMode_t   afMode;           /**< Sensor auto focusing mode */
    uint32_t dataType;                    /**< Sensor data type */
    uint32_t itfType;                     /**< Sensor interface type */
    IsiSensorAeInfo_t   aeInfo;           /**< Sensor AE info */
}CamDeviceSensorDrvModeInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor mode list information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorListInfo_s {
    uint32_t number;    /**< Sensor index number */
    char name[20];   /**< Sensor name */
    CamDeviceSensorDrvModeInfo_t sensorModeInfo[SENSOR_DRV_NUM];   /**< Sensor mode information */
}CamDeviceSensorListInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor connection port information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorConnectPortInfo_s {
    uint32_t chipId;  /**< Sensor chip ID */
    char name[20]; /**< Pointer to sensor name */
}CamDeviceSensorConnectPortInfo_t;


/*****************************************************************************/
/**
 *          SensorDrvConfigMapping
 *
 * @brief   sensor config mapping.
 *
 * @param   pSensorName      Pointer to the sensor name
 * @param   pSensorConfig    Pointer to the isi sensor driver config
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT SensorDrvConfigMapping
(
    const CamDeviceSensorModuleMapCfg_t  *pModuleInfo,
    IsiCamDrvConfig_t **pSensorConfig
);

/****************************************************************************/
/**
 * @brief   Gets the number of sensors.
 *
 * @param   pNumber     Pointer to the sensor number
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetNumber
(
    uint16_t            *pNumber
);

/****************************************************************************/
/**
 * @brief   Gets all sensor mode information.
 *
 * @param   pSensorListInfo     Pointer to the sensor mode list
 * @param   sensorNum           The number of sensors
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetListInfo
(
    CamDeviceSensorListInfo_t *pSensorListInfo,
    const uint16_t sensorNum
);

/****************************************************************************/
/**
 * @brief   Gets the name of sensor which is connected to FPGA i2c-8/9/10/11.
 * Currently, only i2c-8/9 query is supported.
 *
 * @param   portId              FPGA port index
 * @param   pPortInfo           Pointer to the port information
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetConnectPortInfo
(
    SensorDrvId_t  sensorDrvId,
    CamDeviceSensorConnectPortInfo_t *pPortInfo
);

/****************************************************************************/
/**
 * @brief   Mapping the sensor driver.
 *
 * @param   pSensorName         Pointer to sensor driver name
 * @param   pSensorDrvhandle    Sensor driver handle pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 * @retval  RET_INVALID_PARM    Invalid parameter
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorMapping
(
    const CamDeviceSensorModuleMapCfg_t  *pModuleInfo,
    CamDeviceSensorDrvHandle_t *pSensorDrvhandle
);

#ifdef __cplusplus
}
#endif

#endif    // __SENSOR_DRV_H__
