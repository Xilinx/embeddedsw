/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int SysMonIntrExample(XIntc* IntcInstancePtr,
                      XSysMon* SysMonInstPtr,
                      u16 SysMonDeviceId,
                      u16 SysMonIntrId,
                      int *Temp);
#else
int SysMonIntrExample(XScuGic* IntcInstancePtr,
                      XSysMon* SysMonInstPtr,
                      u16 SysMonDeviceId,
                      u16 SysMonIntrId,
                      int *Temp);

#endif
