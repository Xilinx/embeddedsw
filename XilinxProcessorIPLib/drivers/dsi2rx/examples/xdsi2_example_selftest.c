/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xdsi2rx_example_selftest.c
*
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.0  Kunal 18/4/24 First Revision.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xdsi2rx.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xstatus.h"

/******************** Constant Definitions **********************************/

/* Device hardware build related constants. */
#ifndef TESTAPP_GEN
#define DSI2RX_BASE	XPAR_XDSI_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 DsiSelfTestExample(u32 BASE);

/************************** Variable Definitions *****************************/

/* Device instance definitions */
XDsi2Rx Dsi2Rx;

/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the self test example */
	Status = Dsi2RxSelfTestExample(DSI2RX_BASE);

	if (Status != XST_SUCCESS) {

		xil_printf("DsiSelfTest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran DsiSelfTest Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function will perform DSI self test prints GUI parameters
*
* @param	BASE is the DSI2RX Controller base address.
*
* @return
*c		- XST_SUCCESS if Lane Count match
*		- XST_FAILURE if Lane Count don't match.
*
* @note		None.
*
******************************************************************************/
u32 Dsi2RxSelfTestExample(u32 BASE)
{
	XDsi2Rx_Config *CfgPtr;
	u32 Status = XST_SUCCESS;

	CfgPtr = XDsi2Rx_LookupConfig(BASE);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XDsi2Rx_CfgInitialize(&Dsi2Rx, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XDsi2Rx_SelfTest(&Dsi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
