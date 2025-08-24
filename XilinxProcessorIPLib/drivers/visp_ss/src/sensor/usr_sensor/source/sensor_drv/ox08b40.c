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
#include "ox08b40_priv.h"

#define X8B_DEBUG_LOG 1

#define X8B_MIN_AGAIN_STEP    ( 1.0f/16.0f )
#define X8B_MIN_DGAIN_STEP    ( 1.0f/1024.0f )  /**< min gain step size used by GUI (hardware min = 1/16; 1/16..32/16 depending on actual gain ) */
#define X8B_MIN_WBGAIN_STEP   ( 1.0f/1024.0f )

#define X8B_EXP_INDEX_HCG 0
#define X8B_EXP_INDEX_LCG 1
#define X8B_EXP_INDEX_SPD 2
#define X8B_EXP_INDEX_VS  3

#define X8B_NATIVE_RATIO_HCG_LCG 0
#define X8B_NATIVE_RATIO_LCG_SPD 1
#define X8B_NATIVE_RATIO_SPD_VS  2

#define X8B_DCG_CONVERSION_RATIO       (3.3f)
#define X8B_DCG_SPD_SENSITIVITY_RATIO  10
/*****************************************************************************
 *Sensor Info
*****************************************************************************/

static IsiSensorMode_t pox08b40_mode_info[] = {
	{
		.index = 0,
		.size = {
			.boundsWidth = 3840,
			.boundsHeight = 2160,
			.top = 0,
			.left = 0,
			.width = 3840,
			.height = 2160,
		},
		.aeInfo = {
			.intTimeDelayFrame = 2,
			.gainDelayFrame = 2,
		},
		.fps = 9 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_HDR_NATIVE,
		.nativeMode = ISI_SENSOR_NATIVE_DCG_SPD_VS,
		.bitWidth = 12,
		.compress.enable = 1,
		.compress.xBit = 24,
		.compress.yBit = 12,
		.bayerPattern = ISI_BPAT_BGGR,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
	},
	{
		.index = 1,
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
		.fps = 15 * ISI_FPS_QUANTIZE,
		.hdrMode = ISI_SENSOR_MODE_HDR_NATIVE,
		.nativeMode = ISI_SENSOR_NATIVE_DCG_SPD_VS,
		.bitWidth = 12,
		.compress.enable = 1,
		.compress.xBit = 24,
		.compress.yBit = 12,
		.bayerPattern = ISI_BPAT_BGGR,
		.afMode = ISI_SENSOR_AF_MODE_NOTSUPP,
		.dataType = ISI_MODE_BAYER,
		.mipiLane = ISI_MIPI_4LANES,
	},
};

static RESULT Ox08b40_IsiReadRegIss(IsiSensorHandle_t handle, const uint16_t addr, uint16_t *pValue)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;
	u8 slave_addr = (g_fmc_single.sensor_array[pOx08b40Ctx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx08b40Ctx->sensorDevId]->readIIC(pOx08b40Ctx->i2cId,
		 slave_addr, addr, 0x2, pValue, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal read sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	xil_printf("%s (exit) result = %d\n", __func__, result);
	return (result);
}

static RESULT Ox08b40_IsiWriteRegIss(IsiSensorHandle_t handle, const uint16_t addr,
				     const uint16_t value)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	u8 slave_addr = (g_fmc_single.sensor_array[pOx08b40Ctx->sensorDevId]->sensor_alias_addr) >> 1;
	result = g_fmc_single.accessiic_array[pOx08b40Ctx->sensorDevId]->writeIIC(pOx08b40Ctx->i2cId,
		 slave_addr, addr, 0x2, value, 1);
	if (result != RET_SUCCESS) {
		xil_printf("%s: hal write sensor register error!\n", __func__);
		return (RET_FAILURE);
	}

	xil_printf("%s (exit) result = %d\n", __func__, result);
	return (result);
}

static RESULT Ox08b40_IsiGetModeIss(IsiSensorHandle_t handle, IsiSensorMode_t *pMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return (RET_WRONG_HANDLE);
	if (pMode == NULL)
		return (RET_WRONG_HANDLE);

	memcpy(pMode, &(pOx08b40Ctx->sensorMode), sizeof(pOx08b40Ctx->sensorMode));

	xil_printf("%s (exit)\n", __func__);
	return (RET_SUCCESS);
}

static RESULT Ox08b40_IsiEnumModeIss(IsiSensorHandle_t handle, IsiSensorEnumMode_t *pEnumMode)
{
	xil_printf("%s (enter)\n", __func__);
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	if (pEnumMode->index >= (sizeof(pox08b40_mode_info) / sizeof(pox08b40_mode_info[0])))
		return RET_OUTOFRANGE;

	for (uint32_t i = 0; i < (sizeof(pox08b40_mode_info) / sizeof(pox08b40_mode_info[0])); i++) {
		if (pox08b40_mode_info[i].index == pEnumMode->index) {
			memcpy(&pEnumMode->mode, &pox08b40_mode_info[i], sizeof(IsiSensorMode_t));
			xil_printf("%s (exit)\n", __func__);
			return RET_SUCCESS;
		}
	}

	return RET_NOTSUPP;
}

static RESULT Ox08b40_IsiGetCapsIss(IsiSensorHandle_t handle, IsiCaps_t *pCaps)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (pOx08b40Ctx == NULL)
		return (RET_WRONG_HANDLE);

	if (pCaps == NULL)
		return (RET_NULL_POINTER);

	pCaps->bitWidth = pOx08b40Ctx->sensorMode.bitWidth;
	pCaps->mode = ISI_MODE_BAYER;
	pCaps->bayerPattern = pOx08b40Ctx->sensorMode.bayerPattern;
	pCaps->resolution.width = pOx08b40Ctx->sensorMode.size.width;
	pCaps->resolution.height = pOx08b40Ctx->sensorMode.size.height;
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

static RESULT Ox08b40_IsiCreateIss(IsiSensorInstanceConfig_t *pConfig, IsiSensorHandle_t *pHandle)
{
	RESULT result = RET_SUCCESS;
	uint32_t desId = 0, pipeId = 0;
	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) osMalloc(sizeof(Ox08b40_Context_t));
	if (!pOx08b40Ctx) {
		xil_printf("%s: Can't allocate ox08b40 context\n", __func__);
		return (RET_OUTOFMEM);
	}

	MEMSET(pOx08b40Ctx, 0, sizeof(Ox08b40_Context_t));

	pOx08b40Ctx->isiCtx.pSensor = pConfig->pSensor;
	pOx08b40Ctx->groupHold = BOOL_FALSE;
	pOx08b40Ctx->oldGain = 0;
	pOx08b40Ctx->oldIntegrationTime = 0;
	pOx08b40Ctx->configured = BOOL_FALSE;
	pOx08b40Ctx->streaming = BOOL_FALSE;
	pOx08b40Ctx->testPattern = BOOL_FALSE;
	pOx08b40Ctx->isAfpsRun = BOOL_FALSE;
	pOx08b40Ctx->sensorMode.index = 0;
	pOx08b40Ctx->i2cId	= 0;
	pOx08b40Ctx->sensorDevId = pConfig->cameraDevId;

	uint8_t busId = (uint8_t)pOx08b40Ctx->i2cId;
	pipeId = pOx08b40Ctx->sensorDevId;

	*pHandle = (IsiSensorHandle_t) pOx08b40Ctx;

	if (pipeId >= IN_PIPE_LAST) {
		xil_printf("%s: sensor device ID %d is mot support!\n", __func__, pipeId);
		return RET_UNSUPPORT_ID;
	}
	desId = MAPPING_INPIPE_TO_DES_ID(pipeId);

	init_iic_access(pOx08b40Ctx->i2cId, pipeId);
	init_des(desId);
	init_sensor(pipeId, desId);

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_AecSetModeParameters(IsiSensorHandle_t handle)
{
	RESULT result = RET_SUCCESS;
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	xil_printf("%s: (enter)\n", __func__);
	uint32_t dgain = 0;
	uint16_t value = 0, exp_line = 0, again = 0;

	//initial default info
	result = Ox08b40_IsiReadRegIss(handle, 0x3501, &value);//DCG/LCG exp time
	exp_line = (value & 0x00ff) << 8;
	result |= Ox08b40_IsiReadRegIss(handle, 0x3502, &value);
	exp_line |= value & 0x00ff;

	pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG] = exp_line * pOx08b40Ctx->oneLineDCGExpTime;

	value = 0;
	result |= Ox08b40_IsiReadRegIss(handle, 0x3508, &value);//DCG again
	again = (value & 0x0f) << 4;
	result |= Ox08b40_IsiReadRegIss(handle, 0x3509, &value);
	again |= (value & 0xf0) >> 4;

	pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG] = (float32_t)again * X8B_MIN_AGAIN_STEP;

	value = 0;
	result |= Ox08b40_IsiReadRegIss(handle, 0x350a, &value);//DCG
	dgain = (value & 0x0f) << 10;
	result |= Ox08b40_IsiReadRegIss(handle, 0x350b, &value);
	dgain = dgain | ((value & 0xff) << 2);
	result |= Ox08b40_IsiReadRegIss(handle, 0x350c, &value);
	dgain = dgain | ((value & 0xc0) >> 6);
	pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG] = (float32_t)dgain * X8B_MIN_DGAIN_STEP;
	pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG] *
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG];

	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {
	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {
		//LCG
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG] =
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG];

		value = 0;
		again = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3588, &value);//LCG again
		again = (value & 0x0f) << 4;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3589, &value);
		again |= (value & 0xf0) >> 4;

		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)again * X8B_MIN_AGAIN_STEP;

		value = 0;
		dgain = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x358a, &value);//LCG dgain
		dgain = (value & 0x0f) << 10;
		result |= Ox08b40_IsiReadRegIss(handle, 0x358b, &value);
		dgain = dgain | ((value & 0xff) << 2);
		result |= Ox08b40_IsiReadRegIss(handle, 0x358c, &value);
		dgain = dgain | ((value & 0xc0) >> 6);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)dgain * X8B_MIN_DGAIN_STEP;
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG];
		//SPD
		value = 0;
		exp_line = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3541, &value);//SPD exp time
		exp_line = (value & 0x00ff) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3542, &value);
		exp_line |= value & 0x00ff;
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] = exp_line * pOx08b40Ctx->oneLineSPDExpTime;

		value = 0;
		again = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3548, &value);//SPD again
		again = (value & 0x0f) << 4;
		result |= Ox08b40_IsiReadRegIss(handle, 0x3549, &value);
		again |= (value & 0xf0) >> 4;

		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_SPD] = (float32_t)again * X8B_MIN_AGAIN_STEP;

		value = 0;
		dgain = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x354a, &value);//SPD dgain
		dgain = (value & 0x0f) << 10;
		result |= Ox08b40_IsiReadRegIss(handle, 0x354b, &value);
		dgain = dgain | ((value & 0xff) << 2);
		result |= Ox08b40_IsiReadRegIss(handle, 0x354c, &value);
		dgain = dgain | ((value & 0xc0) >> 6);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD] = (float32_t)dgain * X8B_MIN_DGAIN_STEP;
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_SPD] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD];
		//VS
		value = 0;
		exp_line = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35c1, &value);//VS exp time
		exp_line = (value & 0x00ff) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35c2, &value);
		exp_line |= value & 0x00ff;
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] = (exp_line + 0.5) *
			pOx08b40Ctx->oneLineVSExpTime;

		value = 0;
		again = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35c8, &value);//VS again
		again = (value & 0x0f) << 4;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35c9, &value);
		again |= (value & 0xf0) >> 4;
		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_VS] = (float32_t)again * X8B_MIN_AGAIN_STEP;

		value = 0;
		dgain = 0;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35ca, &value);//VS dgain
		dgain = (value & 0x0f) << 10;
		result |= Ox08b40_IsiReadRegIss(handle, 0x35cb, &value);
		dgain = dgain | ((value & 0xff) << 2);
		result |= Ox08b40_IsiReadRegIss(handle, 0x35cc, &value);
		dgain = dgain | ((value & 0xc0) >> 6);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS] = (float32_t)dgain * X8B_MIN_DGAIN_STEP;
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_VS] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS];

		//ratio
		pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_HCG_LCG] = X8B_DCG_CONVERSION_RATIO *
			pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG] / pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG];
		pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_LCG_SPD] = X8B_DCG_SPD_SENSITIVITY_RATIO *
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG] * pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG] /
			(pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] * pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD]);
		pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_SPD_VS] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD]
			* pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD] /
			(X8B_DCG_SPD_SENSITIVITY_RATIO * pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] *
			 pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS]);

	} else {
		xil_printf("%s: X8B not support sensor mode %d !!\n", __func__, pOx08b40Ctx->sensorMode.hdrMode);
		return RET_NOTSUPP;
	}

	//initial AEC info
	pOx08b40Ctx->aecMinIntegrationTime = pOx08b40Ctx->oneLineDCGExpTime *
					     pOx08b40Ctx->minDCGIntegrationLine;
	pOx08b40Ctx->aecMaxIntegrationTime = pOx08b40Ctx->oneLineDCGExpTime *
					     pOx08b40Ctx->maxDCGIntegrationLine;
	pOx08b40Ctx->aecIntegrationTimeIncrement = pOx08b40Ctx->oneLineDCGExpTime;

	pOx08b40Ctx->aecMaxGain = pOx08b40Ctx->aGain.max * pOx08b40Ctx->dGain.max;
	pOx08b40Ctx->aecMinGain = pOx08b40Ctx->aGain.min * pOx08b40Ctx->dGain.min;
	pOx08b40Ctx->aecGainIncrement = X8B_MIN_DGAIN_STEP;

#if X8B_DEBUG_LOG
	for (int i = 0; i < X8B_EXP_INDEX_VS + 1; i++) {
		printf("%s: currInt[%d]=%f, currAgain[%d]=%f, currDgain[%d]=%f, currTotalGain[%d]=%f\n", __func__,
		       i, pOx08b40Ctx->curIntTime.intTime[i],
		       i, pOx08b40Ctx->curAgain.gain[i],
		       i, pOx08b40Ctx->curDgain.gain[i],
		       i, pOx08b40Ctx->curGain.gain[i]);
	}
	printf("%s: AEC Range: expTime[%f, %f, step(%f)], gain[%f,%f, step(%f)]\n", __func__,
	       pOx08b40Ctx->aecMinIntegrationTime, pOx08b40Ctx->aecMaxIntegrationTime,
	       pOx08b40Ctx->aecIntegrationTimeIncrement,
	       pOx08b40Ctx->aecMinGain, pOx08b40Ctx->aecMaxGain, pOx08b40Ctx->aecGainIncrement);

#endif

	xil_printf("%s: result %d (exit)\n", __func__, result);

	return (result);
}

static RESULT Ox08b40_InitialModeParameters(IsiSensorHandle_t handle)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	uint16_t value = 0;
	uint32_t b_gain, gb_gain, gr_gain, r_gain;
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;

	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;

	//set HCG channel awb gain
	result = Ox08b40_IsiReadRegIss(handle, 0x5280, &value);
	b_gain = (value & 0x7f) << 8;
	result |= Ox08b40_IsiReadRegIss(handle, 0x5281, &value);
	b_gain |= (value & 0xff);
	result |= Ox08b40_IsiReadRegIss(handle, 0x5282, &value);
	gb_gain = (value & 0x7f) << 8;
	result |= Ox08b40_IsiReadRegIss(handle, 0x5283, &value);
	gb_gain |= (value & 0xff);
	result |= Ox08b40_IsiReadRegIss(handle, 0x5284, &value);
	gr_gain = (value & 0x7f) << 8;
	result |= Ox08b40_IsiReadRegIss(handle, 0x5285, &value);
	gr_gain |= (value & 0xff);
	result |= Ox08b40_IsiReadRegIss(handle, 0x5286, &value);
	r_gain = (value & 0x7f) << 8;
	result |= Ox08b40_IsiReadRegIss(handle, 0x5287, &value);
	r_gain |= (value & 0xff);

	//updata wb setting
	pOx08b40Ctx->sensorWb.bGain = b_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.gbGain = gb_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.grGain = gr_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.rGain = r_gain * X8B_MIN_WBGAIN_STEP;


	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {
	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {
		//awb not support individual exposure wb configure
		//set LCG channel awb gain
		result |= Ox08b40_IsiReadRegIss(handle, 0x5480, &value);
		b_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5481, &value);
		b_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5482, &value);
		gb_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5483, &value);
		gb_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5484, &value);
		gr_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5485, &value);
		gr_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5486, &value);
		r_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5487, &value);
		r_gain |= (value & 0xff);

		//set S channel awb gain
		result |= Ox08b40_IsiReadRegIss(handle, 0x5680, &value);
		b_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5681, &value);
		b_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5682, &value);
		gb_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5683, &value);
		gb_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5684, &value);
		gr_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5685, &value);
		gr_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5686, &value);
		r_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5687, &value);
		r_gain |= (value & 0xff);

		//set VS channel awb gain
		result |= Ox08b40_IsiReadRegIss(handle, 0x5880, &value);
		b_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5881, &value);
		b_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5882, &value);
		gb_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5883, &value);
		gb_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5884, &value);
		gr_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5885, &value);
		gr_gain |= (value & 0xff);
		result |= Ox08b40_IsiReadRegIss(handle, 0x5886, &value);
		r_gain = (value & 0x7f) << 8;
		result |= Ox08b40_IsiReadRegIss(handle, 0x5887, &value);
		r_gain |= (value & 0xff);
	} else {
		xil_printf("%s: X8B not support sensor mode %d !!\n", __func__, pOx08b40Ctx->sensorMode.hdrMode);
		return RET_NOTSUPP;
	}

	xil_printf("%s: result %d (exit)\n", __func__, result);
	return (result);
}

static RESULT Ox08b40_IsiOpenIss(IsiSensorHandle_t handle, uint32_t mode)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if (!pOx08b40Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pOx08b40Ctx->streaming != BOOL_FALSE)
		return RET_WRONG_STATE;

	pOx08b40Ctx->sensorMode.index = mode;
	IsiSensorMode_t *SensorDefaultMode = NULL;
	for (int i = 0; i < sizeof(pox08b40_mode_info) / sizeof(IsiSensorMode_t); i++) {
		if (pox08b40_mode_info[i].index == pOx08b40Ctx->sensorMode.index) {
			SensorDefaultMode = &(pox08b40_mode_info[i]);
			break;
		}
	}

	if (SensorDefaultMode != NULL) {
		int Status = XST_SUCCESS;


#if 1
		switch (SensorDefaultMode->index) {
			case 0:
				for (int i = 0;
				     i < sizeof(Ox08b40_mipi4lane_8M_native4dol_init) / sizeof(Ox08b40_mipi4lane_8M_native4dol_init[0]);
				     i++) {
					if (Ox08b40_mipi4lane_8M_native4dol_init[i][0] == TABLE_WAIT)
						vTaskDelay(Ox08b40_mipi4lane_8M_native4dol_init[i][1]);

					else if (Ox08b40_mipi4lane_8M_native4dol_init[i][0] == TABLE_END)
						break;

					else
						Ox08b40_IsiWriteRegIss(handle, Ox08b40_mipi4lane_8M_native4dol_init[i][0],
								       Ox08b40_mipi4lane_8M_native4dol_init[i][1]);
				}
				break;
			case 1:
				for (int i = 0;
				     i < sizeof(Ox08b40_mipi4lane_binning1080p_native4dol_init) / sizeof(
					     Ox08b40_mipi4lane_binning1080p_native4dol_init[0]); i++) {
					if (Ox08b40_mipi4lane_binning1080p_native4dol_init[i][0] == TABLE_WAIT)
						vTaskDelay(Ox08b40_mipi4lane_binning1080p_native4dol_init[i][1]);

					else if (Ox08b40_mipi4lane_binning1080p_native4dol_init[i][0] == TABLE_END)
						break;

					else
						Ox08b40_IsiWriteRegIss(handle, Ox08b40_mipi4lane_binning1080p_native4dol_init[i][0],
								       Ox08b40_mipi4lane_binning1080p_native4dol_init[i][1]);
				}
				break;
			default:
				xil_printf("%s:not support sensor mode %d\n", __func__, pOx08b40Ctx->sensorMode.index);
				osFree(pOx08b40Ctx);
				return RET_NOTSUPP;
				break;
		}
#endif
		memcpy(&(pOx08b40Ctx->sensorMode), SensorDefaultMode, sizeof(IsiSensorMode_t));
	} else {
		xil_printf("%s: Invalid SensorDefaultMode\n", __func__);
		return (RET_NULL_POINTER);
	}

	switch (pOx08b40Ctx->sensorMode.index) {
		case 0:
			pOx08b40Ctx->oneLineDCGExpTime = 0.0000448;
			pOx08b40Ctx->oneLineSPDExpTime = 0.0000216;
			pOx08b40Ctx->oneLineVSExpTime = 0.0000214;
			pOx08b40Ctx->frameLengthLines = 0x520;
			pOx08b40Ctx->curFrameLengthLines = pOx08b40Ctx->frameLengthLines;
			pOx08b40Ctx->maxDCGIntegrationLine = pOx08b40Ctx->frameLengthLines - 13 - 1;
			pOx08b40Ctx->minDCGIntegrationLine = 2;
			pOx08b40Ctx->maxSPDIntegrationLine = pOx08b40Ctx->frameLengthLines - 13;
			pOx08b40Ctx->minSPDIntegrationLine = 2;
			pOx08b40Ctx->maxVSIntegrationLine = 35;
			pOx08b40Ctx->minVSIntegrationLine = 0;
			pOx08b40Ctx->aGain.min = 1.0;
			pOx08b40Ctx->aGain.max = 15.5;
			pOx08b40Ctx->aGain.step = X8B_MIN_AGAIN_STEP;
			pOx08b40Ctx->dGain.min = 1.0;
			pOx08b40Ctx->dGain.max = 15.99;
			pOx08b40Ctx->dGain.step = X8B_MIN_DGAIN_STEP;
		case 1:
			pOx08b40Ctx->oneLineDCGExpTime = 0.00009664;
			pOx08b40Ctx->oneLineSPDExpTime = 0.00004592;
			pOx08b40Ctx->oneLineVSExpTime = 0.00004856;
			pOx08b40Ctx->frameLengthLines = 0x46a;
			pOx08b40Ctx->curFrameLengthLines = pOx08b40Ctx->frameLengthLines;
			pOx08b40Ctx->maxDCGIntegrationLine = pOx08b40Ctx->frameLengthLines - 13 - 1;
			pOx08b40Ctx->minDCGIntegrationLine = 2;
			pOx08b40Ctx->maxSPDIntegrationLine = pOx08b40Ctx->frameLengthLines - 13;
			pOx08b40Ctx->minSPDIntegrationLine = 2;
			pOx08b40Ctx->maxVSIntegrationLine = 35;
			pOx08b40Ctx->minVSIntegrationLine = 0;
			pOx08b40Ctx->aGain.min = 1.0;
			pOx08b40Ctx->aGain.max = 15.5;
			pOx08b40Ctx->aGain.step = X8B_MIN_AGAIN_STEP;
			pOx08b40Ctx->dGain.min = 1.0;
			pOx08b40Ctx->dGain.max = 15.99;
			pOx08b40Ctx->dGain.step = X8B_MIN_DGAIN_STEP;
			break;
		default:
			xil_printf("%s:not support sensor mode %d\n", __func__, pOx08b40Ctx->sensorMode.index);
			return RET_NOTSUPP;
			break;
	}

	pOx08b40Ctx->maxFps = pOx08b40Ctx->sensorMode.fps;
	pOx08b40Ctx->minFps = 1 * ISI_FPS_QUANTIZE;
	pOx08b40Ctx->currFps = pOx08b40Ctx->maxFps;

	xil_printf("%s: Ox08b40 System-Reset executed\n", __func__);
	osSleep(100);

	result = Ox08b40_AecSetModeParameters(handle);
	if (result != RET_SUCCESS) {
		xil_printf("%s: Ox08b40_AecSetModeParameters failed.\n", __func__);
		return (result);
	}
	result = Ox08b40_InitialModeParameters(handle);
	if (result != RET_SUCCESS) {
		xil_printf("%s: Ox08b40_InitialModeParameters failed!\n", __func__);
		return (result);
	}
	pOx08b40Ctx->configured = BOOL_TRUE;


	xil_printf("%s: (exit)\n", __func__);
	return 0;
}

static RESULT Ox08b40_IsiCloseIss(IsiSensorHandle_t handle)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx08b40Ctx == NULL)
		return (RET_WRONG_HANDLE);

	(void)Ox08b40_IsiSetStreamingIss(pOx08b40Ctx, BOOL_FALSE);

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_IsiReleaseIss(IsiSensorHandle_t handle)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	if (pOx08b40Ctx == NULL)
		return (RET_WRONG_HANDLE);

	MEMSET(pOx08b40Ctx, 0, sizeof(Ox08b40_Context_t));
	osFree(pOx08b40Ctx);
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_IsiCheckConnectionIss(IsiSensorHandle_t handle)
{
	RESULT result = RET_SUCCESS;

	uint32_t sensor_id = 0;
	uint32_t correct_id = 0x580841;

	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	result = Ox08b40_IsiGetRevisionIss(handle, &sensor_id);
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

static RESULT Ox08b40_IsiGetRevisionIss(IsiSensorHandle_t handle, uint32_t *pValue)
{
	RESULT result = RET_SUCCESS;
	uint16_t reg_val;
	uint32_t sensor_id;

	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	reg_val = 0;
	result = Ox08b40_IsiReadRegIss(handle, 0x300a, &reg_val);
	sensor_id = (reg_val & 0xff) << 16;

	reg_val = 0;
	result |= Ox08b40_IsiReadRegIss(handle, 0x300b, &reg_val);
	sensor_id |= ((reg_val & 0xff) << 8);

	reg_val = 0;
	result |= Ox08b40_IsiReadRegIss(handle, 0x300c, &reg_val);
	sensor_id |= (reg_val & 0xff);

	*pValue = sensor_id;
	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_IsiSetStreamingIss(IsiSensorHandle_t handle, bool_t on)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	if (pOx08b40Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	result = Ox08b40_IsiWriteRegIss(handle, 0x0100, on);
	if (result != RET_SUCCESS) {
		xil_printf("%s: set sensor streaming error! \n", __func__);
		return (RET_FAILURE);
	}

	pOx08b40Ctx->streaming = on;

	xil_printf("%s (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_pIsiGetAeBaseInfoIss(IsiSensorHandle_t handle, IsiAeBaseInfo_t *pAeBaseInfo)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pOx08b40Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pAeBaseInfo == NULL) {
		xil_printf("%s: NULL pointer received!!\n");
		return (RET_NULL_POINTER);
	}
	//current value
	pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_HCG] =
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG];
	pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG];
	//range
	pAeBaseInfo->aLongGain.max = pOx08b40Ctx->aGain.max;
	pAeBaseInfo->aLongGain.min = pOx08b40Ctx->aGain.min;
	pAeBaseInfo->aLongGain.step = pOx08b40Ctx->aGain.step;
	pAeBaseInfo->dLongGain.max = pOx08b40Ctx->dGain.max;
	pAeBaseInfo->dLongGain.min = pOx08b40Ctx->dGain.min;
	pAeBaseInfo->dLongGain.step = pOx08b40Ctx->dGain.step;

	pAeBaseInfo->longGain.max = pOx08b40Ctx->aecMaxGain;
	pAeBaseInfo->longGain.min = pOx08b40Ctx->aecMinGain;
	pAeBaseInfo->aecGainStep = pOx08b40Ctx->aecGainIncrement;
	pAeBaseInfo->longIntTime.max = pOx08b40Ctx->aecMaxIntegrationTime;
	pAeBaseInfo->longIntTime.min = pOx08b40Ctx->aecMinIntegrationTime;
	pAeBaseInfo->aecIntTimeStep = pOx08b40Ctx->aecIntegrationTimeIncrement;

	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {
		pAeBaseInfo->aecCurIntTime = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG];
		pAeBaseInfo->aecCurGain = pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG];
	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {

		pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_LCG] =
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG];
		pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_SPD] =
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD];
		pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_VS] =
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS];
		pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG];
		pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD];
		pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS];

		pAeBaseInfo->nativeMode = pOx08b40Ctx->sensorMode.nativeMode;
		pAeBaseInfo->conversionGainDCG = X8B_DCG_CONVERSION_RATIO;// HCG/LCG
		pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_HCG_LCG] =
			pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_HCG_LCG];
		pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_LCG_SPD] =
			pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_LCG_SPD];
		pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_SPD_VS] =
			pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_SPD_VS];

	} else {
		xil_printf("%s: X8B not support sensor mode %d !!\n", __func__, pOx08b40Ctx->sensorMode.hdrMode);
		return RET_NOTSUPP;
	}

#if X8B_DEBUG_LOG
	printf("%s: current exptime[0]=%f,exptime[1]=%f,exptime[2]=%f,exptime[3]=%f\n", __func__,
	       pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_HCG],
	       pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_LCG],
	       pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_SPD],
	       pAeBaseInfo->curIntTime.intTime[X8B_EXP_INDEX_VS]);

	printf("%s: current curGain[0]=%f,curGain[1]=%f,curGain[2]=%f,curGain[3]=%f\n", __func__,
	       pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_HCG],
	       pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_LCG],
	       pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_SPD],
	       pAeBaseInfo->curGain.gain[X8B_EXP_INDEX_VS]);

	printf("%s: current ratio[0]=%f,ratio[1]=%f,ratio[2]=%f\n", __func__,
	       pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_HCG_LCG],
	       pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_LCG_SPD],
	       pAeBaseInfo->nativeHdrRatio[X8B_NATIVE_RATIO_SPD_VS]);
#endif

	xil_printf("%s: (enter)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiSetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	float32_t realHCGAgain = 0.0; //realLCGAgain = 0.0, realSPDAgain = 0.0, realVSAgain = 0.0;
	uint16_t againHCG = 0, againLCG = 0, againSPD = 0, againVS = 0;

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	//set HCG analog gain
	realHCGAgain = pSensorAGain->gain[ISI_LINEAR_PARAS];
	realHCGAgain = MAX(pOx08b40Ctx->aGain.min, MIN(realHCGAgain, pOx08b40Ctx->aGain.max));
	againHCG = (uint16_t)(realHCGAgain * (1 / X8B_MIN_AGAIN_STEP) + 0.5);
	pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG] = (float32_t)(againHCG * X8B_MIN_AGAIN_STEP);

	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {
		result = Ox08b40_IsiWriteRegIss(handle, 0x3508, (againHCG & 0xf0) >> 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3509, (againHCG & 0x0f) << 4);
	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {
		//follow ratio
		againLCG = againHCG;
		againSPD = againHCG;
		againVS = againHCG;

		//    	//set LCG analog gain
		//	realLCGAgain = pSensorAGain->gain[ISI_QUAD_EXP_S_PARAS];
		//	realLCGAgain = MAX(pOx08b40Ctx->aGain.min, MIN(realLCGAgain, pOx08b40Ctx->aGain.max));
		// againLCG = (uint16_t)(realLCGAgain * (1/X8B_MIN_AGAIN_STEP) + 0.5);
		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)(againLCG * X8B_MIN_AGAIN_STEP);
		//		//set SPD analog gain
		//	realLCGAgain = pSensorAGain->gain[ISI_QUAD_EXP_VS_PARAS];
		//	realLCGAgain = MAX(pOx08b40Ctx->aGain.min, MIN(realLCGAgain, pOx08b40Ctx->aGain.max));
		// againLCG = (uint16_t)(realLCGAgain * (1/X8B_MIN_AGAIN_STEP) + 0.5);
		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)(againLCG * X8B_MIN_AGAIN_STEP);
		//		//set VS analog gain
		//	realLCGAgain = pSensorAGain->gain[ISI_QUAD_EXP_VVS_PARAS];
		//	realLCGAgain = MAX(pOx08b40Ctx->aGain.min, MIN(realLCGAgain, pOx08b40Ctx->aGain.max));
		// againLCG = (uint16_t)(realLCGAgain * (1/X8B_MIN_AGAIN_STEP) + 0.5);
		pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)(againLCG * X8B_MIN_AGAIN_STEP);

		result = Ox08b40_IsiWriteRegIss(handle, 0x3508, (againHCG & 0xf0) >> 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3509, (againHCG & 0x0f) << 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3588, (againLCG & 0xf0) >> 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3589, (againLCG & 0x0f) << 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3548, (againSPD & 0xf0) >> 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3549, (againSPD & 0x0f) << 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35c8, (againVS & 0xf0) >> 4);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35c9, (againVS & 0x0f) << 4);

	} else {
		xil_printf("%s:not support this ExpoFrmType.\n", __func__);
		return RET_NOTSUPP;
	}

#if X8B_DEBUG_LOG
	xil_printf("%s: current curAgain[0]=%f,curAgain[1]=%f,curAgain[2]=%f,curAgain[3]=%f\n", __func__,
		   pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG],
		   pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG],
		   pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_SPD],
		   pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_VS]);
#endif

	xil_printf("%s: result %d (exit)\n", __func__, result);
	return (result);
}

RESULT Ox08b40_IsiSetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	float realHCGDgain = 0.0;// realLCGDgain = 0.0, realSPDDgain = 0.0, realVSDgain = 0.0;
	uint32_t dgainHCG = 0, dgainLCG = 0, dgainSPD = 0, dgainVS = 0;

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] < pOx08b40Ctx->dGain.min) {
		xil_printf("%s: invalid too small dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx08b40Ctx->dGain.min;
	}

	if (pSensorDGain->gain[ISI_LINEAR_PARAS] > pOx08b40Ctx->dGain.max) {
		xil_printf("%s: invalid too big dgain parameter!\n", __func__);
		pSensorDGain->gain[ISI_LINEAR_PARAS] = pOx08b40Ctx->dGain.max;
	}
	//set HCG digital gain
	realHCGDgain = pSensorDGain->gain[ISI_LINEAR_PARAS];
	realHCGDgain = MAX(pOx08b40Ctx->dGain.min, MIN(realHCGDgain, pOx08b40Ctx->dGain.max));
	dgainHCG = (uint32_t)(realHCGDgain * (1 / X8B_MIN_DGAIN_STEP) + 0.5);
	pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG] = (float32_t)(dgainHCG * X8B_MIN_DGAIN_STEP);

	pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG] *
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG];

	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {

		result = Ox08b40_IsiWriteRegIss(handle, 0x350a, (dgainHCG >> 10) & 0x0f);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x350b, (dgainHCG >> 2) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x350c, (dgainHCG & 0x3) << 6);

	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {
		//follow ratio
		dgainLCG = dgainHCG;
		dgainSPD = dgainHCG;
		dgainVS = dgainHCG;

		//set LCG digital gain
		//	realLCGDgain = pSensorDGain->gain[ISI_QUAD_EXP_S_PARAS];
		//	realLCGDgain = MAX(pOx08b40Ctx->dGain.min, MIN(realLCGDgain, pOx08b40Ctx->dGain.max));
		//	dgainLCG = (uint32_t)(realLCGDgain * (1/X8B_MIN_DGAIN_STEP) + 0.5);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG] = (float32_t)(dgainLCG * X8B_MIN_DGAIN_STEP);
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG];
		//
		//		//set SPD digital gain
		//	realSPDDgain = pSensorDGain->gain[ISI_QUAD_EXP_VS_PARAS];
		//	realSPDDgain = MAX(pOx08b40Ctx->dGain.min, MIN(realSPDDgain, pOx08b40Ctx->dGain.max));
		//	dgainSPD = (uint32_t)(realSPDDgain * (1/X8B_MIN_DGAIN_STEP) + 0.5);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD] = (float32_t)(dgainSPD * X8B_MIN_DGAIN_STEP);
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_SPD] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD];
		//
		//		//set VS digital gain
		//	realVSDgain = pSensorDGain->gain[ISI_QUAD_EXP_VVS_PARAS];
		//	realVSDgain = MAX(pOx08b40Ctx->dGain.min, MIN(realVSDgain, pOx08b40Ctx->dGain.max));
		//	dgainVS = (uint32_t)(realVSDgain * (1/X8B_MIN_DGAIN_STEP) + 0.5);
		pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS] = (float32_t)(dgainVS * X8B_MIN_DGAIN_STEP);
		pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_VS] *
			pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS];

		result = Ox08b40_IsiWriteRegIss(handle, 0x350a, (dgainHCG >> 10) & 0x0f);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x350b, (dgainHCG >> 2) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x350c, (dgainHCG & 0x3) << 6);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x358a, (dgainLCG >> 10) & 0x0f);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x358b, (dgainLCG >> 2) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x358c, (dgainLCG & 0x3) << 6);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x354a, (dgainSPD >> 10) & 0x0f);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x354b, (dgainSPD >> 2) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x354c, (dgainSPD & 0x3) << 6);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35ca, (dgainVS >> 10) & 0x0f);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35cb, (dgainVS >> 2) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35cc, (dgainVS & 0x3) << 6);

	} else {
		xil_printf("%s:not support this ExpoFrmType.\n", __func__);
		return RET_NOTSUPP;
	}
#if X8B_DEBUG_LOG
	xil_printf("%s: current curDgain[0]=%f,curDgain[1]=%f,curDgain[2]=%f,curDgain[3]=%f\n", __func__,
		   pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG],
		   pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG],
		   pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD],
		   pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS]);

	xil_printf("%s: current curGain[0]=%f,curGain[1]=%f,curGain[2]=%f,curGain[3]=%f\n", __func__,
		   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_HCG],
		   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG],
		   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD],
		   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS]);
#endif

	xil_printf("%s: result %d (exit)\n", __func__, result);
	return (result);
}

RESULT Ox08b40_IsiGetAGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorAGain)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx08b40Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorAGain == NULL)
		return (RET_NULL_POINTER);
	pSensorAGain->gain[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_HCG];
	pSensorAGain->gain[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_LCG];
	pSensorAGain->gain[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_SPD];
	pSensorAGain->gain[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curAgain.gain[X8B_EXP_INDEX_VS];

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiGetDGainIss(IsiSensorHandle_t handle, IsiSensorGain_t *pSensorDGain)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx08b40Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (pSensorDGain == NULL)
		return (RET_NULL_POINTER);

	pSensorDGain->gain[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_HCG];
	pSensorDGain->gain[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_LCG];
	pSensorDGain->gain[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_SPD];
	pSensorDGain->gain[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curDgain.gain[X8B_EXP_INDEX_VS];

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiSetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	uint16_t expDCGLine = 0, expSPDLine = 0, expVSLine = 0;

	if (!pOx08b40Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}
	//set DCG(HCG & LCG) exp
	expDCGLine = (uint16_t)(pSensorIntTime->intTime[ISI_LINEAR_PARAS] / pOx08b40Ctx->oneLineDCGExpTime +
				0.5);
	expDCGLine = MIN(pOx08b40Ctx->maxDCGIntegrationLine, MAX(pOx08b40Ctx->minDCGIntegrationLine,
			 expDCGLine));
	pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG] = expDCGLine * pOx08b40Ctx->oneLineDCGExpTime;
	pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG] =
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG];

	if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_LINEAR) {

		result = Ox08b40_IsiWriteRegIss(handle, 0x3501, (expDCGLine >> 8) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3502, (expDCGLine & 0xff));

	} else if (pOx08b40Ctx->sensorMode.hdrMode == ISI_SENSOR_MODE_HDR_NATIVE) {

		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] = X8B_DCG_SPD_SENSITIVITY_RATIO *
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG] * pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG]
			/ pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_LCG_SPD] / pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD];
		expSPDLine = (uint16_t)(pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] /
					pOx08b40Ctx->oneLineSPDExpTime + 0.5);
		expSPDLine = MIN(pOx08b40Ctx->maxSPDIntegrationLine, MAX(pOx08b40Ctx->minSPDIntegrationLine,
				 expSPDLine));
		//update exptime and ratio
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] = expSPDLine * pOx08b40Ctx->oneLineSPDExpTime;
		pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_LCG_SPD] = X8B_DCG_SPD_SENSITIVITY_RATIO *
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG] * pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_LCG]
			/ pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] / pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD];

		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] =
			pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD] * pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD]
			/ (X8B_DCG_SPD_SENSITIVITY_RATIO * pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_SPD_VS] *
			   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS]);
		expVSLine = (uint16_t)(pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] /
				       pOx08b40Ctx->oneLineVSExpTime + 0.5);
		expVSLine = MIN(pOx08b40Ctx->maxVSIntegrationLine, MAX(pOx08b40Ctx->minVSIntegrationLine,
				expVSLine));
		pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] = expVSLine * pOx08b40Ctx->oneLineVSExpTime;
		pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_SPD_VS] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD]
			* pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_SPD]
			/ (X8B_DCG_SPD_SENSITIVITY_RATIO * pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS] *
			   pOx08b40Ctx->curGain.gain[X8B_EXP_INDEX_VS]);

		result = Ox08b40_IsiWriteRegIss(handle, 0x3501, (expDCGLine >> 8) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3502, (expDCGLine & 0xff));
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3541, (expSPDLine >> 8) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x3542, (expSPDLine & 0xff));
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35c1, (expVSLine >> 8) & 0xff);
		result |= Ox08b40_IsiWriteRegIss(handle, 0x35c2, (expVSLine & 0xff));
	} else {
		xil_printf("%s:not support this ExpoFrmType.\n", __func__);
		return RET_NOTSUPP;
	}

#if X8B_DEBUG_LOG
	printf("%s: current exptime[0]=%f,exptime[1]=%f,exptime[2]=%f,exptime[3]=%f\n", __func__,
	       pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG],
	       pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG],
	       pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD],
	       pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS]);

	printf("%s: current ratio[0]=%f,ratio[1]=%f,ratio[2]=%f\n", __func__,
	       pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_HCG_LCG],
	       pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_LCG_SPD],
	       pOx08b40Ctx->hdrRatio[X8B_NATIVE_RATIO_SPD_VS]);
#endif

	xil_printf("%s: result %d (exit)\n", __func__, result);
	return (result);
}


RESULT Ox08b40_IsiGetIntTimeIss(IsiSensorHandle_t handle, IsiSensorIntTime_t *pSensorIntTime)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (!pOx08b40Ctx) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (!pSensorIntTime)
		return (RET_NULL_POINTER);

	pSensorIntTime->intTime[X8B_EXP_INDEX_HCG] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_HCG];
	pSensorIntTime->intTime[X8B_EXP_INDEX_LCG] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_LCG];
	pSensorIntTime->intTime[X8B_EXP_INDEX_SPD] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_SPD];
	pSensorIntTime->intTime[X8B_EXP_INDEX_VS] = pOx08b40Ctx->curIntTime.intTime[X8B_EXP_INDEX_VS];
	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiGetFpsIss(IsiSensorHandle_t handle, uint32_t *pFps)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	if (pOx08b40Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	*pFps = pOx08b40Ctx->currFps;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiSetFpsIss(IsiSensorHandle_t handle, uint32_t fps)
{
	RESULT result = RET_SUCCESS;
	int32_t NewVts = 0;
	uint32_t vs_exp = 0;
	uint8_t value = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL) {
		xil_printf("%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return (RET_WRONG_HANDLE);
	}

	if (fps > pOx08b40Ctx->maxFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx08b40Ctx->maxFps, pOx08b40Ctx->minFps, pOx08b40Ctx->maxFps);
		fps = pOx08b40Ctx->maxFps;
	}
	if (fps < pOx08b40Ctx->minFps) {
		xil_printf("%s: set fps(%d) out of range, correct to %d (%d, %d)\n", __func__, fps,
			   pOx08b40Ctx->minFps, pOx08b40Ctx->minFps, pOx08b40Ctx->maxFps);
		fps = pOx08b40Ctx->minFps;
	}

	NewVts = pOx08b40Ctx->frameLengthLines * pOx08b40Ctx->sensorMode.fps / fps;
	result = Ox08b40_IsiWriteRegIss(handle, 0x380e, NewVts >> 8);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x380f, NewVts & 0xff);
	pOx08b40Ctx->currFps = fps;
	pOx08b40Ctx->curFrameLengthLines = NewVts;
	result = Ox08b40_IsiReadRegIss(handle, 0x35c1, &value);
	vs_exp = (value & 0xff) << 8;
	result = Ox08b40_IsiReadRegIss(handle, 0x38c2, &value);
	vs_exp |= (value & 0xff);
	pOx08b40Ctx->maxDCGIntegrationLine = pOx08b40Ctx->curFrameLengthLines - 13 - vs_exp;
	pOx08b40Ctx->maxSPDIntegrationLine = pOx08b40Ctx->curFrameLengthLines - 13;
	pOx08b40Ctx->aecMaxIntegrationTime = pOx08b40Ctx->maxDCGIntegrationLine *
					     pOx08b40Ctx->oneLineDCGExpTime;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiGetIspStatusIss(IsiSensorHandle_t handle, IsiIspStatus_t *pIspStatus)
{
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;
	xil_printf("%s: (enter)\n", __func__);

	pIspStatus->useSensorAE = false;
	pIspStatus->useSensorBLC = true;
	pIspStatus->useSensorAWB = true;

	xil_printf("%s: (exit)\n", __func__);
	return RET_SUCCESS;
}

RESULT Ox08b40_IsiSetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t tpg)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_NULL_POINTER;

	if (pOx08b40Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (tpg.enable == 0) {
		result = Ox08b40_IsiWriteRegIss(handle, 0x5004, 0x1e);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5005, 0x1e);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5006, 0x1e);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5007, 0x1e);

		result = Ox08b40_IsiWriteRegIss(handle, 0x5240, 0x00);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5440, 0x00);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5640, 0x00);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5840, 0x00);
	} else {
		result = Ox08b40_IsiWriteRegIss(handle, 0x5004, 0x1f);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5005, 0x1f);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5006, 0x1f);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5007, 0x1f);

		result = Ox08b40_IsiWriteRegIss(handle, 0x5240, 0x01);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5440, 0x01);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5640, 0x01);
		result = Ox08b40_IsiWriteRegIss(handle, 0x5840, 0x01);
	}

	pOx08b40Ctx->testPattern = tpg.enable;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT Ox08b40_IsiGetTpgIss(IsiSensorHandle_t handle, IsiSensorTpg_t *pTpg)
{
	RESULT result = RET_SUCCESS;
	uint16_t hcgValue = 0, lcgValue = 0, spdValue = 0, vsValue = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL || pTpg == NULL)
		return RET_NULL_POINTER;

	if (pOx08b40Ctx->configured != BOOL_TRUE)
		return RET_WRONG_STATE;

	if (!Ox08b40_IsiReadRegIss(handle, 0x5240, &hcgValue) & !Ox08b40_IsiReadRegIss(handle, 0x5440,
			&lcgValue)
	    & !Ox08b40_IsiReadRegIss(handle, 0x5640, &spdValue) & !Ox08b40_IsiReadRegIss(handle, 0x5840,
			    &vsValue)) {
		pTpg->enable = (((hcgValue & 0x01) != 0) && ((lcgValue & 0x01) != 0) && ((spdValue & 0x01) != 0)
				&& ((vsValue & 0x01) != 0)) ? 1 : 0;
		if (pTpg->enable)
			pTpg->pattern = (0xff & hcgValue);
		pOx08b40Ctx->testPattern = pTpg->enable;
	}

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_IsiSetWBIss(IsiSensorHandle_t handle, IsiSensorWb_t *pWb)
{
	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	uint32_t b_gain, gb_gain, gr_gain, r_gain;
	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pWb == NULL)
		return RET_NULL_POINTER;

	b_gain = (uint32_t)(pWb->bGain * (1 / X8B_MIN_WBGAIN_STEP) + 0.5);
	gb_gain = (uint32_t)(pWb->gbGain * (1 / X8B_MIN_WBGAIN_STEP) + 0.5);
	gr_gain = (uint32_t)(pWb->grGain * (1 / X8B_MIN_WBGAIN_STEP) + 0.5);
	r_gain = (uint32_t)(pWb->rGain * (1 / X8B_MIN_WBGAIN_STEP) + 0.5);

	//set HCG channel awb gain
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5280, (b_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5281, b_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5282, (gb_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5283, gb_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5284, (gr_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5285, gr_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5286, (r_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5287, r_gain & 0xff);

	//set LCG channel awb gain
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5480, (b_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5481, b_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5482, (gb_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5483, gb_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5484, (gr_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5485, gr_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5486, (r_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5487, r_gain & 0xff);

	//set S channel awb gain
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5680, (b_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5681, b_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5682, (gb_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5683, gb_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5684, (gr_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5685, gr_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5686, (r_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5687, r_gain & 0xff);

	//set VS channel awb gain
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5880, (b_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5881, b_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5882, (gb_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5883, gb_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5884, (gr_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5885, gr_gain & 0xff);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5886, (r_gain >> 8) & 0x7f);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x5887, r_gain & 0xff);

	//updata wb setting
	pOx08b40Ctx->sensorWb.bGain = b_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.gbGain = gb_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.grGain = gr_gain * X8B_MIN_WBGAIN_STEP;
	pOx08b40Ctx->sensorWb.rGain = r_gain * X8B_MIN_WBGAIN_STEP;

	xil_printf("%s: (exit)\n", __func__);

	return result;
}

static RESULT Ox08b40_IsiGetWBIss(IsiSensorHandle_t handle, IsiSensorWb_t *pWb)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pWb == NULL)
		return RET_NULL_POINTER;

	pWb->bGain = pOx08b40Ctx->sensorWb.bGain;
	pWb->gbGain = pOx08b40Ctx->sensorWb.gbGain;
	pWb->grGain = pOx08b40Ctx->sensorWb.grGain;
	pWb->rGain = pOx08b40Ctx->sensorWb.rGain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

static RESULT Ox08b40_IsiSetBlcIss(IsiSensorHandle_t handle, IsiSensorBlc_t *pBlc)
{
	RESULT result = RET_SUCCESS;
	uint16_t blcGain = 0;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pBlc == NULL)
		return RET_NULL_POINTER;

	blcGain = pBlc->red;
	//set HCG blc
	result = Ox08b40_IsiWriteRegIss(handle, 0x4026, (blcGain >> 8) & 0x03);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x4027, blcGain & 0xff);
	//set LCG blc
	result |= Ox08b40_IsiWriteRegIss(handle, 0x4028, (blcGain >> 8) & 0x03);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x4029, blcGain & 0xff);
	//set S blc
	result |= Ox08b40_IsiWriteRegIss(handle, 0x402a, (blcGain >> 8) & 0x03);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x402b, blcGain & 0xff);
	//set VS blc
	result |= Ox08b40_IsiWriteRegIss(handle, 0x402c, (blcGain >> 8) & 0x03);
	result |= Ox08b40_IsiWriteRegIss(handle, 0x402d, blcGain & 0xff);

	pOx08b40Ctx->sensorBlc = *pBlc;

	xil_printf("%s: (exit)\n", __func__);
	return result;
}

static RESULT Ox08b40_IsiGetBlcIss(IsiSensorHandle_t handle, IsiSensorBlc_t *pBlc)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pBlc == NULL)
		return RET_NULL_POINTER;

	*pBlc = pOx08b40Ctx->sensorBlc;

	xil_printf("%s: (exit)\n", __func__);
	return result;
}


static RESULT Ox08b40_IsiGetExpandCurveIss(IsiSensorHandle_t handle,
		IsiSensorCompandCurve_t *pCurve)
{
	RESULT result = RET_SUCCESS;
#if 0
	xil_printf("%s: (enter)\n", __func__);

	Ox08b40_Context_t *pOx08b40Ctx = (Ox08b40_Context_t *) handle;
	if (pOx08b40Ctx == NULL)
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
#endif
	return (result);
}

RESULT Ox08b40_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
	RESULT result = RET_SUCCESS;
	static const char SensorName[16] = "Ox08b40";

	if (pIsiSensor != NULL) {
		pIsiSensor->pszName = SensorName;
		pIsiSensor->pIsiCreateIss = Ox08b40_IsiCreateIss;
		pIsiSensor->pIsiOpenIss = Ox08b40_IsiOpenIss;
		pIsiSensor->pIsiCloseIss = Ox08b40_IsiCloseIss;
		pIsiSensor->pIsiReleaseIss = Ox08b40_IsiReleaseIss;
		pIsiSensor->pIsiReadRegIss = Ox08b40_IsiReadRegIss;
		pIsiSensor->pIsiWriteRegIss = Ox08b40_IsiWriteRegIss;
		pIsiSensor->pIsiGetModeIss = Ox08b40_IsiGetModeIss;
		pIsiSensor->pIsiEnumModeIss = Ox08b40_IsiEnumModeIss;
		pIsiSensor->pIsiGetCapsIss = Ox08b40_IsiGetCapsIss;
		pIsiSensor->pIsiCheckConnectionIss = Ox08b40_IsiCheckConnectionIss;
		pIsiSensor->pIsiGetRevisionIss = Ox08b40_IsiGetRevisionIss;
		pIsiSensor->pIsiSetStreamingIss = Ox08b40_IsiSetStreamingIss;

		/* AEC */
		pIsiSensor->pIsiGetAeBaseInfoIss = Ox08b40_pIsiGetAeBaseInfoIss;
		pIsiSensor->pIsiGetAGainIss = Ox08b40_IsiGetAGainIss;
		pIsiSensor->pIsiSetAGainIss = Ox08b40_IsiSetAGainIss;
		pIsiSensor->pIsiGetDGainIss = Ox08b40_IsiGetDGainIss;
		pIsiSensor->pIsiSetDGainIss = Ox08b40_IsiSetDGainIss;
		pIsiSensor->pIsiGetIntTimeIss = Ox08b40_IsiGetIntTimeIss;
		pIsiSensor->pIsiSetIntTimeIss = Ox08b40_IsiSetIntTimeIss;
		pIsiSensor->pIsiGetFpsIss = Ox08b40_IsiGetFpsIss;
		pIsiSensor->pIsiSetFpsIss = Ox08b40_IsiSetFpsIss;

		/* SENSOR ISP */
		pIsiSensor->pIsiGetIspStatusIss = Ox08b40_IsiGetIspStatusIss;
		pIsiSensor->pIsiSetWBIss = Ox08b40_IsiSetWBIss;
		pIsiSensor->pIsiGetWBIss = Ox08b40_IsiGetWBIss;
		pIsiSensor->pIsiSetBlcIss = Ox08b40_IsiSetBlcIss;
		pIsiSensor->pIsiGetBlcIss = Ox08b40_IsiGetBlcIss;

		/* SENSOE OTHER FUNC*/
		pIsiSensor->pIsiSetTpgIss = Ox08b40_IsiSetTpgIss;
		pIsiSensor->pIsiGetTpgIss = Ox08b40_IsiGetTpgIss;
		pIsiSensor->pIsiGetExpandCurveIss = Ox08b40_IsiGetExpandCurveIss;

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

	return (result);
}

/*****************************************************************************
* each sensor driver need declare this struct for isi load
*****************************************************************************/
IsiCamDrvConfig_t Ox08b40_IsiCamDrvConfig = {
	.cameraDriverID = 0x580841,
	.pIsiGetSensorIss = Ox08b40_IsiGetSensorIss,
};
