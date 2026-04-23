// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
#define LOGTAG "APU_CTRL"
#include <cam_device_app.h>
//#include <vvdevice.h>
#include "control.h"
extern CamDeviceIspcore_t gCamDevIspcore;

extern uint32_t apuBufferInform[CAMDEV_VIRTUAL_ID_MAX *
				CAMDEV_HARDWARE_ID_MAX][CAMDEV_PIPE_OUTPATH_RAW + 1];

#define APU_RECV_BUF_QUEUE_DEPTH 8

static MediaBuffer_t *apuRecvBufQueue[CAMDEV_VIRTUAL_ID_MAX *
				       CAMDEV_HARDWARE_ID_MAX][CAMDEV_PIPE_OUTPATH_RAW + 1][APU_RECV_BUF_QUEUE_DEPTH];
static volatile uint32_t apuRecvBufHead[CAMDEV_VIRTUAL_ID_MAX *
					CAMDEV_HARDWARE_ID_MAX][CAMDEV_PIPE_OUTPATH_RAW + 1];
static volatile uint32_t apuRecvBufTail[CAMDEV_VIRTUAL_ID_MAX *
					CAMDEV_HARDWARE_ID_MAX][CAMDEV_PIPE_OUTPATH_RAW + 1];

uint32_t CtrlFullBufferInform(void *data)
{
	Payload_packet *packet = (Payload_packet *)data;
	uint8_t *p_data = packet->payload_data;

	uint32_t instanceId;
	uint32_t chainId;

	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(&chainId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	if (instanceId >= CAMDEV_VIRTUAL_ID_MAX * CAMDEV_HARDWARE_ID_MAX ||
	    chainId > CAMDEV_PIPE_OUTPATH_RAW) {
		xil_printf("CtrlFullBufferInform: invalid instanceId %u or chainId %u\r\n",
			   instanceId, chainId);
		return RET_OUTOFRANGE;
	}

	/* Allocate MediaBuffer_t to hold the pre-dequeued buffer from RPU */
	MediaBuffer_t *pBuf = osMalloc(sizeof(MediaBuffer_t));
	if (!pBuf)
		return RET_OUTOFMEM;
	memset(pBuf, 0, sizeof(MediaBuffer_t));

	/* Parse buffer fields sent by RPU's CtrlCamDeviceFullBufferQueNotifyCb */
	memcpy(&pBuf->baseAddress, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(&pBuf->baseSize, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(&pBuf->lockCount, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(&pBuf->isFull, p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);
	memcpy(&pBuf->index, p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);
	memcpy(&pBuf->bufMode, p_data, sizeof(BUFF_MODE));
	p_data += sizeof(BUFF_MODE);
	memcpy(&pBuf->pOwner, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	/* Allocate PicBufMetaData_t - zero-init instead of bulk memcpy
	 * because RPU and APU have different struct layouts
	 * (RPU has extra bufOffset/bufStride fields and larger metaInfo).
	 * Only Type and Layout are at the same offset in both versions.
	 * Data union fields will be populated by the caller from path config. */
	pBuf->pMetaData = (uint32_t)(uintptr_t)osMalloc(sizeof(PicBufMetaData_t));
	if (!pBuf->pMetaData) {
		osFree(pBuf);
		return RET_OUTOFMEM;
	}
	PicBufMetaData_t *pMeta = (PicBufMetaData_t *)(uintptr_t)pBuf->pMetaData;
	memset(pMeta, 0, sizeof(PicBufMetaData_t));
	/* Copy Type (offset 0) and Layout (offset 4) - same in both versions */
	memcpy(&pMeta->Type, p_data, sizeof(PicBufType_t));
	memcpy(&pMeta->Layout, p_data + sizeof(PicBufType_t), sizeof(PicBufLayout_t));

	pBuf->pIplAddress = pBuf->baseAddress;

	/* Enqueue into local ring buffer */
	uint32_t head = apuRecvBufHead[instanceId][chainId];
	uint32_t next = (head + 1) % APU_RECV_BUF_QUEUE_DEPTH;
	if (next == apuRecvBufTail[instanceId][chainId]) {
		/* Queue full - drop buffer */
		xil_printf("CtrlFullBufferInform: queue full [%d][%d]\n", instanceId, chainId);
		osFree((void *)(uintptr_t)pBuf->pMetaData);
		osFree(pBuf);
		return RET_OUTOFRANGE;
	}
	apuRecvBufQueue[instanceId][chainId][head] = pBuf;
	apuRecvBufHead[instanceId][chainId] = next;

	apuBufferInform[instanceId][chainId]++;

	return RET_SUCCESS;
}

RESULT ApuDeQueReceivedBuffer(uint32_t instanceId, uint32_t chainId, MediaBuffer_t **ppBuf)
{
	uint32_t tail = apuRecvBufTail[instanceId][chainId];
	if (tail == apuRecvBufHead[instanceId][chainId]) {
		return RET_FAILURE;
	}
	*ppBuf = apuRecvBufQueue[instanceId][chainId][tail];
	apuRecvBufTail[instanceId][chainId] = (tail + 1) % APU_RECV_BUF_QUEUE_DEPTH;
	return RET_SUCCESS;
}


uint32_t CtrlCamDeviceFusaEventCb(void *data)
{
	RESULT result = RET_SUCCESS;
#if 0
	uint32_t instanceId;
	FwCommonEventId_t commonEventId;
	FwCommonIspFuSaMisVal_t commonIspFuSaMisVal;

	Payload_packet *packet = (Payload_packet *)data;
	bool fastRstEn = false;

	uint8_t *p_data = packet->payload_data;
	memcpy(&instanceId, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(&commonEventId, p_data, sizeof(FwCommonEventId_t));
	p_data += sizeof(FwCommonEventId_t);
	memcpy(&commonIspFuSaMisVal, p_data, sizeof(FwCommonIspFuSaMisVal_t));
	p_data += sizeof(FwCommonIspFuSaMisVal_t);

	CamDeviceContext_t *pCamDeviceContext = (CamDeviceContext_t *)CtrlGetCamDeviceHandle(instanceId);

	fastRstEn = (commonIspFuSaMisVal.fusaLv1MisVal & 0x73ffffff) | (commonIspFuSaMisVal.fusaParityMisVal
		    & 0x000fffff);

	if (0) {
		if (pCamDeviceContext->camDeviceFusaEventCb.pUserContext != NULL) {
			// pCamDeviceContext->camDeviceFusaEventCb.func(commonEventId, &commonIspFuSaMisVal, pCamDeviceContext->camDeviceFusaEventCb.pUserContext);
		} else
			xil_printf("%s: fusa event cb func had not register\r\n");
		VvbenchVvdev_t *caseCtx = (VvbenchVvdev_t *)pCamDeviceContext->camDeviceFusaEventCb.pUserContext;
		xil_printf("\r\nvvbench start fast reset\r\n");
		VsiVvdeviceDelay(3);
		for (uint32_t iLoop = 0; iLoop < 1; ++iLoop) {
			for (int iInstance = 0; iInstance < caseCtx->totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					xil_printf("Skip fast reset stop for instance %d \r\n", iInstance);
					continue;
				}
				result = VsiCamDeviceSwFastStop(pCamDeviceContext);
				if (0 != result) {
					xil_printf("fastReset for instance: %d, VsiCamDeviceSwFastStop failed! result:%d \r\n", iInstance,
						   result);
					return -1;
				} else
					xil_printf("success call fast reset stop for instance %d \r\n", iInstance);
			}

			VsiVvdeviceDelay(2);
			for (int iInstance = 0; iInstance < caseCtx->totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					xil_printf("Skip fast reset hw system reset for instance %d \r\n", iInstance);
					continue;
				}
				result = VsiCamDeviceHwSystemReset(pCamDeviceContext);
				if (0 != result) {
					xil_printf("fastReset VsiCamDeviceHwSystemReset failed! result:%d \r\n", result);
					return -1;
				} else
					xil_printf("success call fast reset hw system reset for instance %d \r\n", iInstance);
			}
			VsiVvdeviceDelay(1);

			for (int iInstance = 0; iInstance < caseCtx->totalInstance; iInstance++) {
				if (caseCtx->instanceCfgCtx[iInstance].instanceEnable == 0) {
					xil_printf("Skip fast reset start for instance %d \r\n", iInstance);
					continue;
				}
				result = VsiCamDeviceSwFastStart(pCamDeviceContext);
				if (0 != result) {
					xil_printf("fastReset for instance: %d, VsiCamDeviceSwFastStart failed! result:%d \r\n", iInstance,
						   result);
					return -1;
				} else
					xil_printf("success call fast reset start for instance %d \r\n", iInstance);
			}
			VsiVvdeviceDelay(3);
		}
		xil_printf("\r\nvvbench end fast reset\r\n");
	}

	if (fastRstEn) {
		pCamDeviceContext->cookie ++;

		Payload_packet sw_rst_packet;
		memset(&sw_rst_packet, 0, sizeof(Payload_packet));

		sw_rst_packet.cookie = pCamDeviceContext->cookie;
		sw_rst_packet.type = CMD;
		sw_rst_packet.payload_size = 0;

		uint8_t *sw_rst_p_data = sw_rst_packet.payload_data;
		memcpy(sw_rst_p_data, &pCamDeviceContext->instanceId, sizeof(uint32_t));
		sw_rst_p_data += sizeof(uint32_t);
		sw_rst_packet.payload_size += sizeof(uint32_t);

		if (sw_rst_packet.payload_size > MAX_ITEM)
			return RET_OUTOFRANGE;

		result = Send_Command(APU_2_RPU_MB_CMD_SOFT_RESET, &sw_rst_packet,
				      sw_rst_packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
		if (0 != result)
			return RET_FAILURE;
		sw_rst_packet.resp_field.error_subcode_t = apu_wait_for_ACK(sw_rst_packet.cookie,
			sw_rst_packet.payload_data); //replace with wait_response();
		result = sw_rst_packet.resp_field.error_subcode_t;
	}

#endif
	return result;
}


CamDeviceHandle_t CtrlGetCamDeviceHandle(uint32_t instanceId)
{
	uint32_t hwId, pvtId;
	hwId = instanceId / CAMDEV_HARDWARE_ID_MAX;
	pvtId = instanceId % CAMDEV_HARDWARE_ID_MAX;
	if (gCamDevIspcore.hCamDevSet[hwId][pvtId] == NULL) {
		xil_printf("%s wrong instanceId or this instance had be released\n", __func__);
		return NULL;
	} else
		return gCamDevIspcore.hCamDevSet[hwId][pvtId];
}

uint32_t CtrlGetCamDeviceInstanceId(CamDeviceHandle_t hCamDevice)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	return pCamDevCtx->instanceId;
}

RESULT ControlSendResponse(void *data, MboxCoreId core_id)
{
	RESULT ret = RET_SUCCESS;

	uint32_t real_size = payload_extra_size;

	Payload_packet packet;
	memcpy(&packet, data, real_size);

	packet.type = RESP;

	//Send the response to RPU
	ret = Send_Response(MB_CMD_RES_SUCCESS, &packet, real_size, dest_cpu_id, core_id);

	return ret;
}
RESULT ControlSendBackData(void *data, MboxCoreId core_id)
{
	RESULT ret = RET_SUCCESS;

	uint32_t real_size = ((Payload_packet *)data)->payload_size + payload_extra_size;
	if (real_size > sizeof(Payload_packet)) {
		xil_printf("ControlSendBackData: payload_size %u exceeds max, rejecting\r\n",
			   ((Payload_packet *)data)->payload_size);
		return RET_OUTOFRANGE;
	}

	Payload_packet packet;
	memcpy(&packet, data, real_size);

	packet.type = RESP;

	//Send back data to APU
	ret = Send_Response(MB_CMD_GET_SUCCESS, &packet, real_size, dest_cpu_id, core_id);

	return ret;
}
