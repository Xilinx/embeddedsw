/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr,
			    u16 DeviceId);
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
                               u16 DeviceId);
