/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include <trace.h>
#include <builtins.h>
#include <misc.h>
#include <oslayer.h>
#include <isi_fmc.h>
#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "ox05b1s_priv.h"


#define IR_POWER_DEVICE_SLAVE_ADDRESS 0x4a
#define Ox05b1s_MIN_GAIN_STEP    ( 1.0f/1024.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */

#define OS05B1S_IR_LIGHT_STRENGTH_MAX 255


typedef struct OX05B1S_IR_setting_s {
	bool_t irOn;
	uint32_t irStrength;
} OX05B1S_IR_setting_t;

typedef struct OX05B1S_ABmode_Setting_s {
	uint32_t expLine;
	uint32_t again;
	uint32_t dgain;
	OX05B1S_IR_setting_t irCfg;
} OX05B1S_ABmode_Setting_t;

typedef enum OX05B1S_ABmode_Index_e {
	OS05B1S_AB_MODE_EXP_LINE = 0,
	OS05B1S_AB_MODE_A_GAIN,
	OS05B1S_AB_MODE_D_GAIN,
	OS05B1S_AB_MODE_IR_PARAMS
} OX05B1S_ABmode_Index_t;

struct OX05B1S_ABmode_Setting_s gOX05B1S_ABmode[2] = {0};

/*****************************************************************************
 *Sensor Info
*****************************************************************************/

static IsiSensorMode_t pox05b1s_mode_info[] = {
	{
		.index = 0,
		.size = {
			.boundsWidth = 2592,
			.boundsHeight = 1944,
			.top = 0,
			.left = 0,
			.width = 2592,
			.height = 1944,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 5 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_LINEAR,
		.bitWidth = 10,
		.bayerPattern = ISI_BPAT_BGGIR,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
	{
		.index = 1,//AB mode, RGB frame
		.size = {
			.boundsWidth = 2592,
			.boundsHeight = 1944,
			.top = 0,
			.left = 0,
			.width = 2592,
			.height = 1944,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 5 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_LINEAR,
		.bitWidth = 10,
		.bayerPattern = ISI_BPAT_BGGIR,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
	{
		.index = 2,//AB mode, IR frame
		.size = {
			.boundsWidth = 2592,
			.boundsHeight = 1944,
			.top = 0,
			.left = 0,
			.width = 2592,
			.height = 1944,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 5 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_LINEAR,
		.bitWidth = 10,
		.bayerPattern = ISI_BPAT_BGGIR,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
};

void enable_IR_power(IsiSensorHandle_t handle)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	uint32_t register_addr = 0x04;
	uint8_t wr_data[2];
	uint8_t read_data[2] = {0};
	int Status = XST_SUCCESS;
	uint16_t bytes_read = 1;

	wr_data[0] = 0x0f;
	Status = g_fmc_single.accessiic_array[pOx05b1sCtx->sensorDevId]->writeIIC(pOx05b1sCtx->i2cId,
		 IR_POWER_DEVICE_SLAVE_ADDRESS, register_addr, 0x1, wr_data[0], 1);
	if (Status != RET_SUCCESS) {
		xil_printf("%s: IR write sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	register_addr = 0x02;
	wr_data[0] = 0xe1;
	Status = g_fmc_single.accessiic_array[pOx05b1sCtx->sensorDevId]->writeIIC(pOx05b1sCtx->i2cId,
		 IR_POWER_DEVICE_SLAVE_ADDRESS, register_addr, 0x1, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);

	register_addr = 0x05;
	wr_data[0] = 0x00;
	Status = g_fmc_single.accessiic_array[pOx05b1sCtx->sensorDevId]->writeIIC(pOx05b1sCtx->i2cId,
		 IR_POWER_DEVICE_SLAVE_ADDRESS, register_addr, 0x1, wr_data[0], 1);
	if (Status != XST_SUCCESS)
		DCT_ASSERT(0);
}

static RESULT Ox05b1s_IsiReadRegIss(IsiSensorHandle_t handle, const uint16_t addr, uint16_t *pValue)
{
	RESULT result = RET_SUCCESS;
	//xil_printf( "%s (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	u8 slave_addr = (g_fmc_single.sensor_array[pOx05b1sCtx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx05b1sCtx->sensorDevId]->readIIC(pOx05b1sCtx->i2cId,
		 slave_addr, addr, 0x2, pValue, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal read sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	//xil_printf( "%s (exit) result = %d\n", __func__, result);
	return (result);
}

static RESULT Ox05b1s_IsiWriteRegIss(IsiSensorHandle_t handle, const uint16_t addr,
				     const uint16_t value)
{
	RESULT result = RET_SUCCESS;
	//xil_printf( "%s (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	u8 slave_addr = (g_fmc_single.sensor_array[pOx05b1sCtx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx05b1sCtx->sensorDevId]->writeIIC(pOx05b1sCtx->i2cId,
		 slave_addr, addr, 0x2, value, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal write sensor register error!\n", __func__);
		return (RET_FAILURE);
	}
	//xil_printf( "%s (exit) result = %d\n", __func__, result);
	return (result);
}

static RESULT Ox05b1s_IsiGetModeIss(IsiSensorHandle_t handle, IsiSensorMode_t *pMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return (RET_WRONG_HANDLE);
	if (pMode == NULL)
		return (RET_WRONG_HANDLE);

	memcpy(pMode, &(pOx05b1sCtx->sensorMode), sizeof(pOx05b1sCtx->sensorMode));

	xil_printf("%s (exit)\n", __func__);
	return (RET_SUCCESS);
}

static RESULT Ox05b1s_IsiEnumModeIss(IsiSensorHandle_t handle, IsiSensorEnumMode_t *pEnumMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	if (pEnumMode->index >= (sizeof(pox05b1s_mode_info) / sizeof(pox05b1s_mode_info[0])))
		return RET_OUTOFRANGE;

	for (uint32_t i = 0; i < (sizeof(pox05b1s_mode_info) / sizeof(pox05b1s_mode_info[0])); i++) {
		if (pox05b1s_mode_info[i].index == pEnumMode->index) {
			memcpy(&pEnumMode->mode, &pox05b1s_mode_info[i], sizeof(IsiSensorMode_t));
			xil_printf("%s (exit)\n", __func__);
			return RET_SUCCESS;
		}
	}

	return RET_NOTSUPP;
}

static RESULT Ox05b1s_IsiGetCapsIss(IsiSensorHandle_t handle, IsiCaps_t *pCaps)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (pOx05b1sCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pCaps == NULL)
		return (RET_NULL_POINTER);

	pCaps->bitWidth = pOx05b1sCtx->sensorMode.bitWidth;
	pCaps->mode = ISI_MODE_BAYER;
	pCaps->bayerPattern = pOx05b1sCtx->sensorMode.bayerPattern;
	pCaps->resolution.width = pOx05b1sCtx->sensorMode.size.width;
	pCaps->resolution.height = pOx05b1sCtx->sensorMode.size.height;
	pCaps->mipiLanes = ISI_MIPI_4LANES;
	pCaps->vinType = ISI_ITF_TYPE_MIPI;

	if (pCaps->bitWidth == 10)
		pCaps->mipiMode = ISI_FORMAT_RAW_10;

	else if (pCaps->bitWidth == 12)
		pCaps->mipiMode = ISI_FORMAT_RAW_12;

	else
		pCaps->mipiMode = ISI_MIPI_OFF;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_IsiCreateIss(IsiSensorInstanceConfig_t *pConfig, IsiSensorHandle_t *pHandle)
{
	RESULT result = RET_SUCCESS;
	uint32_t desId = 0, pipeId = 0;
	xil_printf("%s (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) osMalloc(sizeof(Ox05b1s_Context_t));
	if (!pOx05b1sCtx) {
		xil_printf("%s: Can't allocate ox05b1s context\n", __func__);
		return (RET_OUTOFMEM);
	}

	MEMSET(pOx05b1sCtx, 0, sizeof(Ox05b1s_Context_t));

	pOx05b1sCtx->isiCtx.pSensor = pConfig->pSensor;
	pOx05b1sCtx->groupHold = BOOL_FALSE;
	pOx05b1sCtx->configured = BOOL_FALSE;
	pOx05b1sCtx->streaming = BOOL_FALSE;
	pOx05b1sCtx->testPattern = BOOL_FALSE;
	pOx05b1sCtx->isAfpsRun = BOOL_FALSE;
	pOx05b1sCtx->sensorMode.index = 0;
	pOx05b1sCtx->i2cId	= 0;
	pOx05b1sCtx->sensorDevId = pConfig->cameraDevId;

	uint8_t busId = (uint8_t)pOx05b1sCtx->i2cId;
	pOx05b1sCtx->sensorDevId = 12;
	pipeId = pOx05b1sCtx->sensorDevId;

	*pHandle = (IsiSensorHandle_t) pOx05b1sCtx;

	// result = init_MUX();
	// if (result != XST_SUCCESS) {
	// xil_printf("\n\rIIC Init Failed \n\r");
	// return result;
	// }

	if (pipeId != IN_PIPE_12) {
		xil_printf("%s: sensor device ID %d is mot support!\n", __func__, pipeId);
		return RET_UNSUPPORT_ID;
	}
	desId = MAPPING_INPIPE_TO_DES_ID(pipeId);

	init_iic_access(pOx05b1sCtx->i2cId, pipeId);

	static int8_t mcmABmode_initCount = 0;

	if (mcmABmode_initCount <= 0) {
		init_des(desId);
		init_sensor(pipeId, desId);
	}
	mcmABmode_initCount ++;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_AecSetModeParameters(IsiSensorHandle_t handle, Ox05b1s_Context_t *pOx05b1sCtx)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s%s: (enter)\n", __func__, pOx05b1sCtx->isAfpsRun ? "(AFPS)" : "");
	uint32_t exp_line = 0, again = 0, dgain = 0, irLine;
	uint16_t value = 0;

	pOx05b1sCtx->aecMinIntegrationTime = pOx05b1sCtx->oneLineExpTime *
					     pOx05b1sCtx->minIntegrationLine;
	pOx05b1sCtx->aecMaxIntegrationTime = pOx05b1sCtx->oneLineExpTime *
					     pOx05b1sCtx->maxIntegrationLine;
	xil_printf("%s: AecMaxIntegrationTime = %f \n", __func__, pOx05b1sCtx->aecMaxIntegrationTime);

	pOx05b1sCtx->aecGainIncrement = Ox05b1s_MIN_GAIN_STEP;
	pOx05b1sCtx->aecIntegrationTimeIncrement = pOx05b1sCtx->oneLineExpTime;

	pOx05b1sCtx->irLightInfo.irStrength.minIrStrength = 1;
	pOx05b1sCtx->irLightInfo.irStrength.maxIrStrength = OS05B1S_IR_LIGHT_STRENGTH_MAX;
	pOx05b1sCtx->irLightInfo.irStrength.irStrengthStep = 1;
	pOx05b1sCtx->irLightInfo.irDelayFrame = 0;
	if (pOx05b1sCtx->sensorMode.index == 2)
		pOx05b1sCtx->irLightInfo.irSuppAeCtrl = 1;

	else
		pOx05b1sCtx->irLightInfo.irSuppAeCtrl = 0;

	//reflects the state of the sensor registers, must equal default settings
	//get again
	Ox05b1s_IsiReadRegIss(handle, 0x3508, &value);
	again = (value & 0x0f) << 4;
	Ox05b1s_IsiReadRegIss(handle, 0x3509, &value);
	again = again | ((value & 0xf0) >> 4);

	//get dgain
	Ox05b1s_IsiReadRegIss(handle, 0x350a, &value);
	dgain = (value & 0x0f) << 10;
	Ox05b1s_IsiReadRegIss(handle, 0x350b, &value);
	dgain = dgain | ((value & 0xff) << 2);
	Ox05b1s_IsiReadRegIss(handle, 0x350c, &value);
	dgain = dgain | ((value & 0xc0) >> 6);
	pOx05b1sCtx->aecCurGain = ((float)again / 16.0) * ((float)dgain / 1024.0);

	//get exp_line
	Ox05b1s_IsiReadRegIss(handle, 0x3500, &value);
	exp_line = (value & 0xff) << 16;
	Ox05b1s_IsiReadRegIss(handle, 0x3501, &value);
	exp_line = exp_line | ((value & 0xff) << 8);
	Ox05b1s_IsiReadRegIss(handle, 0x3502, &value);
	exp_line = exp_line | (value & 0xff);
	pOx05b1sCtx->aecCurIntegrationTime = exp_line * pOx05b1sCtx->oneLineExpTime;

	value = 0;
	result |= Ox05b1s_IsiReadRegIss(handle, 0x3b20, &value);//enable strobe 0xa5
	if (value != 0)
		pOx05b1sCtx->irLightExp.irOn = BOOL_TRUE;

	else
		pOx05b1sCtx->irLightExp.irOn = BOOL_FALSE;

	result |= Ox05b1s_IsiReadRegIss(handle, 0x3b25, &value);//strobe width
	irLine = (value & 0xff) << 24;
	result |= Ox05b1s_IsiReadRegIss(handle, 0x3b26, &value);//0x00
	irLine |= (value & 0xff) << 16;
	result |= Ox05b1s_IsiReadRegIss(handle, 0x3b27, &value);//0x00
	irLine |= (value & 0xff) << 8;
	result |= Ox05b1s_IsiReadRegIss(handle, 0x3b28, &value); //0x1a
	irLine |= value & 0xff;
	pOx05b1sCtx->irLightExp.irStrength = MAX(MIN(irLine,
					     pOx05b1sCtx->irLightInfo.irStrength.maxIrStrength),
					     pOx05b1sCtx->irLightInfo.irStrength.minIrStrength);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

static RESULT Ox05b1s_IsiOpenIss(IsiSensorHandle_t handle, uint32_t mode)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (!pOx05b1sCtx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pOx05b1sCtx->streaming != BOOL_FALSE)
		return RET_WRONG_STATE;

	pOx05b1sCtx->sensorMode.index = mode;
	IsiSensorMode_t *SensorDefaultMode = NULL;
	for (int i = 0; i < sizeof(pox05b1s_mode_info) / sizeof(IsiSensorMode_t); i++) {
		if (pox05b1s_mode_info[i].index == pOx05b1sCtx->sensorMode.index) {
			SensorDefaultMode = &(pox05b1s_mode_info[i]);
			break;
		}
	}

	if (pOx05b1sCtx->sensorMode.index == 1) {
		int32_t osRet = OSLAYER_OK;
		if (osRet != OSLAYER_OK)
			return RET_FAILURE;
	}
	if (SensorDefaultMode != NULL) {
		int Status = XST_SUCCESS;


#if 1
		switch (SensorDefaultMode->index) {
			case 0:
				for (int i = 0;
				     i < sizeof(Ox05b1s_mipi4lane_2592_1944_linear_init) / sizeof(
					     Ox05b1s_mipi4lane_2592_1944_linear_init[0]); i++) {
					if (Ox05b1s_mipi4lane_2592_1944_linear_init[i][0] == OX05B1S_TABLE_WAIT)
						vTaskDelay(Ox05b1s_mipi4lane_2592_1944_linear_init[i][1]);

					else if (Ox05b1s_mipi4lane_2592_1944_linear_init[i][0] == OX05B1S_TABLE_END)
						break;

					else
						Ox05b1s_IsiWriteRegIss(handle, Ox05b1s_mipi4lane_2592_1944_linear_init[i][0],
								       Ox05b1s_mipi4lane_2592_1944_linear_init[i][1]);

				}
				break;
			case 1:
				for (int i = 0;
				     i < sizeof(Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init) / sizeof(
					     Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[0]); i++) {
					if (Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[i][0] == OX05B1S_TABLE_WAIT)
						vTaskDelay(Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[i][1]);

					else if (Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[i][0] == OX05B1S_TABLE_END)
						break;

					else
						Ox05b1s_IsiWriteRegIss(handle, Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[i][0],
								       Ox05b1s_mipi4lane_2592_1944_linear_ABmode_init[i][1]);

				}
				break;
			case 2:
				for (int i = 0;
				     i < sizeof(Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init) / sizeof(
					     Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[0]); i++) {
					if (Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[i][0] == OX05B1S_TABLE_WAIT)
						vTaskDelay(Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[i][1]);

					else if (Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[i][0] == OX05B1S_TABLE_END)
						break;

					else
						Ox05b1s_IsiWriteRegIss(handle, Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[i][0],
								       Ox05b1s_mipi4lane_2592_1944_linear_ABmode_IRframe_init[i][1]);

				}
				break;
			default:
				xil_printf("%s:not support sensor mode %d\n", __func__, pOx05b1sCtx->sensorMode.index);
				osFree(pOx05b1sCtx);
				return RET_NOTSUPP;
				break;
		}
#endif
		memcpy(&(pOx05b1sCtx->sensorMode), SensorDefaultMode, sizeof(IsiSensorMode_t));
	} else {
		xil_printf("%s: Invalid SensorDefaultMode\n", __func__);
		return (RET_NULL_POINTER);
	}

	switch (pOx05b1sCtx->sensorMode.index) {
		case 0:
			pOx05b1sCtx->oneLineExpTime = 0.000027;
			pOx05b1sCtx->frameLengthLines = 0x1d00;
			pOx05b1sCtx->curFrameLengthLines = pOx05b1sCtx->frameLengthLines;
			pOx05b1sCtx->maxIntegrationLine = pOx05b1sCtx->frameLengthLines - 30 ;
			pOx05b1sCtx->minIntegrationLine = 1;
			pOx05b1sCtx->aecMaxGain = 230;
			pOx05b1sCtx->aecMinGain = 1.0;
			pOx05b1sCtx->aGain.min = 1.0;
			pOx05b1sCtx->aGain.max = 15.5;
			pOx05b1sCtx->aGain.step = (1.0f / 16.0f);
			pOx05b1sCtx->dGain.min = 1.0;
			pOx05b1sCtx->dGain.max = 15;
			pOx05b1sCtx->dGain.step = (1.0f / 1024.0f);
			break;
		case 1:
		case 2:
			pOx05b1sCtx->oneLineExpTime = 0.000027;//0x500
			pOx05b1sCtx->frameLengthLines = 0x1d00;
			pOx05b1sCtx->curFrameLengthLines = pOx05b1sCtx->frameLengthLines;
			pOx05b1sCtx->maxIntegrationLine = pOx05b1sCtx->frameLengthLines - 30 ;
			pOx05b1sCtx->minIntegrationLine = 1;
			pOx05b1sCtx->aecMaxGain = 230;
			pOx05b1sCtx->aecMinGain = 1.0;
			pOx05b1sCtx->aGain.min = 1.0;
			pOx05b1sCtx->aGain.max = 15.5;
			pOx05b1sCtx->aGain.step = (1.0f / 16.0f);
			pOx05b1sCtx->dGain.min = 1.0;
			pOx05b1sCtx->dGain.max = 15;
			pOx05b1sCtx->dGain.step = (1.0f / 1024.0f);
			break;
		default:
			xil_printf("%s:not support sensor mode %d\n", __func__, pOx05b1sCtx->sensorMode.index);
			return RET_NOTSUPP;
			break;
	}

	pOx05b1sCtx->maxFps = pOx05b1sCtx->sensorMode.fps;
	pOx05b1sCtx->minFps = 1 * ISI_FPS_QUANTIZE;
	pOx05b1sCtx->currFps = pOx05b1sCtx->maxFps;
	pOx05b1sCtx->sensorWb.rGain = 1.8;
	pOx05b1sCtx->sensorWb.gbGain = 1.0;
	pOx05b1sCtx->sensorWb.grGain = 1.0;
	pOx05b1sCtx->sensorWb.bGain = 1.65;

	xil_printf("%s: Ox05b1s System-Reset executed\n", __func__);
	osSleep(100);

	result = Ox05b1s_AecSetModeParameters(handle, pOx05b1sCtx);
	if (result != RET_SUCCESS) {
		xil_printf("%s: SetupOutputWindow failed.\n", __func__);
		return (result);
	}

	pOx05b1sCtx->configured = BOOL_TRUE;


	xil_printf("%s: (exit)\n", __func__);
	return 0;
}

static RESULT Ox05b1s_IsiCloseIss(IsiSensorHandle_t handle)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx05b1sCtx == NULL)
		return (RET_WRONG_HANDLE);

	(void)Ox05b1s_IsiSetStreamingIss(pOx05b1sCtx, BOOL_FALSE);


	if (pOx05b1sCtx->sensorMode.index == 2) {
		int32_t osRet = OSLAYER_OK;
		if (osRet != OSLAYER_OK)
			return RET_FAILURE;
	}
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_IsiReleaseIss(IsiSensorHandle_t handle)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx05b1sCtx == NULL)
		return (RET_WRONG_HANDLE);

	MEMSET(pOx05b1sCtx, 0, sizeof(Ox05b1s_Context_t));
	osFree(pOx05b1sCtx);
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_IsiCheckConnectionIss(IsiSensorHandle_t handle)
{
	RESULT result = RET_SUCCESS;

	uint32_t sensor_id = 0;
	uint32_t correct_id = 0x5805;

	xil_printf("%s (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	result = Ox05b1s_IsiGetRevisionIss(handle, &sensor_id);
	if (result != RET_SUCCESS) {
		xil_printf("%s: Read Sensor ID Error! \n", __func__);
		return (RET_FAILURE);
	}

	if (correct_id != sensor_id) {
		xil_printf("%s:ChipID =0x%x sensor_id=%x error! \n", __func__, correct_id, sensor_id);
		return (RET_FAILURE);
	}

	xil_printf("%s ChipID = 0x%08x, sensor_id = 0x%08x, success! \n", __func__, correct_id, sensor_id);
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_IsiGetRevisionIss(IsiSensorHandle_t handle, uint32_t *pValue)
{
	RESULT result = RET_SUCCESS;
	uint16_t reg_val;
	uint32_t sensor_id;

	xil_printf("%s (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	reg_val = 0;
	result = Ox05b1s_IsiReadRegIss(handle, 0x300a, &reg_val);
	sensor_id = (reg_val & 0xff) << 8;

	reg_val = 0;
	result |= Ox05b1s_IsiReadRegIss(handle, 0x300b, &reg_val);
	sensor_id |= (reg_val & 0xff);

	*pValue = sensor_id;
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_IsiSetABmodeGroup
(
	IsiSensorHandle_t handle,
	const uint8_t mode,
	OX05B1S_ABmode_Setting_t setting
)
{
	RESULT result = RET_SUCCESS;

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	int32_t osRet = OSLAYER_OK;
	xil_printf("%s: mode: %d (enter)\n", __func__, pOx05b1sCtx->sensorMode.index);

	if (osRet != OSLAYER_OK)
		return RET_FAILURE;

	if (pOx05b1sCtx->sensorMode.index == 1) {//A mode: RGB frame

		if ((mode & 0x1) != 0)
			gOX05B1S_ABmode[0].expLine = setting.expLine;
		if ((mode & 0x2) != 0)
			gOX05B1S_ABmode[0].again = setting.again;
		if ((mode & 0x4) != 0)
			gOX05B1S_ABmode[0].dgain = setting.dgain;
		if ((mode & 0x8) != 0) {
			gOX05B1S_ABmode[0].irCfg.irOn = setting.irCfg.irOn;
			gOX05B1S_ABmode[0].irCfg.irStrength = setting.irCfg.irStrength;
		}

	} else if (pOx05b1sCtx->sensorMode.index == 2) {//B mode: IR frame

		if ((mode & 0x1) != 0)
			gOX05B1S_ABmode[1].expLine = setting.expLine;
		if ((mode & 0x2) != 0)
			gOX05B1S_ABmode[1].again = setting.again;
		if ((mode & 0x4) != 0)
			gOX05B1S_ABmode[1].dgain = setting.dgain;

		if ((mode & 0x8) != 0) {
			gOX05B1S_ABmode[1].irCfg.irOn = setting.irCfg.irOn;
			gOX05B1S_ABmode[1].irCfg.irStrength = setting.irCfg.irStrength;
		}
	} else
		return RET_UNSUPPORT_ID;

	result = Ox05b1s_IsiWriteRegIss(handle, 0x320a, 0x01);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x320b, 0x01);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x320c, 0x00);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x320d, 0x00);

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3208, 0x00);//group0 start
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x431c, 0x00);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x4813, 0x01);//VC0: RGB frame

	//disable strobe output
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b20, 0x00);

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3500, (gOX05B1S_ABmode[0].expLine >> 16) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3501, (gOX05B1S_ABmode[0].expLine >> 8) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3502, (gOX05B1S_ABmode[0].expLine & 0xff));
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3508, (gOX05B1S_ABmode[0].again >> 4) & 0x0f);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3509, (gOX05B1S_ABmode[0].again & 0x0f) << 4);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350a, (gOX05B1S_ABmode[0].dgain >> 10) & 0x0f);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350b, (gOX05B1S_ABmode[0].dgain >> 2) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350c, (gOX05B1S_ABmode[0].dgain & 0x03) << 6);

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3208, 0x10);//group0 end

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3208, 0x01);//group1 start
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x431c, 0x09);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x4813, 0x04);//VC1: IR frame

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3500, (gOX05B1S_ABmode[1].expLine >> 16) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3501, (gOX05B1S_ABmode[1].expLine >> 8) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3502, (gOX05B1S_ABmode[1].expLine & 0xff));
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3508, (gOX05B1S_ABmode[1].again >> 4) & 0x0f);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3509, (gOX05B1S_ABmode[1].again & 0x0f) << 4);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350a, (gOX05B1S_ABmode[1].dgain >> 10) & 0x0f);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350b, (gOX05B1S_ABmode[1].dgain >> 2) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x350c, (gOX05B1S_ABmode[1].dgain & 0x03) << 6);

	if (gOX05B1S_ABmode[1].irCfg.irOn == BOOL_TRUE)
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b20, 0xff);

	else
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b20, 0x00);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b1e, 0x00);//auto with exposure width,bit[0] 1 enable

	// result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b21, (gOX05B1S_ABmode[1].irCfg.strobeShift >> 24) & 0xff);//strobe shfit
	// result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b22, (gOX05B1S_ABmode[1].irCfg.strobeShift >> 16) & 0xff);
	// result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b23, (gOX05B1S_ABmode[1].irCfg.strobeShift >> 8) & 0xff);
	// result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b24, gOX05B1S_ABmode[1].irCfg.strobeShift & 0xff);

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b25,
					 (gOX05B1S_ABmode[1].irCfg.irStrength >> 24) & 0xff);//strobe width
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b26,
					 (gOX05B1S_ABmode[1].irCfg.irStrength >> 16) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b27, (gOX05B1S_ABmode[1].irCfg.irStrength >> 8) & 0xff);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b28, gOX05B1S_ABmode[1].irCfg.irStrength & 0xff);

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b2f, 0x4a);//affect ir Light enable!!

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3208, 0x11);//group1 end

	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3211, 0x30);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x3208, 0xa0);

	if (osRet != OSLAYER_OK)
		return RET_FAILURE;

	xil_printf("%s: (exit)\n", __func__);
	return result;
}

static RESULT Ox05b1s_IsiSetStreamingIss(IsiSensorHandle_t handle, bool_t on)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	xil_printf("%s Enabling IR Power...!\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	if (pOx05b1sCtx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (pOx05b1sCtx->sensorMode.index != 2) { // in B mode, do not need repeat streaming on
		enable_IR_power(handle);
		result = Ox05b1s_IsiWriteRegIss(handle, 0x0100, on);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set sensor streaming error! \n", __func__);
			return (RET_FAILURE);
		}
	}

	//in AB mode, A frame first in
	if (pOx05b1sCtx->sensorMode.index == 1) {
		//initial group setting in sensor streaming

		uint8_t mode = 0;
		OX05B1S_ABmode_Setting_t setting = {0, 0, 0, {0, 0}};

		//config RGB frame initial exp value;
		gOX05B1S_ABmode[0].expLine = 0x50;
		gOX05B1S_ABmode[0].again = 0x00;
		gOX05B1S_ABmode[0].dgain = 0x400;
		gOX05B1S_ABmode[0].irCfg.irOn = BOOL_FALSE;
		memcpy(&setting, gOX05B1S_ABmode, sizeof(OX05B1S_ABmode_Setting_t));
		//config IR frame initial exp value;
		gOX05B1S_ABmode[1].expLine = 0x50;
		gOX05B1S_ABmode[1].again = 0x00;
		gOX05B1S_ABmode[1].dgain = 0x400;
		gOX05B1S_ABmode[1].irCfg.irOn = BOOL_TRUE;
		gOX05B1S_ABmode[1].irCfg.irStrength = 0x10;
		mode = ((1 << OS05B1S_AB_MODE_EXP_LINE)
			| (1 << OS05B1S_AB_MODE_A_GAIN)
			| (1 << OS05B1S_AB_MODE_D_GAIN)
			| (1 << OS05B1S_AB_MODE_IR_PARAMS));

		result = Ox05b1s_IsiSetABmodeGroup(handle, mode, setting);

		if (result != RET_SUCCESS) {
			xil_printf("%s: set init Ox05b1s_IsiSetABmodeGroup error! %d \n", __func__, result);
			return (result);
		}
		pOx05b1sCtx->irLightExp.irOn = BOOL_FALSE;
		pOx05b1sCtx->irLightExp.irStrength = 0;
	} else if (pOx05b1sCtx->sensorMode.index == 2) {
		pOx05b1sCtx->irLightExp.irOn = BOOL_TRUE;
		pOx05b1sCtx->irLightExp.irStrength = 0x10;
	}

	pOx05b1sCtx->streaming = on;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox05b1s_pIsiGetAeBaseInfoIss(IsiSensorHandle_t handle, IsiAeBaseInfo_t *pAeBaseInfo)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pOx05b1sCtx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pAeBaseInfo == NULL) {
		xil_printf("%s: NULL pointer received!!\n");
		return (RET_NULL_POINTER);
	}

	pAeBaseInfo->gain.min = pOx05b1sCtx->aecMinGain;//total gain
	pAeBaseInfo->gain.max = pOx05b1sCtx->aecMaxGain;
	pAeBaseInfo->intTime.min = pOx05b1sCtx->aecMinIntegrationTime;
	pAeBaseInfo->intTime.max = pOx05b1sCtx->aecMaxIntegrationTime;

	pAeBaseInfo->aGain = pOx05b1sCtx->aGain;//min, max, step
	pAeBaseInfo->dGain = pOx05b1sCtx->dGain;

	pAeBaseInfo->aecCurGain = pOx05b1sCtx->aecCurGain;
	pAeBaseInfo->aecCurIntTime = pOx05b1sCtx->aecCurIntegrationTime;
	pAeBaseInfo->aecGainStep = pOx05b1sCtx->aecGainIncrement;
	pAeBaseInfo->aecIntTimeStep = pOx05b1sCtx->aecIntegrationTimeIncrement;

	pAeBaseInfo->aecIrLightExp = pOx05b1sCtx->irLightExp;
	pAeBaseInfo->aecIrLightInfo = pOx05b1sCtx->irLightInfo;

	xil_printf("%s: (enter)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiSetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	uint32_t again = 0;

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	if (pSensorAGain->gain[ISI_LINEAR_PARAS] < pOx05b1sCtx->aGain.min) {
		xil_printf("%s: invalid too small again parameter!\n", __func__);
		pSensorAGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->aGain.min;
	}

	if (pSensorAGain->gain[ISI_LINEAR_PARAS] > pOx05b1sCtx->aGain.max) {
		xil_printf("%s: invalid too big again parameter!\n", __func__);
		pSensorAGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->aGain.max;
	}


	again = (uint32_t)(pSensorAGain->gain[ISI_LINEAR_PARAS] * 16);
	xil_printf("%s: in mode %d again %d\n", __func__, pOx05b1sCtx->sensorMode.index, again);

	if (pOx05b1sCtx->sensorMode.index == 1 || pOx05b1sCtx->sensorMode.index == 2) {
		uint8_t mode = 0;
		OX05B1S_ABmode_Setting_t setting = {0, 0, 0, {0, 0}};

		mode = (1 << OS05B1S_AB_MODE_A_GAIN);
		setting.again = again;

		result = Ox05b1s_IsiSetABmodeGroup(handle, mode, setting);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set Ox05b1s_IsiSetABmodeGroup error! %d \n", __func__, result);
			return (result);
		}
	} else {
		result = Ox05b1s_IsiWriteRegIss(handle, 0x3508, (again >> 4) & 0x0f);
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x3509, (again & 0x0f) << 4);
	}

	pOx05b1sCtx->curAgain = (float)again / 16.0f;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiSetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	uint32_t dgain = 0;

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] < pOx05b1sCtx->dGain.min) {
		xil_printf("%s: invalid too small dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->dGain.min;
	}

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] > pOx05b1sCtx->dGain.max) {
		xil_printf("%s: invalid too big dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->dGain.max;
	}

	dgain = (uint32_t)(pSensorDGain->gain[ISI_LINEAR_PARAS] * 1024);
	xil_printf("%s: in mode %d, dgain %d\n", __func__, pOx05b1sCtx->sensorMode.index, dgain);

	if (pOx05b1sCtx->sensorMode.index == 1 || pOx05b1sCtx->sensorMode.index == 2) {
		uint8_t mode = 0;
		OX05B1S_ABmode_Setting_t setting = {0, 0, 0, {0, 0}};

		mode = (1 << OS05B1S_AB_MODE_D_GAIN);
		setting.dgain = dgain;

		result = Ox05b1s_IsiSetABmodeGroup(handle, mode, setting);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set Ox05b1s_IsiSetABmodeGroup error! %d \n", __func__, result);
			return (result);
		}
	} else {
		result = Ox05b1s_IsiWriteRegIss(handle, 0x350a, (dgain >> 10) & 0x0f);
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x350b, (dgain >> 2) & 0xff);
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x350c, (dgain & 0x03) << 6);
	}

	pOx05b1sCtx->curDgain = (float)dgain / 1024.0f;
	pOx05b1sCtx->aecCurGain = pOx05b1sCtx->curAgain * pOx05b1sCtx->curDgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx05b1sCtx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorAGain == NULL)
		return (RET_NULL_POINTER);

	pSensorAGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->curAgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx05b1sCtx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorDGain == NULL)
		return (RET_NULL_POINTER);

	pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx05b1sCtx->curDgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiSetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;

	if (!pOx05b1sCtx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	uint32_t expLine = 0;
	expLine = pSensorIntTime->intTime[ISI_LINEAR_PARAS] / pOx05b1sCtx->oneLineExpTime;
	expLine = MIN(pOx05b1sCtx->maxIntegrationLine, MAX(pOx05b1sCtx->minIntegrationLine, expLine));
	xil_printf("%s: in mode %d, set expLine = 0x%04x \n", __func__, pOx05b1sCtx->sensorMode.index,
		   expLine);

	if (pOx05b1sCtx->sensorMode.index == 1 || pOx05b1sCtx->sensorMode.index == 2) {
		uint8_t mode = 0;
		OX05B1S_ABmode_Setting_t setting = {0, 0, 0, {0, 0}};

		mode = (1 << OS05B1S_AB_MODE_EXP_LINE);
		setting.expLine = expLine;

		result = Ox05b1s_IsiSetABmodeGroup(handle, mode, setting);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set Ox05b1s_IsiSetABmodeGroup error! %d \n", __func__, result);
			return (result);
		}
	} else {
		result = Ox05b1s_IsiWriteRegIss(handle, 0x3500, (expLine >> 16) & 0xff);
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x3501, (expLine >> 8) & 0xff);
		result |= Ox05b1s_IsiWriteRegIss(handle, 0x3502, (expLine & 0xff));
	}

	pOx05b1sCtx->aecCurIntegrationTime = expLine * pOx05b1sCtx->oneLineExpTime;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (!pOx05b1sCtx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (!pSensorIntTime)
		return (RET_NULL_POINTER);

	pSensorIntTime->intTime[ISI_LINEAR_PARAS] = pOx05b1sCtx->aecCurIntegrationTime;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiSetIRLightExpIss(IsiSensorHandle_t handle, const IsiIrLightExp_t *pIrExpParam)
{
	RESULT result = RET_SUCCESS;
	uint32_t irStrobeLine = 0, irStrobeShift = 0, expLine = 0;
	xil_printf("%s: (enter)\n", __func__);
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;

	if (!pOx05b1sCtx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}
	if (pIrExpParam == NULL)
		return RET_NULL_POINTER;

	if (pIrExpParam->irOn == BOOL_TRUE)
		irStrobeLine = MAX(MIN(pIrExpParam->irStrength, pOx05b1sCtx->irLightInfo.irStrength.maxIrStrength),
				   pOx05b1sCtx->irLightInfo.irStrength.maxIrStrength);

	else
		irStrobeLine = pOx05b1sCtx->irLightExp.irStrength;

	xil_printf("%s: in mode %d, set irStrobeLine = 0x%04x \n", __func__, pOx05b1sCtx->sensorMode.index,
		   irStrobeLine);

	if (pOx05b1sCtx->sensorMode.index == 1 || pOx05b1sCtx->sensorMode.index == 2) {
		uint8_t mode = 0;
		OX05B1S_ABmode_Setting_t setting = {0, 0, 0, {0, 0}};

		mode = (1 << OS05B1S_AB_MODE_IR_PARAMS);

		setting.irCfg.irOn = pIrExpParam->irOn;
		setting.irCfg.irStrength = irStrobeLine;

		result = Ox05b1s_IsiSetABmodeGroup(handle, mode, setting);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set Ox05b1s_IsiSetABmodeGroup error! %d \n", __func__, result);
			return (result);
		}
	} else {
		if (pIrExpParam->irOn == BOOL_TRUE) {
			result = Ox05b1s_IsiWriteRegIss(handle, 0x3b20, 0xff);
			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b1e, 0);//01 cannot
			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b25, (irStrobeLine >> 24) && 0xff);
			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b26, (irStrobeLine >> 16) && 0xff);
			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b27, (irStrobeLine >> 8) && 0xff);
			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b28, irStrobeLine & 0xff);

			result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b2f, 0x4a);//affect ir Light enable!!

			//result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b21, (irStrobeShift >> 24) && 0xff);
			//result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b22, (irStrobeShift >> 16) && 0xff);
			//result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b23, (irStrobeShift >> 8) && 0xff);
			//result |= Ox05b1s_IsiWriteRegIss(handle, 0x3b24, irStrobeShift & 0xff);
		} else
			result = Ox05b1s_IsiWriteRegIss(handle, 0x3b20, 0);
	}
	pOx05b1sCtx->irLightExp.irOn = pIrExpParam->irOn;
	pOx05b1sCtx->irLightExp.irStrength = irStrobeLine;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetIRLightExpIss(IsiSensorHandle_t handle, IsiIrLightExp_t *pIrExpParam)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (!pOx05b1sCtx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (!pIrExpParam)
		return (RET_NULL_POINTER);

	pIrExpParam->irOn = pOx05b1sCtx->irLightExp.irOn;
	pIrExpParam->irStrength = pOx05b1sCtx->irLightExp.irStrength;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetFpsIss(IsiSensorHandle_t handle, uint32_t *pFps)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx05b1sCtx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	*pFps = pOx05b1sCtx->currFps;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiSetFpsIss(IsiSensorHandle_t handle, uint32_t fps)
{
	RESULT result = RET_SUCCESS;
	int32_t NewVts = 0;

	xil_printf("%s: (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (fps > pOx05b1sCtx->maxFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx05b1sCtx->maxFps, pOx05b1sCtx->minFps, pOx05b1sCtx->maxFps);
		fps = pOx05b1sCtx->maxFps;
	}
	if (fps < pOx05b1sCtx->minFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx05b1sCtx->minFps, pOx05b1sCtx->minFps, pOx05b1sCtx->maxFps);
		fps = pOx05b1sCtx->minFps;
	}

	NewVts = pOx05b1sCtx->frameLengthLines * pOx05b1sCtx->sensorMode.fps / fps;
	result = Ox05b1s_IsiWriteRegIss(handle, 0x380e, NewVts >> 8);
	result |= Ox05b1s_IsiWriteRegIss(handle, 0x380f, NewVts & 0xff);
	pOx05b1sCtx->currFps = fps;
	pOx05b1sCtx->curFrameLengthLines = NewVts;
	pOx05b1sCtx->maxIntegrationLine = pOx05b1sCtx->curFrameLengthLines - 30;
	pOx05b1sCtx->aecMaxIntegrationTime = pOx05b1sCtx->maxIntegrationLine * pOx05b1sCtx->oneLineExpTime;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetIspStatusIss(IsiSensorHandle_t handle, IsiIspStatus_t *pIspStatus)
{
	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_WRONG_HANDLE;
	xil_printf("%s: (enter)\n", __func__);

	pIspStatus->useSensorAE = false;
	pIspStatus->useSensorBLC = false;
	pIspStatus->useSensorAWB = false;

	xil_printf("%s: (exit)\n", __func__);
	return RET_SUCCESS;
}

RESULT Ox05b1s_IsiSetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t tpg)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL)
		return RET_NULL_POINTER;

	if (pOx05b1sCtx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (tpg.enable == 0)
		result = Ox05b1s_IsiWriteRegIss(handle, 0x5100, 0x00);

	else
		result = Ox05b1s_IsiWriteRegIss(handle, 0x5100, 0x80);

	pOx05b1sCtx->testPattern = tpg.enable;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t *pTpg)
{
	RESULT result = RET_SUCCESS;
	uint16_t value = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox05b1s_Context_t *pOx05b1sCtx = (Ox05b1s_Context_t *) handle;
	if (pOx05b1sCtx == NULL || pTpg == NULL)
		return RET_NULL_POINTER;

	if (pOx05b1sCtx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (!Ox05b1s_IsiReadRegIss(handle, 0x5100, &value)) {
		pTpg->enable = ((value & 0x80) != 0) ? 1 : 0;
		if (pTpg->enable)
			pTpg->pattern = (0xff & value);
		pOx05b1sCtx->testPattern = pTpg->enable;
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox05b1s_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
	RESULT result = RET_SUCCESS;
	static const char SensorName[16] = "Ox05b1s";

	if (pIsiSensor != NULL) {
		pIsiSensor->pszName = SensorName;
		pIsiSensor->pIsiCreateIss = Ox05b1s_IsiCreateIss;
		pIsiSensor->pIsiOpenIss = Ox05b1s_IsiOpenIss;
		pIsiSensor->pIsiCloseIss = Ox05b1s_IsiCloseIss;
		pIsiSensor->pIsiReleaseIss = Ox05b1s_IsiReleaseIss;
		pIsiSensor->pIsiReadRegIss = Ox05b1s_IsiReadRegIss;
		pIsiSensor->pIsiWriteRegIss = Ox05b1s_IsiWriteRegIss;
		pIsiSensor->pIsiGetModeIss = Ox05b1s_IsiGetModeIss;
		pIsiSensor->pIsiEnumModeIss = Ox05b1s_IsiEnumModeIss;
		pIsiSensor->pIsiGetCapsIss = Ox05b1s_IsiGetCapsIss;
		pIsiSensor->pIsiCheckConnectionIss = Ox05b1s_IsiCheckConnectionIss;
		pIsiSensor->pIsiGetRevisionIss = Ox05b1s_IsiGetRevisionIss;
		pIsiSensor->pIsiSetStreamingIss = Ox05b1s_IsiSetStreamingIss;

		/* AEC */
		pIsiSensor->pIsiGetAeBaseInfoIss = Ox05b1s_pIsiGetAeBaseInfoIss;
		pIsiSensor->pIsiGetAGainIss = Ox05b1s_IsiGetAGainIss;
		pIsiSensor->pIsiSetAGainIss = Ox05b1s_IsiSetAGainIss;
		pIsiSensor->pIsiGetDGainIss = Ox05b1s_IsiGetDGainIss;
		pIsiSensor->pIsiSetDGainIss = Ox05b1s_IsiSetDGainIss;
		pIsiSensor->pIsiGetIntTimeIss = Ox05b1s_IsiGetIntTimeIss;
		pIsiSensor->pIsiSetIntTimeIss = Ox05b1s_IsiSetIntTimeIss;
		pIsiSensor->pIsiGetFpsIss = Ox05b1s_IsiGetFpsIss;
		pIsiSensor->pIsiSetFpsIss = Ox05b1s_IsiSetFpsIss;

		/* SENSOR ISP */
		pIsiSensor->pIsiGetIspStatusIss = Ox05b1s_IsiGetIspStatusIss;
		pIsiSensor->pIsiSetWBIss = NULL;
		pIsiSensor->pIsiGetWBIss = NULL;
		pIsiSensor->pIsiSetBlcIss = NULL;
		pIsiSensor->pIsiGetBlcIss = NULL;

		/* SENSOE OTHER FUNC*/
		pIsiSensor->pIsiSetTpgIss = Ox05b1s_IsiSetTpgIss;
		pIsiSensor->pIsiGetTpgIss = Ox05b1s_IsiGetTpgIss;
		pIsiSensor->pIsiGetExpandCurveIss = NULL;

		/* AF */
		pIsiSensor->pIsiFocusCreateIss = NULL;
		pIsiSensor->pIsiFocusReleaseIss = NULL;
		pIsiSensor->pIsiFocusGetCalibrateIss = NULL;
		pIsiSensor->pIsiFocusSetIss = NULL;
		pIsiSensor->pIsiFocusGetIss = NULL;

		pIsiSensor->pIsiSetIRLightExpIss = Ox05b1s_IsiSetIRLightExpIss;
		pIsiSensor->pIsiGetIRLightExpIss = Ox05b1s_IsiGetIRLightExpIss;

	} else
		result = RET_NULL_POINTER;

	return (result);
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t Ox05b1s_IsiCamDrvConfig = {
	.cameraDriverID = 0x5805,
	.pIsiGetSensorIss = Ox05b1s_IsiGetSensorIss,
};
