/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 * @file hal_altera_pci.c
 *
 * Description:
 *   This file implements the IRQ interface realized by the Altera
 *   PCI-Express board access function. You should use it for your
 *   PC implementation in combination with the Altera FPGA board.\n
 *
 *****************************************************************************/

#include <trace.h>
#include "hal_api.h"
#include "hal_i2c.h"
#include "hal_axi_i2c.h"
#include "hal_ps_i2c.h"
#include "sensor_cmd.h"
#include <xiicps.h>

#define IIC_COUNT 3
static HalI2cContext_t HalI2cInstance[IIC_COUNT];

extern XIicPs g_Iic[3];

USE_TRACER(HAL_INFO);
USE_TRACER(HAL_DEBUG);
USE_TRACER(HAL_WARNING);
USE_TRACER(HAL_ERROR);

#if defined (AMP_COMMON_I2C_CONTROLLER)
	extern	osSpinLock I2c0_access_lock;
#endif

static void BindHalI2cOps(HalI2cContext_t *pHalI2cCtx, HalI2cMode_t HalI2cMode)
{

	if ((HalI2cMode > HAL_PS_I2C_MODE) || (HalI2cMode < HAL_AXI_I2C_MODE))
		return NULL;

	if (HalI2cMode == HAL_PS_I2C_MODE)
		pHalI2cCtx->pHalI2cApiOps = HalPsI2cGetOps();
}

static RESULT CheckI2cBusId(uint8_t i2cBusId)
{
	RESULT result = RET_SUCCESS;
	if (i2cBusId >= IIC_COUNT) {
		return RET_UNSUPPORT_ID;
	}
	return result;
}

RESULT HalI2cApuInit(HalI2cConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

	if (pConfig == NULL || (CheckI2cBusId(pConfig->i2cBusId) != RET_SUCCESS))
		return RET_NULL_POINTER;

	HalI2cContext_t *HalI2cCtx = (HalI2cContext_t*)&HalI2cInstance[pConfig->i2cBusId];

	if (HalI2cCtx->refCount) {
		// HalAddRef(HalI2cCtx, HAL_DEV_I2C);
		return result;
	}

	memset(HalI2cCtx, 0, sizeof(HalI2cContext_t));

	HalI2cCtx->i2cBusId = pConfig->i2cBusId;

	BindHalI2cOps(HalI2cCtx, pConfig->HalI2cMode);
	if (HalI2cCtx->pHalI2cApiOps == NULL)
		return RET_NULL_POINTER;

	result = HalI2cCtx->pHalI2cApiOps->pHalI2cInit(HalI2cCtx->i2cBusId);
	if (result != RET_SUCCESS)
		return result;


	HalI2cCtx->refCount = 1;

	return RET_SUCCESS;
}

RESULT HalI2cDeInit(uint8_t i2cBusId)
{
	RESULT result = RET_SUCCESS;

	if (CheckI2cBusId(i2cBusId) != RET_SUCCESS) {
		xil_printf("middha - %s - invalid i2cbusid:%d\n", i2cBusId);
		return RET_UNSUPPORT_ID;
	}

	HalI2cContext_t *HalI2cCtx = (HalI2cContext_t*)&HalI2cInstance[i2cBusId];

	//HalDelRef(HalI2cCtx, HAL_DEV_I2C);
	if (HalI2cCtx->refCount) {
		xil_printf("middha - %s - i2cbusId:%d, yet to delete other ref.\n");
		return result;
	}

	if (RET_SUCCESS != result) {
		xil_printf("%s:mutex destroy failed!(result=%d)\n", __func__, result);
		return result;
	}

	if (HalI2cCtx->pHalI2cApiOps == NULL)
		return RET_NULL_POINTER;

	result = HalI2cCtx->pHalI2cApiOps->pHalI2cDeInit(i2cBusId);
	if (result != RET_SUCCESS)
		return result;

	memset(HalI2cCtx, 0, sizeof(HalI2cContext_t));


	return result;
}

int HalI2cInit(HalI2cConfig_t* pHalI2cConfig)
{
	RESULT result = RET_SUCCESS;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));
	packet.cookie = 0x99;
	packet.type = CMD;

	packet.payload_size = sizeof(HalI2cConfig_t);
	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, pHalI2cConfig, packet.payload_size);
	result = Send_Command(APU_2_RPU_MB_CMD_I2C_INIT, &packet, sizeof(packet), dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result)
		return RET_FAILURE;

	apu_wait_for_ACK(packet.cookie, packet.payload_data);

	return result;
}


RESULT HalReadI2CReg(uint8_t i2cBusId, uint8_t slaveAddr, uint16_t regAddr, uint8_t regWidth,
		     uint16_t *data, uint8_t dataWidth)
{
	RESULT result = RET_SUCCESS;

	if (CheckI2cBusId(i2cBusId) != RET_SUCCESS) {
		return RET_UNSUPPORT_ID;
	}

	HalI2cContext_t *HalI2cCtx = (HalI2cContext_t*)&HalI2cInstance[i2cBusId];
	if (HalI2cCtx->pHalI2cApiOps == NULL)
		return RET_NULL_POINTER;

	result = HalI2cCtx->pHalI2cApiOps->pHalI2cReadReg(i2cBusId, slaveAddr, regAddr, regWidth, data,
		 dataWidth);
	if (result != RET_SUCCESS)
		return result;

	return RET_SUCCESS;
}

RESULT HalWriteI2CReg(uint8_t i2cBusId, uint8_t slaveAddr, uint16_t regAddr, uint8_t regWidth,
		      uint16_t data, uint8_t dataWidth)
{
	RESULT result = RET_SUCCESS;

	if (CheckI2cBusId(i2cBusId) != RET_SUCCESS) {
		xil_printf("middha - %s - invalid i2cbusid:%d\n", i2cBusId);
		return RET_UNSUPPORT_ID;
	}

	HalI2cContext_t *HalI2cCtx = (HalI2cContext_t*)&HalI2cInstance[i2cBusId];
	if (HalI2cCtx->pHalI2cApiOps == NULL) {
		xil_printf("%s (null pointer)\r\n", __func__);
		return RET_NULL_POINTER;
	}

	result = HalI2cCtx->pHalI2cApiOps->pHalI2cWriteReg(i2cBusId, slaveAddr, regAddr, regWidth, data,
		 dataWidth);
	if (result != RET_SUCCESS)
		return result;

	return RET_SUCCESS;
}


RESULT HalOpenI2cDevice
(
	char *pathName
)
{
	int32_t fd = -1;
	// fd = open(pathName, O_RDWR);
	return fd;
}

RESULT HalCloseI2cDevice
(
	int32_t fd
)
{
	RESULT result = RET_SUCCESS;
	// result = close(fd);
	return result;
}

RESULT HalIoCtl
(
	int32_t fd,
	uint32_t slaveAddr
)
{
	RESULT result = RET_SUCCESS;
	// result = ioctl(fd, I2C_SLAVE_FORCE, slaveAddr);
	return result;
}


RESULT HalSensorDrvReadI2CReg
(
	uint32_t slaveAddr,
	uint8_t regWidth,
	uint8_t dataWidth,
	uint16_t regAddr,
	uint16_t *data,
	int32_t fd
)
{
	RESULT result = RET_SUCCESS;

	return result;
}

void vTaskDelay(u16 time_slice)
{
	if (time_slice < 200)
		sleep(1);
	else
		sleep(time_slice / 200);
}
