/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xstatus.h"
#include "xil_types.h"

#include "xparameters.h"

#include "xaxidma.h"
#include "xaxiethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#else
#define INTC		XScuGic
#endif

#ifndef SDT
int AxiEthernetSgDmaIntrExample(INTC * IntcInstancePtr,
                          XAxiEthernet * AxiEthernetInstancePtr,
                          XAxiDma * DmaInstancePtr,
                          u16 AxiEthernetDeviceId,
                          u16 AxiDmaDeviceId, u16 AxiEthernetIntrId,
                          u16 DmaRxIntrId, u16 DmaTxIntrId);
#else
int AxiEthernetSgDmaIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				UINTPTR AxiEthernetBaseAddress);
#endif

#ifdef __cplusplus
}
#endif
