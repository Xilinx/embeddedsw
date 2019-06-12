/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_rc_enumerate_example.c
*
* This file contains a design example for using XDMA PCIe IP and its driver.
* This is an example to show the usage of driver APIs when XDMA PCIe IP is
* configured as a Root Port.
*
* The example initializes the XDMA PCIe IP and shows how to enumerate the PCIe
* system.
*
* @note
*
* This example should be used only when XDMA PCIe IP is configured as
* root complex.
*
* This code will illustrate how the XDmaPcie IP and its standalone driver can
* be used to:
*	- Initialize a XDMA PCIe IP core built as a root complex
*	- Enumerate PCIe end points in the system
*	- Assign BARs to end points
*	- find capabilities on end point
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
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	Initial version of XDMA PCIe root complex example
*</pre>
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"	/* Defines for XPAR constants */
#include "xdmapcie.h"		/* XDmaPcie level 1 interface */
#include "stdio.h"
#include "xil_printf.h"
#include "sleep.h"


/************************** Constant Definitions ****************************/

/* Parameters for the waiting for link up routine */
#define XDMAPCIE_LINK_WAIT_MAX_RETRIES 		10
#define XDMAPCIE_LINK_WAIT_USLEEP_MIN 		90000

#define XDMAPCIE_DEVICE_ID 	XPAR_XDMAPCIE_0_DEVICE_ID

/*
 * Command register offsets
 */
#define PCIE_CFG_CMD_IO_EN	0x00000001 /* I/O access enable */
#define PCIE_CFG_CMD_MEM_EN	0x00000002 /* Memory access enable */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 /* Bus master enable */
#define PCIE_CFG_CMD_PARITY	0x00000040 /* parity errors response */
#define PCIE_CFG_CMD_SERR_EN	0x00000100 /* SERR report enable */

/*
 * PCIe Configuration registers offsets
 */

#define PCIE_CFG_ID_REG			0x0000 /* Vendor ID/Device ID offset */
#define PCIE_CFG_CMD_STATUS_REG		0x0001 /*
						* Command/Status Register
						* Offset
						*/
#define PCIE_CFG_PRI_SEC_BUS_REG	0x0006 /*
						* Primary/Sec.Bus Register
						* Offset
						*/
#define PCIE_CFG_CAH_LAT_HD_REG		0x0003 /*
						* Cache Line/Latency Timer/
						* Header Type/
						* BIST Register Offset
						*/
#define PCIE_CFG_BAR_0_REG		0x0004 /* PCIe Base Addr 0 */

#define PCIE_CFG_FUN_NOT_IMP_MASK	0xFFFF
#define PCIE_CFG_HEADER_TYPE_MASK	0x00EF0000
#define PCIE_CFG_MUL_FUN_DEV_MASK	0x00800000

#define PCIE_CFG_MAX_NUM_OF_BUS		256
#define PCIE_CFG_MAX_NUM_OF_DEV		1
#define PCIE_CFG_MAX_NUM_OF_FUN		8

#define PCIE_CFG_PRIM_SEC_BUS		0x00070100
#define PCIE_CFG_BAR_0_ADDR		0x00001111



/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, u16 DeviceId);


/************************** Variable Definitions ****************************/

/* Allocate PCIe Root Complex IP Instance */
XDmaPcie XdmaPcieInstance;

/****************************************************************************/
/**
* This function is the entry point for PCIe Root Complex Enumeration Example
*
* @param 	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful.
*
* @note 	None.
*
*****************************************************************************/
int main(void)
{

	int Status;

	/* Initialize Root Complex */
	Status = PcieInitRootComplex(&XdmaPcieInstance, XDMAPCIE_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("XdmaPcie rc enumerate Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Scan PCIe Fabric */
	XDmaPcie_EnumerateFabric(&XdmaPcieInstance);

	xil_printf("Successfully ran XdmaPcie rc enumerate Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function initializes a XDMA PCIe IP built as a root complex
*
*
* @param	XdmaPciePtr is a pointer to an instance of XDmaPcie data
*		structure represents a root complex IP.
* @param 	DeviceId is XDMA PCIe IP unique ID
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note 	None.
*
*
******************************************************************************/
int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, u16 DeviceId)
{
	int Status;
	u32 HeaderData;
	u32 InterruptMask;
	u8  BusNumber;
	u8  DeviceNumber;
	u8  FunNumber;
	u8  PortNumber;

	XDmaPcie_Config *ConfigPtr;

	ConfigPtr = XDmaPcie_LookupConfig(DeviceId);

	Status = XDmaPcie_CfgInitialize(XdmaPciePtr, ConfigPtr,
						ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize PCIe Root Complex"
							"IP Instance\r\n");
		return XST_FAILURE;
	}

	if(!XdmaPciePtr->Config.IncludeRootComplex) {
		xil_printf("Failed to initialize...XDMA PCIE is configured"
							" as endpoint\r\n");
		return XST_FAILURE;
	}


	/* See what interrupts are currently enabled */
	XDmaPcie_GetEnabledInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	/* Make sure all interrupts disabled. */
	XDmaPcie_DisableInterrupts(XdmaPciePtr, XDMAPCIE_IM_ENABLE_ALL_MASK);


	/* See what interrupts are currently pending */
	XDmaPcie_GetPendingInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Just if there is any pending interrupt then clear it.*/
	XDmaPcie_ClearPendingInterrupts(XdmaPciePtr,
						XDMAPCIE_ID_CLEAR_ALL_MASK);

	/*
	 * Read enabled interrupts and pending interrupts
	 * to verify the previous two operations and also
	 * to test those two API functions
	 */

	XDmaPcie_GetEnabledInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	XDmaPcie_GetPendingInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Make sure link is up. */
	int Retries;
	Status = FALSE;
	/* check if the link is up or not */
        for (Retries = 0; Retries < XDMAPCIE_LINK_WAIT_MAX_RETRIES; Retries++) {
		if (XDmaPcie_IsLinkUp(XdmaPciePtr)){
			Status = TRUE;
		}
                usleep(XDMAPCIE_LINK_WAIT_USLEEP_MIN);
	}
	if (Status != TRUE ) {
		xil_printf("Link is not up\r\n");
		return XST_FAILURE;
	}

	xil_printf("Link is up\r\n");

	/*
	 * Read back requester ID.
	 */
	XDmaPcie_GetRequesterId(XdmaPciePtr, &BusNumber,
				&DeviceNumber, &FunNumber, &PortNumber);

	xil_printf("Bus Number is %02X\r\n"
			"Device Number is %02X\r\n"
				"Function Number is %02X\r\n"
					"Port Number is %02X\r\n",
			BusNumber, DeviceNumber, FunNumber, PortNumber);


	/* Set up the PCIe header of this Root Complex */
	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, &HeaderData);

	HeaderData |= (PCIE_CFG_CMD_BUSM_EN | PCIE_CFG_CMD_MEM_EN |
				PCIE_CFG_CMD_IO_EN | PCIE_CFG_CMD_PARITY |
							PCIE_CFG_CMD_SERR_EN);

	XDmaPcie_WriteLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, HeaderData);

	/*
	 * Read back local config reg.
	 * to verify the write.
	 */

	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, &HeaderData);

	xil_printf("PCIe Local Config Space is %8X at register"
					" CommandStatus\r\n", HeaderData);

	/*
	 * Set up Bus number
	 */

	HeaderData = PCIE_CFG_PRIM_SEC_BUS;

	XDmaPcie_WriteLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_PRI_SEC_BUS_REG, HeaderData);

	/*
	 * Read back local config reg.
	 * to verify the write.
	 */
	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_PRI_SEC_BUS_REG, &HeaderData);

	xil_printf("PCIe Local Config Space is %8X at register "
					"Prim Sec. Bus\r\n", HeaderData);

	/* Now it is ready to function */

	xil_printf("Root Complex IP Instance has been successfully"
							" initialized\r\n");

	return XST_SUCCESS;
}
