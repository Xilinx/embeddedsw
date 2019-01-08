/*
 * Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
 */

#include "pm_csudma.h"

XCsuDma CsuDma;          /* Instance of the Csu_Dma Device */

XStatus PmDmaInit(void)
{
	s32 Status = XST_SUCCESS;
	XCsuDma_Config *Config;

	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig(CSUDMA_DEVICE_ID);
	if (NULL == Config) {
		Status = XST_FAILURE;
		goto Done;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto Done;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(&CsuDma);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto Done;
	}

Done:
	return Status;
}

void PmSetCsuDmaLoopbackMode(void)
{
	/*
	 * Setting CSU_DMA in loop back mode.
	 */
	Xil_Out32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET,
		((Xil_In32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET) & 0xF0000U) |
		  CSUDMA_LOOPBACK_CFG));
}

void PmDma64BitTransfer(u32 DstAddrLow, u32 DstAddrHigh,
			 u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size)
{
	XCsuDma_64BitTransfer(&CsuDma, XCSUDMA_DST_CHANNEL, DstAddrLow,
			      DstAddrHigh, Size, 0);
	XCsuDma_64BitTransfer(&CsuDma, XCSUDMA_SRC_CHANNEL, SrcAddrLow,
			      SrcAddrHigh, Size, 0);

	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);
}
