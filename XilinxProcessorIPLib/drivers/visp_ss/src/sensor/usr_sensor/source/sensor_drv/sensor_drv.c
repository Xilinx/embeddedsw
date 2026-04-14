/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "sensor_cmd.h"
#include "mbox_cmd.h"

#include <types.h>
#include <trace.h>
#include <return_codes.h>
#include <string.h>


extern IsiCamDrvConfig_t Ox03f10_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox08b40_IsiCamDrvConfig;
extern IsiCamDrvConfig_t Ox05b1s_IsiCamDrvConfig;
extern IsiCamDrvConfig_t virtualSensor_IsiCamDrvConfig;

extern IsiSensorMode_t pox03f10_mode_info[];

RESULT VsiCamDeviceSensorGetNumber
(
	uint16_t *pNumber
)
{
	RESULT result = RET_SUCCESS;

	if (NULL == pNumber)
		return (RET_NULL_POINTER);

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 0;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	// memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
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
	CamDeviceSensorListInfo_t *pSensorListInfo,
    const uint16_t sensorNum
)
{
	RESULT result = RET_SUCCESS;

	if (NULL == pSensorListInfo)
		return (RET_NULL_POINTER);

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 1;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	// memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	memcpy(p_data, &sensorNum, sizeof(uint16_t));
	p_data += sizeof(uint16_t);
	packet.payload_size = sizeof(uint16_t) + sizeof(CamDeviceSensorListInfo_t)* sensorNum;

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_LIST_INFO, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pSensorListInfo, p_data, sizeof(CamDeviceSensorListInfo_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSensorGetConnectPortInfo
(
	uint32_t sensorDrvId,
	CamDeviceSensorConnectPortInfo_t *pPortInfo
)
{
	RESULT result = RET_SUCCESS;

	if (NULL == pPortInfo)
		return (RET_NULL_POINTER);

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 2;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &sensorDrvId, sizeof(SensorDrvId_t));
    packet.payload_size += sizeof(SensorDrvId_t);

    p_data += sizeof(SensorDrvId_t);
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

RESULT VsiCamDeviceSensorMapping
(
	const CamDeviceSensorModuleMapCfg_t  *pModuleInfo,
    CamDeviceSensorDrvHandle_t *pSensorDrvhandle
)
{
	RESULT result = RET_SUCCESS;
	uint8_t i = 0;


	if (NULL == pModuleInfo || NULL == pSensorDrvhandle)
		return (RET_NULL_POINTER);


	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 3;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pModuleInfo->moduleName, SENSOR_MODULE_NAME);

    p_data += SENSOR_MODULE_NAME;
    packet.payload_size += SENSOR_MODULE_NAME;
    memcpy(p_data, &pModuleInfo->sensorDevId, sizeof(uint32_t));

    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);

    packet.payload_size += sizeof(IsiCamDrvConfigMbox_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_MAPPING, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;


	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	*pSensorDrvhandle = (IsiCamDrvConfigMbox_t *)osMalloc(sizeof(IsiCamDrvConfigMbox_t));
    if(*pSensorDrvhandle == NULL) {
	printf("APU Failed to allocate memory for sensor mapping.\r\n");
	return RET_OUTOFMEM;
    }
    memcpy(*pSensorDrvhandle, p_data, sizeof(IsiCamDrvConfigMbox_t));

    IsiCamDrvConfigMbox_t *pcamcfg = (IsiCamDrvConfigMbox_t *)*pSensorDrvhandle;
    printf("Apu mapping end: cameraDriverID: %x \r\n", pcamcfg->cameraDriverID);
    printf("Apu mapping end: sensorDevId: %d \r\n", pcamcfg->sensorDevId);
    printf("Apu mapping end: pIsiGetSensorIss: %d \r\n", pcamcfg->pIsiGetSensorIss);


	return packet.resp_field.error_subcode_t;
}
