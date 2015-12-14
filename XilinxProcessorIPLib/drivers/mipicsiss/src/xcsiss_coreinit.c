/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xcsiss_coreinit.c
* @addtogroup csiss
* @{
* @details

* MIPI CSI Rx Subsystem Sub-Cores initialization
* The functions in this file provides an abstraction from the initialization
* sequence for included sub-cores. Subsystem is assigned an address and range
* on the axi-lite interface. This address space is condensed where-in each
* sub-core is at a fixed offset from the subsystem base address. For processor
* to be able to access the sub-core this offset needs to be transalted into a
* absolute address within the subsystems addressable range
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vs   08/03/15   Initial Release

* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xcsiss_coreinit.h"
/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/
/**************************** Local Global ***********************************/
/************************** Function Prototypes ******************************/
static int ComputeSubCoreAbsAddr(u32 SsBaseAddr,
					u32 SsHighAddr,
					u32 Offset,
					u32 *BaseAddr);
/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  CsiSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XCsiSs_SubCoreInitCsi(XCsiSs *CsiSsPtr)
{
	int Status;
	u32 AbsAddr;
	XCsi_Config *ConfigPtr;

	if (!CsiSsPtr->CsiPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xil_printf("->Initializing CSI Rx Controller...\r\n");
	ConfigPtr = XCsi_LookupConfig(CsiSsPtr->Config.CsiInfo.DeviceId);
	if (ConfigPtr == NULL) {
		xil_printf("CSISS ERR:: CSI not found\r\n");
		return (XST_FAILURE);
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.CsiInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: CSI core base address\
				(0x%x) invalid %d\r\n", AbsAddr);
		return (XST_FAILURE);
	}

	/* Initialize core */
	Status = XCsi_CfgInitialize(CsiSsPtr->CsiPtr, ConfigPtr, AbsAddr);

	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: CSI core Initialization\
				failed\r\n");
		return (XST_FAILURE);
	}
	return (XST_SUCCESS);
}

#if (XPAR_XIIC_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  CsiSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XCsiSs_SubCoreInitIic(XCsiSs *CsiSsPtr)
{
	int Status;
	u32 AbsAddr;
	XIic_Config *ConfigPtr;

	if (!CsiSsPtr->IicPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xil_printf("->Initializing IIC MIPI CSI subsystem..\r\n");
	ConfigPtr = XIic_LookupConfig(CsiSsPtr->Config.IicInfo.DeviceId);
	if (!ConfigPtr) {
		xil_printf("CSISS ERR:: IIC not found\r\n");
		return (XST_FAILURE);
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.IicInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: Iic core base address\
				(0x%x) invalid %d\r\n", AbsAddr);
		return (XST_FAILURE);
	}

	/* Initialize core */
	Status = XIic_CfgInitialize(CsiSsPtr->IicPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: Iic core Initialization\
				failed\r\n");
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);
}
#endif

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  CsiSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XCsiSs_SubCoreInitDphy(XCsiSs *CsiSsPtr)
{
	int Status;
	u32 AbsAddr;
	XDphy_Config *ConfigPtr;

	if (!CsiSsPtr->DphyPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xil_printf("->Initializing DPHY ...\r\n");
	ConfigPtr = XDphy_LookupConfig(CsiSsPtr->Config.DphyInfo.DeviceId);
	if (!ConfigPtr) {
		xil_printf("CSISS ERR:: DPHY not found\r\n");
		return (XST_FAILURE);
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.DphyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: DPHY core base address\
				(0x%x) invalid %d\r\n", AbsAddr);
		return (XST_FAILURE);
	}

	/* Initialize core */
	Status = XDphy_CfgInitialize(CsiSsPtr->DphyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS ERR:: Dphy core Initialization\
				failed\r\n");
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);
}
#endif
/*****************************************************************************/
/**
* This function computes the subcore absolute address on axi-lite interface
* Subsystem is mapped at an absolute address and all included sub-cores are
* at pre-defined offset from the subsystem base address. To access the subcore
* register map from host CPU an absolute address is required.
* The subsystem is aligned to 64K address and has address range of max 192K
* (0x00000-0x2FFFF) in case IIC is present and DPHY register interface is
* selected. By default, CSI is at offset 0x0_0000, IIC is at offset 0x1_0000
* and DPHY is at offset 0x2_0000.
* In case of IIC being absent and DPHY has register interface, the address
* range shrinks to 128K (0x00000 - 0x1FFFF) with DPHY moving to offset
* 0x1_0000. In case DPHY register interface is also absent then the address
* range shrinks to 64K with only the CSI subcore at offset 0x0_0000.
*
* @param  SsBaseAddr is the base address of the the Subsystem instance
* @param  SsHighAddr is the max address of the Subsystem instance
* @param  Offset is the offset of the specified core
* @param  BaseAddr is the computed absolute base address of the subcore
*
* @return XST_SUCCESS if base address computation is successful and within
*         subsystem address range else XST_FAILURE
*
******************************************************************************/
static int ComputeSubCoreAbsAddr(u32 SsBaseAddr,
		                         u32 SsHighAddr,
		                         u32 Offset,
		                         u32 *BaseAddr)
{
	int Status;
	u32 AbsAddr;

	AbsAddr = SsBaseAddr + Offset;

	if ((AbsAddr >= SsBaseAddr) && (AbsAddr < SsHighAddr)) {
		*BaseAddr = AbsAddr;
		Status = XST_SUCCESS;
	}
	else {
		*BaseAddr = 0;
		Status = XST_FAILURE;
	}

	return (Status);
}

/** @} */
