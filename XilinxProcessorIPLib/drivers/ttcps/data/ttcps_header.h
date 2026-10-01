/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file ttcps_header.h
 * @brief Declarations for the TTC PS driver example functions.
*/

#ifndef TTCPS_HEADER_H
#define TTCPS_HEADER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

#ifndef SDT
int TmrInterruptExample(XTtcPs *TtcPsInst, u16 DeviceID, u16 TtcTickIntrID,
			XScuGic *InterruptController);
#else
int TmrInterruptExample(XTtcPs *TtcPsInst, u32 BaseAddr);
#endif

#ifdef __cplusplus
}
#endif

#endif
