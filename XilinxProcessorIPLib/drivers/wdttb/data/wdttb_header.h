/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef WDDTB_HEADER_H
#define WDDTB_HEADER_H

/*****************************************************************************/
/**
* @file wdttb_header.h
*
* @brief Declares prototypes for the WDTTB driver examples.
*
* This header file contains the include files used to run the TestApp of
* the Watchdog Timer.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* 4.5   nsk  08/07/19 Fix warnings while generating testapp
* 5.9   ht   07/22/24 Add support for peripheral tests in SDT flow.
* 5.13  vn   09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/** @cond INTERNAL */
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC			XIntc
#else
#define INTC			XScuGic
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif

#ifndef SDT
int WdtTbSelfTestExample(u16 DeviceId);
int GWdtTbExample(u16 DeviceId);
int WdtTbIntrExample(INTC *IntcInstancePtr, XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId, u16 WdtTbIntrId);
int WinWdtTbExample(u16 DeviceId);
int WinWdtIntrExample(INTC *IntcInstancePtr, XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId, u16 WdtTbIntrId);
int WdtTbExample(u16 DeviceId);

#else
int WdtTbSelfTestExample(UINTPTR BaseAddress);
int GWdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
int WdtTbIntrExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
int WinWdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
int WinWdtIntrExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
int WdtTbExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif
