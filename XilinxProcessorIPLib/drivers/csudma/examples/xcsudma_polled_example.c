/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_polled_example.c
*
* This file contains an example using the XCsuDma driver in polled mode.
*
* This function works in loop back mode and tests whether transfer of data is
* completed properly or not.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vnsld   22/10/14 First release
* 1.4   adk     04/12/17 Added support for PMC DMA.
* 1.14  adk     04/05/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xparameters.h"

/************************** Function Prototypes ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define CSUDMA_DEVICE_ID 	XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#else
#define CSUDMA_BASEADDR		XPAR_XCSUDMA_0_BASEADDR /* CSU DMA Baseaddress */
#endif
#define CSU_SSS_CONFIG_OFFSET	0x008		/**< CSU SSS_CFG Offset */
#define CSUDMA_LOOPBACK_CFG	0x00000050	/**< LOOP BACK configuration
						  *  macro */
#define PMC_SSS_CONFIG_OFFSET	0x500		/**< CSU SSS_CFG Offset */
#define PMCDMA0_LOOPBACK_CFG	0x0000000D	/**< LOOP BACK configuration
						  *  macro for PMCDMA0*/
#define PMCDMA1_LOOPBACK_CFG	0x00000090	/**< LOOP BACK configuration
						  *  macro for PMCDMA1*/

#define SIZE		0x100			/**< Size of the data to be
						  *  transfered */
#if defined(__ICCARM__)
#pragma data_alignment = 64
u32 SrcBuf[SIZE]; /**< Destination buffer */
#pragma data_alignment = 64
u32 DstBuf[SIZE]; /**< Source buffer */
#else
u32 SrcBuf[SIZE] __attribute__ ((aligned (64)));	/**< Destination buffer */
u32 DstBuf[SIZE] __attribute__ ((aligned (64)));	/**< Source buffer */
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


#ifndef SDT
int XCsuDma_PolledExample(u16 DeviceId);
#else
int XCsuDma_PolledExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/


XCsuDma CsuDma;		/**<Instance of the Csu_Dma Device */
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
#ifndef SDT
	Status = XCsuDma_PolledExample((u16)CSUDMA_DEVICE_ID);
#else
	Status = XCsuDma_PolledExample(CSUDMA_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CSU_DMA Polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CSU_DMA Polled Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function performs data transfer in loop back mode in polled mode
* and verify the data.
*
* @param	DeviceId is the XPAR_<CSUDMA Instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int XCsuDma_PolledExample(u16 DeviceId)
#else
int XCsuDma_PolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XCsuDma_Config *Config;
	u32 Index = 0;
	u32 *SrcPtr = SrcBuf;
	u32 *DstPtr = DstBuf;
	u32 Test_Data = 0xABCD1234;
	u32 *Ptr = SrcBuf;
	u32 EnLast = 0;
	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XCsuDma_LookupConfig(DeviceId);
#else
	Config = XCsuDma_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal)
	if (Config->DmaType != XCSUDMA_DMATYPEIS_CSUDMA) {
		XCsuDma_PmcReset(Config->DmaType);
	}
#endif

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(&CsuDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setting CSU_DMA in loop back mode.
	 */
	if (Config->DmaType == XCSUDMA_DMATYPEIS_CSUDMA) {
		Xil_Out32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET,
			  ((Xil_In32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET) & 0xF0000) |
			   CSUDMA_LOOPBACK_CFG));
#if defined (versal)
	} else if (Config->DmaType == XCSUDMA_DMATYPEIS_PMCDMA0) {
		Xil_Out32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET,
			  ((Xil_In32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET) & 0xFF000000) |
			   PMCDMA0_LOOPBACK_CFG));
	} else {
		Xil_Out32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET,
			  ((Xil_In32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET) & 0xFF000000) |
			   PMCDMA1_LOOPBACK_CFG));
#endif
	}

	/* Data writing at source address location */

	for (Index = 0; Index < SIZE; Index++) {
		*Ptr = Test_Data;
		Test_Data += 0x1;
		Ptr++;
	}


	/* Data transfer in loop back mode */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, (UINTPTR)DstPtr, SIZE, EnLast);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (UINTPTR)SrcPtr, SIZE, EnLast);


	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	/*
	 * Verifying data of transfered by comparing data at source
	 * and address locations.
	 */

	/* Cache Operations after transfer completion
	 * No action required for PSU_PMU.
	 * Perform cache operations on ARM64 and R5
	 */
#if defined(ARMR52)
	Xil_DCacheInvalidateRange((INTPTR)DstPtr, SIZE * 4);
#elif defined(ARMR5)
	Xil_DCacheFlushRange((INTPTR)DstPtr, SIZE * 4);
#endif
#if defined(__aarch64__)
	Xil_DCacheInvalidateRange((INTPTR)DstPtr, SIZE * 4);
#endif

	for (Index = 0; Index < SIZE; Index++) {
		if (*SrcPtr != *DstPtr) {
			return XST_FAILURE;
		} else {
			SrcPtr++;
			DstPtr++;
		}
	}

	return XST_SUCCESS;
}
