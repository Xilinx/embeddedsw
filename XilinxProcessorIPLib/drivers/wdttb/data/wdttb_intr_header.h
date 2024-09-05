/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file wdttb_intr_header.h
*
* This header file contains the include files used to run the TestApp of
* the Watchdog Timer.
*
* @note
*
* None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
*
* </pre>
*
******************************************************************************/
#ifndef WDDTB_INTR_HEADER_H		/* prevent circular inclusions */
#define WDDTB_INTR_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef XPAR_INTC_0_DEVICE_ID
int WdtTbIntrExample(XIntc *IntcInstancePtr, \
                         XWdtTb *WdtTbInstancePtr, \
                         u16 WdtTbDeviceId, \
                         u16 WdtTbIntrId);
int WinWdtIntrExample(XIntc *IntcInstancePtr, \
                        XWdtTb *WdtTbInstancePtr, \
                        u16 WdtTbDeviceId, \
                        u16 WdtTbIntrId);
#else
int WdtTbIntrExample(XScuGic *IntcInstancePtr, \
                         XWdtTb *WdtTbInstancePtr, \
                         u16 WdtTbDeviceId, \
                         u16 WdtTbIntrId);
int WinWdtIntrExample(XScuGic *IntcInstancePtr, \
                        XWdtTb *WdtTbInstancePtr, \
                        u16 WdtTbDeviceId, \
                        u16 WdtTbIntrId);

#endif

#ifdef __cplusplus
}
#endif

#endif

