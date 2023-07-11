/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspips_dual_flash_stack_lqspi_example.c
*
* This file contains a design example using the QSPI driver (XQspiPs) in
* Linear QSPI mode, with two serial Flash devices in stacked mode.
* One flash s accessed at a time on a common bus by using separate selects.
* This example writes to the two flash memories in  QSPI mode and reads
* the data back from the flash memories, in Linear QSPI mode.
* It is recommended to use Manual CS + Auto start for best performance.
*
* The hardware which this example runs on, must have a serial Flash (Numonyx
* N25Q, Winbond W25Q, Spansion S25FL, ISSI IS25WP) for it to run. This example
* has been tested with the Numonyx Serial Flash (N25Q128) and IS25WP series
* flash parts.
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
* 2.02a hk  05/07/13 First release
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block and modified
*                    filename tag to include the file in doxygen examples.
* 3.5	tjs 07/16/18 Added support for low density ISSI flash parts.
*		     Added FlashQuadEnable API to enable quad mode in flash.
*		     Added FlashReadID API to read and identify the flash.
* 3.10  akm 08/17/22 Fix logical error in NumSect calculation.
* 3.11  akm 07/10/23 Add support for system device-tree flow for example.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"	/* SDK generated parameters */
#endif
#include "xqspips.h"		/* QSPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID
#endif

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD		0x02
#define READ_CMD		0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				   * reads
				   */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				   * quad reads
				   */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */
#define RD_CFG_SIZE		2 /* 1 byte Configuration register + RD CFG
				   * command
				   */
#define WR_CFG_SIZE		3 /* WRR command + 1 byte each Status and
				   * Config Reg
				   */

/*
 * The following constants specify the extra bytes which are sent to the
 * Flash on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the Flash.  The page size specifies a max number of
 * bytes that can be written to the Flash with a single transfer.
 */
#define SECTOR_SIZE		0x10000
#define NUM_SECTORS		0x200		/* 100 sectors from each flash */
#define NUM_PAGES		0x20000		/* 10000 pages from each flash */
#define PAGE_SIZE		256

/* The following defines are for dual flash stacked mode interface. */
#define LQSPI_CR_FAST_QUAD_READ		0x0000006B /* Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		0x00000100 /* 1 Dummy Byte between
						     address and return data */
#define LQSPI_CR_FAST_READ			0x0000000B /* Fast Read */
#define LQSPI_CR_NORMAL_READ		0x00000003 /* Fast Read */

#define DUAL_STACK_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

#define DUAL_STACK_CONFIG_QUAD_READ	(XQSPIPS_LQSPI_CR_LINEAR_MASK | \
					 XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

/* Number of flash pages to be written.*/
#define PAGE_COUNT		16

/*
 * Flash address to which data is to be written.
 * Test address defined so as to cover both flash devices
 */
#define TEST_ADDRESS		0xFFF800


#define UNIQUE_VALUE		0x0C
/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash.
 */
#define MAX_DATA		(PAGE_COUNT * PAGE_SIZE)

/* Base address of Flash1 and Flash2*/
#define FLASH1BASE 0x0000000
#define FLASH2BASE 0x1000000

/* Mask for sector start address*/
#define SECTORMASK 0xFFF0000


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashReadID(void);

void FlashQuadEnable(XQspiPs *QspiPtr);

#ifndef SDT
int DualStackExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId);
#else
int DualStackExample(XQspiPs *QspiInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPs QspiInstance;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 2;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];

/*****************************************************************************/
/**
*
* Main function to call the QSPI Dual Stack example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("QSPI Dual Stack Example Test \r\n");

	/* Run the QSPI Dual Stack example.*/
#ifndef SDT
	Status = DualStackExample(&QspiInstance, QSPI_DEVICE_ID);
#else
	Status = DualStackExample(&QspiInstance, XPAR_XQSPIPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI Dual Stack Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI Dual Stack Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XQspiPs
* device driver with two Flash devices in Stacked mode.
* This function writes data to the serial Flash in QSPI mode and
* reads data in Linear QSPI mode.
*
* @param        QspiInstancePtr is a pointer to the QSPIPS driver to use.
* @param        QspiDeviceId is the XPAR_<QSPIPS_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
int DualStackExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId)
#else
int DualStackExample(XQspiPs *QspiInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPs_Config *QspiConfig;


	/* Initialize the QSPI driver so that it's ready to use*/
#ifndef SDT
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
#else
	QspiConfig = XQspiPs_LookupConfig(BaseAddress);
#endif
	if (QspiConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
					QspiConfig->BaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Perform a self-test to check hardware build*/
	Status = XQspiPs_SelfTest(QspiInstancePtr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check if two flash devices are connected in stacked mode*/
	if (QspiConfig->ConnectionMode != XQSPIPS_CONNECTION_MODE_STACKED) {
		xil_printf("QSPI not connected in Stacked Configuration \n");
		return XST_FAILURE;
	}

	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);
	/*
	 * Set the QSPI device as a master and enable manual CS, manual start
	 * and flash interface mode options and drive HOLD_B pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr,  XQSPIPS_FORCE_SSELECT_OPTION |
					    XQSPIPS_MANUAL_START_OPTION |
					    XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Enable two flash memories, shared bus (NOT separate bus0,
	 * L_PAGE selected by default
	 */
	XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_STACK_CONFIG_WRITE);

	FlashReadID();

	FlashQuadEnable(QspiInstancePtr);

	/* Assert the Flash chip select.*/
	XQspiPs_SetSlaveSelect(QspiInstancePtr);

	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));



	/* Erase the flash sectors*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * The two stacked 16MB memories are equivalent to one continuous 32MB.
	 * Write data to the two flash memories on the same bus one at a time,
	 * starting from TEST_ADDRESS. Flash_Write and Flash_Erase functions
	 * will select the lower or upper memory based on the address.
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, ((Page * PAGE_SIZE) +
			   TEST_ADDRESS), PAGE_SIZE, WRITE_CMD);
	}



	/*
	 * Read from the two flash memories one after the other using
	 * LQSPI mode. The LQSPI controller takes care of selecting
	 * the lower or upper memory based on address bit 25.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_LQSPI_MODE_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_STACK_CONFIG_QUAD_READ);

	Status = XQspiPs_LqspiRead(QspiInstancePtr, ReadBuffer, TEST_ADDRESS,
				   MAX_DATA);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {

		if (ReadBuffer[Count] != (u8)(UniqueValue + Test)) {

			return XST_FAILURE;
		}
	}

	/*
	 * Set the QSPI device as a master and enable manual CS, manual start
	 * and flash interface mode options and drive HOLD_B pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr,  XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/* Erase the flash sectors*/
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MAX_DATA);

	/*
	 * The two stacked 16MB memories are equivalent to one continuous 32MB.
	 * Write data to the two flash memories on the same bus one at a time,
	 * starting from TEST_ADDRESS. Flash_Write and Flash_Erase functions
	 * will select the lower or upper memory based on the address.
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr, ((Page * PAGE_SIZE) +
			   TEST_ADDRESS), PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * Read from the two flash memories one after the other using
	 * LQSPI mode. The LQSPI controller takes care of selecting
	 * the lower or upper memory based on address bit 25.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_LQSPI_MODE_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_STACK_CONFIG_QUAD_READ);


	Status = XQspiPs_LqspiRead(QspiInstancePtr, ReadBuffer, TEST_ADDRESS,
				   MAX_DATA);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
	     Count++, UniqueValue++) {

		if (ReadBuffer[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}

	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 LqspiCr;
	u32 RealAddr;


	/* Get the current LQSPI configuration register value*/
	LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
	/* Select lower or upper Flash based on address */
	if (Address & FLASH2BASE) {
		/* Set selection to U_PAGE*/
		XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

		/* Subtract 16MB when accessing second flash*/
		RealAddr = Address & (~FLASH2BASE);

	} else {

		/* Set selection to L_PAGE*/
		XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

		RealAddr = Address;
	}

	/* Assert the Flash chip select.*/
	XQspiPs_SetSlaveSelect(QspiPtr);

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * Flash
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

	/*
	 * Send the write command, address, and data to the Flash to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
				ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the Flash to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial Flash connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 RealAddr;
	int Sector;
	u32 LqspiCr;
	u32 NumSect;

	/*
	 * If erase size is same as the total size of the two flash devices,
	 * use bulk erase command on both flash devices.
	 */
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {


		/* Get the current LQSPI configuration register value*/
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
		/* Set selection to L_PAGE*/
		XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

		/* Assert the Flash chip select.*/
		XQspiPs_SetSlaveSelect(QspiPtr);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/* Setup the bulk erase command*/
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		/* Wait for the erase command to the Flash to be completed*/
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}


		/* Get the current LQSPI configuration register value*/
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
		/* Set selection to U_PAGE*/
		XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

		/* Assert the Flash chip select.*/
		XQspiPs_SetSlaveSelect(QspiPtr);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/* Setup the bulk erase command*/
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		/* Wait for the erase command to the Flash to be completed*/
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		return;
	}

	/*
	 * If entire first flash needs + some sectors in the second flash
	 * need to be erased, then bulk erase first flash here and
	 * second flash sectors will be erased in the following loop.
	 */
	if ((Address == FLASH1BASE) &&
			(ByteCount == (NUM_SECTORS * SECTOR_SIZE) / 2)) {


		/* Get the current LQSPI configuration register value*/
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
		/* Set selection to L_PAGE*/
		XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

		/* Assert the Flash chip select.*/
		XQspiPs_SetSlaveSelect(QspiPtr);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/* Setup the bulk erase command*/
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		/* Wait for the erase command to the Flash to be completed*/
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		/* Increment address to second flash*/
		Address = FLASH2BASE;
	}

	/*
	 * If the erase size is less than the total size of the either flash,
	 * use sector erase command.
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	if (ByteCount % SECTOR_SIZE)
		NumSect = ByteCount/SECTOR_SIZE + 1;
	else
		NumSect = ByteCount/SECTOR_SIZE;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */
	if (((Address + ByteCount) & SECTORMASK) ==
			((Address + (NumSect*SECTOR_SIZE)) & SECTORMASK)) {
		NumSect++;
	}

	/*
	 * Check condition - This is for the case when the byte count is
	 * less than a sector but the address range spreads over two sectors
	 */
	for (Sector = 0; Sector < NumSect; Sector++) {

		/* Get the current LQSPI configuration register value*/
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);

		/* Select lower or upper Flash based on sector address */
		if (Address & FLASH2BASE) {
			/* Set selection to U_PAGE*/
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/* Subtract 16MB when accessing second flash*/
			RealAddr = Address & (~FLASH2BASE);
		} else {

			/* Set selection to L_PAGE*/
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			RealAddr = Address;
		}

		/* Assert the Flash chip select.*/
		XQspiPs_SetSlaveSelect(QspiPtr);
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)(RealAddr >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)(RealAddr >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					SEC_ERASE_SIZE);

		/*
		 * Wait for the sector erase command to the
		 * Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		Address += SECTOR_SIZE;

	}
}

/**
 *
 * This function reads serial FLASH ID connected to the SPI interface.
 *
 *
 * @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashReadID(void)
{
	int Status;

	/* Read ID in Auto mode.*/
	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(&QspiInstance, WriteBuffer, ReadBuffer,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2],
		   ReadBuffer[3]);

	return XST_SUCCESS;
}

/**
 *
 * This function enables quad mode in the serial flash connected to the
 * SPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void FlashQuadEnable(XQspiPs *QspiPtr)
{
	u8 WriteEnableCmd = {WRITE_ENABLE_CMD};
	u8 ReadStatusCmd[] = {READ_STATUS_CMD, 0};
	u8 QuadEnableCmd[] = {WRITE_STATUS_CMD, 0};
	u8 FlashStatus[2];

	if (ReadBuffer[1] == 0x9D) {
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
				FlashStatus, sizeof(ReadStatusCmd));

		QuadEnableCmd[1] = FlashStatus[1] | 1 << 6;

		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));

		XQspiPs_PolledTransfer(QspiPtr, QuadEnableCmd, NULL,
				sizeof(QuadEnableCmd));
	}
}
