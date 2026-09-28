/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file vek385_i2c_bus.h
 * @brief VEK385 I2C1 access: controller setup, PMBUS1 ownership, and byte register read/write.
 */
#ifndef VEK385_I2C_BUS_H
#define VEK385_I2C_BUS_H

#include "xil_types.h"
#include "xstatus.h"

XStatus Vek385_I2cBusInit(void);
void Vek385_I2cBusRelease(void);
XStatus Vek385_I2cWriteRaw(const u8 SlaveAddr, const u8 *Buf, const s32 Len);
XStatus Vek385_I2cAcquirePmbus1(void);
XStatus Vek385_I2cRegReadLocked(const u8 SlaveAddr, u8 Reg, u8 *const Val);

#endif /* VEK385_I2C_BUS_H */
