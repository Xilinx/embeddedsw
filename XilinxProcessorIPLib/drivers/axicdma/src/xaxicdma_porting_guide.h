/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxicdma_porting_guide.h
* @addtogroup axicdma_v4_8
* @{
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
/** @} */
