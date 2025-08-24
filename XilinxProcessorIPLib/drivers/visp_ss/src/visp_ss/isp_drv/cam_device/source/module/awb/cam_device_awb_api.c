#include <string.h>
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

#include <cam_device_app.h>


RESULT VsiCamDeviceRegisterAwbLib
(
	CamDeviceHandle_t hCamDevice,
	void *pAwbLibHandle
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, pAwbLibHandle, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_REGISTER_AWB_LIB, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceUnRegisterAwbLib
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

	ret = Send_Command(APU_2_RPU_MB_CMD_UNREGISTER_AWB_LIB, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbConfig_t *pConfig
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
	memcpy(p_data, pConfig, sizeof(CamDeviceAwbConfig_t));
	packet.payload_size += sizeof(CamDeviceAwbConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_SET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbConfig_t *pConfig
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
	memcpy(p_data, pConfig, sizeof(CamDeviceAwbConfig_t));
	packet.payload_size += sizeof(CamDeviceAwbConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pConfig, p_data, sizeof(CamDeviceAwbConfig_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbSetMode
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceAwbMode_t *pAwbMode
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAwbMode)
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
	memcpy(p_data, pAwbMode, sizeof(CamDeviceAwbMode_t));
	packet.payload_size += sizeof(CamDeviceAwbMode_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_SET_MODE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbGetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbMode_t *pAwbMode
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAwbMode)
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
	memcpy(p_data, pAwbMode, sizeof(CamDeviceAwbMode_t));
	packet.payload_size += sizeof(CamDeviceAwbMode_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_MODE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pAwbMode, p_data, sizeof(CamDeviceAwbMode_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbSetRoi
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceRoi_t *pRoi
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pRoi)
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
	memcpy(p_data, pRoi, sizeof(CamDeviceRoi_t));
	packet.payload_size += sizeof(CamDeviceRoi_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_SET_ROI, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbGetRoi
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceRoi_t *pRoi
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pRoi)
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
	memcpy(p_data, pRoi, sizeof(CamDeviceRoi_t));
	packet.payload_size += sizeof(CamDeviceRoi_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_ROI, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pRoi, p_data, sizeof(CamDeviceRoi_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceAwbGetColorTempWeight
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbTemWeight_t *pAwbTemWeight
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAwbTemWeight)
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
	memcpy(p_data, pAwbTemWeight, sizeof(CamDeviceAwbTemWeight_t));
	packet.payload_size += sizeof(CamDeviceAwbTemWeight_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_ColorTempWeight, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pAwbTemWeight, p_data, sizeof(CamDeviceAwbTemWeight_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbEnable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_ENABLE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbDisable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_DISABLE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbState_t *pAwbStatus
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAwbStatus)
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
	memcpy(p_data, pAwbStatus, sizeof(CamDeviceAwbState_t));
	packet.payload_size += sizeof(CamDeviceAwbState_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_STATUS, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pAwbStatus, p_data, sizeof(CamDeviceAwbState_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbGetVersion
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_VERSION, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAwbReset
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_RESET, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceAwbGetResult
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAwbResult_t *pCamDeviceAwbResult
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pCamDeviceAwbResult)
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
	memcpy(p_data, pCamDeviceAwbResult, sizeof(CamDeviceAwbResult_t));
	packet.payload_size += sizeof(CamDeviceAwbResult_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AWB_GET_RES, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pCamDeviceAwbResult, p_data, sizeof(CamDeviceAwbResult_t));

	return packet.resp_field.error_subcode_t;
}
