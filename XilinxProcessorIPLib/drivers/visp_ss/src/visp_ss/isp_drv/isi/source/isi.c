/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2022> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2022 */

/**
 * @file isi.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <trace.h>
#include <builtins.h>

#include <misc.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"


/*****************************************************************************/
/**
 *          IsiSensorDrvHandleRegisterIss
 *
 * @brief   Sensor deiver handle register.
 *
 * @param   pCamDrvConfig     configuration of the isi camera drv
 * @param   pSensorHandle     produced sensor handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiSensorDrvHandleRegisterIss
(
	IsiCamDrvConfig_t *pCamDrvConfig,
	IsiSensorHandle_t *pSensorHandle
)
{
	RESULT result = RET_SUCCESS;
	IsiSensorInstanceConfig_t instConfig;
	xil_printf("%s: (enter)\n", __func__);

	if (pCamDrvConfig == NULL || pSensorHandle == NULL)
		return RET_NULL_POINTER;
	if (pCamDrvConfig->pIsiGetSensorIss == NULL)
		return RET_WRONG_CONFIG;
	if (*pSensorHandle != NULL) {
		xil_printf("%s: isi sensor handle already exist!\n", __func__);
		return RET_WRONG_STATE;
	}
	MEMSET(&instConfig, 0, sizeof(IsiSensorInstanceConfig_t));
	instConfig.pSensor = osMalloc(sizeof(IsiSensor_t));
	if (instConfig.pSensor == NULL)
		return RET_OUTOFMEM;
	MEMSET(instConfig.pSensor, 0, sizeof(IsiSensor_t));

	result = pCamDrvConfig->pIsiGetSensorIss(instConfig.pSensor);
	if (result != RET_SUCCESS) {
		xil_printf("%s: isi get sensor function pointer error %d!\n", __func__, result);
		osFree(instConfig.pSensor);
		return result;
	}
	instConfig.cameraDevId = pCamDrvConfig->cameraDevId;
	instConfig.instanceID = pCamDrvConfig->instanceId;
	instConfig.cameraDriverID = pCamDrvConfig->cameraDriverID;

	result = instConfig.pSensor->pIsiCreateIss(&instConfig, pSensorHandle);
	if (*pSensorHandle == NULL || result != RET_SUCCESS) {
		xil_printf("%s: isi get sensor handle failed %d!\n", __func__, result);
		osFree(instConfig.pSensor);
		return RET_FAILURE;
	}

	xil_printf("%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          IsiSensorDrvHandleUnRegisterIss
 *
 * @brief   Sensor deiver handle register.
 *
 * @param   handle          isi sensor handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT IsiSensorDrvHandleUnRegisterIss
(
	IsiSensorHandle_t handle
)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);

	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;
	IsiSensor_t *pSensorCb;

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiReleaseIss == NULL)
		return (RET_NOTSUPP);
	pSensorCb = pSensorCtx->pSensor;
	result = pSensorCtx->pSensor->pIsiReleaseIss(pSensorCtx);

	osFree(pSensorCb);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiOpenIss
 *
 * @brief   Open the image sensor considering the given configuration.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiOpenIss
(
	IsiSensorHandle_t handle,
	uint32_t mode
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiOpenIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiOpenIss(pSensorCtx, mode);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiCloseIss
(
	IsiSensorHandle_t handle
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiCloseIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiCloseIss(pSensorCtx);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);


	if (pSensorCtx->pSensor->pIsiGetModeIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetModeIss(pSensorCtx, pMode);

	IsiSensorMode_t *psensor_mode = pMode;
	xil_printf("******************************\n");
	xil_printf("Current Sensor Mode:\n");
	xil_printf("Mode Index: %d \n", psensor_mode->index);
	xil_printf("Resolution: %d * %d\n", psensor_mode->size.width, psensor_mode->size.height);
	xil_printf("fps: %d.%03d \n", (psensor_mode->fps) / ISI_FPS_QUANTIZE,
		   (psensor_mode->fps) % ISI_FPS_QUANTIZE);
	xil_printf("hdr_mode: %d \n", psensor_mode->hdrMode);
	xil_printf("stitching_mode: %d \n", psensor_mode->stitchingMode);
	xil_printf("bit_width: %d \n", psensor_mode->bitWidth);
	xil_printf("bayer_pattern: %d \n", psensor_mode->bayerPattern);
	xil_printf("******************************\n");
	(void)psensor_mode; //avoid compiler's complaint

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}


/*****************************************************************************/
/**
 *          IsiEnumModeIss
 *
 * @brief   query sensor info arry.
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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pEnumMode == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiEnumModeIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiEnumModeIss(pSensorCtx, pEnumMode);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pCaps == NULL)
		return (RET_NULL_POINTER);

	result = pSensorCtx->pSensor->pIsiGetCapsIss(pSensorCtx, pCaps);

	xil_printf("%s: (exit)\n", __func__);

	return (result);

}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pValue == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiReadRegIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiReadRegIss(handle, addr, pValue);

	xil_printf("%s (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiWriteRegIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiWriteRegIss(handle, addr, value);

	xil_printf("%s (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiCheckConnectionIss
 *
 * @brief   Performs the power-up sequence of the camera, if possible.
 *
 * @param   handle      Sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiCheckConnectionIss
(
	IsiSensorHandle_t handle
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiCheckConnectionIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiCheckConnectionIss(pSensorCtx);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiGetRevisionIss
 *
 * @brief   reads the sensor revision register and returns this value
 *
 * @param   handle      pointer to sensor description struct
 * @param   pRevision     pointer to storage value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT IsiGetRevisionIss
(
	IsiSensorHandle_t handle,
	uint32_t *pRevision
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pRevision == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetRevisionIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetRevisionIss(handle, pRevision);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
 * @retval  RET_NOTSUPP
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT IsiSetStreamingIss
(
	IsiSensorHandle_t handle,
	bool_t on
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetStreamingIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetStreamingIss(pSensorCtx, on);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	RESULT result = RET_SUCCESS;
	xil_printf("%s: (enter)\n", __func__);
	float32_t again = 0, dgain = 0;

	if (totalGain < aGain.max) {
		dgain = dGain.min;
		again = totalGain;
		again = ((uint32_t)(again / aGain.step)) * aGain.step;
		again = MAX(aGain.min, MIN(again, aGain.max));
	} else {
		again = aGain.max;
		dgain = totalGain / again;
		dgain = ((uint32_t)(dgain / dGain.step)) * dGain.step;
		dgain = MAX(dGain.min, MIN(dgain, dGain.max));
	}

	*splitAgain = again;
	*splitDgain = dgain;

	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

/*****************************************************************************/
/**
 *     IsiSensorExecuteExposureControl
 *
 *****************************************************************************/
RESULT IsiSensorExecuteExposureControl
(
	IsiSensorHandle_t handle,
	const IsiSensorExpParam_t *pExpParam,
	IsiSensorExpParam_t *pExpResult
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pExpParam == NULL || pExpResult == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiExcuteExpCtrlIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiExcuteExpCtrlIss(pSensorCtx, pExpParam, pExpResult);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *     IsiExpDecomposeControl
 *
 *****************************************************************************/
RESULT IsiExpDecomposeControl
(
	IsiSensorHandle_t handle,
	const IsiExpDecomposeParam_t *pDecParam,
	IsiExpDecomposeResult_t *pDecResult
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pDecParam == NULL || pDecResult == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiExpDecomposeCtrlIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiExpDecomposeCtrlIss(pSensorCtx, pDecParam, pDecResult);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pAeBaseInfo == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetAeBaseInfoIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetAeBaseInfoIss(pSensorCtx, pAeBaseInfo);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *          IsiGetAGainIss
 *
 *****************************************************************************/
RESULT IsiGetAGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorAGain
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorAGain == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetAGainIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetAGainIss(pSensorCtx, pSensorAGain);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *          IsiGetDGainIss
 *
 *****************************************************************************/
RESULT IsiGetDGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorDGain
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorDGain == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetDGainIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetDGainIss(pSensorCtx, pSensorDGain);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *          IsiSetAGainIss
 *
 *****************************************************************************/
RESULT IsiSetAGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorAGain
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorAGain == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetAGainIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetAGainIss(pSensorCtx, pSensorAGain);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *          IsiSetDGainIss
 *
 *****************************************************************************/
RESULT IsiSetDGainIss
(
	IsiSensorHandle_t handle,
	IsiSensorGain_t *pSensorDGain
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorDGain == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetDGainIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetDGainIss(pSensorCtx, pSensorDGain);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiGetIntTimeIss
 *
 *****************************************************************************/
RESULT IsiGetIntTimeIss
(
	IsiSensorHandle_t handle,
	IsiSensorIntTime_t *pSensorIntTime
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorIntTime == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetIntTimeIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetIntTimeIss(pSensorCtx, pSensorIntTime);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

/*****************************************************************************/
/**
 *          IsiSetIntTimeIss
 *
 *****************************************************************************/
RESULT IsiSetIntTimeIss
(
	IsiSensorHandle_t handle,
	const IsiSensorIntTime_t *pSensorIntTime
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorIntTime == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetIntTimeIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetIntTimeIss(pSensorCtx, pSensorIntTime);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pFps == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetFpsIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetFpsIss(pSensorCtx, pFps);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetFpsIss == NULL) {
		xil_printf("%s: pIsiSetFpsIss is NULL\n", __func__);
		return (RET_NOTSUPP);
	}

	result = pSensorCtx->pSensor->pIsiSetFpsIss(pSensorCtx, fps);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetIspStatusIss == NULL) {
		xil_printf("%s: pIsiGetIspStatusIss is NULL\n", __func__);
		return (RET_NOTSUPP);
	}

	result = pSensorCtx->pSensor->pIsiGetIspStatusIss(pSensorCtx, pIspStatus);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiSetBlcIss
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
RESULT IsiSetBlcIss
(
	IsiSensorHandle_t handle,
	const IsiSensorBlc_t *pBlc
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetBlcIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetBlcIss(pSensorCtx, pBlc);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetBlcIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetBlcIss(pSensorCtx, pBlc);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiSetWBIss
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
RESULT IsiSetWBIss
(
	IsiSensorHandle_t handle,
	const IsiSensorWb_t *pWb
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetWBIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetWBIss(pSensorCtx, pWb);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetWBIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetWBIss(pSensorCtx, pWb);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetTpgIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetTpgIss(pSensorCtx, tpg);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if ((pSensorCtx == NULL) || (pSensorCtx->pSensor == NULL))
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetTpgIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetTpgIss(pSensorCtx, pTpg);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}
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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetExpandCurveIss == NULL || pCurve == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetExpandCurveIss(pSensorCtx, pCurve);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiGetCompressCurveIss == NULL || pCurve == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetCompressCurveIss(pSensorCtx, pCurve);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pUserData == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiExtendFuncIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiExtendFuncIss(pSensorCtx, pUserData);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


/*****************************************************************************/
/**
 *          IsiGetOtpDataIss
 *
 * @brief   get sensor otp data.
 *
 * @param   handle                  sensor instance handle
 * @param   pOtpData                sensor otp data
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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pOtpData == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetOtpDataIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetOtpDataIss(pSensorCtx, pOtpData);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiFocusCreateIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiFocusCreateIss(pSensorCtx);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiFocusReleaseIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiFocusReleaseIss(pSensorCtx);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pFocusCalib == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiFocusGetCalibrateIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiFocusGetCalibrateIss(pSensorCtx, pFocusCalib);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pPos == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiFocusSetIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiFocusSetIss(pSensorCtx, pPos);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}


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
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pPos == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiFocusGetIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiFocusGetIss(pSensorCtx, pPos);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

RESULT IsiSetInfraredLightExpParamIss
(
	IsiSensorHandle_t handle,
	IsiIrLightExp_t *pIrExpParam
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);
	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pIrExpParam == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiSetIRLightExpIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetIRLightExpIss(pSensorCtx, pIrExpParam);
	xil_printf("%s: (exit)\n", __func__);
	return (result);
}

RESULT IsiGetInfraredLightExpParamIss
(
	IsiSensorHandle_t handle,
	IsiIrLightExp_t *pIrExpParam
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);
	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pIrExpParam == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetIRLightExpIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetIRLightExpIss(pSensorCtx, pIrExpParam);
	xil_printf("%s: (exit)\n", __func__);
	return (result);
}


RESULT IsiQueryMetadataAttrIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t *pAttr
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pAttr == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiQueryMetadataAttrIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiQueryMetadataAttrIss(pSensorCtx, pAttr);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

RESULT IsiSetMetadataAttrEnableIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t attr
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pSensorCtx->pSensor->pIsiSetMetadataAttrEnableIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiSetMetadataAttrEnableIss(pSensorCtx, attr);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

RESULT IsiGetMetadataAttrEnableIss
(
	IsiSensorHandle_t handle,
	IsiMetadataAttr_t *pAttr
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pAttr == NULL)
		return RET_NULL_POINTER;

	if (pSensorCtx->pSensor->pIsiGetMetadataAttrEnableIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetMetadataAttrEnableIss(pSensorCtx, pAttr);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

RESULT IsiGetMetadataWindowIss
(
	IsiSensorHandle_t handle,
	IsiMetadataWinInfo_t *pMetaWin
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pMetaWin == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiGetMetadataWinIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiGetMetadataWinIss(pSensorCtx, pMetaWin);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}

RESULT IsiParserMetadataIss
(
	IsiSensorHandle_t handle,
	const MetadataBufInfo_t *pMetaBuf,
	IsiSensorMetadata_t *pMetaInfo
)
{
	IsiSensorContext_t *pSensorCtx = (IsiSensorContext_t *)handle;

	RESULT result = RET_SUCCESS;

	xil_printf("%s: (enter)\n", __func__);

	if (pSensorCtx == NULL)
		return (RET_WRONG_HANDLE);

	if (pMetaBuf == NULL || pMetaInfo == NULL)
		return (RET_NULL_POINTER);

	if (pSensorCtx->pSensor->pIsiParserMetadataIss == NULL)
		return (RET_NOTSUPP);

	result = pSensorCtx->pSensor->pIsiParserMetadataIss(pSensorCtx, pMetaBuf, pMetaInfo);

	xil_printf("%s: (exit)\n", __func__);

	return (result);
}
