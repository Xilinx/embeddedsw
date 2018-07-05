/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspi_winbond_flash_xip_example.c
*
*
* This file contains a design example using the Spi driver (XSpi) and the Spi
* device configured in XIP Mode. This example reads data from the Flash Memory
* in the way RAM is accessed.
*
* @note
*
* None.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 3.04a bss  03/21/12 First Release
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*       ms   04/10/17 Modified filename tag to include the file in doxygen
*                     examples.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xspi.h"		/* SPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int SpiXipExample(XSpi *SpiInstancePtr, u16 SpiDeviceId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
 */
static XSpi  SpiInstance;	 /* The instance of the SPI device */


/*****************************************************************************/
/**
*
* Main function to call the Spi XIP example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Spi XIP example.
	 */
	Status = SpiXipExample(&SpiInstance, SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Spi winbond flash xip Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spi winbond flash xip Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads data from Flash memory in the way any memory is accessed.
* The purpose of this function is to illustrate how to use SPI device in XIP
* Mode.
*
* This function reads data from Flash in the same way any other memory is
* accessed
*
*
* @param	SpiInstancePtr is a pointer to the instance of Spi component.
* @param	SpiDeviceId is the Device ID of the Spi Device and is the
*		XPAR_<SPI_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int SpiXipExample(XSpi *SpiInstancePtr, u16 SpiDeviceId)
{
	u32 Flashdata;
	int Status;
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
	ConfigPtr = XSpi_LookupConfig(SpiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(SpiInstancePtr, ConfigPtr,
				  ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read data from flash in the way any memory is accessed using
		Flash Memory Base Address in InstancePtr */

	Flashdata = Xil_In32(SpiInstancePtr->FlashBaseAddr);



	return XST_SUCCESS;
}
