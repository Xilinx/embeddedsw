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


#ifndef __SENSOR_DRV_H__
#define __SENSOR_DRV_H__

#include "isi_iss.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SensorDrvConfig_s {
	const char *pSensorName;
	IsiCamDrvConfig_t *pSensorConfig;
} SensorDrvConfig_t;


typedef struct SensorDrvList_s {
	char name[20];
	IsiSensorMode_t *pSensorMode;
} SensorDrvList_t;

typedef struct sensorPortInfo_s {
	uint32_t chipId;
	char name[20];
} sensorPortInfo_t;

typedef struct SensorDrvConfigList_s {
	char *pSensorName;
	uint32_t slaveAddr;
	uint8_t regWidth;
	uint8_t dataWidth;
	uint16_t regAddr[3];
	uint32_t sensorId;
} SensorDrvConfigList_t;

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
	const char *pSensorName,
	IsiCamDrvConfig_t **pSensorConfig
);

RESULT SensorDrvGetSensorNumber
(
	uint16_t *pNumber
);

RESULT SensorDrvGetConfigList
(
	SensorDrvList_t *pSensorDrvList
);

RESULT SensorDrvGetPortInfo
(
	sensorPortInfo_t *pPortInfo,
	uint32_t sensorDevId
);

#ifdef __cplusplus
}
#endif

#endif    // __SENSOR_DRV_H__
