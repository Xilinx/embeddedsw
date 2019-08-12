/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
*
*******************************************************************************/
/******************************************************************************/
/**
* @file xpciepsu_rc_enumerate_example.c
*
* This file contains a design example for using PS PCIe root complex and its
* driver. This is an example to show the usage of driver APIs which configures
* PS PCIe root complex.
*
* The example initializes the PS PCIe root complex and shows how to enumerate
* the PCIe system.
*
* This code will illustrate how the XPciePsu  and its standalone driver can
* be used to:
*  - Initialize a PS PCIe root complex
*  - Enumerate PCIe end points in the system
*  - Assign BARs to endpoints
*  - find capablities on end point
*
* Please note that this example enumerates and initializes PCIe end points
* only.
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
*  1.0   BS  21/08		Initial version
*</pre>
*******************************************************************************/

/******************************** Include Files *******************************/

#include "xpciepsu.h"
#include "stdio.h"
#include "xil_printf.h"
#include "xparameters.h" /* Defines for XPAR constants */
#include "xpciepsu_common.h"
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/

/***************************** Function Prototypes ****************************/

int PcieInitRootComplex(XPciePsu *PciePsuPtr, u16 DeviceId);

/**************************** Variable Definitions ****************************/

/* PCIe Root Complex IP Instance */
static XPciePsu PciePsuInstance;

/******************************************************************************/
/**
* This function is the entry point for PCIe Root Complex Enumeration Example
*
* @param    None
*
* @return   - XST_SUCCESS if successful
*       - XST_FAILURE if unsuccessful.
*
* @note     None.
*
*******************************************************************************/
int main(void)
{
	int Status;

	/* Initialize Root Complex */
#ifdef XPAR_PSU_PCIE_DEVICE_ID
	Status = PcieInitRootComplex(&PciePsuInstance, XPAR_PSU_PCIE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Psu pcie Root Complex Enumerate "
			"Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Scan PCIe Bus */
	XPciePsu_EnumerateBus(&PciePsuInstance);

	xil_printf("\r\nSuccessfully ran PSU PCIe Root Complex "
			"Enumerate Example\r\n");
#endif
	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function initializes a PSU PCIe root complex.
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
int PcieInitRootComplex(XPciePsu *PciePsuPtr, u16 DeviceId)
{
	int Status;

	XPciePsu_Config *ConfigPtr;

	ConfigPtr = XPciePsu_LookupConfig(DeviceId);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (ConfigPtr->PcieMode == XPCIEPSU_MODE_ENDPOINT) {
		xil_printf("Failed to initialize... PCIE PSU is configured"
							" as endpoint\r\n");
		return XST_FAILURE;
	}
	Status = XPciePsu_CfgInitialize(PciePsuPtr, ConfigPtr,
					ConfigPtr->BrigReg);

	if (Status != XST_SUCCESS) {
		xil_printf(
			"Failed to initialize PCIe Root Complex IP "
			"Instance\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
