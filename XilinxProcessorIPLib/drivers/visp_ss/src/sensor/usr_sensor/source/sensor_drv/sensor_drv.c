/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2023 */

/**
 * @file sensor_drv.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/


#include "sensor_drv.h"

#include <types.h>
#include <trace.h>
#include <return_codes.h>
#include <string.h>


extern IsiCamDrvConfig_t Ox03f10_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox08b40_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox05b1s_IsiCamDrvConfig;
extern IsiCamDrvConfig_t virtualSensor_IsiCamDrvConfig;

extern IsiSensorMode_t pox03f10_mode_info[];

// SensorDrvConfigList_t sensorCfgList[] = {
// {"0x03f10", 0x6c, 2, 1, {0x300a, 0x300b}, 0x5308},
// };

// RESULT SensorDrvConfigMapping
// (
// const char *pSensorName,
// IsiCamDrvConfig_t **pSensorConfig
// )
// {

// TRACE( SENSOR_DRV_INFO, "%s: (enter)\n", __func__);

// uint8_t i = 0, start = 0, end = 7;
// char newSensorName[50] = "";
// char *sptr, *eptr;
// bool_t apuSensor = BOOL_FALSE;

// if ( pSensorName == NULL ) {
// return ( RET_NULL_POINTER );
// }

// for(i = 0; pSensorName[i] != '\0'; ++i){
//	newSensorName[i] = pSensorName[i];
// }
// newSensorName[i] = '\0';
// if(i > 8){
//	sptr = newSensorName + start*sizeof(char);
//	eptr = newSensorName + end*sizeof(char);

//	*eptr = '\0';
//	if(strcmp(sptr, "virtual") == 0){
//	TRACE( SENSOR_DRV_INFO, "%s: register sensor driver from APU.\n", __func__);
//	sptr = eptr + sizeof(char);
//	apuSensor = BOOL_TRUE;
//	}

// }
// SensorDrvConfig_t sensorConfig[] = {
//	{"ox03f10", &Ox03f10_IsiCamDrvConfig},
//	{"ox08b40", &Ox08b40_IsiCamDrvConfig},
//	{"ox05b1s", &Ox05b1s_IsiCamDrvConfig}
//	};

// if(apuSensor){
//	*pSensorConfig = &virtualSensor_IsiCamDrvConfig;
//	for (int i = 0; i < (int)(sizeof(sensorConfig)/sizeof(sensorConfig[0])); i++) {
//	if (strcmp(sptr, sensorConfig[i].pSensorName) == 0) {
// 				// for sending back to APU to register
//	(*pSensorConfig)->cameraDriverID = sensorConfig[i].pSensorConfig->cameraDriverID;

//	TRACE( SENSOR_DRV_INFO, "%s: i=%d, match sensor name: %s success!!\n", __func__, i, sensorConfig[i].pSensorName);
//	return RET_SUCCESS;
//	}
//	}

// }else{

//	for (int i = 0; i < (int)(sizeof(sensorConfig)/sizeof(sensorConfig[0])); i++) {
//	if (strcmp(pSensorName, sensorConfig[i].pSensorName) == 0) {
//	*pSensorConfig = sensorConfig[i].pSensorConfig;

//	TRACE( SENSOR_DRV_INFO, "%s: i=%d, match sensor name: %s success!!\n", __func__, i, sensorConfig[i].pSensorName);
//	return RET_SUCCESS;
//	}
//	}
// }


// TRACE(SENSOR_DRV_ERROR, "%s: Unsupport sensor %s !\n", __func__,pSensorName);
// return RET_NOTSUPP;
// }


// RESULT SensorDrvGetSensorNumber
// (
// uint16_t    *pNumber
// )
// {
// TRACE( SENSOR_DRV_INFO, "%s: (enter)\n", __func__);

// *pNumber = (sizeof(sensorCfgList)/sizeof(sensorCfgList[0]));

// TRACE( SENSOR_DRV_INFO, "%s: (exit)\n", __func__);
// return RET_SUCCESS;
// }

// RESULT SensorDrvGetConfigList
// (
// SensorDrvList_t *pSensorDrvList
// )
// {
// TRACE( SENSOR_DRV_INFO, "%s: (enter)\n", __func__);

// if ( pSensorDrvList == NULL ) {
// return ( RET_NULL_POINTER );
// }


// strcpy(pSensorDrvList[0].name, "0x03f10");
// pSensorDrvList[0].pSensorMode = pox03f10_mode_info;


// TRACE( SENSOR_DRV_INFO, "%s: (exit)\n", __func__);
// return RET_SUCCESS;
// }

// RESULT SensorDrvGetPortInfo
// (
// sensorPortInfo_t *pPortInfo,
//	uint32_t          sensorDevId
// )
// {
// TRACE( SENSOR_DRV_INFO, "%s: (enter)\n", __func__);
// RESULT result = RET_SUCCESS;

// if ( pPortInfo == NULL ) {
// return ( RET_NULL_POINTER );
// }

// int32_t fd = -1;
// uint8_t busId = (uint8_t)sensorDevId;
// char pathDevName[32] = {0};

// sprintf(pathDevName, "/dev/i2c-%u", busId);
// fd = HalOpenI2cDevice(pathDevName);
// if(fd < 0)
// {
// TRACE(SENSOR_DRV_INFO, "%s:Can't open %s!\n", __func__, pathDevName);
// return RET_FAILURE;
// }

// for(uint8_t index = 0; index < (sizeof(sensorCfgList)/sizeof(sensorCfgList[0])); index++) {
// result = HalIoCtl(fd, (sensorCfgList[index].slaveAddr >> 1));
// if (result < 0)
// {
// TRACE(SENSOR_DRV_INFO, "%s:I2C_SLAVE_FORCE error!\n", __func__);
// continue;
// }

// uint16_t regVal[3];
// uint32_t sensorId = 0;

// for(uint8_t i = 0; i < 3; i++) {
// if(sensorCfgList[index].regAddr[i] == 0) {
// continue;
// } else {
// regVal[i]   = 0;
// result    = HalSensorDrvReadI2CReg(sensorCfgList[index].slaveAddr,
// sensorCfgList[index].regWidth,
// sensorCfgList[index].dataWidth,
// sensorCfgList[index].regAddr[i], &regVal[i], fd);
// }
// }

// if((sensorCfgList[index].regAddr[0] != 0) && (sensorCfgList[index].regAddr[1] == 0) && (sensorCfgList[index].regAddr[2] == 0)) {
// sensorId |= (regVal[0] & 0xffff);
// } else if ((sensorCfgList[index].regAddr[0] != 0) && (sensorCfgList[index].regAddr[1] != 0) && (sensorCfgList[index].regAddr[2] == 0)) {
// sensorId = (regVal[0] & 0xff) << 8;
// sensorId |= (regVal[1] & 0xff);
// } else if ((sensorCfgList[index].regAddr[0] != 0) && (sensorCfgList[index].regAddr[1] != 0) && (sensorCfgList[index].regAddr[2] != 0)) {
// sensorId = (regVal[0] & 0xff) << 16;
// sensorId |= ((regVal[1] & 0xff) << 8);
// sensorId |= (regVal[2] & 0xff);
// }

// if (sensorCfgList[index].sensorId == sensorId) {
// TRACE(SENSOR_DRV_INFO, "%s: sensorId=%x found in i2c-%u! \n", __func__, sensorId, busId);
// strcpy(pPortInfo->name, sensorCfgList[index].pSensorName);
// pPortInfo->chipId = sensorId;
// break;
// }
// }

// HalCloseI2cDevice(fd);

// TRACE( SENSOR_DRV_INFO, "%s: (exit)\n", __func__);
// return RET_SUCCESS;
// }


//RESULT HalGetSensorName(HalHandle_t HalHandle, char pSensorName[], uint16_t arraySize)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pSensorName == NULL)
// {
// return RET_NULL_POINTER;
// }
//
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor name error in hal!\n", __func__);
// return result;
// }
//
// snprintf(pSensorName, arraySize, "%s", sensorInfo.pSensorName);
// return RET_SUCCESS;
//}
//
//RESULT HalGetSensorDrvName(HalHandle_t HalHandle, char pSensorDrvName[], uint16_t arraySize)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pSensorDrvName == NULL)
// {
// return RET_NULL_POINTER;
// }
//
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor drv name error in hal!\n", __func__);
// return result;
// }
//
// snprintf(pSensorDrvName, arraySize, "%s", sensorInfo.pSensorDrvName);
// return RET_SUCCESS;
//}
//
//RESULT HalGetSensorCalibXmlName(HalHandle_t HalHandle, char pSensorCalibXmlName[], uint16_t arraySize)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pSensorCalibXmlName == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor calibration XML name error in hal!\n", __func__);
// return result;
// }
//
// snprintf(pSensorCalibXmlName, arraySize, "%s", sensorInfo.pSensorCalibXmlName);
// return RET_SUCCESS;
//
//}
//
//RESULT HalGetSensorDefaultMode(HalHandle_t HalHandle, uint32_t *pMode)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pMode == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor default mode error in hal!\n", __func__);
// return result;
// }
//
// *pMode = sensorInfo.sensorDefaultMode;
// return RET_SUCCESS;
//}
//
//RESULT HalGetSensorCurrMode(HalHandle_t HalHandle, uint32_t *pMode)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pMode == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor current mode error in hal!\n", __func__);
// return result;
// }
//
// *pMode = sensorInfo.sensorCurrMode;
// return RET_SUCCESS;
//}
//
//RESULT HalGetSensorCurrHdrMode(HalHandle_t HalHandle, uint32_t *pMode)
//{
// RESULT result;
// AdaptSensorInfo_t sensorInfo;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || pMode == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptGetSensorInfo(pHalCtx->adaptHandle, &sensorInfo);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: get sensor current hdr mode error in hal!\n", __func__);
// return result;
// }
//
// *pMode = sensorInfo.sensorHdrEnable;
// return RET_SUCCESS;
//}
//
//
//RESULT HalSetSensorMode(HalHandle_t HalHandle, uint32_t mode)
//{
// RESULT result;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptSetSensorMode(pHalCtx->adaptHandle, mode);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: set sensor mode error in hal!\n", __func__);
// return result;
// }
// return RET_SUCCESS;
//}
//
//RESULT HalSetSensorCalibXmlName(HalHandle_t HalHandle, const char* CalibXmlName)
//{
// RESULT result;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL || CalibXmlName == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptSetSensorCalibXmlName(pHalCtx->adaptHandle, CalibXmlName);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: set sensor CalibXmlName error in hal!\n", __func__);
// return result;
// }
// return RET_SUCCESS;
//}
//
//RESULT HaSensorModeLock(HalHandle_t HalHandle)
//{
// RESULT result;
// HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
// if(pHalCtx == NULL)
// {
// return RET_NULL_POINTER;
// }
// result = AdaptSensorModeLock(pHalCtx->adaptHandle);
// if(result != RET_SUCCESS)
// {
// TRACE(HAL_ERROR, "%s: sensor mode lock error in hal!\n", __func__);
// return result;
// }
// return RET_SUCCESS;
//}
