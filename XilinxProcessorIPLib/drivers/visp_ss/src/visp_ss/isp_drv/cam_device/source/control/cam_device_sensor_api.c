#include <string.h>
/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2022 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/


#include <cam_device_app.h>
#include "cam_device_sensor_api.h"
#include "cam_device_api.h"
#include <isi_iss.h>"
#include "oslayer.h"

#define SENSOR_NAME_LEN 20

RESULT VsiCamDeviceSensorOpen
(
	CamDeviceHandle_t hCamDevice,
	uint32_t modeIndex
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &modeIndex, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_OPEN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorDrvHandleRegister
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorDrvCfg_t *pSensorDrv
)
{
	RESULT result = RET_SUCCESS;


	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx || NULL == pSensorDrv)
		return (RET_WRONG_HANDLE);


	// for mailbox
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	IsiCamDrvConfigMbox_t *pcamcfg = (IsiCamDrvConfigMbox_t *)(pSensorDrv->sensorDrvHandle);

	xil_printf("pSensorDrv->sensorDrvHandle: %x.\r\n", pSensorDrv->sensorDrvHandle);
	xil_printf("pSensorDrv->sensorDevId: %d.\r\n", pSensorDrv->sensorDevId);
	xil_printf("\r\n pcamcfg->cameraDriverID: %x.\r\n", pcamcfg->cameraDriverID);
	xil_printf("pcamcfg->pIsiGetSensorIss: %x.\r\n", pcamcfg->pIsiGetSensorIss);

	//pass to RPU ISI layer
	pcamcfg->instanceId = pCamDevCtx->instanceId;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	memcpy(p_data, pSensorDrv->sensorDrvHandle, sizeof(IsiCamDrvConfigMbox_t));
	p_data += sizeof(IsiCamDrvConfigMbox_t);
	packet.payload_size += sizeof(IsiCamDrvConfigMbox_t);
	memcpy(p_data, &pSensorDrv->sensorDevId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorDrvHandleUnRegister
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorClose
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_CLOSE, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorMapping
(
	CamDeviceHandle_t hCamDevice,
	const char *pSensorName,
	CamDeviceSensorDrvHandle_t *pSensorDrvhandle
)
{
	RESULT result = RET_SUCCESS;
	uint8_t i = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pSensorName || NULL == pSensorDrvhandle)
		return (RET_NULL_POINTER);

	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pSensorName, SENSOR_NAME_LEN);
	p_data += SENSOR_NAME_LEN;
	packet.payload_size += SENSOR_NAME_LEN;
	//memcpy(p_data, pSensorDrvhandle, sizeof(uint32_t));
	packet.payload_size += sizeof(IsiCamDrvConfigMbox_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_MAPPING, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;


	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	*pSensorDrvhandle = (IsiCamDrvConfigMbox_t *)osMalloc(sizeof(IsiCamDrvConfigMbox_t));

	if (*pSensorDrvhandle == NULL) {
		xil_printf("APU Failed to allocate memory for sensor mapping.\r\n");
		return RET_OUTOFMEM;
	}
	memcpy(*pSensorDrvhandle, p_data, sizeof(IsiCamDrvConfigMbox_t));

	IsiCamDrvConfigMbox_t camcfg = **(IsiCamDrvConfigMbox_t **)pSensorDrvhandle;
	xil_printf("APU mapping get cameraDriverID: %d.\r\n", camcfg.cameraDriverID);


	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorQuery
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorQuery_t *pQuery
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pQuery)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pQuery, sizeof(CamDeviceSensorQuery_t));
	packet.payload_size += sizeof(CamDeviceSensorQuery_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_QUERY, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pQuery, p_data, sizeof(CamDeviceSensorQuery_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorInfo_t *pSensorInfo
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pSensorInfo)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	//memcpy(p_data, pSensorInfo, sizeof(CamDeviceSensorInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorInfo_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_INFO, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pSensorInfo, p_data, sizeof(CamDeviceSensorInfo_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetTestPattern
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorTestPattern_t *pTestPattern
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pTestPattern)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pTestPattern, sizeof(CamDeviceSensorTestPattern_t));
	packet.payload_size += sizeof(CamDeviceSensorTestPattern_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_TP, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetRegister
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorRegister_t *pRegister
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pRegister)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
	packet.payload_size += sizeof(CamDeviceSensorRegister_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_REG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetRegister
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorRegister_t *pRegister
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pRegister)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
	packet.payload_size += sizeof(CamDeviceSensorRegister_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_REG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pRegister, p_data, sizeof(CamDeviceSensorRegister_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetFrameRate
(
	CamDeviceHandle_t hCamDevice,
	float *pFps
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFps)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pFps, sizeof(float));
	packet.payload_size += sizeof(float);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_FRAMERATE, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetFrameRate
(
	CamDeviceHandle_t hCamDevice,
	float *pFps
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFps)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pFps, sizeof(float));
	packet.payload_size += sizeof(float);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_FRAMERATE, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pFps, p_data, sizeof(float));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetModeInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorModeInfo_t *pMode
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pMode)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMode, sizeof(CamDeviceSensorModeInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorModeInfo_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_MODE_INFO, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pMode, p_data, sizeof(CamDeviceSensorModeInfo_t));
	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorStatus_t *pStatus
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pStatus)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pStatus, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_STATUS, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pStatus, p_data, sizeof(CamDeviceSensorStatus_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetGain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorGain_t *pGain
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pGain)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pGain, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_GAIN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetGain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorGain_t *pGain
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pGain)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pGain, sizeof(CamDeviceSensorGain_t));
	packet.payload_size += sizeof(CamDeviceSensorGain_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_GAIN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pGain, p_data, sizeof(CamDeviceSensorGain_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetExposureControl
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorExposureControl_t *pExpCtrl
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pExpCtrl)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
	packet.payload_size += sizeof(CamDeviceSensorExposureControl_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_EXP_CTL, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetExposureControl
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorExposureControl_t *pExpCtrl

)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pExpCtrl)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
	packet.payload_size += sizeof(float);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_EXP_CTL, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pExpCtrl, p_data, sizeof(CamDeviceSensorExposureControl_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetFocusPositon
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorFocusPos_t *pFocusPos
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFocusPos)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
	packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SetFocusPositon, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);

	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetFocusPositon
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorFocusPos_t *pFocusPos,
	CamDeviceSensorIntegerRange_t *pRangeLimit
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFocusPos || NULL == pRangeLimit)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
	p_data += sizeof(CamDeviceSensorFocusPos_t);
	packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

	memcpy(p_data, pRangeLimit, sizeof(CamDeviceSensorIntegerRange_t));
	packet.payload_size += sizeof(CamDeviceSensorIntegerRange_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GetFocusPositon, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	memcpy(pRangeLimit, p_data, sizeof(CamDeviceSensorIntegerRange_t));

	p_data -= sizeof(CamDeviceSensorFocusPos_t);
	memcpy(pFocusPos, p_data, sizeof(CamDeviceSensorFocusPos_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetStreaming
(
	CamDeviceHandle_t hCamDevice,
	bool_t isEnable
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &isEnable, sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_STREAMING, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetOtpInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorOtpModuleInfo_t *pOtpModuleInfo
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pOtpModuleInfo)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pOtpModuleInfo, sizeof(CamDeviceSensorOtpModuleInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorOtpModuleInfo_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_OPT_INFO, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pOtpModuleInfo, p_data, sizeof(CamDeviceSensorOtpModuleInfo_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetMetadataAttr
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataAttr_t *pMetaAttr
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pMetaAttr)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMetaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_ATTR, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pMetaAttr, p_data, sizeof(CamDeviceSensorMetadataAttr_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetMetadataAttr
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataAttr_t metaAttr
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &metaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_METADATA_ATTR, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetMetadataWin
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataWin_t *pMetaWin
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pMetaWin)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMetaWin, sizeof(CamDeviceSensorMetadataWin_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataWin_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_WIN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pMetaWin, p_data, sizeof(CamDeviceSensorMetadataWin_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetNumber
(
	CamDeviceHandle_t hCamDevice,
	uint16_t *pNumber
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pNumber)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pNumber, sizeof(uint16_t));
	packet.payload_size += sizeof(uint16_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_NUMBER, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pNumber, p_data, sizeof(uint16_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetListInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorListInfo_t *pSensorListInfo,
	const uint16_t sensorNum
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pSensorListInfo)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pSensorListInfo, sizeof(CamDeviceSensorListInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorListInfo_t);

	p_data += sizeof(CamDeviceSensorListInfo_t);
	memcpy(p_data, &sensorNum, sizeof(uint16_t));
	packet.payload_size += sizeof(uint16_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_LIST_INFO, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	p_data -= sizeof(CamDeviceSensorListInfo_t);
	memcpy(pSensorListInfo, p_data, sizeof(CamDeviceSensorListInfo_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetConnectPortInfo
(
	CamDeviceHandle_t hCamDevice,
	uint32_t portId,
	CamDeviceSensorConnectPortInfo_t *pPortInfo
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pPortInfo)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &portId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	p_data += sizeof(uint32_t);
	memcpy(p_data, pPortInfo, sizeof(CamDeviceSensorConnectPortInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorConnectPortInfo_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pPortInfo, p_data, sizeof(CamDeviceSensorConnectPortInfo_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorSetBls
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorBls_t *pBls
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pBls)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pBls, sizeof(CamDeviceSensorBls_t));
	packet.payload_size += sizeof(CamDeviceSensorBls_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_BLS, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetBls
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorBls_t *pBls
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pBls)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pBls, sizeof(CamDeviceSensorBls_t));
	packet.payload_size += sizeof(CamDeviceSensorBls_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_BLS, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pBls, p_data, sizeof(CamDeviceSensorBls_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorSetWb
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorWb_t *pWb
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pWb)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pWb, sizeof(CamDeviceSensorWb_t));
	packet.payload_size += sizeof(CamDeviceSensorWb_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_WB, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSensorGetWb
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorWb_t *pWb
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pWb)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pWb, sizeof(CamDeviceSensorWb_t));
	packet.payload_size += sizeof(CamDeviceSensorWb_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_WB, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pWb, p_data, sizeof(CamDeviceSensorWb_t));

	return packet.resp_field.error_subcode_t;
}
