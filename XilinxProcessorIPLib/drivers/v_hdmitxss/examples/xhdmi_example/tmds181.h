/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef TMDS181_H		 /* prevent circular inclusions */
#define TMDS181_H		 /* by using protection macros */

#include "xil_types.h"
#include "xparameters.h"
#include "xhdmiphy1.h"

// Function prototypes
u8 i2c_tmds181_read(u8 dev, u8 addr);
u32 i2c_tmds181_write(u8 dev, u8 addr, u8 dat);
void i2c_tmds181_dump(void);

#endif
