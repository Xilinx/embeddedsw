/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_ep_enable_example.c
*
* This file contains a design example for using PS PCIe IP and its
* driver. This is an example to show the usage of driver APIs which configures
* PS PCIe EndPoint.
*
* The example initializes the PS PCIe EndPoint and shows how to use the API's.
*
* This code will illustrate how the XPciePsu  and its standalone driver can
* be used to:
*  - Initialize a PS PCIe bridge core built as an end point
*  - Retrieve root complex configuration assigned to end point
*  - Provides ingress translation setup
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
*  1.0   tk  02/13		Initial version
*</pre>
*******************************************************************************/

/******************************** Include Files *******************************/
#include "xpciepsu_ep.h"
#include "stdio.h"
#include "xil_printf.h"
#include "xparameters.h" /* Defines for XPAR constants */
#include "xpciepsu_common.h"

/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define INGRESS_NUM	0x0		/* Ingress num to setup ingress */
#define BAR_NUM		0x2		/* Bar no to setup ingress */
#define PS_DDR_ADDR	0x1000000	/* 32 or 64 bit PS DDR Addr
						to setup ingress */

/***************************** Function Prototypes ****************************/

int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId);

/**************************** Variable Definitions ****************************/
/* PCIe IP Instance */
static XPciePsu PciePsuInstance;

/******************************************************************************/
/**
* This function is the entry point for PCIe EndPoint Example
*
* @param    None
*
* @return   - XST_SUCCESS if successful
*       - XST_FAILURE if unsuccessful.
*
* @note     None.
*
*******************************************************************************/

int main()
{
	int Status = XST_SUCCESS;
#ifdef XPAR_PSU_PCIE_DEVICE_ID
	XPciePsu_InitEndPoint(&PciePsuInstance, XPAR_PSU_PCIE_DEVICE_ID);

	xil_printf("Waiting for PCIe Link up\r\n");
	XPciePsu_EP_WaitForLinkup(&PciePsuInstance);
	xil_printf("PCIe Link up...\r\n");

	XPciePsu_EP_BridgeInitialize(&PciePsuInstance);
	xil_printf("Bridge Init done...\r\n");

	XPciePsu_EP_WaitForEnumeration(&PciePsuInstance);

	xil_printf("Host driver indicated ready\r\n");
	int result = XPciePsu_EP_SetupIngress(&PciePsuInstance,
			INGRESS_NUM, BAR_NUM, PS_DDR_ADDR);
	if (result == XST_FAILURE) {
		xil_printf("PCIE ingress setup failed\r\n");
	} else {
		xil_printf("PCIE Ingress Test done\r\n");
	}

#endif
	return Status;
}

/******************************************************************************/
/**
* This function initializes a PSU PCIe EndPoint complex.
*
* @param    PciePsuPtr is a pointer to an instance of XPciePsu data
*       structure represents a root complex.
* @param    DeviceId is PSU PCIe root complex unique ID
*
* @return   - XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note     None.
*
*
*******************************************************************************/

int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId)
{
	const XPciePsu_Config *ConfigPtr;
	ConfigPtr = XPciePsu_LookupConfig(DeviceId);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	if (ConfigPtr->PcieMode != XPCIEPSU_MODE_ENDPOINT) {
		xil_printf("Psu pcie mode is not configured as endpoint\r\n");
		return XST_FAILURE;
	}
	XPciePsu_EP_CfgInitialize(PciePsuPtr, ConfigPtr);
	return XST_SUCCESS;
}
