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
#include <isi_fmc.h>
#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "ox03f10_priv.h"


#define Ox03f10_MIN_GAIN_STEP    ( 1.0f/1024.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */

/*****************************************************************************
 *Sensor Info
*****************************************************************************/

IsiSensorMode_t pox03f10_mode_info[] = {
	{
		.index = 0,
		.size = {
			.boundsWidth = 1920,
			.boundsHeight = 1080,
			.top = 0,
			.left = 0,
			.width = 1920,
			.height = 1080,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 30 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_HDR_NATIVE,
		.nativeMode = ISI_SENSOR_NATIVE_DCG_SPD_VS,
		.bitWidth = 12,
		.compress.enable = 1,
		.compress.xBit = 24,
		.compress.yBit = 12,
		.bayerPattern = ISI_BPAT_GRBG,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
	{
		.index = 1,
		.size = {
			.boundsWidth = 1280,
			.boundsHeight = 720,
			.top = 0,
			.left = 0,
			.width = 1280,
			.height = 720,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 30 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_HDR_NATIVE,
		.nativeMode = ISI_SENSOR_NATIVE_DCG_SPD_VS,
		.bitWidth = 12,
		.compress.enable = 1,
		.compress.xBit = 24,
		.compress.yBit = 12,
		.bayerPattern = ISI_BPAT_GRBG,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
	{
		.index = 2,
		.size = {
			.boundsWidth = 640,
			.boundsHeight = 480,
			.top = 0,
			.left = 0,
			.width = 640,
			.height = 480,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 30 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_HDR_NATIVE,
		.nativeMode = ISI_SENSOR_NATIVE_DCG_SPD_VS,
		.bitWidth = 12,
		.compress.enable = 1,
		.compress.xBit = 24,
		.compress.yBit = 12,
		.bayerPattern = ISI_BPAT_GRBG,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	}
};

static RESULT Ox03f10_IsiReadRegIss(IsiSensorHandle_t handle, const uint16_t addr, uint16_t *pValue)
{
	RESULT result = RET_SUCCESS;

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	u8 slave_addr = (g_fmc_single.sensor_array[pOx03f10Ctx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx03f10Ctx->sensorDevId]->readIIC(pOx03f10Ctx->i2cId,
		 slave_addr, addr, 0x2, pValue, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal read sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	return (result);
}

static RESULT Ox03f10_IsiWriteRegIss(IsiSensorHandle_t handle, const uint16_t addr,
				     const uint16_t value)
{
	RESULT result = RET_SUCCESS;

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	u8 slave_addr = (g_fmc_single.sensor_array[pOx03f10Ctx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx03f10Ctx->sensorDevId]->writeIIC(pOx03f10Ctx->i2cId,
		 slave_addr, addr, 0x2, value, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal write sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	return (result);
}

static RESULT Ox03f10_IsiGetModeIss(IsiSensorHandle_t handle, IsiSensorMode_t *pMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return (RET_WRONG_HANDLE);
	if (pMode == NULL)
		return (RET_WRONG_HANDLE);

	memcpy(pMode, &(pOx03f10Ctx->sensorMode), sizeof(pOx03f10Ctx->sensorMode));

	xil_printf("%s (exit)\n", __func__);
	return (RET_SUCCESS);
}

static RESULT Ox03f10_IsiEnumModeIss(IsiSensorHandle_t handle, IsiSensorEnumMode_t *pEnumMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	if (pEnumMode->index >= (sizeof(pox03f10_mode_info) / sizeof(pox03f10_mode_info[0])))
		return RET_OUTOFRANGE;

	for (uint32_t i = 0; i < (sizeof(pox03f10_mode_info) / sizeof(pox03f10_mode_info[0])); i++) {
		if (pox03f10_mode_info[i].index == pEnumMode->index) {
			memcpy(&pEnumMode->mode, &pox03f10_mode_info[i], sizeof(IsiSensorMode_t));
			xil_printf("%s (exit)\n", __func__);
			return RET_SUCCESS;
		}
	}

	return RET_NOTSUPP;
}

static RESULT Ox03f10_IsiGetCapsIss(IsiSensorHandle_t handle, IsiCaps_t *pCaps)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (pOx03f10Ctx == NULL)
		return (RET_WRONG_HANDLE);

	if (pCaps == NULL)
		return (RET_NULL_POINTER);

	pCaps->bitWidth = pOx03f10Ctx->sensorMode.bitWidth;
	pCaps->mode = ISI_MODE_BAYER;
	pCaps->bayerPattern = pOx03f10Ctx->sensorMode.bayerPattern;
	pCaps->resolution.width = pOx03f10Ctx->sensorMode.size.width;
	pCaps->resolution.height = pOx03f10Ctx->sensorMode.size.height;
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

static RESULT Ox03f10_IsiCreateIss(IsiSensorInstanceConfig_t *pConfig, IsiSensorHandle_t *pHandle)
{
	RESULT result = RET_SUCCESS;
	uint32_t desId = 0, pipeId = 0;
	xil_printf("%s (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) osMalloc(sizeof(Ox03f10_Context_t));
	if (!pOx03f10Ctx) {
		xil_printf("%s: Can't allocate ox03f10 context\n", __func__);
		return (RET_OUTOFMEM);
	}

	MEMSET(pOx03f10Ctx, 0, sizeof(Ox03f10_Context_t));

	pOx03f10Ctx->isiCtx.pSensor = pConfig->pSensor;
	pOx03f10Ctx->groupHold = BOOL_FALSE;
	pOx03f10Ctx->oldGain = 0;
	pOx03f10Ctx->oldIntegrationTime = 0;
	pOx03f10Ctx->configured = BOOL_FALSE;
	pOx03f10Ctx->streaming = BOOL_FALSE;
	pOx03f10Ctx->testPattern = BOOL_FALSE;
	pOx03f10Ctx->isAfpsRun = BOOL_FALSE;
	pOx03f10Ctx->sensorMode.index = 0;
	pOx03f10Ctx->i2cId	= 0;
	pOx03f10Ctx->sensorDevId = pConfig->cameraDevId;

	uint8_t busId = (uint8_t)pOx03f10Ctx->i2cId;
	pipeId = pOx03f10Ctx->sensorDevId;

	*pHandle = (IsiSensorHandle_t) pOx03f10Ctx;

#if 0
	result = init_MUX();
	if (result != XST_SUCCESS) {
		xil_printf("\n\rIIC Init Failed \n\r");
		return result;
	}
#endif

	if (pipeId >= IN_PIPE_LAST) {
		xil_printf("%s: sensor device ID %d is mot support!\n", __func__, pipeId);
		return RET_UNSUPPORT_ID;
	}
	desId = MAPPING_INPIPE_TO_DES_ID(pipeId);

	xil_printf("\r\n desId %d pipeId:%d\r\n", desId, pipeId);

	init_iic_access(pOx03f10Ctx->i2cId, pipeId);
	init_des(desId);
	init_sensor(pipeId, desId);

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_AecSetModeParameters(IsiSensorHandle_t handle, Ox03f10_Context_t *pOx03f10Ctx)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s%s: (enter)\n", __func__, pOx03f10Ctx->isAfpsRun ? "(AFPS)" : "");
	uint32_t exp_line = 0, again = 0, dgain = 0;
	uint16_t value = 0;


	pOx03f10Ctx->aecMinIntegrationTime = pOx03f10Ctx->oneLineDCGExpTime *
					     pOx03f10Ctx->minDCGIntegrationLine;
	pOx03f10Ctx->aecMaxIntegrationTime = pOx03f10Ctx->oneLineDCGExpTime *
					     pOx03f10Ctx->maxDCGIntegrationLine;
	xil_printf("%s: AecMaxIntegrationTime = %f \n", __func__, pOx03f10Ctx->aecMaxIntegrationTime);

	pOx03f10Ctx->aecGainIncrement = Ox03f10_MIN_GAIN_STEP;
	pOx03f10Ctx->aecIntegrationTimeIncrement = pOx03f10Ctx->oneLineDCGExpTime;
	pOx03f10Ctx->oldGain = 0;
	pOx03f10Ctx->oldIntegrationTime = 0;

	//reflects the state of the sensor registers, must equal default settings
	//get again
	Ox03f10_IsiReadRegIss(handle, 0x3508, &value);
	again = (value & 0x0f) << 4;
	Ox03f10_IsiReadRegIss(handle, 0x3509, &value);
	again = again | ((value & 0xf0) >> 4);

	//get dgain
	Ox03f10_IsiReadRegIss(handle, 0x350a, &value);
	dgain = (value & 0x0f) << 10;
	Ox03f10_IsiReadRegIss(handle, 0x350b, &value);
	dgain = dgain | ((value & 0xff) << 2);
	Ox03f10_IsiReadRegIss(handle, 0x350c, &value);
	dgain = dgain | ((value & 0xc0) >> 6);
	pOx03f10Ctx->curGain.gain[0] = ((float)again / 16.0) * ((float)dgain / 1024.0);

	//get exp_line
	Ox03f10_IsiReadRegIss(handle, 0x3501, &value);
	exp_line = (value & 0xff) << 8;
	Ox03f10_IsiReadRegIss(handle, 0x3502, &value);
	exp_line = exp_line | (value & 0xff);
	pOx03f10Ctx->curIntTime.intTime[0] = exp_line * pOx03f10Ctx->oneLineDCGExpTime;


	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

static RESULT Ox03f10_IsiOpenIss(IsiSensorHandle_t handle, uint32_t mode)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (!pOx03f10Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pOx03f10Ctx->streaming != BOOL_FALSE)
		return RET_WRONG_STATE;

	pOx03f10Ctx->sensorMode.index = mode;
	IsiSensorMode_t *SensorDefaultMode = NULL;
	for (int i = 0; i < sizeof(pox03f10_mode_info) / sizeof(IsiSensorMode_t); i++) {
		if (pox03f10_mode_info[i].index == pOx03f10Ctx->sensorMode.index) {
			SensorDefaultMode = &(pox03f10_mode_info[i]);
			break;
		}
	}

	if (SensorDefaultMode != NULL) {
		int Status = XST_SUCCESS;


#if 1
		switch (SensorDefaultMode->index) {
			case 0:
				for (int i = 0;
				     i < sizeof(Ox03f10_mipi4lane_1080p_native4dol_init) / sizeof(
					     Ox03f10_mipi4lane_1080p_native4dol_init[0]); i++) {
					if (Ox03f10_mipi4lane_1080p_native4dol_init[i][0] == OX03F10_TABLE_WAIT)
						vTaskDelay(Ox03f10_mipi4lane_1080p_native4dol_init[i][1]);

					else if (Ox03f10_mipi4lane_1080p_native4dol_init[i][0] == OX03F10_TABLE_END)
						break;

					else
						Ox03f10_IsiWriteRegIss(handle, Ox03f10_mipi4lane_1080p_native4dol_init[i][0],
								       Ox03f10_mipi4lane_1080p_native4dol_init[i][1]);
				}
				break;
			case 1:
				for (int i = 0;
				     i < sizeof(Ox03f10_mipi4lane_1080p_native4dol_1280_720_init) / sizeof(
					     Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[0]); i++) {
					if (Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[i][0] == OX03F10_TABLE_WAIT)
						vTaskDelay(Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[i][1]);

					else if (Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[i][0] == OX03F10_TABLE_END)
						break;

					else
						Ox03f10_IsiWriteRegIss(handle, Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[i][0],
								       Ox03f10_mipi4lane_1080p_native4dol_1280_720_init[i][1]);
				}
				break;
			case 2:
				for (int i = 0;
				     i < sizeof(Ox03f10_mipi4lane_1080p_native4dol_640_480_init) / sizeof(
					     Ox03f10_mipi4lane_1080p_native4dol_640_480_init[0]); i++) {
					if (Ox03f10_mipi4lane_1080p_native4dol_640_480_init[i][0] == OX03F10_TABLE_WAIT)
						vTaskDelay(Ox03f10_mipi4lane_1080p_native4dol_640_480_init[i][1]);

					else if (Ox03f10_mipi4lane_1080p_native4dol_640_480_init[i][0] == OX03F10_TABLE_END)
						break;

					else
						Ox03f10_IsiWriteRegIss(handle, Ox03f10_mipi4lane_1080p_native4dol_640_480_init[i][0],
								       Ox03f10_mipi4lane_1080p_native4dol_640_480_init[i][1]);

				}
				break;
			default:
				xil_printf("%s:not support sensor mode %d\n", __func__, pOx03f10Ctx->sensorMode.index);
				osFree(pOx03f10Ctx);
				return RET_NOTSUPP;
				break;
		}
#endif
		memcpy(&(pOx03f10Ctx->sensorMode), SensorDefaultMode, sizeof(IsiSensorMode_t));
	} else {
		xil_printf("%s: Invalid SensorDefaultMode\n", __func__);
		return (RET_NULL_POINTER);
	}

	switch (pOx03f10Ctx->sensorMode.index) {
		case 0:
		case 1:
		case 2:
			pOx03f10Ctx->oneLineDCGExpTime = 0.0000242;
			pOx03f10Ctx->oneLineSPDExpTime = 0.0000081;
			pOx03f10Ctx->oneLineVSExpTime = 0.0000081;
			pOx03f10Ctx->frameLengthLines = 0x330;
			pOx03f10Ctx->curFrameLengthLines = pOx03f10Ctx->frameLengthLines;
			pOx03f10Ctx->maxDCGIntegrationLine = 801;
			pOx03f10Ctx->minDCGIntegrationLine = 2;
			pOx03f10Ctx->maxSPDIntegrationLine = 803;
			pOx03f10Ctx->minSPDIntegrationLine = 2;
			pOx03f10Ctx->maxVSIntegrationLine = 52;
			pOx03f10Ctx->minVSIntegrationLine = 1;
			pOx03f10Ctx->aecMaxGain = 240;
			pOx03f10Ctx->aecMinGain = 1.0;
			pOx03f10Ctx->aGain.min = 1.0;
			pOx03f10Ctx->aGain.max = 15.5;
			pOx03f10Ctx->aGain.step = (1.0f / 16.0f);
			pOx03f10Ctx->dGain.min = 1.0;
			pOx03f10Ctx->dGain.max = 15.99;
			pOx03f10Ctx->dGain.step = (1.0f / 1024.0f);
			break;
		default:
			xil_printf("%s:not support sensor mode %d\n", __func__, pOx03f10Ctx->sensorMode.index);
			return RET_NOTSUPP;
			break;
	}

	pOx03f10Ctx->maxFps = pOx03f10Ctx->sensorMode.fps;
	pOx03f10Ctx->minFps = 0.25 * ISI_FPS_QUANTIZE;
	pOx03f10Ctx->currFps = pOx03f10Ctx->maxFps;
	pOx03f10Ctx->sensorWb.rGain = 1.8;
	pOx03f10Ctx->sensorWb.gbGain = 1.0;
	pOx03f10Ctx->sensorWb.grGain = 1.0;
	pOx03f10Ctx->sensorWb.bGain = 1.65;

	xil_printf("%s: Ox03f10 System-Reset executed\n", __func__);
	osSleep(100);

	result = Ox03f10_AecSetModeParameters(handle, pOx03f10Ctx);
	if (result != RET_SUCCESS) {
		xil_printf("%s: SetupOutputWindow failed.\n", __func__);
		return (result);
	}

	pOx03f10Ctx->configured = BOOL_TRUE;


	xil_printf("%s: (exit)\n", __func__);
	return 0;
}

static RESULT Ox03f10_IsiCloseIss(IsiSensorHandle_t handle)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx03f10Ctx == NULL)
		return (RET_WRONG_HANDLE);

	(void)Ox03f10_IsiSetStreamingIss(pOx03f10Ctx, BOOL_FALSE);

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_IsiReleaseIss(IsiSensorHandle_t handle)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx03f10Ctx == NULL)
		return (RET_WRONG_HANDLE);

	MEMSET(pOx03f10Ctx, 0, sizeof(Ox03f10_Context_t));
	osFree(pOx03f10Ctx);
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_IsiCheckConnectionIss(IsiSensorHandle_t handle)
{
	RESULT result = RET_SUCCESS;

	uint32_t sensor_id = 0;
	uint32_t correct_id = 0x580346;

	xil_printf("%s (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	result = Ox03f10_IsiGetRevisionIss(handle, &sensor_id);
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

static RESULT Ox03f10_IsiGetRevisionIss(IsiSensorHandle_t handle, uint32_t *pValue)
{
	RESULT result = RET_SUCCESS;
	uint16_t reg_val;
	uint32_t sensor_id;

	xil_printf("%s (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	reg_val = 0;
	result = Ox03f10_IsiReadRegIss(handle, 0x300a, &reg_val);
	sensor_id = (reg_val & 0xff) << 16;

	reg_val = 0;
	result |= Ox03f10_IsiReadRegIss(handle, 0x300b, &reg_val);
	sensor_id |= ((reg_val & 0xff) << 8);

	reg_val = 0;
	result |= Ox03f10_IsiReadRegIss(handle, 0x300c, &reg_val);
	sensor_id |= (reg_val & 0xff);

	*pValue = sensor_id;
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_IsiSetStreamingIss(IsiSensorHandle_t handle, bool_t on)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	if (pOx03f10Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	result = Ox03f10_IsiWriteRegIss(handle, 0x0100, on);
	if (result != RET_SUCCESS) {
		xil_printf("%s: set sensor streaming error! \n", __func__);
		return (RET_FAILURE);
	}

	pOx03f10Ctx->streaming = on;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_pIsiGetAeBaseInfoIss(IsiSensorHandle_t handle, IsiAeBaseInfo_t *pAeBaseInfo)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pOx03f10Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pAeBaseInfo == NULL) {
		xil_printf("%s: NULL pointer received!!\n");
		return (RET_NULL_POINTER);
	}

	//get time limit and total gain limit
	pAeBaseInfo->longGain.min = pOx03f10Ctx->aecMinGain;
	pAeBaseInfo->longGain.max = pOx03f10Ctx->aecMaxGain;
	pAeBaseInfo->longIntTime.min = pOx03f10Ctx->aecMinIntegrationTime;
	pAeBaseInfo->longIntTime.max = pOx03f10Ctx->aecMaxIntegrationTime;

	//get again/dgain info
	pAeBaseInfo->aLongGain = pOx03f10Ctx->aGain;
	pAeBaseInfo->dLongGain = pOx03f10Ctx->dGain;

	//get current intTime and gain
	pAeBaseInfo->curIntTime = pOx03f10Ctx->curIntTime;
	pAeBaseInfo->curGain = pOx03f10Ctx->curGain;

	pAeBaseInfo->aecGainStep = pOx03f10Ctx->aecGainIncrement;
	pAeBaseInfo->aecIntTimeStep = pOx03f10Ctx->aecIntegrationTimeIncrement;
	pAeBaseInfo->nativeMode = pOx03f10Ctx->sensorMode.nativeMode;
	pAeBaseInfo->nativeHdrRatio[0] = 14.4;
	pAeBaseInfo->nativeHdrRatio[1] = 29.76;
	pAeBaseInfo->nativeHdrRatio[2] = 4.48;
	pAeBaseInfo->conversionGainDCG = 7.2;

	xil_printf("%s: (enter)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiSetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	uint32_t again = 0;

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	if (pSensorAGain->gain[ISI_LINEAR_PARAS] < pOx03f10Ctx->aGain.min) {
		xil_printf("%s: invalid too small again parameter!\n", __func__);
		pSensorAGain->gain[ISI_LINEAR_PARAS] = pOx03f10Ctx->aGain.min;
	}

	if (pSensorAGain->gain[ISI_LINEAR_PARAS] > pOx03f10Ctx->aGain.max) {
		xil_printf("%s: invalid too big again parameter!\n", __func__);
		pSensorAGain->gain[ISI_LINEAR_PARAS] = pOx03f10Ctx->aGain.max;
	}

	//set HCG analog gain, base exp, conversion gain = HCG/LCG = 7.2
	again = (uint32_t)(pSensorAGain->gain[ISI_LINEAR_PARAS] * 16);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3508, (again & 0xf0) >> 4);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3509, (again & 0x0f) << 4);
	pOx03f10Ctx->curAgain.gain[0] = (float)again / 16.0f;

	//set LCG analog gain
	again = (uint32_t)(pSensorAGain->gain[ISI_LINEAR_PARAS] / 2 * 16);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3588, (again & 0xf0) >> 4);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3589, (again & 0x0f) << 4);
	pOx03f10Ctx->curAgain.gain[1] = (float)again / 16.0f;

	//set S analog gain, base exp
	again = (uint32_t)(pSensorAGain->gain[ISI_LINEAR_PARAS] * 16);
	result = Ox03f10_IsiWriteRegIss(handle, 0x3548, (again & 0xf0) >> 4);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3549, (again & 0x0f) << 4);
	pOx03f10Ctx->curAgain.gain[2] = (float)again / 16.0f;

	//set VS analog gain
	again = (uint32_t)(pSensorAGain->gain[ISI_LINEAR_PARAS] / 4 * 16);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35c8, (again & 0xf0) >> 4);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35c9, (again & 0x0f) << 4);
	pOx03f10Ctx->curAgain.gain[3] = (float)again / 16.0f;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiSetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	uint32_t dgain = 0;

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] < pOx03f10Ctx->dGain.min) {
		xil_printf("%s: invalid too small dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx03f10Ctx->dGain.min;
	}

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] > pOx03f10Ctx->dGain.max) {
		xil_printf("%s: invalid too big dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx03f10Ctx->dGain.max;
	}

	//set HCG digital gain, base exp
	dgain = (uint32_t)(pSensorDGain->gain[ISI_LINEAR_PARAS] * 1024);
	result = Ox03f10_IsiWriteRegIss(handle, 0x350a, (dgain >> 10) & 0x0f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x350b, (dgain >> 2) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x350c, (dgain & 0x03) << 6);
	pOx03f10Ctx->curDgain.gain[0] = (float)dgain / 1024.0f;
	pOx03f10Ctx->curGain.gain[0] = pOx03f10Ctx->curAgain.gain[0] * pOx03f10Ctx->curDgain.gain[0];

	//set LCG digital gain
	dgain = (uint32_t)(pSensorDGain->gain[ISI_LINEAR_PARAS] * 1024);
	result = Ox03f10_IsiWriteRegIss(handle, 0x358a, (dgain >> 10) & 0x0f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x358b, (dgain >> 2) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x358c, (dgain & 0x03) << 6);
	pOx03f10Ctx->curDgain.gain[1] = (float)dgain / 1024.0f;
	pOx03f10Ctx->curGain.gain[1] = pOx03f10Ctx->curAgain.gain[1] * pOx03f10Ctx->curDgain.gain[1];

	//set S digital gain
	dgain = (uint32_t)(pSensorDGain->gain[ISI_LINEAR_PARAS] * 1024);
	result = Ox03f10_IsiWriteRegIss(handle, 0x354a, (dgain >> 10) & 0x0f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x354b, (dgain >> 2) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x354c, (dgain & 0x03) << 6);
	pOx03f10Ctx->curDgain.gain[2] = (float)dgain / 1024.0f;
	pOx03f10Ctx->curGain.gain[2] = pOx03f10Ctx->curAgain.gain[2] * pOx03f10Ctx->curDgain.gain[2];

	//set VS digital gain
	dgain = (uint32_t)(pSensorDGain->gain[ISI_LINEAR_PARAS] * 1024);
	result = Ox03f10_IsiWriteRegIss(handle, 0x35ca, (dgain >> 10) & 0x0f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35cb, (dgain >> 2) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35cc, (dgain & 0x03) << 6);
	pOx03f10Ctx->curDgain.gain[3] = (float)dgain / 1024.0f;
	pOx03f10Ctx->curGain.gain[3] = pOx03f10Ctx->curAgain.gain[3] * pOx03f10Ctx->curDgain.gain[3];

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx03f10Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorAGain == NULL)
		return (RET_NULL_POINTER);

	*pSensorAGain = pOx03f10Ctx->curAgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx03f10Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorDGain == NULL)
		return (RET_NULL_POINTER);

	*pSensorDGain = pOx03f10Ctx->curDgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiSetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;

	if (!pOx03f10Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pOx03f10Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {
		result = Ox03f10_SetIntTime(handle, pSensorIntTime->intTime[ISI_LINEAR_PARAS]);
		if (result != RET_SUCCESS) {
			xil_printf("%s: set sensor IntTime[ISI_LINEAR_PARAS] error!\n", __func__);
			return RET_FAILURE;
		}

	} else {
		xil_printf("%s:not support this ExpoFrmType.\n", __func__);
		return RET_NOTSUPP;
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_SetIntTime(IsiSensorHandle_t handle, float newIntegrationTime)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	uint32_t exp_line = 0;
	float sIntegrationTime = 0, vsIntegrationTime = 0;

	if (!pOx03f10Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	//set DCG(HCG & LCG) exp
	exp_line = newIntegrationTime / pOx03f10Ctx->oneLineDCGExpTime;
	exp_line = MIN(pOx03f10Ctx->maxDCGIntegrationLine, MAX(pOx03f10Ctx->minDCGIntegrationLine,
		       exp_line));
	xil_printf("%s: set DCG exp_line = 0x%04x\n", __func__, exp_line);
	result = Ox03f10_IsiWriteRegIss(handle, 0x3501, (exp_line >> 8) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3502, (exp_line & 0xff));
	pOx03f10Ctx->curIntTime.intTime[0] = exp_line * pOx03f10Ctx->oneLineDCGExpTime;
	pOx03f10Ctx->curIntTime.intTime[1] = exp_line * pOx03f10Ctx->oneLineDCGExpTime;

	//set SPD exp
	sIntegrationTime = newIntegrationTime;
	exp_line = sIntegrationTime / pOx03f10Ctx->oneLineSPDExpTime;
	exp_line = MIN(pOx03f10Ctx->maxSPDIntegrationLine, MAX(pOx03f10Ctx->minSPDIntegrationLine,
		       exp_line));
	xil_printf("%s: set SPD exp_line = 0x%04x\n", __func__, exp_line);
	result = Ox03f10_IsiWriteRegIss(handle, 0x3541, (exp_line >> 8) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x3542, (exp_line & 0xff));
	pOx03f10Ctx->curIntTime.intTime[2] = exp_line * pOx03f10Ctx->oneLineSPDExpTime;

	//set VS exp
	vsIntegrationTime = newIntegrationTime / 4 / 16;
	exp_line = vsIntegrationTime / pOx03f10Ctx->oneLineVSExpTime;
	exp_line = MIN(pOx03f10Ctx->maxVSIntegrationLine, MAX(pOx03f10Ctx->minVSIntegrationLine, exp_line));
	xil_printf("%s: set VS exp_line = 0x%04x\n", __func__, exp_line);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35c1, (exp_line >> 8) & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x35c2, (exp_line & 0xff));
	pOx03f10Ctx->curIntTime.intTime[3] = exp_line * pOx03f10Ctx->oneLineVSExpTime;
	pOx03f10Ctx->maxDCGIntegrationLine = pOx03f10Ctx->curFrameLengthLines - 13 - exp_line;

	xil_printf("%s: set IntTime = %f\n", __func__, pOx03f10Ctx->curIntTime.intTime[0]);
	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (!pOx03f10Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (!pSensorIntTime)
		return (RET_NULL_POINTER);


	if (pOx03f10Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE)
		*pSensorIntTime = pOx03f10Ctx->curIntTime;


	else {
		xil_printf("%s:not support this ExpoFrmType.\n", __func__);
		return RET_NOTSUPP;
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetFpsIss(IsiSensorHandle_t handle, uint32_t *pFps)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx03f10Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	*pFps = pOx03f10Ctx->currFps;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiSetFpsIss(IsiSensorHandle_t handle, uint32_t fps)
{
	RESULT result = RET_SUCCESS;
	int32_t NewVts = 0;
	int32_t vs_exp = 0, data = 0;

	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (fps > pOx03f10Ctx->maxFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx03f10Ctx->maxFps, pOx03f10Ctx->minFps, pOx03f10Ctx->maxFps);
		fps = pOx03f10Ctx->maxFps;
	}
	if (fps < pOx03f10Ctx->minFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx03f10Ctx->minFps, pOx03f10Ctx->minFps, pOx03f10Ctx->maxFps);
		fps = pOx03f10Ctx->minFps;
	}

	NewVts = pOx03f10Ctx->frameLengthLines * pOx03f10Ctx->sensorMode.fps / fps;
	result = Ox03f10_IsiWriteRegIss(handle, 0x380e, NewVts >> 8);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x380f, NewVts & 0xff);
	pOx03f10Ctx->currFps = fps;
	pOx03f10Ctx->curFrameLengthLines = NewVts;
	result |= Ox03f10_IsiReadRegIss(handle, 0x35c1, &data);
	vs_exp = (data & 0xff) << 8;
	result |= Ox03f10_IsiReadRegIss(handle, 0x35c2, &data);
	vs_exp |= (data & 0xff);
	pOx03f10Ctx->maxDCGIntegrationLine = pOx03f10Ctx->curFrameLengthLines - 13 - vs_exp;
	pOx03f10Ctx->maxSPDIntegrationLine = pOx03f10Ctx->curFrameLengthLines - 13;
	pOx03f10Ctx->aecMaxIntegrationTime = pOx03f10Ctx->maxDCGIntegrationLine *
					     pOx03f10Ctx->oneLineDCGExpTime;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetIspStatusIss(IsiSensorHandle_t handle, IsiIspStatus_t *pIspStatus)
{
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_WRONG_HANDLE;
	xil_printf("%s: (enter)\n", __func__);

	pIspStatus->useSensorAE = false;
	pIspStatus->useSensorBLC = true;
	pIspStatus->useSensorAWB = true;

	xil_printf("%s: (exit)\n", __func__);
	return RET_SUCCESS;
}

RESULT Ox03f10_IsiSetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t tpg)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	if (pOx03f10Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (tpg.enable == 0) {
		result = Ox03f10_IsiWriteRegIss(handle, 0x5004, 0x1e);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5005, 0x1e);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5006, 0x1e);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5007, 0x1e);

		result = Ox03f10_IsiWriteRegIss(handle, 0x5240, 0x00);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5440, 0x00);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5640, 0x00);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5840, 0x00);
	} else {
		result = Ox03f10_IsiWriteRegIss(handle, 0x5004, 0x1f);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5005, 0x1f);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5006, 0x1f);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5007, 0x1f);

		result = Ox03f10_IsiWriteRegIss(handle, 0x5240, 0x01);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5440, 0x01);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5640, 0x01);
		result = Ox03f10_IsiWriteRegIss(handle, 0x5840, 0x01);
	}

	pOx03f10Ctx->testPattern = tpg.enable;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t *pTpg)
{
	RESULT result = RET_SUCCESS;
	uint16_t hcgValue = 0, lcgValue = 0, spdValue = 0, vsValue = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL || pTpg == NULL)
		return RET_NULL_POINTER;

	if (pOx03f10Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (!Ox03f10_IsiReadRegIss(handle, 0x5240, &hcgValue) & !Ox03f10_IsiReadRegIss(handle, 0x5440,
			&lcgValue)
	    & !Ox03f10_IsiReadRegIss(handle, 0x5640, &spdValue) & !Ox03f10_IsiReadRegIss(handle, 0x5840,
			    &vsValue)) {
		pTpg->enable = (((hcgValue & 0x01) != 0) && ((lcgValue & 0x01) != 0) && ((spdValue & 0x01) != 0)
				&& ((vsValue & 0x01) != 0)) ? 1 : 0;
		if (pTpg->enable)
			pTpg->pattern = (0xff & hcgValue);
		pOx03f10Ctx->testPattern = pTpg->enable;
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_IsiSetWBIss(IsiSensorHandle_t handle, IsiSensorWb_t *pWb)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	uint32_t b_gain, gb_gain, gr_gain, r_gain;
	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pWb == NULL)
		return RET_NULL_POINTER;

	b_gain = (uint32_t)(pWb->bGain * 1024);
	gb_gain = (uint32_t)(pWb->gbGain * 1024);
	gr_gain = (uint32_t)(pWb->grGain * 1024);
	r_gain = (uint32_t)(pWb->rGain * 1024);

	//set HCG channel awb gain
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5280, (b_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5281, b_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5282, (gb_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5283, gb_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5284, (gr_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5285, gr_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5286, (r_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5287, r_gain & 0xff);

	//set LCG channel awb gain
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5480, (b_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5481, b_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5482, (gb_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5483, gb_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5484, (gr_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5485, gr_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5486, (r_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5487, r_gain & 0xff);

	//set S channel awb gain
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5680, (b_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5681, b_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5682, (gb_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5683, gb_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5684, (gr_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5685, gr_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5686, (r_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5687, r_gain & 0xff);

	//set VS channel awb gain
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5880, (b_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5881, b_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5882, (gb_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5883, gb_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5884, (gr_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5885, gr_gain & 0xff);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5886, (r_gain >> 8) & 0x7f);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x5887, r_gain & 0xff);

	memcpy(&pOx03f10Ctx->sensorWb, pWb, sizeof(IsiSensorWb_t));

	xil_printf("%s: (exit)\n", __func__);
	return result;
}

static RESULT Ox03f10_IsiGetWBIss(IsiSensorHandle_t handle, IsiSensorWb_t *pWb)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pWb == NULL)
		return RET_NULL_POINTER;

	memcpy(pWb, &pOx03f10Ctx->sensorWb, sizeof(IsiSensorWb_t));

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

static RESULT Ox03f10_IsiSetBlcIss(IsiSensorHandle_t handle, IsiSensorBlc_t *pBlc)
{
	RESULT result = RET_SUCCESS;
	uint16_t blcGain = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pBlc == NULL)
		return RET_NULL_POINTER;

	blcGain = pBlc->red;
	//set HCG blc
	result = Ox03f10_IsiWriteRegIss(handle, 0x4026, (blcGain >> 8) & 0x03);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x4027, blcGain & 0xff);
	//set LCG blc
	result |= Ox03f10_IsiWriteRegIss(handle, 0x4028, (blcGain >> 8) & 0x03);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x4029, blcGain & 0xff);
	//set S blc
	result |= Ox03f10_IsiWriteRegIss(handle, 0x402a, (blcGain >> 8) & 0x03);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x402b, blcGain & 0xff);
	//set VS blc
	result |= Ox03f10_IsiWriteRegIss(handle, 0x402c, (blcGain >> 8) & 0x03);
	result |= Ox03f10_IsiWriteRegIss(handle, 0x402d, blcGain & 0xff);

	pOx03f10Ctx->sensorBlc = *pBlc;

	xil_printf("%s: (exit)\n", __func__);
	return result;
}

static RESULT Ox03f10_IsiGetBlcIss(IsiSensorHandle_t handle, IsiSensorBlc_t *pBlc)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pBlc == NULL)
		return RET_NULL_POINTER;

	*pBlc = pOx03f10Ctx->sensorBlc;

	xil_printf("%s: (exit)\n", __func__);
	return result;
}


static RESULT Ox03f10_IsiGetExpandCurveIss(IsiSensorHandle_t handle,
		IsiSensorCompandCurve_t *pCurve)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox03f10_Context_t *pOx03f10Ctx = (Ox03f10_Context_t *) handle;
	if (pOx03f10Ctx == NULL)
		return RET_NULL_POINTER;

	//suppose isp pipeline is 24bit, expand_px left shift 12bit
	uint8_t expand_px[64] = {22, 20, 12, 20, 20, 20, 20, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18,
				 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 12
				};//dx_exp[i], index number, x[i]=x[i-1]+2^dx_exp[i]
	memcpy(pCurve->compandPx, expand_px, sizeof(expand_px));

	pCurve->compandXData[0] = 0;//x value in expand curve
	pCurve->compandYData[0] = 0;//y value in expand curve
	for (int i = 1; i < 65; i++) {
		if (pCurve->compandXData[i - 1] == 0 && pCurve->compandPx[i - 1] > 0)
			pCurve->compandXData[i] = pCurve->compandXData[i - 1] + ((1 << pCurve->compandPx[i - 1]) - 1);

		else if (pCurve->compandXData[i - 1] > 0 && pCurve->compandPx[i - 1] > 0)
			pCurve->compandXData[i] = pCurve->compandXData[i - 1] + (1 << pCurve->compandPx[i - 1]);

		else if (pCurve->compandXData[i - 1] > 0 && pCurve->compandPx[i - 1] == 0)
			pCurve->compandXData[i] = pCurve->compandXData[i - 1];

		else {
			xil_printf("%s: invalid paramter\n", __func__);
			return RET_INVALID_PARM;
		}
	}

	uint16_t expandXValue[34] = {0, 1023, 1279, 1279, 1535, 1791, 2047, 2303, 2431, 2559, 2687, 2815, 2943, 3007, 3071,
				     3135, 3199, 3263, 3327, 3391, 3455, 3519, 3583, 3647, 3711, 3775, 3839, 3903, 3967, 3999,
				     4031, 4063, 4095, 4095
				    };
	uint32_t expandYValue[34] = {0, 1023, 2047, 2047, 4095, 8191, 12287, 16383, 20479, 24575, 32767, 40959, 49151, 57343,
				     65535, 81919, 98303, 114687, 131071, 163839, 196607, 262143, 393215, 524287, 786431, 1048575,
				     1572863, 2097151, 3145727, 4194303, 8388607, 12582911, 16777215, 16777215
				    };
	float slope[34] = {0};
	for (int i = 0; i < 34; i++)
		slope[i] = (expandYValue[i + 1] - expandYValue[i]) / (expandXValue[i + 1] - expandXValue[i]);

	for (int i = 1; i < 65; i++) {
		for (int j = 1; j < 34; j++) {
			if (pCurve->compandXData[i] >= expandXValue[j - 1] && pCurve->compandXData[i] < expandXValue[j])
				pCurve->compandYData[i] = expandYValue[j - 1] + (pCurve->compandXData[i] - expandXValue[j - 1]) *
							  slope[j - 1];
		}
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox03f10_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
	RESULT result = RET_SUCCESS;
	static const char SensorName[16] = "Ox03f10";
	xil_printf("%s (enter)\n", __func__);

	if (pIsiSensor != NULL) {
		pIsiSensor->pszName = SensorName;
		pIsiSensor->pIsiCreateIss = Ox03f10_IsiCreateIss;
		pIsiSensor->pIsiOpenIss = Ox03f10_IsiOpenIss;
		pIsiSensor->pIsiCloseIss = Ox03f10_IsiCloseIss;
		pIsiSensor->pIsiReleaseIss = Ox03f10_IsiReleaseIss;
		pIsiSensor->pIsiReadRegIss = Ox03f10_IsiReadRegIss;
		pIsiSensor->pIsiWriteRegIss = Ox03f10_IsiWriteRegIss;
		pIsiSensor->pIsiGetModeIss = Ox03f10_IsiGetModeIss;
		pIsiSensor->pIsiEnumModeIss = Ox03f10_IsiEnumModeIss;
		pIsiSensor->pIsiGetCapsIss = Ox03f10_IsiGetCapsIss;
		pIsiSensor->pIsiCheckConnectionIss = Ox03f10_IsiCheckConnectionIss;
		pIsiSensor->pIsiGetRevisionIss = Ox03f10_IsiGetRevisionIss;
		pIsiSensor->pIsiSetStreamingIss = Ox03f10_IsiSetStreamingIss;

		/* AEC */
		pIsiSensor->pIsiGetAeBaseInfoIss = Ox03f10_pIsiGetAeBaseInfoIss;
		pIsiSensor->pIsiGetAGainIss = Ox03f10_IsiGetAGainIss;
		pIsiSensor->pIsiSetAGainIss = Ox03f10_IsiSetAGainIss;
		pIsiSensor->pIsiGetDGainIss = Ox03f10_IsiGetDGainIss;
		pIsiSensor->pIsiSetDGainIss = Ox03f10_IsiSetDGainIss;
		pIsiSensor->pIsiGetIntTimeIss = Ox03f10_IsiGetIntTimeIss;
		pIsiSensor->pIsiSetIntTimeIss = Ox03f10_IsiSetIntTimeIss;
		pIsiSensor->pIsiGetFpsIss = Ox03f10_IsiGetFpsIss;
		pIsiSensor->pIsiSetFpsIss = Ox03f10_IsiSetFpsIss;

		/* SENSOR ISP */
		pIsiSensor->pIsiGetIspStatusIss = Ox03f10_IsiGetIspStatusIss;
		pIsiSensor->pIsiSetWBIss = Ox03f10_IsiSetWBIss;
		pIsiSensor->pIsiGetWBIss = Ox03f10_IsiGetWBIss;
		pIsiSensor->pIsiSetBlcIss = Ox03f10_IsiSetBlcIss;
		pIsiSensor->pIsiGetBlcIss = Ox03f10_IsiGetBlcIss;

		/* SENSOE OTHER FUNC*/
		pIsiSensor->pIsiSetTpgIss = Ox03f10_IsiSetTpgIss;
		pIsiSensor->pIsiGetTpgIss = Ox03f10_IsiGetTpgIss;
		pIsiSensor->pIsiGetExpandCurveIss = Ox03f10_IsiGetExpandCurveIss;

		/* AF */
		pIsiSensor->pIsiFocusCreateIss = NULL;
		pIsiSensor->pIsiFocusReleaseIss = NULL;
		pIsiSensor->pIsiFocusGetCalibrateIss = NULL;
		pIsiSensor->pIsiFocusSetIss = NULL;
		pIsiSensor->pIsiFocusGetIss = NULL;

		pIsiSensor->pIsiSetIRLightExpIss = NULL;
		pIsiSensor->pIsiGetIRLightExpIss = NULL;
	} else
		result = RET_NULL_POINTER;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t Ox03f10_IsiCamDrvConfig = {
	.cameraDriverID = 0x580346,
	.pIsiGetSensorIss = Ox03f10_IsiGetSensorIss,
};
