/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xaxipcie_ep_cdma_example.c
*
* This file contains a design example for using AXI PCIe IP and its
* driver.
*
* The example handles AXI PCIe IP when it is configured as an end point.
* It shows how to transfer data between system memory and end point memory.
* The user has to enter both addresses for source and destination.
* One of the addresses should be in system memory and the other one in
* the memory local to an end point(mapped to memory space of the system).
*
* This example assumes that there is an AXI CDMA IP in the system. The user
* has to specify the Source, Destination and the Length of the DMA transfer
* which are valid for this system and are defined AXICDMA_SRC_ADDR,
* AXICDMA_DEST_ADDR and AXICDMA_LENGTH respectively in this example.
*
*
* @note
*
* This code will illustrate how the AXI Pcie IP and its standalone driver can
* be used to:
*   - Initialize a PCIe bridge core built as an end point
*   - Retrieve root complex configuration assigned to end point
*   - Move data form system memory to end point memory using AXICDMA IP.
*
* We tried to use as much of the driver's API calls as possible to show the
* reader how each call could be used and that probably made the example not
* the shortest way of doing the tasks shown as they could be done.
*
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rkv  03/07/11 Initial version based on PLB PCIE example
* 2.00a nm   10/19/11 Renamed function call XAxiPcie_GetRequestId to
*		      XAxiPcie_GetRequesterId
* 3.1   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     for proper documentation while generating doxygen.
*</pre>
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"	/* Defines for XPAR constants */
#include "xaxipcie.h"		/* XAxiPcie interface */
#include "xaxicdma.h"		/* AXICDMA interface */
#include "stdio.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define AXIPCIE_DEVICE_ID 	XPAR_AXIPCIE_0_DEVICE_ID
#define AXIDMA_DEVICE_ID	XPAR_AXICDMA_0_DEVICE_ID

/*
 * AXICDMA Transfer Parameters. These have to be defined properly based
 * on the HW system.
 */
#define AXICDMA_SRC_ADDR	0x48000000	/* Source Address */
#define AXICDMA_DEST_ADDR	0xD0000000	/* Destination Address */
#define AXICDMA_LENGTH		0x400		/* Length */


/*
 * Define the offset within the PCIE configuration space from
 * the beginning of the PCIE configuration space.
 */
#define PCIE_CFG_ID_REG			0x0000	/* Vendor ID/Device ID
						 * offset */
#define PCIE_CFG_CMD_STATUS_REG		0x0001	/* Command/Status Register
						 * Offset */
#define PCIE_CFG_CAH_LAT_HD_REG		0x0003	/* Cache Line/Latency
						 * Timer/Header Type/BIST
						 * Register Offset */
#define PCIE_CFG_BAR_ZERO_REG		0x0004	/* Bar 0 offset */


#define PCIE_CFG_CMD_BUSM_EN		0x00000004 /* Bus master enable */



/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

int PCIeEndPointInitialize(XAxiPcie *XlnxEndPointPtr, u16 DeviceId);
int DmaDataTransfer(u16 CdmaID);

/************************** Variable Definitions ****************************/

/* Allocate AXI PCIe End Point IP Instance */
XAxiPcie XlnxEndPoint_0;

/* Allocate AXI CDMA IP Instance */
XAxiCdma CdmaInstance;

/****************************************************************************/
/**
* This function is the entry point for AXI PCIe End Point with AXI CDMA Example
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	int Status;

	/* Initialize End Point */
	Status = PCIeEndPointInitialize(&XlnxEndPoint_0, AXIPCIE_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return (XST_FAILURE);
	}


	/* Use AXICDMA to transfer data to/from end point. */
	Status = DmaDataTransfer(AXIDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Axipcie ep cdma Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Axipcie ep cdma Example\r\n");
	return(0);
}

/****************************************************************************/
/**
* This initialize an IP built as an end point.
*
* @param	XlnxEndPointPtr is a pointer to an instance of XAxiPcie data
*		structure represents an end point IP.
* @param	DeviceId is PCIe IP unique ID
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None.
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

	/* Initialization of the driver */
	ConfigPtr = XAxiPcie_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		xil_printf("Failed to initialize PCIe End Point Instance\r\n");
		return XST_FAILURE;
	}


	Status = XAxiPcie_CfgInitialize(XlnxEndPointPtr, ConfigPtr,
						ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize PCIe End Point Instance\r\n");
		return Status;
	}


	/* See what interrupts are currently enabled */
	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	/* Disable.all interrupts */
	XAxiPcie_DisableInterrupts(XlnxEndPointPtr,
						XAXIPCIE_IM_ENABLE_ALL_MASK);

	/* See what interrupts are currently pending */
	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Clear the pending interrupt */
	XAxiPcie_ClearPendingInterrupts(XlnxEndPointPtr,
						XAXIPCIE_ID_CLEAR_ALL_MASK);

	/*
	 * Read enabled interrupts and pending interrupts
	 * to verify the previous two operations and also
	 * to test those two API functions
	 */
	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);


	/* Make sure link is up. */
	Status = XAxiPcie_IsLinkUp(XlnxEndPointPtr);
	if (Status != TRUE ) {
		xil_printf("Link is not up\r\n");
		return XST_FAILURE;
	}


	xil_printf("Link is up\r\n");


	/* See if root complex has already configured this end point. */

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,
								&HeaderData);

	xil_printf("PCIe Command/Status Register is %08X\r\n", HeaderData);

	if (HeaderData & PCIE_CFG_CMD_BUSM_EN) {

		xil_printf("Root Complex has configured this end point\r\n");
	}
	else {
		xil_printf("Root Complex has NOT yet configured this"
							" end point\r\n");
		return XST_FAILURE;
	}

	XAxiPcie_GetRequesterId(XlnxEndPointPtr, &BusNum,
				&DeviceNum, &FunctionNum, &PortNumber);

	xil_printf("Bus Number is %02X\r\n"
				"Device Number is %02X\r\n"
				"Function Number is %02X\r\n"
				"Port Number is %02X\r\n",
				BusNum, DeviceNum, FunctionNum, PortNumber);

	/* Read my configuration space */
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_ID_REG,
								&HeaderData);
	xil_printf("PCIe Vendor ID/Device ID Register is %08X\r\n",
								HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,
								&HeaderData);

	xil_printf("PCIe Command/Status Register is %08X\r\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CAH_LAT_HD_REG,
								&HeaderData);

	xil_printf("PCIe Header Type/Latency Timer Register is %08X\r\n",
								HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR_ZERO_REG,
								&HeaderData);

	xil_printf("PCIe BAR 0 is %08X\r\n", HeaderData);


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function transfers data from Source Address to Destination Address
* using the AXI CDMA.
* User has to specify the Source Address, Destination Address and Transfer
* Length in AXICDMA_SRC_ADDR, AXICDMA_DEST_ADDR and AXICDMA_LENGTH defines
* respectively.
*
* @param	DeviceId is device ID of the XAxiCdma Device.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE.if unsuccessful.
*
* @note		If the hardware system is not built correctly, this function
*		may never return to the caller.
*
******************************************************************************/
int DmaDataTransfer (u16 DeviceID)
{
	int Status;
	volatile int Error;
	XAxiCdma_Config *ConfigPtr;

	Error = 0;

	/*
	 * Make sure we have a valid addresses for Src and Dst.
	 */
	if (AXICDMA_SRC_ADDR == 0) {
		return XST_FAILURE;
	}

	if (AXICDMA_DEST_ADDR == 0) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the AXI CDMA IP.
	 */
	ConfigPtr = XAxiCdma_LookupConfig(DeviceID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(&CdmaInstance,
				ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Reset the AXI CDMA device.
	 */
	XAxiCdma_Reset(&CdmaInstance);

	/*
	 * Disable AXI CDMA Interrupts
	 */
	XAxiCdma_IntrDisable(&CdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	/*
	 * Start Transferring Data from source to destination in polled mode
	 */
	XAxiCdma_SimpleTransfer (&CdmaInstance, AXICDMA_SRC_ADDR,
					AXICDMA_DEST_ADDR, AXICDMA_LENGTH, 0, 0);

	/*
	 * Poll Status register waiting for either Completion or Error
	 */
	while (XAxiCdma_IsBusy(&CdmaInstance));

	Error = XAxiCdma_GetError(&CdmaInstance);

	if (Error != 0x0) {

		xil_printf("AXI CDMA Transfer Error =  %8.8x\r\n");
		return XST_FAILURE;
	}

	xil_printf("AXI CDMA Transfer is Complete\r\n");


	return XST_SUCCESS;
}

