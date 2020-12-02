/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SYSMONPSU_HEADER_H		/* prevent circular inclusions */
#define SYSMONPSU_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int SysMonPsuPolledPrintfExample(u16 DeviceId);

#ifdef XPAR_SCUGIC_0_DEVICE_ID
int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr,
 			XSysMonPsu* SysMonInstPtr, 
			u16 SysMonDeviceId,
 			u16 SysMonIntrId);
#endif
#endif
