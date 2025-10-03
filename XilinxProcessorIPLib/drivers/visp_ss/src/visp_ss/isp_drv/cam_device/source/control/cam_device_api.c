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
#include <types.h>
#include "cam_device_api.h"
#include "sensor_cmd.h"
#include "mbox_cmd.h"

//#define __section_t(S)          __attribute__((__section__(#S)))
//#define __cam_load_calib        __section_t(.camdevice_load_calib)
//
uint8_t cam_load_calib[160010] __attribute__((section(".cam_load_calib")));

static RESULT MyMemcpy(
	void *dest,
	void *source,
	uint32_t size
)
{
	if (NULL == dest || NULL == source)
		return RET_NULL_POINTER;
	if (size < 0)
		return RET_OUTOFRANGE;
	if (source < dest && source + size > dest)
		return RET_FAILURE;

	char *p_dest = dest;
	char *p_source = source;
	for (uint32_t i = 0; i < size; ++i)
		*p_dest++ = *p_source++;

	return RET_SUCCESS;
}

RESULT SendFirmwareCompability()
{

	RESULT result = RET_SUCCESS;
	int ret = 0;
Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = 0x99;
	packet.type = CMD;
	packet.payload_size = 0;
	bool_t FwCompatFlag = FALSE;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data,&FwCompatFlag , sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;
	ret = Send_Command(APU_2_RPU_MB_LINUX_COMPAT, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return ret;

}


RESULT VsiCamDeviceCreate
(
	CamDeviceConfig_t *pCamConfig,
	CamDeviceHandle_t *pHandleCamDevice
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	if (NULL == pCamConfig || NULL == pHandleCamDevice)
		return RET_NULL_POINTER;
	uint32_t hwId = pCamConfig->ispHwId;

	/*
	 * AMD : Multi-Core Changes
	 * Verifying Multi-core
	 */

#ifdef DEBUG_FLAG
#else
	if (hwId >= CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
#endif

	CamDeviceIspcoreInit();

	/*Get Cam Device Handle*/
	uint32_t virtualId = 0;
	CamDeviceHandle_t hCamDevice = NULL;
	result = CamDeviceRequestInstance(hwId, &hCamDevice, &virtualId);
	if (RET_SUCCESS != result || NULL == hCamDevice)
		return RET_FAILURE;
	memset(hCamDevice, 0, sizeof(CamDeviceContext_t));

	/*Mapping instance id for a single device*/
	uint32_t instanceId = 0;
	result = CamDeviceInstanceIdMapping(hwId, virtualId, &instanceId);
	if (RET_SUCCESS != result)
		return RET_UNSUPPORT_ID;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	pCamDevCtx->ispHwId = hwId;
	pCamDevCtx->ispVtId = virtualId;
	pCamDevCtx->instanceId = instanceId;
	pCamDevCtx->cookie = 0;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, pCamConfig, sizeof(CamDeviceConfig_t));
	packet.payload_size += sizeof(CamDeviceConfig_t);

#ifdef DEBUG_FLAG
	// xil_printf("APU create cam device payload size:%d.\r\n", packet.payload_size);
	// xil_printf("ispHwId: %d\r\n", pCamConfig->ispHwId);
	// xil_printf("inputType: %d\r\n", pCamConfig->inputCfg.inputType);
	// xil_printf("workMode: %d\r\n", pCamConfig->workCfg.workMode);
	// xil_printf("outputType: %d\r\n", pCamConfig->outputCfg.outputType);
	// xil_printf("priority: %d\r\n", pCamConfig->priority);
#endif

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_CREATE_INSTANCE, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return ret;

	*pHandleCamDevice = hCamDevice;

	//#ifdef DEBUG_FLAG
	// xil_printf("APU create cam device waiting for return.\r\n");
	//#endif

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();


#ifdef DEBUG_FLAG
	//xil_printf("APU create cam device successfully return.\r\n");
#endif
	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceDestroy
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

	ret = Send_Command(APU_2_RPU_MB_CMD_DESTORY, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	result = CamDeviceFreeInstance(hCamDevice, pCamDevCtx->ispHwId);
	if (result != RET_SUCCESS)
		return result;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceLoadCalibration
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceCalibIllumType_t defCalibIllum,
	CamDeviceCalibCfg_t *pCalibCfg
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pCalibCfg)
		return (RET_NULL_POINTER);
#ifdef DEBUG_FLAG
	xil_printf("APU enter VsiCamDeviceLoadCalibration.\r\n");
#endif

	pCamDevCtx->cookie ++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = cam_load_calib;

	//	xil_printf("p_data: %x.\r\n", p_data);
	//	xil_printf("&cam_load_calib: %x.\r\n", &cam_load_calib);
	//	xil_printf("cam_load_calib: %x.\r\n", cam_load_calib);
	//	xil_printf("cam_load_calib[0]: %d.\r\n", cam_load_calib[0]);
	//	xil_printf("&cam_load_calib[0]: %x.\r\n", &cam_load_calib[0]);
	//
	//	cam_load_calib[100] = 77;
	//	xil_printf("cam_load_calib[100]: %d.\r\n", cam_load_calib[100]);
	//	xil_printf("&cam_load_calib[100]: %x.\r\n", &cam_load_calib[100]);
	////

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &defCalibIllum, sizeof(CamDeviceCalibIllumType_t));
	packet.payload_size += sizeof(CamDeviceCalibIllumType_t);
	p_data += sizeof(CamDeviceCalibIllumType_t);

	result = MyMemcpy(p_data, pCalibCfg->header.date, sizeof(pCalibCfg->header.date));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(pCalibCfg->header.date) ;
	p_data += sizeof(pCalibCfg->header.date) ;

	result = MyMemcpy(p_data, pCalibCfg->header.creator, sizeof(pCalibCfg->header.creator));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(pCalibCfg->header.creator);
	p_data += sizeof(pCalibCfg->header.creator);

	result = MyMemcpy(p_data, pCalibCfg->header.sensorName, sizeof(pCalibCfg->header.sensorName));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(pCalibCfg->header.sensorName);
	p_data += sizeof(pCalibCfg->header.sensorName);

	result = MyMemcpy(p_data, pCalibCfg->header.generatorVersion,
			  sizeof(pCalibCfg->header.generatorVersion));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(pCalibCfg->header.generatorVersion);
	p_data += sizeof(pCalibCfg->header.generatorVersion);

	result = MyMemcpy(p_data, (pCalibCfg->header.pResolutions)->name,
			  sizeof((pCalibCfg->header.pResolutions)->name));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof((pCalibCfg->header.pResolutions)->name);
	p_data += sizeof((pCalibCfg->header.pResolutions)->name);

	result = MyMemcpy(p_data, (pCalibCfg->header.pResolutions)->id,
			  sizeof((pCalibCfg->header.pResolutions)->id));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof((pCalibCfg->header.pResolutions)->id);
	p_data += sizeof((pCalibCfg->header.pResolutions)->id);

	result = MyMemcpy(p_data, &((pCalibCfg->header.pResolutions)->width), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, &((pCalibCfg->header.pResolutions)->height), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, (pCalibCfg->header.pResolutions)->pFramerate,
			  sizeof(CamDeviceCalibHeaderResolutionFramerate_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibHeaderResolutionFramerate_t);
	p_data += sizeof(CamDeviceCalibHeaderResolutionFramerate_t);

	result = MyMemcpy(p_data, &((pCalibCfg->header.pResolutions)->framerateNumber), sizeof(uint8_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	result = MyMemcpy(p_data, &(pCalibCfg->header.resolutionNumber), sizeof(uint8_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	result = MyMemcpy(p_data, ((pCalibCfg->sensor.awb).pGlobals),
			  sizeof(CamDeviceCalibSensorAwbGlobal_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSensorAwbGlobal_t);
	p_data += sizeof(CamDeviceCalibSensorAwbGlobal_t);

	result = MyMemcpy(p_data, &((pCalibCfg->sensor.awb).globalNumber), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, &((pCalibCfg->sensor.awb).illuminationNumber), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, ((pCalibCfg->sensor.awb).pIlluminations),
			  sizeof(CamDeviceCalibSensorAwbIllumination_t) * (pCalibCfg->sensor.awb).illuminationNumber);
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSensorAwbIllumination_t) *
			       (pCalibCfg->sensor.awb).illuminationNumber;
	p_data += sizeof(CamDeviceCalibSensorAwbIllumination_t) *
		  (pCalibCfg->sensor.awb).illuminationNumber;

	pCalibCfg->sensor.lscNumber = 2;

	result = MyMemcpy(p_data, &(pCalibCfg->sensor.lscNumber), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, (pCalibCfg->sensor.pLsc),
			  sizeof(CamDeviceCalibSensorLsc_t) * (pCalibCfg->sensor.lscNumber));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSensorLsc_t) * (pCalibCfg->sensor.lscNumber);
	p_data += sizeof(CamDeviceCalibSensorLsc_t) * (pCalibCfg->sensor.lscNumber);

	result = MyMemcpy(p_data, &(pCalibCfg->sensor.ccNumber), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, (pCalibCfg->sensor.pCc),
			  sizeof(CamDeviceCalibSensorCc_t) * (pCalibCfg->sensor.ccNumber));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSensorCc_t) * (pCalibCfg->sensor.ccNumber);
	p_data += sizeof(CamDeviceCalibSensorCc_t) * (pCalibCfg->sensor.ccNumber);

	result = MyMemcpy(p_data, &(pCalibCfg->sensor.blsNumber), sizeof(uint16_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(uint16_t);
	p_data += sizeof(uint16_t);

	result = MyMemcpy(p_data, (pCalibCfg->sensor.pBls),
			  sizeof(CamDeviceCalibSensorBls_t) * (pCalibCfg->sensor.blsNumber));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSensorBls_t) * (pCalibCfg->sensor.blsNumber);
	p_data += sizeof(CamDeviceCalibSensorBls_t) * (pCalibCfg->sensor.blsNumber);


	//	result = MyMemcpy(p_data, (pCalibCfg->sensor.pDpcc), sizeof(CamDeviceCalibSensorDpcc_t)*(pCalibCfg->sensor.dpccNumber));
	//	if(result != RET_SUCCESS){
	//	xil_printf("can not do memcpy, error code: %d.\r\n", result);
	//	return result;
	//	}
	//	packet.payload_size += sizeof(CamDeviceCalibSensorDpcc_t)*(pCalibCfg->sensor.dpccNumber);
	//	p_data += sizeof(CamDeviceCalibSensorDpcc_t)*(pCalibCfg->sensor.dpccNumber);

	result = MyMemcpy(p_data, &(pCalibCfg->system), sizeof(CamDeviceCalibSystem_t));
	if (result != RET_SUCCESS) {
		xil_printf("can not do memcpy, error code: %d.\r\n", result);
		return result;
	}
	packet.payload_size += sizeof(CamDeviceCalibSystem_t);
	p_data += sizeof(CamDeviceCalibSystem_t);

	// if(packet.payload_size > MAX_ITEM)
	//	return RET_OUTOFRANGE;

	packet.payload_size = 0;

	result = Send_Command(APU_2_RPU_MB_CMD_LOAD_CALIBRATION, &packet, payload_extra_size, dest_cpu_id,
			      src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSetOutFormat
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeOutPathType_t path,
	CamDevicePipeOutFmt_t *pFmt
)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFmt)
		return (RET_NULL_POINTER);
#ifdef DEBUG_FLAG
	// xil_printf("APU set out format :%d.\r\n");
	// xil_printf("out width: %d.\r\n", pFmt->outWidth);
	// xil_printf("databits: %d.\r\n", pFmt->dataBits);
	// xil_printf("outfmt: %d.\r\n", pFmt->outFormat);
#endif
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
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeOutFmt_t));
	packet.payload_size += sizeof(CamDevicePipeOutFmt_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	ret = Send_Command(APU_2_RPU_MB_CMD_SET_OUT_FORMAT, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSetInFormat
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeInPathType_t path,
	CamDevicePipeInFmt_t *pFmt
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pFmt)
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
	memcpy(p_data, &path, sizeof(CamDevicePipeInPathType_t));
	p_data += sizeof(CamDevicePipeInPathType_t);
	packet.payload_size += sizeof(CamDevicePipeInPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeInFmt_t));
	packet.payload_size += sizeof(CamDevicePipeInFmt_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SET_IN_FORMAT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetOutFormat
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeOutPathType_t path,
	CamDevicePipeOutFmt_t *pFmt
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);

	(pCamDevCtx->cookie)++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeOutFmt_t));
	packet.payload_size += sizeof(CamDevicePipeOutFmt_t);

#ifdef DEBUG_FLAG
	// xil_printf("APU get out format payload cookie:%d.\r\n", packet.cookie);
	// xil_printf("APU get out format payload size:%d.\r\n", packet.payload_size);
#endif

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_OUT_FORMAT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pFmt, p_data, sizeof(CamDevicePipeOutFmt_t));

#ifdef DEBUG_FLAG
	// xil_printf("APU get out format results:%d.\r\n");
	// xil_printf("packet.resp_field.error_subcode_t %d.\r\n", packet.resp_field.error_subcode_t);
	// xil_printf("databits: %d.\r\n", pFmt->dataBits);
	// xil_printf("outfmt: %d.\r\n", pFmt->outFormat);
#endif

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetInFormat
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeInPathType_t path,
	CamDevicePipeInFmt_t *pFmt
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);

	(pCamDevCtx->cookie)++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeInPathType_t));
	packet.payload_size += sizeof(CamDevicePipeInPathType_t);
	p_data += sizeof(CamDevicePipeInPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeInFmt_t));
	packet.payload_size += sizeof(CamDevicePipeInFmt_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_IN_FORMAT, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pFmt, p_data, sizeof(CamDevicePipeInFmt_t));

#ifdef DEBUG_FLAG
	// xil_printf("APU get in format results:%d.\r\n");
	// xil_printf("in width: %d.\r\n", pFmt->inWidth);
	// xil_printf("infmt: %d.\r\n", pFmt->inFormat);
#endif

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSetIspWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeOutPathType_t path,
	const CamDevicePipeIspWindow_t *pIspWindow
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pIspWindow)
		return (RET_NULL_POINTER);
#ifdef DEBUG_FLAG
	xil_printf("APU set isp window :.\r\n");
	xil_printf("cropWindow.hOffset: %d.\r\n", pIspWindow->cropWindow.hOffset);
	xil_printf("cropWindow.height: %d.\r\n", pIspWindow->cropWindow.height);
#endif

	(pCamDevCtx->cookie)++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pIspWindow, sizeof(CamDevicePipeIspWindow_t));
	packet.payload_size += sizeof(CamDevicePipeIspWindow_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SET_ISP_WINDOW, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetIspWindow
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePipeOutPathType_t path,
	CamDevicePipeIspWindow_t *pIspWindow
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);

	(pCamDevCtx->cookie)++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pIspWindow, sizeof(CamDevicePipeIspWindow_t));
	packet.payload_size += sizeof(CamDevicePipeIspWindow_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_ISP_WINDOW, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pIspWindow, p_data, sizeof(CamDevicePipeIspWindow_t));

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceConnectCamera
(
	CamDeviceHandle_t hCamDevice,
	const CamDevicePipeSubmoduleCtrl_u *pSubCtrl
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pSubCtrl)
		return (RET_NULL_POINTER);
#ifdef DEBUG_FLAG
	// xil_printf("APU connect camera :%d.\r\n");
	// xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
	// xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
	// xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
#endif
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
	memcpy(p_data, pSubCtrl, sizeof(CamDevicePipeSubmoduleCtrl_u));
	packet.payload_size += sizeof(CamDevicePipeSubmoduleCtrl_u);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_CONNECT_CAMERA, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceDisconnectCamera
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

	result = Send_Command(APU_2_RPU_MB_CMD_DISCONNECT_CAMERA, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetSoftwareVersion
(
	CamDeviceHandle_t hCamDevice,
	char *pVersionId
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pVersionId)
		return (RET_NULL_POINTER);

	char *version = "V6.0.0";
	memcpy(pVersionId, version, strlen(version));

	return RET_SUCCESS;
}

RESULT VsiCamDeviceStartStreaming
(
	CamDeviceHandle_t hCamDevice,
	uint32_t frames                //0-continue
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
	memcpy(p_data, &frames, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_START_STREAMING, &packet, sizeof(packet), dest_cpu_id,
			      src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceStopStreaming
(
	CamDeviceHandle_t hCamDevice
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	uint32_t hwId = pCamDevCtx->ispHwId;

	/*
	 * AMD : Multi-Core Changes
	 * Verifying Multi-core
	 */
	xil_printf("%s %d ispHwId=%x \n\r", __func__, __LINE__, hwId);
	selectDestinationCore(hwId);

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

	result = Send_Command(APU_2_RPU_MB_CMD_STOP_STREAMING, &packet, sizeof(packet), dest_cpu_id,
			      src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSetPathStreaming
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePathStreamingCfg_t *pConfig
)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDevicePathStreamingCfg_t));
	packet.payload_size += sizeof(CamDevicePathStreamingCfg_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SET_PATH_STREAMING, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetPathStreaming
(
	CamDeviceHandle_t hCamDevice,
	CamDevicePathStreamingCfg_t *pConfig
)
{
	RESULT result = RET_SUCCESS;
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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDevicePathStreamingCfg_t));
	packet.payload_size += sizeof(CamDevicePathStreamingCfg_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_GET_PATH_STREAMING, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pConfig, p_data, sizeof(CamDevicePathStreamingCfg_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceGetHardwareId
(
	CamDeviceHandle_t hCamDevice,
	uint32_t *pHwId
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx) {
		//TRACE(CAM_DEV_API_ERR, " %s: init Cam Device Context firstly!\n", __func__);
		return RET_WRONG_HANDLE;
	}
	if (NULL == pHwId) {
		//TRACE(CAM_DEV_API_ERR, " %s: invalid null pointer!\n", __func__);
		return RET_NULL_POINTER;
	}
	if (pCamDevCtx->ispHwId > CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;

	else {
		*pHwId = pCamDevCtx->ispHwId;
		return 0;
	}
}

RESULT VsiCamDeviceAllocResMemory
(
	CamDeviceHandle_t hCamDevice,
	uint32_t byteSize,
	uint32_t *pBaseAddress,
	void **pIplAddress
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
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
	memcpy(p_data, &byteSize, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	p_data += sizeof(uint32_t);
	memcpy(p_data, pBaseAddress, sizeof(uint32_t));
	packet.payload_size += 2 * sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_ALLOC_RES_MEMORY, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pBaseAddress, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(pIplAddress, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceFreeResMemory
(
	CamDeviceHandle_t hCamDevice,
	uint32_t baseAddress
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
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
	memcpy(p_data, &baseAddress, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_FREE_RES_MEMORY, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceWriteRegister
(
	CamDeviceHandle_t hCamDevice,
	uint32_t address,
	uint32_t value
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
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
	memcpy(p_data, &address, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &value, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_WRITE_REGISTER, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceReadRegister
(
	CamDeviceHandle_t hCamDevice,
	uint32_t address,
	uint32_t *value
)
{
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;
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
	memcpy(p_data, &address, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, value, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_READ_REGISTER, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(value, p_data, sizeof(uint32_t));

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceConfigMetadata
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceMetadataConfig_t *pMetadata
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx)
		return (RET_WRONG_HANDLE);
	if (NULL == pMetadata)
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
	memcpy(p_data, &pMetadata, sizeof(CamDeviceMetadataConfig_t));
	p_data += sizeof(CamDeviceMetadataConfig_t);
	packet.payload_size += sizeof(CamDeviceMetadataConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_IMAGE_SET_METADATA, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceNrRelocEnable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_NrRelocEnable, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceNrRelocDisable
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

	ret = Send_Command(APU_2_RPU_MB_CMD_NrRelocDisable, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSwFastStop
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

	ret = Send_Command(APU_2_RPU_MB_CMD_SwFastStop, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceHwSystemReset
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

	ret = Send_Command(APU_2_RPU_MB_CMD_HwSystemReset, &packet,
			   packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}


RESULT VsiCamDeviceSwFastStart
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

	ret = Send_Command(APU_2_RPU_MB_CMD_SwFastStart, &packet, packet.payload_size + payload_extra_size,
			   dest_cpu_id, src_cpu_id);
	if (0 != ret)
		return RET_FAILURE;

	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceSetIspLowPower
(
	CamDeviceHandle_t hCamDevice,
	bool_t lowPower
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
	memcpy(p_data, &lowPower, sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_SET_ISP_LOW_POWER, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceModuleAutoCtrlSetConfig
(
	CamDeviceHandle_t hCamDevice,
	const CamDeviceModuleAutoCtrlConfig_t	*pConfig
)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDeviceModuleAutoCtrlConfig_t));
	packet.payload_size += sizeof(CamDeviceModuleAutoCtrlConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_ModuleAutoCtrlSetConfig, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie,
					    packet.payload_data); //replace with wait_response();

	return packet.resp_field.error_subcode_t;
}

RESULT VsiCamDeviceModuleAutoCtrlGetConfig
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceModuleAutoCtrlConfig_t	*pConfig
)
{
	RESULT result = RET_SUCCESS;
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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	packet.payload_size += sizeof(CamDeviceModuleAutoCtrlConfig_t);

	if (packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = Send_Command(APU_2_RPU_MB_CMD_ModuleAutoCtrlGetConfig, &packet,
			      packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if (0 != result)
		return RET_FAILURE;
	packet.resp_field.error_subcode_t = apu_wait_for_ACK(packet.cookie, packet.payload_data);
	memcpy(pConfig, p_data, sizeof(CamDeviceModuleAutoCtrlConfig_t));

	return packet.resp_field.error_subcode_t;
}
