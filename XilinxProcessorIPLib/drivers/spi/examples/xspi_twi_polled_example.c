/******************************************************************************
*
* Copyright (C) 2008 - 2019 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
* @file xspi_twi_polled_example.c
*
*
* This file contains a design example using the Spi driver (XSpi) and the Spi
* device using the polled twi mode.
* TWI (three wire interface) is a half-duplex SPI interface, where the MOSI and
* the MISO is common and usually called SDIO. In this mode the direction
* (tri-state) of the SDIO must be set during transfers. (Usually at the end of
* the read)
*
* To put the driver in polled mode the Global Interrupt must be disabled after
* the Spi is Initialized and Spi driver is started.
*
* This example tested on 7-series Zynq-ARM with SDK 2017.4 on Win-10.
*
*
* @note
*
* None.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------------
* 1.00a raczben 14/01/20 First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xspi.h"		/* SPI device driver */
#include "xspi_l.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID

/*
 *  This is the size of the buffer to be transmitted/received in this example.
 */
#define BUFFER_SIZE		12

/************************** Function Prototypes ******************************/

int SpiTwiPolledExample(XSpi *SpiInstancePtr, u16 SpiDeviceId);
void switch_sdio_direction(int direction);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
 */
static XSpi  SpiInstance;	 /* The instance of the SPI device */


/*
 * The SDIO direction switcher function. Must be implemented by the application.
 * This case uses a register at address: 0x43C70108, which LSB bit is connected
 * directly to the tri-state enable bit of the SDIO buffer. Therefore writing '0'
 * disables the tri-state, and the FPGA drives the SDIO pin. Writing '1'
 * enables the tri-state, and the FPGA can read from the SDIO pin.
 */
void switch_sdio_direction(int direction){
	if(direction == XSP_SDIO_TO_OUTPUT){
		Xil_Out32(0x43C70108, 0);
	} else {
		Xil_Out32(0x43C70108, 1);
	}
}


/*****************************************************************************/
/**
*
* Main function to call the Spi twi Polled example.
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
	 * Run the Spi Twi Polled example.
	 */
	Status = SpiTwiPolledExample(&SpiInstance, SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Spi twi polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Spi twi polled Example\r\n");
	while(1);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Spi device and driver in twi mode
* (aka. half-duplex SPI mode.)
*
* This function writes test data value (0x55) to a test address (0x19) into a
* standard twi device.*
*
* @param	SpiInstancePtr is a pointer to the instance of Spi component.
* @param	SpiDeviceId is the Device ID of the Spi Device and is the
*		XPAR_<SPI_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*
******************************************************************************/
int SpiTwiPolledExample(XSpi *SpiInstancePtr, u16 SpiDeviceId)
{
	int Status;
	u8 Buffer[BUFFER_SIZE];
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

	/*
	 * Set the Spi device as a master.
	 */
	Status = XSpi_SetOptions(SpiInstancePtr, XSP_CLK_PHASE_1_OPTION |
			 XSP_CLK_ACTIVE_LOW_OPTION | XSP_MASTER_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Direction switcher callback for the SDIO pin.
	 */
	SpiInstancePtr->DirectionSwitcher = switch_sdio_direction;

	/*
	 * Select the slave on the SPI bus, the EEPROM device so that it can be
	 * read and written using the SPI bus
	 */
	Status = XSpi_SetSlaveSelect(&SpiInstance, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that the device is enabled.
	 */
	XSpi_Start(SpiInstancePtr);

	/*
	 * Disable Global interrupt to use polled mode operation
	 */
	XSpi_IntrGlobalDisable(SpiInstancePtr);

	/*
	 * Initialize the buffer with pattern to write
	 */
	Buffer[0] = 0x00;
	Buffer[1] = 0x19;
	Buffer[2] = 0x55; //Test data

	/*
	 * Transmit the data. (Write to IC's register)
	 */
	SpiInstancePtr->SwitchDirectionAfter = -1;
	XSpi_Transfer(SpiInstancePtr, Buffer, Buffer, 3);

	/*
	 * Read back the data.
	 * Read address is same as write address, but the MSB bit is '1'
	 */
	Buffer[0] = 0x80| Buffer[0];
	Buffer[2] = 0x2c; 				// Dummy data

	/*
	 * Set SwitchDirectionAfter to 2 because the third byte will
	 * be driven by the slave device.
	 */
	SpiInstancePtr->SwitchDirectionAfter = 2;
	XSpi_Transfer(SpiInstancePtr, Buffer, Buffer, 3);

	/*
	 * Compare the data received with the data that was transmitted.
	 */
	if (Buffer[2] != 0x55) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
