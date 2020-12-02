/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
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

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"

int TmrCtrIntrExample(XIntc* IntcInstancePtr,
                          XTmrCtr* InstancePtr,
                          u16 DeviceId,
                          u16 IntrId,
                          u8 TmrCtrNumber);
#else
#include "xscugic.h"

int TmrCtrIntrExample(XScuGic* IntcInstancePtr,
                          XTmrCtr* InstancePtr,
                          u16 DeviceId,
                          u16 IntrId,
                          u8 TmrCtrNumber);
#endif
#endif

