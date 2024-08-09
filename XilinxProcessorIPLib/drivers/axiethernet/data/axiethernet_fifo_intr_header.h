/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xstatus.h"
#include "xil_types.h"
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#else
#define INTC		XScuGic
#endif

int AxiEthernetFifoIntrExample(INTC * IntcInstancePtr,
                         XAxiEthernet * AxiEthernetInstancePtr,
                         XLlFifo * FifoInstancePtr,
                         u16 AxiEthernetDeviceId, u16 FifoDeviceId,
                         u16 AxiEthernetIntrId, u16 FifoIntrId);

#ifdef __cplusplus
}
#endif
