/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilisf_ospipsv_flash_polled_example.c
*
*
* This file contains a design example using the XILISF Library in
* polled mode with a serial FLASH device.
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
* 5.13  sk  02/11/19 First Release.
* 5.14  akm 08/01/19 Initialized Status variable to XST_FAILURE.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xil_exception.h"
#include "xil_printf.h"
#include "xilisf.h"             /* Serial Flash Library header file */

/************************** Constant Definitions *****************************/
/** @name Device ID's
 *
 * @{
 */
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID

/*@}*/

/**
 * The following constants specify the page size and number of
 * pages for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define PAGE_SIZE	256	/**< Page Size for Read/Write Operation */

/**
 * Number of flash pages to be written.
 */
#define PAGE_COUNT	10	/**< Number of Pages for r/w Operation */

/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/**
 * Flash address to which data is ot be written.
 */
#define TEST_ADDRESS	0x0	/**< Test Address in the flash  */
#define UNIQUE_VALUE	0x08		/**< Unique Value for Test */


/**
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA	PAGE_COUNT * PAGE_SIZE	/**< Max Data Calculated by
					multiplying Page count and Page Size */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int OspiFlashPollExample(XOspiPsv *OspiInstancePtr, u16 OspiDeviceId);
int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount);
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount,
							u8 Command);
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount,
							u8 Command);
static u32 SectorMask(u32 SectorSize);
int FlashSetSDRDDRMode(XIsf *InstancePtr, int Mode);

/************************** Variable Definitions *****************************/

/**
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;
static XIsf Isf;

/**
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/**
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test_Polled = 7;

/**
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(64)));
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(4)));
u8 IsfWriteBuffer[PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];/**< IsfWrite Buffer
					used in XilISF Initialization */

/*****************************************************************************/
/**
*
* Main function to call the OSPI Flash example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("OSPIPSV FLASH Polling Example Test \r\n");

	/*
	 * Run the OSPIPSV Polled example.
	 */
	Status = OspiFlashPollExample(&OspiPsvInstance, OSPIPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("OSPIPSV FLASH Polling Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran OSPIPSV FLASH Polling Example Test\r\n");
	return XST_SUCCESS;
}


/****************************************************************************/
/**
* The purpose of this function is to illustrate how to use the XOspiPsv
* device driver in interrupt mode. This function writes and reads data
* from a serial FLASH.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
*
*****************************************************************************/
int OspiFlashPollExample(XOspiPsv *OspiInstancePtr, u16 OspiDeviceId)
{
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	int Status = XST_FAILURE;
	u32 Options;
	XOspiPsv_Config *ConfigPtr;

	/*
	* Lookup the device configuration in the temporary CROM table. Use this
	* configuration info down below when initializing this component.
	*/
	ConfigPtr = XOspiPsv_LookupConfig(OspiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XOspiPsv_CfgInitialize(OspiInstancePtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the OSPI options
	 */
	Options = XOSPIPSV_IDAC_EN_OPTION;
	XIsf_SetSpiConfiguration(&Isf, OspiInstancePtr, Options, XISF_SPI_PRESCALER);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Isf.XIsf_Iface_SetSlaveSelect(Isf.SpiInstPtr, XOSPIPSV_SELECT_FLASH_CS0);
	Isf.IsReady = TRUE;
	Status = FlashSetSDRDDRMode(&Isf, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Isf.IsReady = FALSE;

	/* Initialize the XILISF Library */
	XIsf_Initialize(&Isf, OspiInstancePtr, XOSPIPSV_SELECT_FLASH_CS0,
				   IsfWriteBuffer);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read,
	 * the test value that is added to the unique value allows the value
	 * to be changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
		Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test_Polled);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	Status = FlashErase(&Isf, TEST_ADDRESS, MAX_DATA);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	if (Isf.SpiInstPtr->OpMode == XOSPIPSV_DAC_MODE) {
		Status = FlashWrite(&Isf, TEST_ADDRESS, MAX_DATA, XISF_WRITE);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	} else {
		for (Page = 0; Page < PAGE_COUNT; Page++) {
			Status = FlashWrite(&Isf,
				(Page * PAGE_SIZE) + TEST_ADDRESS, PAGE_SIZE, XISF_WRITE);
			if(Status != XST_SUCCESS){
				return XST_FAILURE;
			}
		}
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using OCTAL IO
	 * Fast Read command
	 */
	Status = FlashRead(&Isf, TEST_ADDRESS, MAX_DATA, XISF_OCTAL_IO_FAST_READ);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = ReadBuffer;
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
			Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue + Test_Polled)) {
			return XST_FAILURE;
		}
	}

	Status = FlashSetSDRDDRMode(&Isf, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the OSPIPSV interface.
* The FLASH contains a 256 byte write buffer which can be filled and then a
* write is automatically performed by the device.  All the data put into the
* buffer must be in the same page of the device with page boundaries being on
* 256 byte boundaries.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount,
								u8 Command)
{
	XIsf_WriteParam WriteParam;

	int Status = XST_FAILURE;

	WriteParam.Address = Address;
	WriteParam.NumBytes = ByteCount;
	WriteParam.WritePtr = WriteBuffer;

	/*
	 * Perform the Write operation.
	 */
	Status = XIsf_Write(&Isf, Command, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads from the  serial FLASH connected to the
* OSPIPSV interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command)
{
	XIsf_ReadParam ReadParam;
	int Status = XST_FAILURE;

	/*
	 * Set the
	 * - Address in the Serial Flash where the data is to be read from.
	 * - Number of bytes ReadParam.NumDummyBytesto be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ByteCount;
	ReadParam.ReadPtr = ReadBuffer;
	ReadParam.NumDummyBytes = 16;

	/*
	 * Perform the Read operation.
	 */
	Status = XIsf_Read(&Isf, Command, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This API enters the flash device and OSPI into Octal DDR mode or exit from
* octal DDR mode (switches to Extended SPI mode).
*
* @param	OspiPtr is a pointer to the XIsf component to use.
* @param	Enable is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashSetSDRDDRMode(XIsf *InstancePtr, int Mode)
{
	int Status = XST_FAILURE;
	u8 ConfigReg[2] __attribute__ ((aligned(64)));
	u8 Data[2] __attribute__ ((aligned(4)));
	XIsf_WriteParam WriteParam;
	XIsf_ReadParam ReadParam;

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Data[0] = 0xE7;
		Data[1] = 0xE7;
	} else {
		Data[0] = 0xFF;
		Data[1] = 0xFF;
	}

	WriteParam.Address = 0x0;
	WriteParam.NumBytes = 1;
	WriteParam.WritePtr = Data;
	/*
	 * Perform the Write operation.
	 */
	Status = XIsf_Write(InstancePtr, XISF_WRITE_VOLATILE_CONFIG_REG, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XOspiPsv_SetSdrDdrMode(InstancePtr->SpiInstPtr, Mode);

	ReadParam.Address = 0x0;
	ReadParam.NumBytes = 1;
	ReadParam.ReadPtr = ConfigReg;
	ReadParam.NumDummyBytes = 8;
	/*
	 * Perform the Read operation.
	 */
	Status = XIsf_Read(InstancePtr, XISF_READ_VCR, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0])
		return XST_FAILURE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* OSPIPSV interface.
*
* @param	InstancePtr is a pointer to the XIsf component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount)
{
	int Status = XST_FAILURE;
	int Sector;
	u32 NumSect;
	u32 SectorSize;
	u32 NumSectors;
	u32 Sector_Mask;
	/*
	 * Get the value of Sector Size and Number of Sectors for the flash
	 */
	SectorSize = Isf.SectorSize;
	NumSectors = Isf.NumSectors;

	/* Get the sector mask value */
	Sector_Mask = SectorMask(SectorSize);

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount == (NumSectors * SectorSize)) {
		/*
		 * Call Bulk erase
		 */
		Status = XIsf_Erase(InstancePtr, XISF_BULK_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		return Status;
	}

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/SectorSize + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & Sector_Mask) ==
			((Address + (NumSect * SectorSize)) &
					Sector_Mask) ) {
		NumSect++;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Perform the Sector Erase operation.
		 */
		Status = XIsf_Erase(InstancePtr, XISF_SECTOR_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Address += SectorSize;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calculates the sector mask based upon the sector size value
*
*
* @param	SectorSize is the size of the sector of the flash
* 			available on the board.
*
* @return	will return the sector mask after calculation.
*
* @note		None.
*
******************************************************************************/
u32 SectorMask(u32 SectorSize){

	u32 Mask;

	switch(SectorSize){
	case 0x10000:
				Mask = 0xFFFF0000;
				break;

	case 0x20000:
				Mask = 0xFFFE0000;
				break;
	case 0x40000:
				Mask = 0xFFFC0000;
				break;

	case 0x80000:
				Mask = 0xFFF80000;
				break;

	default:
			break;
	}

	return Mask;
}
