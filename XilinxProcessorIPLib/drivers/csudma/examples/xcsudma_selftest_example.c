/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_selftest_example.c
*
* This file contains an example using the XCsudma driver to do self test
* on the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vnsld  22/10/14 First release
*       ms     04/10/17 Modified filename tag to include the file in doxygen
*                       examples.
* 1.2   adk    11/22/17 Added peripheral test app support.
* 1.14  adk    05/04/23 Added support for system device-tree flow.
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
#define CSUDMA_DEVICE_ID  XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#else
#define CSUDMA_BASEADDR	XPAR_XCSUDMA_0_BASEADDR /* CSU DMA base address */
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


#ifndef SDT
int XCsuDma_SelfTestExample(u16 DeviceId);
#else
int XCsuDma_SelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/


XCsuDma CsuDma;		/**<Instance of the Csu_Dma Device */
#ifndef TESTAPP_GEN
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
	Status = XCsuDma_SelfTestExample((u16)CSUDMA_DEVICE_ID);
#else
	Status = XCsuDma_SelfTestExample(CSUDMA_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CSU_DMA Selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CSU_DMA Selftest Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal test on the XCsudma driver.
*
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
int XCsuDma_SelfTestExample(u16 DeviceId)
#else
int XCsuDma_SelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XCsuDma_Config *Config;

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

	/*
	 * Performs the self-test to check hardware build.
	 */

	Status = XCsuDma_SelfTest(&CsuDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
