/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef DP159_H		 /* prevent circular inclusions */
#define DP159_H		 /* by using protection macros */

#include "xil_types.h"
#include "xparameters.h"
#ifndef versal
#include "xvphy.h"
#else
#include "xhdmiphy1.h"
#endif

// Function prototypes
#ifndef versal
u32 i2c_dp159(XVphy *VphyPtr, u8 QuadId, u64 TxLineRate);
#else
u32 i2c_dp159(XHdmiphy1 *Hdmiphy1Ptr, u8 QuadId, u64 TxLineRate);
#endif
u32 i2c_dp159_write(u8 dev, u8 addr, u8 dat);
u8 i2c_dp159_read(u8 dev, u8 addr);
void i2c_dp159_dump(void);

#endif
