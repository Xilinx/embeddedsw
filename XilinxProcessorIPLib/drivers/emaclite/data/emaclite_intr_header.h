/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef EMACLITE_INTR_HEADER_H		/* prevent circular inclusions */
#define EMACLITE_INTR_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"


#ifdef XPAR_INTC_0_DEVICE_ID
int EmacLiteIntrExample(XIntc* IntcInstancePtr,
                        XEmacLite* EmacLiteInstPtr,
                        u16 EmacLiteDeviceId,
                        u16 EmacLiteIntrId);
#else
int EmacLiteIntrExample(XScuGic* IntcInstancePtr,
                        XEmacLite* EmacLiteInstPtr,
                        u16 EmacLiteDeviceId,
                        u16 EmacLiteIntrId);

#endif

#ifdef __cplusplus
}
#endif

#endif
