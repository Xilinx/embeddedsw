/******************************************************************************
*
* Copyright (C) 2014-2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
#define CSUDMA_DEVICE_ID 	XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#define CSU_SSS_CONFIG_OFFSET	0x008		/**< CSU SSS_CFG Offset */
#define CSUDMA_LOOPBACK_CFG	0x00000050	/**< LOOP BACK configuration
						  *  macro */
#define PMC_SSS_CONFIG_OFFSET	0x500		/**< CSU SSS_CFG Offset */
#define PMCDMA0_LOOPBACK_CFG	0x0000000D	/**< LOOP BACK configuration
						  *  macro for PMCDMA0*/
#define PMCDMA1_LOOPBACK_CFG	0x00000090	/**< LOOP BACK configuration
						  *  macro for PMCDMA1*/

#define SRC_ADDR	0x04200000		/**< Source Address */
#define DST_ADDR	0x04300000		/**< Destination Address */
#define SIZE		0x100			/**< Size of the data to be
						  *  transfered */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


int XCsuDma_PolledExample(u16 DeviceId);

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
	Status = XCsuDma_PolledExample((u16)CSUDMA_DEVICE_ID);
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
int XCsuDma_PolledExample(u16 DeviceId)
{
	int Status;
	XCsuDma_Config *Config;
	u32 Index = 0;
	u32 *SrcPtr = (u32 *)SRC_ADDR;
	u32 *DstPtr = (u32 *)DST_ADDR;
	u32 Test_Data = 0xABCD1234;
	u32 *Ptr = (u32 *)SRC_ADDR;
	u32 EnLast = 0;
	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal)
	if (Config->DmaType != XCSUDMA_DMATYPEIS_CSUDMA)
		XCsuDma_PmcReset(Config->DmaType);
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
	} else if(Config->DmaType == XCSUDMA_DMATYPEIS_PMCDMA0) {
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

	for(Index = 0; Index < SIZE; Index++)
	{
		*Ptr = Test_Data;
		Test_Data += 0x1;
		Ptr++;
	}


	/* Data transfer in loop back mode */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, DST_ADDR, SIZE, EnLast);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, SRC_ADDR, SIZE, EnLast);


	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	/*
	 * Verifying data of transfered by comparing data at source
	 * and address locations.
	 */

	for (Index = 0; Index < SIZE; Index++) {
		if (*SrcPtr != *DstPtr) {
			return XST_FAILURE;
		}
		else {
			SrcPtr++;
			DstPtr++;
		}
	}

	return XST_SUCCESS;
}
