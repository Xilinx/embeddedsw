/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
