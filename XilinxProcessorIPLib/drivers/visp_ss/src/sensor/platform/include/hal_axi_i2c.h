// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
#include <hal_i2c.h>
#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xscugic.h"

#define IIC_SCLK_RATE		100000


#define TEST_BUFFER_SIZE	8

int Xil_IICWritepolled16(XIicPs *, u8 *, u32, u16, u8);
int Xil_IICWritepolled(XIicPs *, u8 *, u32, u16, u8);

int Xil_IICReadpolled(XIicPs *, u8 *, u16, s32, u8);
int Xil_IICReadpolled_16bit(XIicPs *, u8 *, u16, s32, u8);

HalI2cApiOps_t *HalAxiI2cGetOps(void);

RESULT Xil_I2c_HalI2cWriteReg(u8 bus_num, u8 slave_addr, u16 reg_address, u8 reg_addr_size,
			      u8 reg_value, u32 databytes);
RESULT Xil_I2c_HalI2cReadReg(u8 bus_num, u8 slave_addr, u32 reg_address,
			     u8 reg_addr_size, void *preg_value, u8 datacount);
