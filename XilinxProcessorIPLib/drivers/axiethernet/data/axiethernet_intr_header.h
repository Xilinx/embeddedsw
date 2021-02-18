/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xstatus.h"
#include "xil_types.h"

#include "xparameters.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#else
#define INTC		XScuGic
#endif

int AxiEthernetSgDmaIntrExample(INTC * IntcInstancePtr,
                          XAxiEthernet * AxiEthernetInstancePtr,
                          XAxiDma * DmaInstancePtr,
                          u16 AxiEthernetDeviceId,
                          u16 AxiDmaDeviceId, u16 AxiEthernetIntrId,
                          u16 DmaRxIntrId, u16 DmaTxIntrId);
