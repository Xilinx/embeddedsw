/*
 * Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 */

#include "pm_csudma.h"

XCsuDma CsuDma;          /* Instance of the Csu_Dma Device */

/*****************************************************************************/
/**
 * @brief	This function initializes the CSU DMA driver.
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XST_FAILURE on failure
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief	This function sets CSU DMA in loop back mode.
 *
 *****************************************************************************/
void PmSetCsuDmaLoopbackMode(void)
{
	/*
	 * Setting CSU_DMA in loop back mode.
	 */
	Xil_Out32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET,
		((Xil_In32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET) & 0xF0000U) |
		  CSUDMA_LOOPBACK_CFG));
}

/*****************************************************************************/
/**
 * @brief	This function performs 64 bit transfer using CSU DMA.
 *
 * @param	DstAddrLow	Lower 32 bits of the destination address
 * @param	DstAddrHigh	Upper 32 bits of the destination address
 * @param	SrcAddrLow	Lower 32 bits of the source address
 * @param	SrcAddrHigh	Upper 32 bits of the source address
 * @param	Size		Number of 4-byte words to be transferred
 *
 *****************************************************************************/
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

/*****************************************************************************/
/**
 * @brief	This function initializes the CSU DMA driver, sets it in
 * 			loop back mode and performs 64 bit transfer.
 *
 * @param	DstAddrLow	Lower 32 bits of the destination address
 * @param	DstAddrHigh	Upper 32 bits of the destination address
 * @param	SrcAddrLow	Lower 32 bits of the source address
 * @param	SrcAddrHigh	Upper 32 bits of the source address
 * @param	Size		Number of 4-byte words to be transferred
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XST_FAILURE on CSU DMA initialization failure.
 *
 *****************************************************************************/
XStatus PmCsuDmaXfer(u32 DstAddrLow, u32 DstAddrHigh, u32 SrcAddrLow,
			 u32 SrcAddrHigh, u32 Size)
{
	s32 Status;

	Status = PmDmaInit();
	if (XST_SUCCESS != Status) {
		goto END;
	}

	PmSetCsuDmaLoopbackMode();

	PmDma64BitTransfer(DstAddrLow, DstAddrHigh, SrcAddrLow, SrcAddrHigh, Size);

END:
	return Status;
}
