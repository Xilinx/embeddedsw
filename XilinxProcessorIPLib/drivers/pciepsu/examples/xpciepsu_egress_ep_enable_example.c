/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_egress_ep_enable_example.c
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
*  - Provides Egress translation setup
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
#include <xscugic.h>
#include <xil_exception.h>
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define EGRESS_NUM	0x1		/* Egress num */
#ifndef SDT
	#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#endif
#define PS_PCIE_AXI_INTR_ID	(117U + 32U)

/***************************** Function Prototypes ****************************/

#ifndef SDT
int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId);
#else
int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, UINTPTR BrigReg);
#endif

void XPciePsu_EP_IntrHandler(XPciePsu *PciePsuPtr);
int XPciePsu_EP_InitIntr(void);

/**************************** Variable Definitions ****************************/
/* PCIe IP Instance */
static XPciePsu PciePsuInstance;
XScuGic INTCinst;

/******************************************************************************/
/**
* This function handles doorbell interrupts for PCIE Endpoint
*
* @param   PciePsuPtr is a pointer to an instance of XPciePsu data
*
* @return  -None
*
* @note    None
*
*********************************************************************************/

void XPciePsu_EP_IntrHandler(XPciePsu *PciePsuPtr)
{
	u32 val, size;

	XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
			DMA0_CHAN_AXI_INTR_STATUS + 0x80,
			AXI_INTR_STATUS);
	val=XPciePsu_ReadReg(PciePsuPtr->Config.DmaBaseAddr,
			HANDSHAKE_DONE_OFFSET);
	xil_printf("In Interrupt handler Value @SCRATCH0 = %x  \n", val);

	if (val == EGRESS_TEST_DONE)
	{
		 val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,EGRESS0_CONTROL);
		if (val & 0x00000001U) {
			size = (1 << EGRESS_SIZE_ENCODING) * (1 << EGRESS_MIN_SIZE);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET + 0x80,
					PCIE_INTR_STATUS);
		}
		else {
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET + 0x80,
					PCIE_INTR_STATUS);
		}
	}
}

/******************************************************************************/
/**
* This function Initializes interrupts for PCIe EndPoint
*
* @param    None
*
* @return   - XST_SUCCESS if successful
*           - XST_FAILURE if unsuccessful
*
* @note     None
*
********************************************************************************/

int XPciePsu_EP_InitIntr(void)
{
	XScuGic_Config *IntcConfig;
	int Status;

#ifndef SDT
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
#else
	IntcConfig = XScuGic_LookupConfig(XPAR_XSCUGIC_0_BASEADDR);
#endif
	if (NULL == IntcConfig) {
			return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&INTCinst, IntcConfig, IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &INTCinst);

	Status = XScuGic_Connect(&INTCinst, PS_PCIE_AXI_INTR_ID, (Xil_ExceptionHandler) XPciePsu_EP_IntrHandler, (void *) &PciePsuInstance);
	if (Status != XST_SUCCESS)
			return XST_FAILURE;
	XScuGic_Enable(&INTCinst, PS_PCIE_AXI_INTR_ID);

	Xil_ExceptionEnable();

	return Status;
}

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
	int Val;

#ifndef SDT
	XPciePsu_InitEndPoint(&PciePsuInstance, XPAR_PSU_PCIE_DEVICE_ID);
#else
	XPciePsu_InitEndPoint(&PciePsuInstance, XPAR_PCIE_BASEADDR);
#endif
	XPciePsu_EP_InitIntr();
	xil_printf("Waiting for PCIe Link up\r\n");
	XPciePsu_EP_WaitForLinkup(&PciePsuInstance);
	xil_printf("PCIe Link up...\r\n");

	XPciePsu_Egress_EP_BridgeInitialize(&PciePsuInstance);
	xil_printf("Bridge Init done...\r\n");

	do {
		Val = XPciePsu_ReadReg(HANDSHAKE_DONE_OFFSET, 0U);
	} while (Val != HANDSHAKE_DONE);

	xil_printf("Host driver indicated ready\r\n");
	int result = XPciePsu_EP_SetupEgress(&PciePsuInstance,
			EGRESS_NUM);
	if (result == XST_FAILURE) {
		xil_printf("PCIE Egress setup failed\r\n");
	} else {
		xil_printf("PCIE Egress Test done\r\n");
	}

	XPciePsu_WriteReg(HANDSHAKE_DONE_OFFSET, 0U, 0x1234ABCD);
	xil_printf("Handshake Done\n");

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
#ifndef SDT
int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId)
#else
int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, UINTPTR BrigReg)
#endif
{
	XPciePsu_Config *ConfigPtr;

#ifndef SDT
	ConfigPtr = XPciePsu_LookupConfig(DeviceId);
#else
	ConfigPtr = XPciePsu_LookupConfig(BrigReg);
#endif
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (ConfigPtr->PcieMode != XPCIEPSU_MODE_ENDPOINT) {
		xil_printf("Psu pcie mode is not configured as endpoint\r\n");
		return XST_FAILURE;
	}
	XPciePsu_EP_CfgInitialize(PciePsuPtr, ConfigPtr);
	return XST_SUCCESS;
}
