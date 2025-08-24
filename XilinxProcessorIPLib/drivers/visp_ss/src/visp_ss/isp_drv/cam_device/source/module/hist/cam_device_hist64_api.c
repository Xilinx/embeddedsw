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

RESULT VsiCamDeviceHist64SetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Config_t *pConfig
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pConfig)
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
	memcpy(p_data, pConfig, sizeof(CamDeviceHist64Config_t));
	packet.payload_size += sizeof(CamDeviceHist64Config_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_SET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64GetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Config_t *pConfig
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pConfig)
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
	memcpy(p_data, pConfig, sizeof(CamDeviceHist64Config_t));
	packet.payload_size += sizeof(CamDeviceHist64Config_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_GET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pConfig, p_data, sizeof(CamDeviceHist64Config_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64Enable
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

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

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_ENABLE, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64Disable
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

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

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_DISABLE, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64GetStatistic
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Bins_t *pHistBins
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pHistBins)
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
	memcpy(p_data, pHistBins, sizeof(CamDeviceHist64Bins_t));
	packet.payload_size += sizeof(CamDeviceHist64Bins_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_GET_STATISTIC, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pHistBins, p_data, sizeof(CamDeviceHist64Bins_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64GetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceHist64Status_t *pStatus
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

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
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, pStatus, sizeof(CamDeviceHist64Status_t));
	packet.payload_size += sizeof(CamDeviceHist64Status_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_GET_STATUS, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pStatus, p_data, sizeof(CamDeviceHist64Status_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64GetVersion
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pVersion
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

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

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_GET_VERSION, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHist64Reset
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

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

	ret = Send_Command(APU_2_RPU_MB_CMD_HIST64_RESET, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}
