/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file sysmon_intr_header.h
*
* @brief Declares prototypes for the SysMon interrupt example.
*
*****************************************************************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
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
/** @endcond */
