/******************************************************************************
*
* (c) Copyright 2010-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xaxicdma_porting_guide.h
*
* This is a guide on how to move from using the XPS Central DMA driver,
* dmacentral, to use xaxidma driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/18/10 First release
* </pre>
*
* <b>Overview</b>
*
* The AXI Central DMA engine is to replace the XPS Central DMA engine using the
* AXI bus interface.
*
* The AXI Central DMA hardware is different from  the XPS Central DMA engine.
* The APS Central DMA engine only supports simple mode transfer, where only one
* buffer transfer can be sumitted to the hardware each time.
*
* The AXI Central DMA engine, on the other hand, also supports scatter gather
* transfers, where each submission to the hardware can have multiple buffers.
* Each transfer buffer is defined by a buffer descriptor (BD).
*
* The AXI Central DMA engine can be configured at build time to be in simple
* mode only, which is similar to the XPS Central DMA. It can also be configured
* as hybrid mode, where scatter gather mode is also supported.
*
* <b>Simple Mode API Difference</b>
*
* The difference from the driver API level is as the following:
*
* - User defined interrupt handler is no longer directly connected to the
*   interrupt controller. Instead, it is registered as a call back function
*   when the submit the transfer:
*
* <pre>
*   XAxiCdma_SimpleTransfer(InstancePtr, TxAddr, RxAddr, Length, CallBackFn,
*        CallBackRef);
* </pre>
*
*   Please refer to example_simple_intr.c to see how it is used.
*
* - Transfer submission API is different:
*
*   Before:
* <pre>
*   XDmaCentral_Transfer(...)
* </pre>
*
*   Now:
* <pre>
*   XAxiCdma_SimpleTransfer(...)
* </pre>
*
* Note that arguments list is also different. Please see example_simple_poll.c
* or example_simple_intr.c for how to submit simple transfers.
*
* - Polling for transfer completion is different:
*
*   Before:
* <pre>
*   do {
*       RegValue = XDmaCentral_GetStatus(DmaInstance);
*   }
*   while ((RegValue & XDMC_DMASR_BUSY_MASK) == XDMC_DMASR_BUSY_MASK);
* </pre>
*
*   Now:
*
* <pre>
*   while (XAxiCdma_IsBusy(InstancePtr)) {
*      ;
*   }
* </pre>
*
******************************************************************************/
