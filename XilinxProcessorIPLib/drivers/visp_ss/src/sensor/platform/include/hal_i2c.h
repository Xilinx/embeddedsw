// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#ifndef _HALI2CAPI_H_
#define _HALI2CAPI_H_
#include <return_codes.h>
#include <types.h>
#include <oslayer.h>
#include <dct_assert.h>
#include <buf_defs.h>
#include "hal_api.h"
#include "xparameters.h"

#ifdef FMC_PS_IIC
	#define IIC_DEVICE_ID XPAR_XIICPS_1_DEVICE_ID
	#define IIC_MUX_ADDR		0x74
	#define IIC_MUX_FMC_CHANNEL 0x6
#endif
#ifdef FMC_AXI_IIC
	#define IIC_DEVICE_ID XPAR_DEVICE_ID
	#define AXI_IIC_MUX_ADDR		0x71
	#define AXI_IIC_MUX_FMC_CHANNEL 0x2
	#define IIC_INTR_ID		127
#endif

typedef enum HalI2cMode_e {
	HAL_AXI_I2C_MODE = 0x0000,
	HAL_PS_I2C_MODE = 0x0001,
	DUMMY_HAL_I2C_MODE = 0xDEADFEED
} HalI2cMode_t;

typedef struct HalI2cConfig_s {
	void *hHalI2c;
	uint8_t i2cBusId;
	HalI2cMode_t HalI2cMode;
} HalI2cConfig_t;

typedef struct HalI2cApiOps_s HalI2cApiOps_t;

typedef void *HalI2cHandle_t;

/*****************************************************************************/
/**
 * @brief   hal I2C configuration struct
 *****************************************************************************/
typedef struct HalI2cContext_s {
	uint8_t i2cBusId;
	uint8_t slaveAddr;
	uint8_t regWidth;
	uint8_t dataWidth;
	int32_t fd;
	uint32_t refCount;
	// osMutex            refMutex;
	HalI2cApiOps_t *pHalI2c;
	HalI2cApiOps_t *pHalI2cApiOps;
	void *pPrivateCtx;
} HalI2cContext_t;

typedef RESULT (*HalI2cInit_t) (uint8_t bus_num);
typedef RESULT (*HalI2cDeInit_t) (uint8_t bus_num);
typedef RESULT (*HalI2cReadReg_t) (u8 bus_num, u8 slave_addr, u32 reg_address, u8 reg_addr_size,
				   void *preg_value, u8 datacount);
typedef RESULT (*HalI2cWriteReg_t) (u8 bus_num, u8 slave_addr, u16 reg_address, u8 reg_addr_size,
				    u8 reg_value, u32 databytes);

struct HalI2cApiOps_s {
	const char *pi2cName;

	HalI2cInit_t pHalI2cInit;
	HalI2cDeInit_t pHalI2cDeInit;
	HalI2cReadReg_t pHalI2cReadReg;
	HalI2cWriteReg_t pHalI2cWriteReg;
};

RESULT HalI2cApuInit(HalI2cConfig_t *pConfig);
RESULT HalI2cInit (HalI2cConfig_t *pConfig);
RESULT HalI2cDeInit (uint8_t i2cBusId);
RESULT HalReadI2CReg (uint8_t i2cBusId, uint8_t slaveAddr, uint16_t regAddr, uint8_t regWidth,
		      uint16_t *data, uint8_t dataWidth);
RESULT HalWriteI2CReg (uint8_t i2cBusId, uint8_t slaveAddr, uint16_t regAddr, uint8_t regWidth,
		       uint16_t data, uint8_t dataWidth);

void vTaskDelay(u16 time_slice);

#endif
