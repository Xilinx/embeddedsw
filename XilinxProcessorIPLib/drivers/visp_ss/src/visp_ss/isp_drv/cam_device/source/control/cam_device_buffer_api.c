#include <string.h>
/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

/*Copy from cam_device.cpp*/

#include <cam_device_app.h>
#include "cam_device_buffer_api.h"
#include "cam_device_api.h"
#include "sensor_cmd.h"
#include "oslayer.h"

extern uint32_t cookie;

RESULT VsiCamDeviceInitBufChain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId,
	CamDeviceBufChainConfig_t *pConfig
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx || NULL == pConfig)
		return RET_NULL_POINTER;
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
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);
	memcpy(p_data, pConfig, sizeof(CamDeviceBufChainConfig_t));
	packet.payload_size += sizeof(CamDeviceBufChainConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_INIT_BUF_CHAIN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceDeInitBufChain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_DEINIT_BUF_CHAIN, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceCreateBufPool
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId,
	CamDeviceBufPoolConfig_t *hBufferPoolCfg
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx || NULL == hBufferPoolCfg)
		return RET_NULL_POINTER;
	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	// to solve 64-bit space -> 32-bit space problem, we can not directly copy 	CamDeviceBufPoolSetupCfg_t

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	// memcpy(p_data, &(hBufferPoolSetupCfg->bufIo), sizeof(CamDeviceBufChainId_t));
	// packet.payload_size += sizeof(CamDeviceBufChainId_t);
	// p_data += sizeof(CamDeviceBufChainId_t);
	//
	// memcpy(p_data, &(hBufferPoolSetupCfg->bufferChain), sizeof(CamDeviceBufChainConfig_t));
	//	packet.payload_size += sizeof(CamDeviceBufChainConfig_t);
	//	p_data += sizeof(CamDeviceBufChainConfig_t);

	memcpy(p_data, &(hBufferPoolCfg->bufMode), sizeof(CamDeviceBufMode_t));
	packet.payload_size += sizeof(CamDeviceBufMode_t);
	p_data += sizeof(CamDeviceBufMode_t);

	memcpy(p_data, &(hBufferPoolCfg->bufNum), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(hBufferPoolCfg->bufSize), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	// 6 physical address
	memcpy(p_data, hBufferPoolCfg->pBaseAddrList, hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet.payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

	memcpy(p_data, &(hBufferPoolCfg->is_mapped), sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, hBufferPoolCfg->pIplAddrList, hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet.payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_CREATE_BUFFER_POOL, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceDestroyBufPool
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_DESTORY_BUFFER_POOL, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSetupBufMgmt
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SETUP_BUF_MGMT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceReleaseBufMgmt
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceDeQueBuffer
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceBufChainId_t bufId,
    MediaBuffer_t **pMediaBuf
)
{
    RESULT result = RET_SUCCESS;

    *pMediaBuf = osMalloc(sizeof(MediaBuffer_t));
    if (NULL == *pMediaBuf)
        return RET_NULL_POINTER;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        osFree(*pMediaBuf);
        *pMediaBuf = NULL;
        return RET_NULL_POINTER;
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    (*pMediaBuf)->pMetaData = osMalloc(sizeof(PicBufMetaData_t));
    if (NULL == (*pMediaBuf)->pMetaData) {
        osFree(*pMediaBuf);
        *pMediaBuf = NULL;
        return RET_NULL_POINTER;
    }
    memset((*pMediaBuf)->pMetaData, 0, sizeof(PicBufMetaData_t));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    /* === SEND: must match RPU's ControlVsiCamDeviceDeQueBuffer read order ===
     * RPU reads: instanceId, bufId, baseAddress, baseSize, lockCount,
     *            isFull, index, bufMode, pOwner
     */
    uint8_t *p_data = packet.payload_data;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet.payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);

    memcpy(p_data, &((*pMediaBuf)->baseAddress), sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->baseSize), sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->lockCount), sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->isFull), sizeof(bool_t));
    packet.payload_size += sizeof(bool_t);
    p_data += sizeof(bool_t);

    memcpy(p_data, &((*pMediaBuf)->index), sizeof(uint8_t));
    packet.payload_size += sizeof(uint8_t);
    p_data += sizeof(uint8_t);

    memcpy(p_data, &((*pMediaBuf)->bufMode), sizeof(BUFF_MODE));
    packet.payload_size += sizeof(BUFF_MODE);
    p_data += sizeof(BUFF_MODE);

    memcpy(p_data, &((*pMediaBuf)->pOwner), sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    /* Reserve space for PicBufMetaData_t that RPU appends in response */
    packet.payload_size += sizeof(PicBufMetaData_t);

    if (packet.payload_size > MAX_ITEM) {
        osFree((*pMediaBuf)->pMetaData);
        osFree(*pMediaBuf);
        *pMediaBuf = NULL;
        return RET_OUTOFRANGE;
    }

    result = Send_Command(APU_2_RPU_MB_CMD_DEQUE_BUFFER, &packet,
                  packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if (RET_SUCCESS != result) {
        osFree((*pMediaBuf)->pMetaData);
        osFree(*pMediaBuf);
        *pMediaBuf = NULL;
        return RET_FAILURE;
    }

    packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

    /* === RECEIVE: must match RPU's ControlVsiCamDeviceDeQueBuffer write order ===
     * RPU overwrites in-place at same offsets:
     *   [instanceId] [bufId] baseAddress, baseSize, lockCount,
     *   isFull, index, bufMode, pOwner(*), PicBufMetaData_t
     *
     * (*) NOTE: RPU has a known bug — it writes &pMediaBuf (local pointer)
     *     instead of pMediaBuf->pOwner. Value will be meaningless.
     */
    p_data = packet.payload_data;
    p_data += (sizeof(uint32_t) + sizeof(CamDeviceBufChainId_t));

    memcpy(&((*pMediaBuf)->baseAddress), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&((*pMediaBuf)->baseSize), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&((*pMediaBuf)->lockCount), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&((*pMediaBuf)->isFull), p_data, sizeof(bool_t));
    p_data += sizeof(bool_t);

    memcpy(&((*pMediaBuf)->index), p_data, sizeof(uint8_t));
    p_data += sizeof(uint8_t);

    memcpy(&((*pMediaBuf)->bufMode), p_data, sizeof(BUFF_MODE));
    p_data += sizeof(BUFF_MODE);

	/* pOwner holds the RPU-side MediaBuffer_t* pointer.
	 * APU must pass it back in EnQueBuffer so RPU can enqueue the correct buffer. */
	memcpy(&((*pMediaBuf)->pOwner), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

    /* PicBufMetaData_t is appended at the end by RPU */
    memcpy((*pMediaBuf)->pMetaData, p_data, sizeof(PicBufMetaData_t));

    (*pMediaBuf)->pIplAddress = (*pMediaBuf)->baseAddress;

    return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceEnQueBuffer
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId,
	MediaBuffer_t *pMediaBuf
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx || NULL == pMediaBuf)
		return RET_NULL_POINTER;
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
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	/* SEND: must match RPU ControlVsiCamDeviceEnQueBuffer read order:
	 * instanceId, bufId, baseAddress, baseSize, lockCount,
	 * isFull, index, bufMode, pOwner
	 * (no pIplAddress - RPU does not read it) */
	memcpy(p_data, &(pMediaBuf->baseAddress), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->baseSize), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->lockCount), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->isFull), sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, &(pMediaBuf->index), sizeof(uint8_t));
	packet.payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	memcpy(p_data, &(pMediaBuf->bufMode), sizeof(BUFF_MODE));
	packet.payload_size += sizeof(BUFF_MODE);
	p_data += sizeof(BUFF_MODE);

	memcpy(p_data, &((pMediaBuf)->pOwner), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) {
		if (pMediaBuf->pMetaData)
			osFree(pMediaBuf->pMetaData);
		osFree(pMediaBuf);
		return RET_OUTOFRANGE;
	}

	if (pMediaBuf->pMetaData)
		osFree(pMediaBuf->pMetaData);
	osFree(pMediaBuf);

	result = Send_Command(APU_2_RPU_MB_CMD_ENQUE_BUFFER, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();
	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceGetBufferSize
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId,
	uint32_t *pBufSize
)
{
	if (NULL == hCamDevice || NULL == pBufSize)
		return RET_NULL_POINTER;
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);
	memcpy(p_data, pBufSize, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_BUFFER_SIZE, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pBufSize, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetBufMgmt
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceBufChainId_t bufId,
	BufMgmtHandle_t *bufHandle
)
{
	if (NULL == hCamDevice || NULL == bufHandle)
		return RET_NULL_POINTER;
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
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);
	memcpy(p_data, bufHandle, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_BUFFER_MGMT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(bufHandle, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}
