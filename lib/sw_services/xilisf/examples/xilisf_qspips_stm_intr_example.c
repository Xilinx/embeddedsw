/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilisf_qspips_flash_intr_example.c
*
*
* This file contains a design example using the XILISF Library in
* interrupt mode with a serial FLASH device. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available.
* The hardware which this example runs on, must have a serial FLASH (Numonyx
* N25Q, Winbond W25Q, or Spansion S25FL) for it to run. This example has been
* tested with the Numonyx Serial Flash (N25Q128).
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
* 1.00  srt 06/20/12 First release
* 1.01  srt 04/26/13 Modified Erase function to perform Write Enable operation
*		     for each sector erase.
* 1.01  srt 08/28/13 Fixed the CR 731919, by setting the proper QSPI options.
* 5.0   sb  08/05/14 Added support for greater than 128MB flash operations.
* 5.14  akm 08/01/19 Initialized Status variable to XST_FAILURE.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/**< EDK generated parameters */
#include "xscugic.h"		/**< Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"
#include <xilisf.h>             /**< Serial Flash Library header file */

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
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define QSPI_INTR_ID		XPAR_XQSPIPS_0_INTR
/*@}*/

/**
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define DATA_OFFSET	4 /**< Start of Data for Read/Write */
#define DUMMY_OFFSET	4 /**< Dummy byte offset for fast, dual
							and quad reads */
#define DUMMY_SIZE	1 /**< Number of dummy bytes for fast,
						dual and quad reads */

/**
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define NUM_PAGES	0x10000 /**< Number of Pages in the flash */
#define PAGE_SIZE	256	/**< Page Size for Read/Write Operation */

/**
 * Number of flash pages to be written.
 */
#define PAGE_COUNT	16	/**< Number of Pages for
							Read/Write Operation */

/**
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS	0x00080000	/**< Test Address in the flash  */
#define UNIQUE_VALUE	0x05		/**< Unique Value for Test */

/**
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA	PAGE_COUNT * PAGE_SIZE	/**< Max Data Calculated by
					 multiplying Page count and Page Size*/

/**
 * The following constant defines the slave select signal that is used to
 * to select the FLASH device on the QSPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_QSPI_SELECT	0x00

#define INTR_MODE 1		/**< Interrupt Mode Enable */

#define FAST_READ_NUM_DUMMY_BYTES	1 /**< Number Dummy Bytes for Fast Read*/
#define DUAL_READ_NUM_DUMMY_BYTES	1 /**< Number Dummy Bytes for Dual Read */
#define QUAD_READ_NUM_DUMMY_BYTES	1 /**< Number Dummy Bytes for Quad Read */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int QspiSetupIntrSystem(XScuGic *IntcInstancePtr,
			XQspiPs *QspiInstancePtr, u16 QspiIntrId);

static void QspiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 QspiIntrId);

void XilIsf_Handler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);

int FlashErase(XIsf *InstancePtr, u32 Address, u32 ByteCount);
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command);
int FlashRead(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command);
int QspiFlashIntrExample(XScuGic *IntcInstancePtr, XQspiPs *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId);
u32 SectorMask(u32 SectorSize);

/************************** Variable Definitions *****************************/

/**
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XScuGic IntcInstance;
static XQspiPs QspiInstance;
static XIsf Isf;

/**
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/**
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int ErrorCount;

/**
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test = 5;

/**
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];		/**< Write Buffer */
u8 IsfWriteBuffer[PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];/**< IsfWrite Buffer
					used in XilISF Initialization */



/**
 * The following defines are for dual flash stacked mode interface.
 */
#define LQSPI_CR_FAST_QUAD_READ		0x0000006B /**< Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		0x00000100 /**< 1 Dummy Byte between
						     address and return data */

#define DUAL_STACK_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)/**< Fast Quad Read output */

#define DUAL_QSPI_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQSPIPS_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)/**< Fast Quad Read output */
/*****************************************************************************/
/**
*
* Main function to call the QSPI Flash example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("QSPI FLASH Interrupt Example Test \r\n");

	/*
	 * Run the Qspi Interrupt example.
	 */
	Status = QspiFlashIntrExample(&IntcInstance, &QspiInstance,
				QSPI_DEVICE_ID, QSPI_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI FLASH Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI FLASH Interrupt Example Test\r\n");
	return XST_SUCCESS;
}


/****************************************************************************/
/**
* The purpose of this function is to illustrate how to use the XQspiPs
* device driver in interrupt mode. This function writes and reads data
* from a serial FLASH.
*
* @param	IntcInstancePtr is the instance of the interrupt
* @param	QspiInstancePtr is the Pointer to the qspi driver instance
* @param	QspiDeviceId is the Device ID of qspi
* @param	QspiIntrId is the interrupt ID for QSPI driver
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
* This function calls other functions which contain loops that may be infinite
* if interrupts are not working such that it may not return. If the device
* slave select is not correct and the device is not responding on bus it will
* read a status of 0xFF for the status register as the bus is pulled up.
*
*****************************************************************************/
int QspiFlashIntrExample(XScuGic *IntcInstancePtr, XQspiPs *QspiInstancePtr,
			 u16 QspiDeviceId, u16 QspiIntrId)
{
	int Status = XST_FAILURE;
	u8 *BufferPtr;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPs_Config *ConfigPtr;	/* Pointer to Configuration ROM data */
	u32 Options;

	/*
	* Lookup the device configuration in the temporary CROM table. Use this
	* configuration info down below when initializing this component.
	*/
	ConfigPtr = XQspiPs_LookupConfig(QspiDeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	* Set the QSPI options
	*/
	Options = XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_MANUAL_START_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION;
	Status = XIsf_SetSpiConfiguration(&Isf, QspiInstancePtr,
			Options, XISF_SPI_PRESCALER);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	if(ConfigPtr->ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED) {
		/*
		 * Enable two flash memories, Shared bus (NOT separate bus),
		 * L_PAGE selected by default
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
						DUAL_STACK_CONFIG_WRITE);
	}

	if(ConfigPtr->ConnectionMode == XQSPIPS_CONNECTION_MODE_PARALLEL) {
		/*
		 * Enable two flash memories on separate buses
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
						DUAL_QSPI_CONFIG_WRITE);
	}


	/* Initialize the XILISF Library */
	XIsf_Initialize(&Isf, QspiInstancePtr, FLASH_QSPI_SELECT,
			IsfWriteBuffer);



	XIsf_SetTransferMode(&Isf, XISF_INTERRUPT_MODE);

	/*
	 * Connect the Qspi device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
	Status = QspiSetupIntrSystem(IntcInstancePtr, QspiInstancePtr,
					QspiIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the QSPI that will be called from the
	 * interrupt context when an QSPI status occurs, specify a pointer to
	 * the QSPI driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XIsf_SetStatusHandler(&Isf, QspiInstancePtr,
				 (XQspiPs_StatusHandler) XilIsf_Handler);

	/*
	 * Initialize the write buffer for a pattern to write to the FLASH
	 * and the read buffer to zero so it can be verified after the read,
	 * the test value that is added to the unique value allows the value
	 * to be changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PAGE_SIZE;
	     Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);
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
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(&Isf, (Page * PAGE_SIZE) + TEST_ADDRESS,
			   PAGE_SIZE, XISF_WRITE);
	}

	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 * command
	 */
	FlashRead(&Isf, TEST_ADDRESS, MAX_DATA, XISF_READ);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */

	BufferPtr = ReadBuffer;

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MAX_DATA;
		Count++, UniqueValue++) {

		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}


	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using Fast Read
	 * command
	 */

	Status = FlashRead(&Isf, TEST_ADDRESS, MAX_DATA, XISF_FAST_READ);
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

		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}


	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using DUAL IO
	 * Fast Read command
	 */
	Status = FlashRead(&Isf, TEST_ADDRESS, MAX_DATA, XISF_DUAL_OP_FAST_READ);
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

		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}


	/*
	 * Read the contents of the FLASH from TEST_ADDRESS, using QUAD OP
	 * Fast Read command
	 */
	Status = FlashRead(&Isf, TEST_ADDRESS, MAX_DATA, XISF_QUAD_OP_FAST_READ);
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
		if (BufferPtr[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	QspiDisableIntrSystem(IntcInstancePtr, QspiIntrId);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the QSPI driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when a transfer of QSPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle QSPI interrupts but is
* application specific.
*
* @param	CallBackRef is a reference passed to the handler.
* @param	StatusEvent is the status of the QSPI .
* @param	ByteCount is the number of bytes transferred.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XilIsf_Handler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	/*
	 * Indicate the transfer on the QSPI bus is no longer in progress
	 * regardless of the status event
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		ErrorCount++;
	}
}

/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the QSPI interface.
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
int FlashWrite(XIsf *InstancePtr, u32 Address, u32 ByteCount, u8 Command)
{
	XIsf_WriteParam WriteParam;

	int Status = XST_FAILURE;

	WriteParam.Address = Address;
	WriteParam.NumBytes = ByteCount;
	WriteParam.WritePtr = WriteBuffer;
	/*
	 * Perform the Write operation.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Write(&Isf, Command, (void*) &WriteParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads from the  serial FLASH connected to the
* QSPI interface.
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
	 * - Number of bytes to be read from the Serial Flash.
	 * - Read Buffer to which the data is to be read.
	 */
	ReadParam.Address = Address;
	ReadParam.NumBytes = ByteCount;
	ReadParam.ReadPtr = ReadBuffer;
	if(Command == XISF_FAST_READ){
		ReadParam.NumDummyBytes = FAST_READ_NUM_DUMMY_BYTES;
	}
	else if(Command == XISF_DUAL_OP_FAST_READ){
		ReadParam.NumDummyBytes = DUAL_READ_NUM_DUMMY_BYTES;
	}
	else{
		ReadParam.NumDummyBytes = QUAD_READ_NUM_DUMMY_BYTES;
	}

	/*
	 * Perform the Read operation.
	 */
	TransferInProgress = TRUE;
	Status = XIsf_Read(&Isf, Command, (void*) &ReadParam);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till the Transfer is complete and check if there are any errors
	 * in the transaction.
	 */
	while(TransferInProgress);
	if(ErrorCount != 0) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPI interface.
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
		 * Perform the Bulk Erase operation.
		 */

		TransferInProgress = TRUE;
		Status = XIsf_Erase(InstancePtr, XISF_BULK_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait till the Transfer is complete and check if there
		 * are any errors in the transaction.
		 */
		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

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
		 * Perform the Bulk Erase operation.
		 */
		TransferInProgress = TRUE;
		Status = XIsf_Erase(InstancePtr, XISF_SECTOR_ERASE, Address);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait till the Transfer is complete and check if there
		 * are any errors in the transaction.
		 */
		while(TransferInProgress);
		if(ErrorCount != 0) {
			return XST_FAILURE;
		}

		Address += SectorSize;
	}
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function setups the interrupt system for an Qspi device.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc device.
* @param	QspiInstancePtr is a pointer to the instance of the Qspi device.
* @param	QspiIntrId is the interrupt Id for an QSPI device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int QspiSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XQspiPs *QspiInstancePtr, u16 QspiIntrId)
{
	int Status = XST_FAILURE;

	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				IntcInstancePtr);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, QspiIntrId,
				(Xil_ExceptionHandler)XQspiPs_InterruptHandler,
				(void *)QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Qspi device.
	 */
	XScuGic_Enable(IntcInstancePtr, QspiIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the Qspi device.
*
* @param	IntcInstancePtr is the pointer to an INTC instance.
* @param	QspiIntrId is the interrupt Id for an QSPI device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void QspiDisableIntrSystem(XScuGic *IntcInstancePtr, u16 QspiIntrId)
{
	/*
	 * Disable the interrupt for the QSPI device.
	 */
	XScuGic_Disable(IntcInstancePtr, QspiIntrId);

	/*
	 * Disconnect and disable the interrupt for the Qspi device.
	 */
	XScuGic_Disconnect(IntcInstancePtr, QspiIntrId);
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
