#include <string.h>
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2023 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/


#include <cam_device_app.h>

RESULT VsiCamDeviceTpgSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceTpgConfig_t *pTpgCfg
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pTpgCfg)
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
	memcpy(p_data, pTpgCfg, sizeof(CamDeviceTpgConfig_t));
	packet.payload_size += sizeof(CamDeviceTpgConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_TPG_SetCfg, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


//RESULT VsiCamDeviceTpgGetConfig
//(
// CamDeviceHandle_t hCamDevice,
// CamDeviceTpgConfig_t *pTpgCfg
//){
// RESULT result = RET_SUCCESS;
//
// CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
// if (NULL == pCamDevCtx) {
// return (RET_WRONG_HANDLE);
// }
// if (NULL == pTpgCfg) {
// return (RET_NULL_POINTER);
// }
// pCamDevCtx->cookie ++;
//
// Payload_packet packet;
// memset(&packet, 0, sizeof(Payload_packet));
//
// packet.cookie = pCamDevCtx->cookie;
// packet.type = CMD;
// packet.payload_size = 0;
//
// uint8_t *p_data = packet.payload_data;
// memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
// p_data += sizeof(uint32_t);
// packet.payload_size += sizeof(uint32_t);
// memcpy(p_data, pTpgCfg, sizeof(CamDeviceTpgConfig_t));
// packet.payload_size += sizeof(CamDeviceTpgConfig_t);
//
//	if(packet.payload_size > MAX_ITEM)
//	return RET_OUTOFRANGE;
//
// result = Send_Command (APU_2_RPU_MB_CMD_TPG_GetCfg, &packet, packet.payload_size + payload_extra_size, dest_cpu_id,src_cpu_id);
// if(0 != result) {
// return RET_FAILURE;
// }
// packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
//	memcpy(pTpgCfg, p_data, sizeof(CamDeviceTpgConfig_t));
//
//	return packet.resp_field.error_subcode_t;
//}


RESULT VsiCamDeviceTpgEnable
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

	result = Send_Command(APU_2_RPU_MB_CMD_TPG_Enable, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceTpgDisable
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

	result = Send_Command(APU_2_RPU_MB_CMD_TPG_Disable, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


//RESULT VsiCamDeviceTpgReset
//(
// CamDeviceHandle_t hCamDevice
//){
// RESULT result = RET_SUCCESS;
//
// CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
// if (NULL == pCamDevCtx) {
// return (RET_WRONG_HANDLE);
// }
// pCamDevCtx->cookie ++;
//
// Payload_packet packet;
// memset(&packet, 0, sizeof(Payload_packet));
//
// packet.cookie = pCamDevCtx->cookie;
// packet.type = CMD;
// packet.payload_size = 0;
//
// uint8_t *p_data = packet.payload_data;
// memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
// p_data += sizeof(uint32_t);
// packet.payload_size += sizeof(uint32_t);
//
//	if(packet.payload_size > MAX_ITEM)
//	return RET_OUTOFRANGE;
//
// result = Send_Command (APU_2_RPU_MB_CMD_TPG_Reset, &packet, packet.payload_size + payload_extra_size, dest_cpu_id,src_cpu_id);
// if(0 != result) {
// return RET_FAILURE;
// }
// packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data); //replace with wait_response();
//
//	return packet.resp_field.error_subcode_t;
//}


RESULT VsiCamDeviceTpgGetVersion
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

	result = Send_Command(APU_2_RPU_MB_CMD_TPG_GetVersion, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pVersion, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}
