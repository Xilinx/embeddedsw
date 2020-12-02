/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xaxipcie_ep_enable_example.c
*
* This file contains a design example for using AXI PCIe IP and its driver.
*
* The example handles AXI PCIe IP when it is configured as an end point.
* It shows how to use the API's.
*
* @note
*
* This code will illustrate how the AXI Pcie IP and its standalone driver can
* be used to:
*   - Initialize a AXI PCIe bridge core built as an end point
*   - Retrieve root complex configuration assigned to end point
*
* We tried to use as much of the driver's API calls as possible to show the
* reader how each call could be used and that probably made the example not
* the shortest way of doing the tasks shown as they could be done.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rkv  03/07/11 Initial version based on PLB PCIE example
* 2.00a rkv  10/19/11 Renamed function call XAxiPcie_GetRequestId to
*		      XAxiPcie_GetRequesterId
* 3.1   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     for proper documentation while generating doxygen.
*
*</pre>
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"	/* Defines for XPAR constants */
#include "xaxipcie.h"		/* XAxiPcie interface */
#include "stdio.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define AXIPCIE_DEVICE_ID 	XPAR_AXIPCIE_0_DEVICE_ID

/*
 * Define the offsets within the PCIE configuration space from the beginning
 * of the PCIE configuration space.
 */
#define PCIE_CFG_ID_REG			0x0000	   /* Vendor ID/Device ID
						    * offset */
#define PCIE_CFG_CMD_STATUS_REG		0x0001	   /* Command/Status Register
						    * Offset */
#define PCIE_CFG_CAH_LAT_HD_REG		0x0003	   /* Cache Line/Latency
						    * Timer/Header Type/BIST
						    * Register Offset */
#define PCIE_CFG_BAR_ZERO_REG		0x0004	   /* Bar 0 offset */


#define PCIE_CFG_CMD_BUSM_EN		0x00000004 /* Bus master enable */

/**************************** Type Definitions ******************************/

#define printf xil_printf

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

int PCIeEndPointInitialize(XAxiPcie *XlnxEndPointPtr, u16 DeviceId);

/************************** Variable Definitions ****************************/

/* Allocate AXI PCIe End Point IP Instance */
XAxiPcie XlnxEndPoint_0;

/****************************************************************************/
/**
* This function is the entry point for PCIe End Point Example
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note 	None.
*
*****************************************************************************/
int main(void)
{
	int Status;

	/* Initialize End Point */
	Status = PCIeEndPointInitialize(&XlnxEndPoint_0,
						XPAR_AXIPCIE_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Axipcie ep enable Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Axipcie ep enable Example\r\n");
	return(XST_SUCCESS);
}

/****************************************************************************/
/**
* This function initializes the AXI PCIE end point IP.
*
* @param	XlnxEndPointPtr is a pointer to an instance of XAxiPcie data
*		structure represents an end point IP.
* @param	DeviceId is AXI PCIe IP unique Device Id
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note 	None.
*
******************************************************************************/
int PCIeEndPointInitialize(XAxiPcie *XlnxEndPointPtr, u16 DeviceId)
{
	int Status;
	u32 HeaderData;
	u32 InterruptMask;
	u8  BusNum;
	u8  DeviceNum;
	u8  FunctionNum;
	u8  PortNumber;
	XAxiPcie_Config *ConfigPtr;

	/* Initialize the driver */
	ConfigPtr = XAxiPcie_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		printf("Failed to initialize PCIe End Point Instance\r\n");
		return XST_FAILURE;
	}

	Status = XAxiPcie_CfgInitialize(XlnxEndPointPtr, ConfigPtr,
						ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		printf("Failed to initialize PCIe End Point Instance\r\n");
		return XST_FAILURE;
	}


	/* See what interrupts are currently enabled */
	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &InterruptMask);
	printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	/* Disable.all interrupts */
	XAxiPcie_DisableInterrupts(XlnxEndPointPtr,
						XAXIPCIE_IM_ENABLE_ALL_MASK);

	/* See what interrupts are currently pending */
	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &InterruptMask);
	printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Clear the pending interrupt */
	XAxiPcie_ClearPendingInterrupts(XlnxEndPointPtr,
						XAXIPCIE_ID_CLEAR_ALL_MASK);

	/*
	 * Read enabled interrupts and pending interrupts to verify the
	 * previous two operations and also to test those two API functions
	 */
	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &InterruptMask);
	printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &InterruptMask);
	printf("Interrupts currently pending are %8X\r\n", InterruptMask);


	/* Make sure link is up. */
	Status = XAxiPcie_IsLinkUp(XlnxEndPointPtr);
	if (Status != TRUE ) {
		printf("Link is not up\r\n");
		return XST_FAILURE;
	}

	printf("Link is up\r\n");

	/* See if root complex has already configured this end point. */

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,
								&HeaderData);

	printf("PCIe Command/Status Register is %08X\r\n", HeaderData);

	if (HeaderData & PCIE_CFG_CMD_BUSM_EN) {

		printf("Root Complex has configured this end point\r\n");
	}
	else {
		printf("Root Complex has NOT yet configured this end"
								" point\r\n");
		return XST_FAILURE;
	}

	XAxiPcie_GetRequesterId(XlnxEndPointPtr, &BusNum,
				&DeviceNum, &FunctionNum, &PortNumber);

	printf("Bus Number is %02X\r\n"
			"Device Number is %02X\r\n"
	 			"Function Number is %02X\r\n"
	 				"Port Number is %02X\r\n",
	 			BusNum, DeviceNum, FunctionNum, PortNumber);

	/* Read my configuration space */
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_ID_REG,
								&HeaderData);
	printf("PCIe Vendor ID/Device ID Register is %08X\r\n",
								HeaderData);


	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,
								&HeaderData);
	printf("PCIe Command/Status Register is %08X\r\n", HeaderData);


	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CAH_LAT_HD_REG,
								&HeaderData);
	printf("PCIe Header Type/Latency Timer Register is %08X\r\n",
								HeaderData);


	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR_ZERO_REG,
								&HeaderData);
	printf("PCIe BAR 0 is %08X\r\n", HeaderData);


	return XST_SUCCESS;
}

