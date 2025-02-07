/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xhwicap_read_config_reg_example.c
*
* This file contains a design example using the HwIcap driver and hardware
* device.
*
* This example prints out the values of all the configuration registers in the
* FPGA.
*
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
* @note
*
* This example can be run on a 7 series device, Zynq device, Ultrascale
* and ZynqMP Ultrascale FPGAs.
*
* In a Zynq device the ICAP needs to be selected using the
* XDcfg_SelectIcapInterface API of the DevCfg driver (clear the PCAP_PR bit of
* Control register in the Device Config Interface)  before it can be
* accessed using the HwIcap.
* In case of ZynqMP clear the PCAP_PR bit of pcap_ctrl register in Module
* Configuration Security Unit(CSU) using register write.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a bjb  11/21/03 First release
* 1.00a sv   07/18/05 Minor changes to comply to Doxygen and coding guidelines
* 1.01a sv   04/10/07 Changes to support V4
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  04/28/10 Added support for S6 support.
* 6.00a hvm  08/05/11 Added support for K7 family
* 8.01a bss  05/14/12 Replaced the define XHI_C0R_1 with XHI_COR_1 for CR718042
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.0  ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 11.2 Nava 01/30/19  Rename the example since sdk is expecting _example
*		      extension to support the import examples feature
*		      from system.mss file.
* 11.6 Nava 06/28/23  Added support for system device-tree flow.
* 11.7 Nava 02/06/25  Updated HWICAP_BASEADDR to use XPAR_HWICAP_0_BASEADDR
*                     instead of XPAR_XHWICAP_0_BASEADDR to align with YAML
*                     changes.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xparameters.h>
#include <xil_types.h>
#include <xil_assert.h>
#include <xhwicap.h>
#include <stdio.h>
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define HWICAP_DEVICEID         XPAR_HWICAP_0_DEVICE_ID
#else
#define HWICAP_BASEADDR		XPAR_HWICAP_0_BASEADDR
#endif

#define printf   xil_printf          /* A smaller footprint printf */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
int HwIcapReadConfigRegExample(u16 DeviceId);
#else
int HwIcapReadConfigRegExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

static XHwIcap HwIcap;

/*****************************************************************************/
/**
*
* Main function to call the HWICAP example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the HwIcap example, specify the Device Id generated in
	 * xparameters.h.
	 */
#ifndef SDT
	Status = HwIcapReadConfigRegExample(HWICAP_DEVICEID);
#else
	Status = HwIcapReadConfigRegExample(HWICAP_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Hwicap read config reg Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Hwicap read config reg Example\r\n");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function reads the configuration  registers inside the FPGA.
*
* @param	DeviceId is the XPAR_<HWICAP_INSTANCE>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int HwIcapReadConfigRegExample(u16 DeviceId)
#else
int HwIcapReadConfigRegExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XHwIcap_Config *CfgPtr;
	u32 ConfigRegData;

	/*
	 * Initialize the HwIcap instance.
	 */
#ifndef SDT
	CfgPtr = XHwIcap_LookupConfig(DeviceId);
#else
	CfgPtr = XHwIcap_LookupConfig(BaseAddress);
#endif
	if (CfgPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XHwIcap_CfgInitialize(&HwIcap, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Run the Self test.
	 */
	Status = XHwIcap_SelfTest(&HwIcap);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	printf("Value of the Configuration Registers. \r\n\r\n");

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CRC, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CRC -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_FAR, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" FAR -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_FDRI, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" FDRI -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_FDRO, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" FDRO -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CMD, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CMD -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CTL, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CTL -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_MASK, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" MASK -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_STAT, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" STAT -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_LOUT, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" LOUT -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_COR, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" COR -> \t %x \t\r\n", ConfigRegData);
	}
	if (XHwIcap_GetConfigReg(&HwIcap, XHI_MFWR, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" MFWR -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CBC, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CBC -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_AXSS, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" AXSS -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_IDCODE, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" IDCODE -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_COR_1, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" COR_1 -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CSOB, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CSOB -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_WBSTAR, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" WBSTAR -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_TIMER, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" TIMER -> \t %x \t\r\n", ConfigRegData);
	}
	if (XHwIcap_GetConfigReg(&HwIcap, XHI_BOOTSTS, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" BOOTSTS -> \t %x \t\r\n", ConfigRegData);
	}

	if (XHwIcap_GetConfigReg(&HwIcap, XHI_CTL_1, (u32 *)&ConfigRegData) ==
	    XST_SUCCESS) {
		printf(" CTL_1 -> \t %x \t\r\n", ConfigRegData);
	}

	printf("\r\n HwIcapReadConfigRegExample Passed Successfully.\r\n\r\n");

	return XST_SUCCESS;
}

