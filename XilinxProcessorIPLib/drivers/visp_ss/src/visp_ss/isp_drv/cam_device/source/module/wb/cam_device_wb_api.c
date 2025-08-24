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


RESULT VsiCamDeviceWbSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceWbConfig_t *pWbCfg
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
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
	memcpy(p_data, pWbCfg, sizeof(CamDeviceWbConfig_t));
	packet.payload_size += sizeof(CamDeviceWbConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_WB_SET_CONFIG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWbConfig_t *pWbCfg
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
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
	memcpy(p_data, pWbCfg, sizeof(CamDeviceWbConfig_t));
	packet.payload_size += sizeof(CamDeviceWbConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_CONFIG, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pWbCfg, p_data, sizeof(CamDeviceWbConfig_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbEnable
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_ENABLE, &packet, packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbDisable
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_DISABLE, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceWbStatus_t *pWbStatus
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pWbStatus)
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
	memcpy(p_data, pWbStatus, sizeof(CamDeviceWbStatus_t));
	packet.payload_size += sizeof(CamDeviceWbStatus_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_STATUS, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pWbStatus, p_data, sizeof(CamDeviceWbStatus_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbReset
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_RESET, &packet, packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWbGetVersion
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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pVersion, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_VERSION, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}
