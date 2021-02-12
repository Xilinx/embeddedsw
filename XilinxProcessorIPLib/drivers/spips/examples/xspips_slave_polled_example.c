/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
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
*</pre>
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
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID

/*
 * The following constant specify the max amount of data the slave is
 * expecting to receive from the master.
 */
#define MAX_DATA		100

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SpiPs_RecvByte(BaseAddress) \
		(u8)XSpiPs_In32((BaseAddress) + XSPIPS_RXD_OFFSET)

#define SpiPs_SendByte(BaseAddress, Data) \
		XSpiPs_Out32((BaseAddress) + XSPIPS_TXD_OFFSET, (Data))

/************************** Function Prototypes ******************************/

void SpiSlaveRead(int ByteCount);

void SpiSlaveWrite(u8 *Sendbuffer, int ByteCount);

int SpiPsSlavePolledExample(u16 SpiDeviceId);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XSpiPs SpiInstance;

/*
 * The ReadBuffer is used to read to the data which it received from the SPI
 * Bus which master has sent.
 */
u8 ReadBuffer[MAX_DATA];

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
	Status = SpiPsSlavePolledExample(SPI_DEVICE_ID);
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
* @param	SpiDeviceId is the Instance Id of SPI in the system.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		None
*
*
*****************************************************************************/
int SpiPsSlavePolledExample(u16 SpiDeviceId)
{
	int Status;
	u8 *BufferPtr;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize((&SpiInstance), SpiConfig,
					SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * The SPI device is a slave by default and the clock phase
	 * have to be set according to its master. In this example, CPOL is set
	 * to quiescent high and CPHA is set to 1.
	 */
	Status = XSpiPs_SetOptions((&SpiInstance), (XSPIPS_CR_CPHA_MASK) | \
			(XSPIPS_CR_CPOL_MASK));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Set the Rx FIFO Threshold to the Max Data
	 */
	XSpiPs_SetRXWatermark((&SpiInstance),MAX_DATA);

	/*
	 * Enable the device.
	 */
	XSpiPs_Enable((&SpiInstance));

	/*
	 * Read the contents of the Receive buffer
	 * Master is expected to send MAX_DATA number of bytes
	 */
	SpiSlaveRead(MAX_DATA);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and the same back
	 */
	BufferPtr = ReadBuffer;

	/*
	 * Send the data received back to Master
	 * Master is expected to send MAX_DATA number of dummy bytes for
	 * the slave to be able to echo previously received data.
	 */
	SpiSlaveWrite(BufferPtr, MAX_DATA);

	/*
	 * Disable the device.
	 */
	XSpiPs_Disable((&SpiInstance));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads from the Rx buffer
*
* @param	ByteCount is the number of bytes to be read from Rx buffer.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SpiSlaveRead(int ByteCount)
{
	int Count;
	u32 StatusReg;

	StatusReg = XSpiPs_ReadReg(SpiInstance.Config.BaseAddress,
					XSPIPS_SR_OFFSET);

	/*
	 * Polling the Rx Buffer for Data
	 */
	do{
		StatusReg = XSpiPs_ReadReg(SpiInstance.Config.BaseAddress,
					XSPIPS_SR_OFFSET);
	}while(!(StatusReg & XSPIPS_IXR_RXNEMPTY_MASK));

	/*
	 * Reading the Rx Buffer
	 */
	for(Count = 0; Count < ByteCount; Count++){
		ReadBuffer[Count] = SpiPs_RecvByte(
				SpiInstance.Config.BaseAddress);
	}

}

/*****************************************************************************/
/**
*
* This function writes Data into the Tx buffer
*
* @param	Sendbuffer is the buffer whose data is to be sent onto the
* 		Tx FIFO.
* @param	ByteCount is the number of bytes to be read from Rx buffer.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SpiSlaveWrite(u8 *Sendbuffer, int ByteCount)
{
	u32 StatusReg;
	int TransCount = 0;

	StatusReg = XSpiPs_ReadReg(SpiInstance.Config.BaseAddress,
				XSPIPS_SR_OFFSET);

	/*
	 * Fill the TXFIFO with as many bytes as it will take (or as
	 * many as we have to send).
	 */
	while ((ByteCount > 0) &&
		(TransCount < XSPIPS_FIFO_DEPTH)) {
		SpiPs_SendByte(SpiInstance.Config.BaseAddress,
				*Sendbuffer);
		Sendbuffer++;
		++TransCount;
		ByteCount--;
	}

	/*
	 * Wait for the transfer to finish by polling Tx fifo status.
	 */
	do {
		StatusReg = XSpiPs_ReadReg(
				SpiInstance.Config.BaseAddress,
					XSPIPS_SR_OFFSET);
	} while ((StatusReg & XSPIPS_IXR_TXOW_MASK) == 0);

}
