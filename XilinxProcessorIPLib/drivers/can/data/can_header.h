/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef CAN_HEADER_H		/* prevent circular inclusions */
#define CAN_HEADER_H		/* by using protection macros */


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int XCanPolledExample(u16 DeviceId);
#else
int XCanPolledExample(XCan *Can, u16 DeviceId);
#endif

#endif
