/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
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

int AxiEthernetFifoIntrExample(INTC * IntcInstancePtr,
                         XAxiEthernet * AxiEthernetInstancePtr,
                         XLlFifo * FifoInstancePtr,
                         u16 AxiEthernetDeviceId, u16 FifoDeviceId,
                         u16 AxiEthernetIntrId, u16 FifoIntrId);
