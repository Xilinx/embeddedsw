/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef SYSMONPSU_HEADER_H
#define SYSMONPSU_HEADER_H

/*****************************************************************************/
/**
* @file sysmonpsu_header.h
* @brief Declares the example entry points for the SYSMONPSU driver.
*
******************************************************************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
int SysMonPsuPolledPrintfExample(u16 DeviceId);

#ifdef XPAR_SCUGIC_0_DEVICE_ID
int SysMonPsuIntrExample(XScuGic* XScuGicInstPtr,
 			XSysMonPsu* SysMonInstPtr, 
			u16 SysMonDeviceId,
 			u16 SysMonIntrId);
#endif
/** @endcond */
#endif
