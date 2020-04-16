/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************
*
* @file xilisf_winbond_rdwr_polled_example.c
*
* This file contains a design example using the In-system and Serial Flash
* Library (XilIsf) for Reading/Writing to a Winbond W25QXX/W25XX Serial Flash
* Device with the Spi driver in polled mode of operation. This example shows
* the Erase, Read and Write features.
*
* This example
* - Erases a Sector
* - Writes to a Page within the Erased Sector
* - Reads back the Page that is written and compares the data
*
* This example  has been tested with a Winbond Serial Flash Memory (W25Q64) on
* a Xilinx SP605 board. For further details about the Flash device refer the
* Winbond Serial Flash Memory (W25Q64) data sheet.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 2.01a sdm 03/17/10 First release
* 5.14  akm 08/01/19 Initialized Status variable to XST_FAILURE.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xilisf.h"		/* Serial Flash Library header file */

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID			XPAR_SPI_0_DEVICE_ID

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define ISF_SPI_SELECT		0x01

/*
 * Number of bytes per page in the flash device.
 */
#define ISF_PAGE_SIZE		256

/*
 * Address of the page to perform Erase, Write and Read operations.
 */
#define ISF_TEST_ADDRESS	0x010000;
#define ISF_TEST_BYTE		0x36 /* Test Byte offset value written */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int IsfWaitForFlashNotBusy(void);
static int IsfWinbondFlashExample(void);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XIsf Isf;
static XSpi Spi;

/*
 * The user needs to allocate a buffer to be used by the In-system and Serial
 * Flash Library to perform any read/write operations on the Serial Flash
 * device.
 * User applications must pass the address of this memory to the Library in
 * Serial Flash Initialization function, for the Library to work.
 * For Write operations:
 * - The size of this buffer should be equal to the Number of bytes to be
 * written to the Serial Flash + XISF_CMD_MAX_EXTRA_BYTES.
 * - The size of this buffer should be large enough for usage across all the
 * applications that use a common instance of the Serial Flash.
 * - A minimum of one byte and a maximum of ISF_PAGE_SIZE bytes can be written
 * to the Serial Flash, through a single Write operation.
 * The size of this buffer should be equal to XISF_CMD_MAX_EXTRA_BYTES, if the
 * application only reads from the Serial Flash (no write operations).
 */
u8 IsfWriteBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];

/*
 * Buffers used during Read/Write transactions.
 */
u8 ReadBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES]; /* Read Buffer */
u8 WriteBuffer[ISF_PAGE_SIZE];				  /* Write buffer */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to call the Winbond Serial Flash Read/Write polled example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("Winbond Serial Flash Read/Write polled example\r\n");

	/*
	 * Run the Winbond flash example.
	 */
	Status = IsfWinbondFlashExample();
	if (Status != XST_SUCCESS) {
		xil_printf("Winbond Serial Flash Read/Write polled example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Winbond Serial Flash Read/Write polled example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Function to execute the Winbond Serial Flash Read/Write polled example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
static int IsfWinbondFlashExample()
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 Address;
	XIsf_WriteParam WriteParam;
	XIsf_ReadParam ReadParam;


	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that the device is enabled.
	 */
	XSpi_Start(&Spi);

	/*
	 * Disable Global interrupt to use the Spi driver in polled mode
	 * operation.
	 */
	XSpi_IntrGlobalDisable(&Spi);

	/*
	 * Initialize the Serial Flash Library.
	 */
	Status = XIsf_Initialize(&Isf, &Spi, ISF_SPI_SELECT, IsfWriteBuffer);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Specify the address in the Serial Flash for the Erase/Write/Read
	 * operations.
	 */
	Address = ISF_TEST_ADDRESS;

	/*
	 * The following code Erases a Sector in the Winbond Serial Flash.
	 */

	/*
	 * Perform the Write Enable operation.
	 */
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform the Sector Erase operation.
	 */
	Status = XIsf_Erase(&Isf, XISF_SECTOR_ERASE, Address);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform the Write Enable operation.
	 */
	Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address within the Serial Flash where the data is to be written.
	 * - Number of bytes to be written to the Serial Flash.
	 * - Write Buffer which contains the data to be written to the Serial
	 *   Flash.
	 */
	WriteParam.Address = Address;
	WriteParam.NumBytes = ISF_PAGE_SIZE;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Prepare the write buffer. Fill in the data need to be written into
	 * Serial Flash.
	 */
	for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
		WriteParam.WritePtr[Index] = Index + ISF_TEST_BYTE;
	}

	/*
	 * Perform the Write operation.
	 */
	Status = XIsf_Write(&Isf, XISF_WRITE, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Flash is not Busy.
	 */
	Status = IsfWaitForFlashNotBusy();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the
	 * - Address in the Serial Flash where the data is to be read from.
	 * - Number of bytes to be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ISF_PAGE_SIZE;
	ReadParam.ReadPtr = ReadBuffer;

	/*
	 * Perform the read operation.
	 */
	Status = XIsf_Read(&Isf, XISF_READ, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Compare the data read against the data Written.
	 */
	for(Index = 0; Index < ISF_PAGE_SIZE; Index++) {
		if(ReadBuffer[Index + XISF_CMD_SEND_EXTRA_BYTES] !=
					(u8)(Index + ISF_TEST_BYTE)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits till the Winbond serial Flash is ready to accept next
* command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the status register of the Serial Flash and
*		waits till the Serial Flash is ready to accept next command.
*
******************************************************************************/
int IsfWaitForFlashNotBusy(void)
{
	int Status = XST_FAILURE;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register.
		 */
		Status = XIsf_GetStatus(&Isf, ReadBuffer);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Check if the Serial Flash is ready to accept the next
		 * command. If so break.
		 */
		StatusReg = ReadBuffer[BYTE2];
		if((StatusReg & XISF_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}
