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


RESULT VsiCamDeviceRegisterAeLib
(
	CamDeviceHandle_t hCamDevice,
	void *pAeLibHandle
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
	memcpy(p_data, pAeLibHandle, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	ret = Send_Command(RPU_2_APU_MB_CMD_REGISTER_AELIB, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceUnRegisterAeLib
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

	ret = Send_Command(RPU_2_APU_MB_CMD_UNREGISTER_AELIB, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeSetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeConfig_t *pConfig
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
	memcpy(p_data, pConfig, sizeof(CamDeviceAeConfig_t));
	packet.payload_size += sizeof(CamDeviceAeConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_SET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeConfig_t *pConfig
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
	memcpy(p_data, pConfig, sizeof(CamDeviceAeConfig_t));
	packet.payload_size += sizeof(CamDeviceAeConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_CONFIG, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pConfig, p_data, sizeof(CamDeviceAeConfig_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeSetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMode_t *pAeMode
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAeMode)
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
	memcpy(p_data, pAeMode, sizeof(CamDeviceAeMode_t));
	packet.payload_size += sizeof(CamDeviceAeMode_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_SET_MODE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetMode
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMode_t *pAeMode
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAeMode)
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
	memcpy(p_data, pAeMode, sizeof(CamDeviceAeMode_t));
	packet.payload_size += sizeof(CamDeviceAeMode_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_MODE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pAeMode, p_data, sizeof(CamDeviceAeMode_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeSetRoi
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_SET_ROI, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetRoi
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_ROI, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pRoi, p_data, sizeof(CamDeviceRoi_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeEnable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_ENABLE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeDisable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_DISABLE, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeReset
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_RESET, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeState_t *pAeStatus
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pAeStatus)
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
	memcpy(p_data, pAeStatus, sizeof(CamDeviceAeState_t));
	packet.payload_size += sizeof(CamDeviceAeState_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_STATUS, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pAeStatus, p_data, sizeof(CamDeviceAeState_t));

	return packet.resp_field.error_subcode_t;
}


//implementation in aec
RESULT VsiCamDeviceAeGetHistogram
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeHistBins_t *pHistogram
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pHistogram)
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
	memcpy(p_data, pHistogram, sizeof(CamDeviceAeHistBins_t));
	packet.payload_size += sizeof(CamDeviceAeHistBins_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_HISTOGRAM, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pHistogram, p_data, sizeof(CamDeviceAeHistBins_t));

	return packet.resp_field.error_subcode_t;
}


//implementation in aec
RESULT VsiCamDeviceAeGetLuminance
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMeanLuma_t *pLuma
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pLuma)
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
	memcpy(p_data, pLuma, sizeof(CamDeviceAeMeanLuma_t));
	packet.payload_size += sizeof(CamDeviceAeMeanLuma_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_LUMINANCE, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pLuma, p_data, sizeof(CamDeviceAeMeanLuma_t));

	return packet.resp_field.error_subcode_t;
}


//implementation in aec
RESULT VsiCamDeviceAeGetObjectRegion
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeMeanLuma_t *pObjectRegion
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pObjectRegion)
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
	memcpy(p_data, pObjectRegion, sizeof(CamDeviceAeMeanLuma_t));
	packet.payload_size += sizeof(CamDeviceAeMeanLuma_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_OBJECT_REGION, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pObjectRegion, p_data, sizeof(CamDeviceAeMeanLuma_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceAeSetExpTable
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceExpTable_t *pExpTable
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pExpTable)
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
	memcpy(p_data, pExpTable, sizeof(CamDeviceExpTable_t));
	packet.payload_size += sizeof(CamDeviceExpTable_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_SET_EXP_TBL, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetExpTable
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceExpTable_t *pExpTable
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pExpTable)
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
	memcpy(p_data, pExpTable, sizeof(CamDeviceExpTable_t));
	packet.payload_size += sizeof(CamDeviceExpTable_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_EXP_TBL, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pExpTable, p_data, sizeof(CamDeviceExpTable_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceAeGetVersion
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

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_VERSION, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceAeGetResult
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceAeResult_t *pCamDeviceAeResult
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pCamDeviceAeResult)
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
	memcpy(p_data, pCamDeviceAeResult, sizeof(CamDeviceAeResult_t));
	packet.payload_size += sizeof(CamDeviceAeResult_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_AE_GET_RES, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pCamDeviceAeResult, p_data, sizeof(CamDeviceAeResult_t));

	return packet.resp_field.error_subcode_t;
}
