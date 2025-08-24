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
 * @file isi_control.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/

#include "isi.h"
#include "isi_control.h"
#include "sensor_drv.h"


extern IsiCamDrvConfig_t Ox03f10_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox08b40_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox05b1s_IsiCamDrvConfig;

IsiSensorInst_t gSInst;

static void IsiSensorHandleInit(uint32_t instId)
{
	gSInst.hIsiSensor[instId] = NULL;
	return;
}

RESULT ControlIsiSensorDrvHandleRegisterIss
(
	void *data
)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorInstanceConfig_t *pConfig;
	IsiSensorHandle_t sensorHandle;
	IsiCamDrvConfig_t *pCamDrvConfig;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(pConfig, p_data, sizeof(IsiSensorInstanceConfig_t));

	IsiSensorHandleInit(instanceId);
	// sensor mapping
	SensorDrvConfig_t sensorConfig[] = {
		{"ox03f10", &Ox03f10_IsiCamDrvConfig},
		{"ox08b40", &Ox08b40_IsiCamDrvConfig},
		{"ox05b1s", &Ox05b1s_IsiCamDrvConfig}
	};
	xil_printf("APU get cameraDriverID: %d.\r\n", pConfig->cameraDriverID);
	for (uint8_t i = 0; i < (int)(sizeof(sensorConfig) / sizeof(sensorConfig[0])); i++) {
		if (pConfig->cameraDriverID == sensorConfig[i].pSensorConfig->cameraDriverID)
			pCamDrvConfig = sensorConfig[i].pSensorConfig;
	}
	// sensor Driver Handle Register Iss
	// only change pConfig.pSensor
	pConfig->pSensor = osMalloc(sizeof(IsiSensor_t));
	if (pConfig->pSensor == NULL)
		return RET_OUTOFMEM;
	memset(pConfig->pSensor, 0, sizeof(IsiSensor_t));
	ret = pCamDrvConfig->pIsiGetSensorIss(pConfig->pSensor);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		osFree(pConfig->pSensor);
		return ret;
	}
	ret = pConfig->pSensor->pIsiCreateIss(pConfig, &sensorHandle);

	xil_printf("%s instanceId: %d., sensorHandle :%x\r\n", __func__, instanceId, sensorHandle);
	gSInst.hIsiSensor[instanceId] = sensorHandle;
	return ret;
}

RESULT ControlIsiSensorDrvHandleUnRegisterIss
(
	void *data
)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];
	ret = IsiSensorDrvHandleUnRegisterIss(sensorHandle);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}

RESULT ControlIsiEnumModeIss
(
	void *data
)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorEnumMode_t enumMode;
	memset(&enumMode, 0, sizeof(IsiSensorEnumMode_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	xil_printf("%s instanceId: %d., sensorHandle :%x\r\n", __func__, instanceId, sensorHandle);

	memcpy(&enumMode, p_data, sizeof(IsiSensorEnumMode_t));

	ret = IsiEnumModeIss(sensorHandle, &enumMode);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &enumMode, sizeof(IsiSensorEnumMode_t));

	return ret;
}

RESULT ControlIsiOpenIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint32_t mode;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&mode, p_data, sizeof(uint32_t));
	ret = IsiOpenIss(sensorHandle, mode);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiCloseIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];
	ret = IsiCloseIss(sensorHandle);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiCheckConnectionIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];
	ret = IsiCheckConnectionIss(sensorHandle);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetModeIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorMode_t sMode;
	memset(&sMode, 0, sizeof(IsiSensorMode_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&sMode, p_data, sizeof(IsiSensorMode_t));

	ret = IsiGetModeIss(sensorHandle, &sMode);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &sMode, sizeof(IsiSensorMode_t));

	return ret;
}
RESULT ControlIsiGetCapsIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiCaps_t caps;
	memset(&caps, 0, sizeof(IsiCaps_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetCapsIss(sensorHandle, &caps);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &caps, sizeof(IsiCaps_t));

	return ret;
}
RESULT ControlIsiSetStreamingIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	bool_t on;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&on, p_data, sizeof(bool_t));
	ret = IsiSetStreamingIss(sensorHandle, on);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetRevisionIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint32_t value;
	memset(&value, 0, sizeof(uint32_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetRevisionIss(sensorHandle, &value);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &value, sizeof(uint32_t));

	return ret;
}

RESULT ControlIsiGetAeBaseInfoIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiAeBaseInfo_t aeBaseInfo;
	memset(&aeBaseInfo, 0, sizeof(IsiAeBaseInfo_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetAeBaseInfoIss(sensorHandle, &aeBaseInfo);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &aeBaseInfo, sizeof(IsiAeBaseInfo_t));

	return ret;
}
RESULT ControlIsiGetAGainIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorGain_t sensorAGain;
	memset(&sensorAGain, 0, sizeof(IsiSensorGain_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetAGainIss(sensorHandle, &sensorAGain);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &sensorAGain, sizeof(IsiSensorGain_t));

	return ret;
}
RESULT ControlIsiGetDGainIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorGain_t sensorDGain;
	memset(&sensorDGain, 0, sizeof(IsiSensorGain_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetDGainIss(sensorHandle, &sensorDGain);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &sensorDGain, sizeof(IsiSensorGain_t));

	return ret;
}
RESULT ControlIsiSetAGainIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorGain_t sensorAGain;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&sensorAGain, p_data, sizeof(IsiSensorGain_t));
	ret = IsiSetAGainIss(sensorHandle, &sensorAGain);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiSetDGainIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorGain_t sensorDGain;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&sensorDGain, p_data, sizeof(IsiSensorGain_t));
	ret = IsiSetDGainIss(sensorHandle, &sensorDGain);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetIntTimeIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorIntTime_t sensorIntTime;
	memset(&sensorIntTime, 0, sizeof(IsiSensorIntTime_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetIntTimeIss(sensorHandle, &sensorIntTime);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &sensorIntTime, sizeof(IsiSensorIntTime_t));

	return ret;
}
RESULT ControlIsiSetIntTimeIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorIntTime_t sensorIntTime;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&sensorIntTime, p_data, sizeof(IsiSensorIntTime_t));
	ret = IsiSetIntTimeIss(sensorHandle, &sensorIntTime);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;;
}
RESULT ControlIsiGetFpsIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint32_t fps;
	memset(&fps, 0, sizeof(uint32_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetFpsIss(sensorHandle, &fps);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &fps, sizeof(uint32_t));

	return ret;
}
RESULT ControlIsiSetFpsIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint32_t fps;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&fps, p_data, sizeof(uint32_t));
	ret = IsiSetFpsIss(sensorHandle, fps);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}

RESULT ControlIsiGetIspStatusIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiIspStatus_t ispStatus;
	memset(&ispStatus, 0, sizeof(IsiIspStatus_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetIspStatusIss(sensorHandle, &ispStatus);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &ispStatus, sizeof(IsiIspStatus_t));

	return ret;
}
RESULT ControlIsiSetWBIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorWb_t swb;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&swb, p_data, sizeof(IsiSensorWb_t));
	ret = IsiSetWBIss(sensorHandle, &swb);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetWBIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorWb_t swb;
	memset(&swb, 0, sizeof(IsiSensorWb_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetWBIss(sensorHandle, &swb);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &swb, sizeof(IsiSensorWb_t));

	return ret;
}
RESULT ControlIsiSetBlcIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorBlc_t sblc;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&sblc, p_data, sizeof(IsiSensorBlc_t));
	ret = IsiSetBlcIss(sensorHandle, &sblc);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetBlcIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorBlc_t sblc;
	memset(&sblc, 0, sizeof(IsiSensorBlc_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetBlcIss(sensorHandle, &sblc);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &sblc, sizeof(IsiSensorBlc_t));

	return ret;
}

RESULT ControlIsiSetTpgIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorTpg_t tpg;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&tpg, p_data, sizeof(IsiSensorTpg_t));
	ret = IsiSetTpgIss(sensorHandle, tpg);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiGetTpgIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorTpg_t stpg;
	memset(&stpg, 0, sizeof(IsiSensorTpg_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetTpgIss(sensorHandle, &stpg);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &stpg, sizeof(IsiSensorTpg_t));

	return ret;
}
RESULT ControlIsiGetExpandCurveIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	IsiSensorCompandCurve_t curve;
	memset(&curve, 0, sizeof(IsiSensorCompandCurve_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	ret = IsiGetExpandCurveIss(sensorHandle, &curve);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &curve, sizeof(IsiSensorCompandCurve_t));

	return ret;
}


RESULT ControlIsiWriteRegIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint16_t addr, value;

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&addr, p_data, sizeof(uint16_t));
	p_data += sizeof(uint16_t);
	memcpy(&value, p_data, sizeof(uint16_t));

	ret = IsiWriteRegIss(sensorHandle, addr, value);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}
	return ret;
}
RESULT ControlIsiReadRegIss(void *data)
{
	RESULT ret = RET_SUCCESS;
	uint32_t instanceId = 0x99;
	uint16_t addr;
	uint16_t rvalue;
	memset(&rvalue, 0, sizeof(uint16_t));

	Payload_packet *packet = (Payload_packet *)data;

	/* get the parameter data from playload_data */
	uint8_t *p_data = packet->payload_data;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	IsiSensorHandle_t sensorHandle = gSInst.hIsiSensor[instanceId];

	memcpy(&addr, p_data, sizeof(uint16_t));
	p_data += sizeof(uint16_t);

	ret = IsiReadRegIss(sensorHandle, addr, &rvalue);
	if (ret != RET_SUCCESS) {
		xil_printf("APU %s FAIL, the result is %d.\n", __func__, ret);
		return ret;
	}

	memcpy(p_data, &rvalue, sizeof(uint16_t));

	return ret;
}
