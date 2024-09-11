/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file tmrctr_intr_header.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 4.6   mus  02/06/20 Add header files specific to the interrupt controller.
*                     It fixes CR#1053672.
* </pre>
*
******************************************************************************/
#ifndef TMRCTR_INTR_HEADER_H		/* prevent circular inclusions */
#define TMRCTR_INTR_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"

int TmrCtrIntrExample(XIntc* IntcInstancePtr,
                          XTmrCtr* InstancePtr,
                          u16 DeviceId,
                          u16 IntrId);
#else
#include "xscugic.h"

int TmrCtrIntrExample(XScuGic* IntcInstancePtr,
                          XTmrCtr* InstancePtr,
                          u16 DeviceId,
                          u16 IntrId);
#endif

#ifdef __cplusplus
}
#endif

#endif

