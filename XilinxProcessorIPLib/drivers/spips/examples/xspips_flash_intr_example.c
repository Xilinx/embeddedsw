/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspips_flash_intr_example.c
*
* @brief Implements a design example using the SPI driver (XSpiPs) in
* interrupt mode with a serial flash device.
*
* The example performs transfers in Manual start mode using interrupts.
* The hardware which this example runs on, must have a serial flash
* for it to run. The example has been tested with SST25W080.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  sg  1/30/13  First release
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation.
* 3.2  nsk  03/26/19 Add support for versal
* 3.9  sb   07/05/23 Added support for system device-tree flow.
* 3.13 sb   09/09/25 Fix flash status register check in erase function
* 3.14 vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* 3.15 vn   08/26/26 Fix Doxygen warnings.
* 3.15 tb   09/30/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/** @addtogroup xspips_flash_intr_example
 * @{
 */

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xplatform_info.h"
#include "xspips.h"		/* SPI device driver */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID	/**< SPI device ID. */
/** Interrupt controller device ID. */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define SPI_INTR_ID		XPAR_XSPIPS_1_INTR /**< SPI interrupt ID. */
#endif

/*
 * The following constants define the commands which may be sent to the flash
 * device.
 */
#define WRITE_STATUS_CMD	0x01	/**< Write status register command. */
#define WRITE_CMD		0x02	/**< Page program command. */
#define READ_CMD		0x03	/**< Read data command. */
#define WRITE_DISABLE_CMD	0x04	/**< Write disable command. */
#define READ_STATUS_CMD		0x05	/**< Read status register command. */
#define WRITE_ENABLE_CMD	0x06	/**< Write enable command. */
#define FAST_READ_CMD		0x0B	/**< Fast read command. */
#define CHIP_ERASE_CMD		0x60	/**< Chip erase command. */
#define BULK_ERASE_CMD		0xC7	/**< Bulk erase command. */
#define	BLOCK_ERASE_64K_CMD	0xD8	/**< 64 KB block erase command. */
#define READ_ID			0x90	/**< Read ID command. */
#define SST_READ_ID		0x9F	/**< SST flash read ID command. */

/** Global Block-Protection Unlock register command. */
#define GLOBAL_BLK_PROT_UNLK	0x98

/** All SST flash parts have ID 0xBF. */
#define SST_FLASH_ID		0xBF

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /**< Flash instruction. */
#define ADDRESS_1_OFFSET	1 /**< MSB byte of address to read or write. */
/** Middle byte of address to read or write. */
#define ADDRESS_2_OFFSET	2
#define ADDRESS_3_OFFSET	3 /**< LSB byte of address to read or write. */
#define DATA_OFFSET		4 /**< Start of Data for Read/Write. */
#define DUMMY_SIZE		1 /**< Number of dummy bytes for fast read. */
#define RD_ID_SIZE		4 /**< Read ID command + 3 bytes ID response. */

/** Flag stating sst flash or not. */
static int is_sst;


/*
 * The following constants specify the extra bytes which are sent to the
 * flash on the SPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4	/**< Command and address bytes. */

#define UNIQUE_VALUE		0x55	/**< Unique test data value. */

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the flash.
 */
#define MAX_DATA		1024*1024	/**< Max data size. */

/*
 * The following constant defines the slave select signal that
 * selects the flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_SPI_SELECT_1	0x01	/**< Flash on chip select 1. */
#define FLASH_SPI_SELECT_0	0x00	/**< Flash on chip select 0. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
static int SpiPsSetupIntrSystem(XScuGic *IntcInstancePtr,
				XSpiPs *SpiInstancePtr, u16 SpiIntrId);

static void SpiPsDisableIntrSystem(XScuGic *IntcInstancePtr, u16 SpiIntrId);
#endif

void SpiPsHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);

static void FlashErase(XSpiPs *SpiPtr);

static void FlashWrite(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command);

static void FlashRead(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command);

static int FlashReadID(XSpiPs *SpiInstance);

#ifndef SDT
int SpiPsFlashIntrExample(XScuGic *IntcInstancePtr, XSpiPs *SpiInstancePtr,
			  u16 SpiDeviceId, u16 SpiIntrId);
#else
int SpiPsFlashIntrExample(XSpiPs *SpiInstancePtr, UINTPTR BaseAddress);
#endif

static int SST_GlobalBlkProtectUnlk(XSpiPs *SpiInstancePtr);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
static XScuGic IntcInstance;	/**< Interrupt controller instance. */
#endif
static XSpiPs SpiInstance;	/**< SPI driver instance. */
#endif

/**
 * @brief Indicates that an SPI transfer is in progress.
 *
 * The variable is shared between non-interrupt processing and
 * interrupt processing such that it must be global.
 */
volatile int TransferInProgress;

/**
 * @brief Tracks any errors that occur during interrupt processing.
 */
int Error;

/**
 * @brief Holds the write address location in the serial flash.
 */
static int TestAddress;

/*
 * The following variables read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
/** Buffer for the data read from the flash. */
u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[MAX_DATA + DATA_OFFSET];	/**< Data to write to the flash. */

#ifndef TESTAPP_GEN
/*****************************************************************************/
/**
*
* @brief Calls the SPI Flash interrupt example.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SPI FLASH Interrupt Example Test \r\n");

	/*
	 * Run the Spi Interrupt example.
	 */
#ifndef SDT
	Status = SpiPsFlashIntrExample(&IntcInstance, &SpiInstance,
				       SPI_DEVICE_ID, SPI_INTR_ID);
#else
	Status = SpiPsFlashIntrExample(&SpiInstance, XPAR_XSPIPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("SPI FLASH Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI FLASH Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif
/*****************************************************************************/
/**
*
* @brief Illustrates how to use the XSpiPs device driver in interrupt mode.
*
* The function writes and reads data from a serial flash.
*
* @param[in]	SpiInstancePtr Pointer to the SPI driver instance to use.
* @if SDT
* @param[in]	BaseAddress The base address of the device.
* @else
* @param[in]	IntcInstancePtr Pointer to Interrupt Controller instance.
* @param[in]	SpiDeviceId The Instance Id of SPI in the system.
* @param[in]	SpiIntrId The Interrupt Id for SPI in the system.
* @endif
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		- In XSCT/classic flow, the device configuration is looked up
*               based on DeviceId.
*               - The function calls other functions which contain
*               loops that may be infinite if interrupts are not working such
*               that it may not return. If the device slave select is not
*               correct and the device is not responding on bus it
*               reads a status of 0xFF for the status register as the bus is
*               pulled up.
*
*****************************************************************************/
#ifndef SDT
int SpiPsFlashIntrExample(XScuGic *IntcInstancePtr, XSpiPs *SpiInstancePtr,
			  u16 SpiDeviceId, u16 SpiIntrId)
#else
int SpiPsFlashIntrExample(XSpiPs *SpiInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	u32 Count;
	u32 MaxSize = MAX_DATA;
	u32 ChipSelect = FLASH_SPI_SELECT_1;
	XSpiPs_Config *SpiConfig;
	u32 Platform;

	Platform = XGetPlatform_Info();
	if ((Platform == XPLAT_ZYNQ_ULTRA_MP) || (Platform == XPLAT_VERSAL)) {
		MaxSize = 1024 * 10;
		ChipSelect = FLASH_SPI_SELECT_0;	/* Device is on CS 0 */
#ifndef SDT
		SpiIntrId = XPAR_XSPIPS_0_INTR;
#endif
	}

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

	Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig,
				      SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the Spi device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
#ifndef SDT
	Status = SpiPsSetupIntrSystem(IntcInstancePtr, SpiInstancePtr,
				      SpiIntrId);
#else
	Status = XSetupInterruptSystem(SpiInstancePtr, &XSpiPs_InterruptHandler,
				       SpiConfig->IntrId,
				       SpiConfig->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the SPI that will be called from the
	 * interrupt context when an SPI status occurs, specify a pointer to
	 * the SPI driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XSpiPs_SetStatusHandler(SpiInstancePtr, SpiInstancePtr,
				(XSpiPs_StatusHandler) SpiPsHandler);

	/*
	 * Set the SPI device as a master with manual start and manual
	 * chip select mode options
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MANUAL_START_OPTION | \
			  XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);

	/*
	 * Set the SPI device pre-scalar to divide by 8
	 */
	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_8);

	memset(WriteBuffer, 0x00, sizeof(WriteBuffer));
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Initialize the write buffer for a pattern to write to the flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue);
	}

	/*
	 * Assert the flash chip select
	 */
	XSpiPs_SetSlaveSelect(SpiInstancePtr, ChipSelect);

	/*
	 * Read the flash ID
	 */
	Status = FlashReadID(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI FLASH Interrupt Example Read ID Failed\r\n");
		return XST_FAILURE;
	}

	if (is_sst == 1) {
		/* Unlock  the Global Block-Protection Unlock register bits */
		SST_GlobalBlkProtectUnlk(SpiInstancePtr);
		if (Status != XST_SUCCESS) {
			xil_printf("SPI Flash Interrupt Example Read ID Failed\r\n");
			return XST_FAILURE;
		}
	}

	/*
	 * Erase the flash
	 */
	FlashErase(SpiInstancePtr);

	/*
	 * Write the data in the write buffer to TestAddress in serial flash
	 */
	FlashWrite(SpiInstancePtr, TestAddress, MaxSize, WRITE_CMD);

	/*
	 * Read the contents of the flash from TestAddress of size MAX_DATA
	 * using Normal Read command
	 */
	FlashRead(SpiInstancePtr, TestAddress, MaxSize, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue)) {
			return XST_FAILURE;
		}
	}

	/*
	 * Set the SPI device as a master with auto start and manual
	 * chip select mode options
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MASTER_OPTION | \
			  XSPIPS_FORCE_SSELECT_OPTION);

	memset(WriteBuffer, 0x00, sizeof(WriteBuffer));
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Initialize the write buffer for a pattern to write to the flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
	     Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue);
	}

	/*
	 * Erase the flash
	 */
	FlashErase(SpiInstancePtr);

	/*
	 * Write the data in the write buffer to TestAddress in serial flash
	 */
	FlashWrite(SpiInstancePtr, TestAddress, MaxSize, WRITE_CMD);

	/*
	 * Read the contents of the flash from TestAddress of size MAX_DATA
	 * using Normal Read command
	 */
	FlashRead(SpiInstancePtr, TestAddress, MaxSize, READ_CMD);

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	BufferPtr = &ReadBuffer[DATA_OFFSET];
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxSize;
	     Count++, UniqueValue++) {
		if (BufferPtr[Count] != (u8)(UniqueValue)) {
			return XST_FAILURE;
		}
	}

#ifndef SDT
	SpiPsDisableIntrSystem(IntcInstancePtr, SpiIntrId);
#else
	XDisconnectInterruptCntrl(SpiConfig->IntrId, SpiConfig->IntrParent);
#endif
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Handles the status events of the SPI driver.
*
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when a transfer of SPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle SPI interrupts but is
* application specific.
*
* @param[in]	CallBackRef A reference passed to the handler.
* @param[in]	StatusEvent The status of the SPI .
* @param[in]	ByteCount The number of bytes transferred.
*
******************************************************************************/
void SpiPsHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
	(void)CallBackRef;
	(void)ByteCount;
	/*
	 * Indicate the transfer on the SPI bus is no longer in progress
	 * regardless of the status event
	 */
	TransferInProgress = FALSE;

	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		Error++;
	}
}

/*****************************************************************************/
/**
*
* @brief Writes to the serial flash connected to the SPI interface.
*
* The flash contains a 256 byte write buffer which can be filled and then a
* write is automatically performed by the device.  All the data put into the
* buffer must be in the same page of the device with page boundaries being on
* 256 byte boundaries.
*
* @param[in,out]	SpiPtr Pointer to the SPI driver component to use.
* @param[in]	Address The address to write data to in the flash.
* @param[in]	ByteCount The number of bytes to write.
* @param[in]	Command The command used to write data to the flash. SPI
*		device supports only Page Program command to write data to the
*		flash.
*
******************************************************************************/
static void FlashWrite(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 WriteDisableCmd = { WRITE_DISABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
	u32 Temp = 0;
	u32 TempAddress = Address;
	u8 TempBuffer[5];

	if (Command == WRITE_CMD) {
		for (Temp = 0; Temp < ByteCount ; Temp++, TempAddress++) {
			/*
			 * Send the write enable command to the flash so
			 * that it can be written to, this needs to be sent
			 * as a separate transfer before the write
			 */
			TransferInProgress = TRUE;

			XSpiPs_Transfer(SpiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

			/*
			 * Wait for the transfer on the SPI bus to be complete before
			 * proceeding
			 */
			while (TransferInProgress);

			/*
			 * Setup the write command with the specified address
			 * and data for the flash
			 */
			TempBuffer[COMMAND_OFFSET] = Command;
			TempBuffer[ADDRESS_1_OFFSET] =
				(u8)((TempAddress & 0xFF0000) >> 16);
			TempBuffer[ADDRESS_2_OFFSET] =
				(u8)((TempAddress & 0xFF00) >> 8);
			TempBuffer[ADDRESS_3_OFFSET] =
				(u8)(TempAddress & 0xFF);
			TempBuffer[DATA_OFFSET] =
				WriteBuffer[DATA_OFFSET + Temp];

			/*
			 * Send the write command, address, and data to the
			 * flash to be written, no receive buffer is specified
			 * since there is nothing to receive
			 */
			TransferInProgress = TRUE;

			XSpiPs_Transfer(SpiPtr, TempBuffer, NULL, 5);

			while (TransferInProgress);


			/*
			 * Wait for the write command to the flash to be ,
			 * completed it takes some time for the data to be
			 * written
			 */
			while (1) {
				/*
				 * Poll the status register of the flash to
				 * determine when it completes, by sending
				 * a read status command and receiving the
				 * status byte
				 */
				TransferInProgress = TRUE;

				XSpiPs_Transfer(SpiPtr, ReadStatusCmd,
						FlashStatus, sizeof(ReadStatusCmd));

				/*
				 * Wait for the transfer on the SPI bus
				 * to be complete before proceeding
				 */
				while (TransferInProgress);

				/*
				 * If the status indicates the write is done,
				 * then stop waiting, if a value of 0xFF in
				 * the status byte is read from the device
				 * and this loop never exits, the device slave
				 * select is possibly incorrect such that the
				 * device status is not being read
				 */
				if ((FlashStatus[1] & 0x01) == 0) {
					break;
				}
			}

			TransferInProgress = TRUE;

			XSpiPs_Transfer(SpiPtr, &WriteDisableCmd, NULL,
					sizeof(WriteDisableCmd));

			/*
			 * Wait for the transfer on the SPI bus to be complete
			 * before proceeding
			 */
			while (TransferInProgress);
		}
	}
}

/*****************************************************************************/
/**
*
* @brief Reads from the serial flash connected to the SPI interface.
*
* @param[in,out]	SpiPtr Pointer to the SPI driver component to use.
* @param[in]	Address The address to read data from in the flash.
* @param[in]	ByteCount The number of bytes to read.
* @param[in]	Command The command used to read data from the flash. SPI
*		device supports one of the Read, Fast Read, Dual Read and Fast
*		Read commands to read data from the flash.
*
******************************************************************************/
static void FlashRead(XSpiPs *SpiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	/*
	 * Setup the write command with the specified address and data for the
	 * flash
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	/*
	 * Send the read command to the flash to read the specified number
	 * of bytes from the flash, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiPtr, WriteBuffer, ReadBuffer,
			ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the transfer on the SPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);
}

/*****************************************************************************/
/**
*
* @brief Unlocks the Global Block-Protection Unlock register bits.
*
* @param[in,out] SpiInstancePtr Pointer to the SPI driver instance to use.
*
* @return
*               - XST_SUCCESS if successful
*               - XST_FAILURE if not successful
*
******************************************************************************/
static int SST_GlobalBlkProtectUnlk(XSpiPs *SpiInstancePtr)
{
	int Status;
	u8 WriteEnable[] = { WRITE_ENABLE_CMD };
	u8 ulbpr[] = { GLOBAL_BLK_PROT_UNLK };

	/* send write enable */
	Status = XSpiPs_PolledTransfer(SpiInstancePtr, WriteEnable, NULL, sizeof(WriteEnable));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Unlock  the Global Block-Protection Unlock register bits */
	Status = XSpiPs_PolledTransfer(SpiInstancePtr, ulbpr, NULL, sizeof(ulbpr));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Reads the ID of the serial flash connected to the SPI interface.
*
* @param[in,out]	SpiInstance Pointer to the SPI driver instance to use.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
******************************************************************************/
static int FlashReadID(XSpiPs *SpiInstance)
{
	u8 Index;
	u8 ByteCount = 4;
	u8 SendBuffer[8];
	u8 RecvBuffer[8];

	SendBuffer[0] = READ_ID;
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;

	for (Index = 0; Index < ByteCount; Index++) {
		SendBuffer[4 + Index] = 0x00;
	}

	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiInstance, SendBuffer, RecvBuffer,
			(4 + ByteCount));

	while (TransferInProgress);

	if ((RecvBuffer[4] == 0xff) || (RecvBuffer[4] == 0x00)) {
		/* Use SST_READ_ID(0x9f) for reading id*/
		SendBuffer[0] = SST_READ_ID;
		XSpiPs_PolledTransfer(SpiInstance, SendBuffer, RecvBuffer,
				      (4 + ByteCount));

		while (TransferInProgress);

		if (RecvBuffer[4] == SST_FLASH_ID) {
			/* SST flash part */
			is_sst = 1;
		}
	}

	for (Index = 0; Index < ByteCount; Index++) {
		xil_printf("ID : %0x\r\n", RecvBuffer[4 + Index]);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Erases the sectors in the serial flash connected to the SPI interface.
*
* @param[in,out]	SpiPtr Pointer to the SPI driver component to use.
*
******************************************************************************/
static void FlashErase(XSpiPs *SpiPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	/* must send 2 bytes */
	u8 WriteStatusCmd[] = { WRITE_STATUS_CMD, 0x0 };
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));
	/*
	 * Wait for the transfer on the SPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);

	/*
	 * Wait for write enable command to the flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		TransferInProgress = TRUE;

		XSpiPs_Transfer(SpiPtr, ReadStatusCmd, FlashStatus,
				sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete
		 * before proceeding
		 */
		while (TransferInProgress);

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x02) == 0x02) {
			break;
		}
	}

	/*
	 * Clear write protect bits using write status command to the flash
	 * this needs to be sent as a separate transfer before the erase
	 */
	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiPtr, WriteStatusCmd, NULL, sizeof(WriteStatusCmd));
	/*
	 * Wait for the transfer on the SPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);

	/*
	 * Check for write status command to the flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		TransferInProgress = TRUE;

		XSpiPs_Transfer(SpiPtr, ReadStatusCmd, FlashStatus,
				sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete
		 * before proceeding
		 */
		while (TransferInProgress);

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x3D) == 0x0) {
			break;
		}
	}

	/*
	 * Send the write enable command to the flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));
	/*
	 * Wait for the transfer on the SPI bus to be complete before
	 * proceeding
	 */
	while (TransferInProgress);

	/*
	 * Wait for write enable command to the flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		TransferInProgress = TRUE;

		XSpiPs_Transfer(SpiPtr, ReadStatusCmd, FlashStatus,
				sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete
		 * before proceeding
		 */
		while (TransferInProgress);

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x02) == 0x02) {
			break;
		}
	}

	/*
	 * Setup the bulk erase or chip-erase command
	 */
	if (is_sst == 0) {
		WriteBuffer[COMMAND_OFFSET] = CHIP_ERASE_CMD;
	} else {
		WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;
	}

	/*
	 * Send the bulk erase command; no receive buffer is specified
	 * since there is nothing to receive
	 */
	TransferInProgress = TRUE;

	XSpiPs_Transfer(SpiPtr, WriteBuffer, NULL, 1);

	while (TransferInProgress);

	/*
	 * Wait for the erase command to the flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		TransferInProgress = TRUE;

		XSpiPs_Transfer(SpiPtr, ReadStatusCmd, FlashStatus,
				sizeof(ReadStatusCmd));

		/*
		 * Wait for the transfer on the SPI bus to be complete
		 * before proceeding
		 */
		while (TransferInProgress);

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

}

#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Sets up the interrupt system for an Spi device.
*
* @param[in]	IntcInstancePtr Pointer to the instance of the Intc device.
* @param[in]	SpiInstancePtr Pointer to the instance of the Spi device.
* @param[in]	SpiIntrId The interrupt Id for an SPI device.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
******************************************************************************/
static int SpiPsSetupIntrSystem(XScuGic *IntcInstancePtr,
				XSpiPs *SpiInstancePtr, u16 SpiIntrId)
{
	int Status;

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
	Status = XScuGic_Connect(IntcInstancePtr, SpiIntrId,
				 (Xil_ExceptionHandler)XSpiPs_InterruptHandler,
				 (void *)SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Spi device.
	 */
	XScuGic_Enable(IntcInstancePtr, SpiIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Disables the interrupts that occur for the Spi device.
*
* @param[in]	IntcInstancePtr Pointer to an INTC instance.
* @param[in]	SpiIntrId The interrupt Id for an SPI device.
*
******************************************************************************/
static void SpiPsDisableIntrSystem(XScuGic *IntcInstancePtr, u16 SpiIntrId)
{
	/*
	 * Disable the interrupt for the SPI device.
	 */
	XScuGic_Disable(IntcInstancePtr, SpiIntrId);

	/*
	 * Disconnect and disable the interrupt for the Spi device.
	 */
	XScuGic_Disconnect(IntcInstancePtr, SpiIntrId);
}
#endif
/** @} */
