/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspips_slave_polled_example.c
*
*
* This file contains a design example using the SPI controller in slave mode.
* This examples performs transfers in polled mode and has been tested with
* Aardvark Analyzer as Master. This example echoes data which it receives
* from the master. The slave controller expects MAX_DATA bytes of data from
* the master to transmit onto the SPI bus which the slave will receive into
* its Rx buffer. It will poll until the Rx FIFO is filled with the Threshold
* limit of data which is set to MAX_DATA. On sending data, the master will
* receive dummy bytes in response. Master has to send MAX_DATA dummy bytes
* to read back the echoed data.
*
*
* @note
*
* The slave mode test needs an external master to send data to the Spi device.
* This example has been tested with Aardvark Analyzer as Master.
* The Clock Polarity and Phase should match between master and the slave.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 2.0   sb  08/22/14  First release
*       ms   04/05/17 Modified Comment lines in functions to
*                     recognize it as documentation block for doxygen
*                     generation.
* 3.9   sb   07/05/23 Added support for system device-tree flow.
* 3.13  sb   09/09/25 Fix GCC warnings.
* 3.14 vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xspips.h"		/* SPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constant map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID
#endif

/*
 * The following constant specify the max amount of data the slave is
 * expecting to receive from the master.
 */
#define MAX_DATA		100

#define UNIQUE_VALUE		0xAA
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int SpiPsSlavePolledExample(u16 SpiDeviceId);
#else
int SpiPsSlavePolledExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XSpiPs SpiInstance;

/*
 * The ReadBuffer/WriteBuffer is used to read/write to the data which it received
 * from the SPI Bus which master has sent.
 */
u8 ReadBuffer[MAX_DATA];
u8 WriteBuffer[MAX_DATA];

/*****************************************************************************/
/**
*
* Main function to call the SPI Slave Example.
*
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("Running SpiPS Slave Polled Example \r\n");

	/*
	 * Run the SpiPs Slave Polled example.
	 */
#ifndef SDT
	Status = SpiPsSlavePolledExample(SPI_DEVICE_ID);
#else
	Status = SpiPsSlavePolledExample(XPAR_XSPIPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SpiPs Slave Polled Example Failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SpiPs Slave Polled Example \r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in Slave mode. This function reads data from a SPI Master
* and will echo it back to the Master.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	SpiDeviceId is the Instance Id of SPI in the system.
* @endif
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		In XSCT/classic flow, DeviceId is used to look up the device
*               configuration.
*
*
*****************************************************************************/
#ifndef SDT
int SpiPsSlavePolledExample(u16 SpiDeviceId)
#else
int SpiPsSlavePolledExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
#ifndef SDT
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
#else
	SpiConfig = XSpiPs_LookupConfig(BaseAddress);
#endif
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize((&SpiInstance), SpiConfig,
				      SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Prepare buffers */
	for (u32 i = 0; i < MAX_DATA; i++) {
		WriteBuffer[i] = (u8)i + UNIQUE_VALUE;
		ReadBuffer[i] = 0;
	}

	Status = XSpiPs_PolledTransfer(&SpiInstance, WriteBuffer, ReadBuffer, MAX_DATA);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
