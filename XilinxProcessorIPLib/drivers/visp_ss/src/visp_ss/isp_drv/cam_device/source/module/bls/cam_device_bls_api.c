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
#include "sensor_cmd.h"

RESULT VsiCamDeviceBlsSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsConfig_t *pBlsCfg
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pBlsCfg)
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
	memcpy(p_data, pBlsCfg, sizeof(CamDeviceBlsConfig_t));
	packet.payload_size += sizeof(CamDeviceBlsConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_SET_CONFIG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceBlsGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsConfig_t *pBlsCfg
)
{
	RESULT result = RET_SUCCESS;
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pBlsCfg)
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
	memcpy(p_data, pBlsCfg, sizeof(CamDeviceBlsConfig_t));
	packet.payload_size += sizeof(CamDeviceBlsConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_GET_CONFIG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pBlsCfg, p_data, sizeof(CamDeviceBlsConfig_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceBlsSetBitWidth
(
	CamDeviceHandle_t hCamDevice,
	uint8_t blsBitWidth
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
	memcpy(p_data, &blsBitWidth, sizeof(uint8_t));
	packet.payload_size += sizeof(uint8_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_SET_BIT_WIDTH, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceBlsGetBitWidth
(
	CamDeviceHandle_t hCamDevice,
	uint8_t *blsBitWidth
)
{
	RESULT result = RET_SUCCESS;
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == blsBitWidth)
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
	packet.payload_size += sizeof(uint8_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_GET_BIT_WIDTH, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(blsBitWidth, p_data, sizeof(uint8_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceBlsGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBlsStatus_t *pStatus
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
	memcpy(p_data, pStatus, sizeof(CamDeviceBlsStatus_t));
	packet.payload_size += sizeof(CamDeviceBlsStatus_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_GET_STATUS, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pStatus, p_data, sizeof(CamDeviceBlsStatus_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceBlsReset
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
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_RESET, &packet, packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceBlsGetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pVersion)
		return (RET_NULL_POINTER);
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, pVersion, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_BLS_GET_VERSION, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}
