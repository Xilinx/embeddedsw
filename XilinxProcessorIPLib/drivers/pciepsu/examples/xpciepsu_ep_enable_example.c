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
#include <xscugic.h>
#include <xil_exception.h>
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define INGRESS_NUM	0x0		/* Ingress num to setup ingress */
#define BAR_NUM		0x2		/* Bar no to setup ingress */
#define PS_DDR_ADDR	0x1000000	/* 32 or 64 bit PS DDR Addr
						to setup ingress */
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define PS_PCIE_AXI_INTR_ID	(117U + 32U)
/***************************** Function Prototypes ****************************/

int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId);
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
			DMA0_CHAN_AXI_INTR_STATUS,
			AXI_INTR_STATUS);
	val=XPciePsu_ReadReg(PciePsuPtr->Config.DmaBaseAddr,
			DMA0_CHAN_SCRATCH0);
	xil_printf("In Interrupt handler Value @SCRATCH0 = %x  \n", val);

	if (val == INGRESS_TEST_DONE)
	{
		 val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,INGRESS0_CONTROL);
		if (val & 0x00000001U) {
			size = (1 << INGRESS_SIZE_ENCODING) * (1 << INGRESS_MIN_SIZE);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					INGRESS_TRANS_SET_OFFSET,
					size);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET,
					PCIE_INTR_STATUS);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA0_CHAN_SCRATCH0,
					0X0U);
		}
		else {
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					INGRESS_TRANS_SET_OFFSET,
					0x0U);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET,
					PCIE_INTR_STATUS);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA0_CHAN_SCRATCH0,
					0x0U);
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

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
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
#ifdef XPAR_PSU_PCIE_DEVICE_ID
	XPciePsu_InitEndPoint(&PciePsuInstance, XPAR_PSU_PCIE_DEVICE_ID);

	XPciePsu_EP_InitIntr();
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
