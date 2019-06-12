/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xospipsv_flash_intr_example.c
*
*
* This file contains a design example using the OSPIPSV driver (xospipsv)
* The example writes to flash in IO mode and reads it back in DMA mode.
* It runs in interrupt mode.
* The hardware which this example runs on, must have an octal serial Flash
* (Micron) for it to run.
*
* This example has been tested with the Micron Octal Serial Flash (mt35xu01gbba).
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
* 1.0   sk  01/09/19 First release
* 1.0	akm 03/29/19 Fixed data alignment issues on IAR compiler
* 1.1   sk  07/23/19 Based on RX Tuning, updated the dummy cycles.
*       sk  08/08/19 Issue device reset to bring back to default state.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xil_printf.h"
#include "xil_cache.h"
#include "xscugic.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */

#define WRITE_STATUS_CMD	0x01
#define WRITE_DISABLE_CMD	0x04
#define WRITE_ENABLE_CMD	0x06
#define BULK_ERASE_CMD		0xC7
#define DIE_ERASE_CMD		0xC4
#define READ_ID			0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01
#define READ_FLAG_STATUS_CMD	0x70
#define WRITE_CMD_4B		0x12
#define SEC_ERASE_CMD_4B	0xDC
#define READ_CMD_OCTAL_IO_4B	0xCC
#define READ_CMD_OCTAL_DDR	0x9D
#define WRITE_CMD_OCTAL_4B	0x84
#define ENTER_4B_ADDR_MODE	0xB7
#define EXIT_4B_ADDR_MODE	0xE9
#define WRITE_CONFIG_REG	0x81
#define READ_CONFIG_REG		0x85

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000
#define ONEMB	0x100000

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x5B
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x1A
 */
#define	MICRON_OCTAL_ID_BYTE0	0x2c
#define MICRON_OCTAL_ID_BYTE2_512	0x1a

/*
 * The index for Flash config table
 */
/* Spansion*/
#define MICRON_INDEX_START			0
#define FLASH_CFG_TBL_OCTAL_SINGLE_512_MC	MICRON_INDEX_START

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define OSPIPSV_INTR_ID		XPS_OSPI_INT_ID

/*
 * Number of flash pages to be written.
 */
#define PAGE_COUNT		32

/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/*
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x0


#define UNIQUE_VALUE		0x09

/**************************** Type Definitions *******************************/

typedef struct{
	u32 SectSize;		/* Individual sector size or
						 * combined sector size in case of parallel config*/
	u32 NumSect;		/* Total no. of sectors in one/two flash devices */
	u32 PageSize;		/* Individual page size or
				 * combined page size in case of parallel config*/
	u32 NumPage;		/* Total no. of pages in one/two flash devices */
	u32 FlashDeviceSize;	/* This is the size of one flash device
				 * NOT the combination of both devices, if present
				 */
	u8 ManufacturerID;	/* Manufacturer ID - used to identify make */
	u8 DeviceIDMemSize;	/* Byte of device ID indicating the memory size */
	u32 SectMask;		/* Mask to get sector start address */
	u8 NumDie;		/* No. of die forming a single flash */
	u32 ReadCmd;		/* Read command used to read data from flash */
	u32 WriteCmd;	/* Write command used to write data to flash */
	u32 EraseCmd;	/* Erase Command */
	u8 StatusCmd;	/* Status Command */
	u8 DummyCycles;	/* Number of Dummy cycles for Read operation */
}FlashInfo;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int OspiPsvInterruptFlashExample(XScuGic *IntcInstancePtr, XOspiPsv *OspiPsvInstancePtr,
		u16 OspiPsvDeviceId);
static int OspiPsvSetupIntrSystem(XScuGic *IntcInstancePtr,
		XOspiPsv *OspiPsvInstancePtr);
static void OspiPsvDisableIntrSystem(XScuGic *IntcInstancePtr, u16 OspiPsvIntrId);
void OspiPsvHandler(void *CallBackRef, u32 StatusEvent);
int FlashReadID(XOspiPsv *OspiPsvPtr);
int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr);
int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr);

int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
u32 GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address);
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
int FlashRegisterRead(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
int FlashRegisterWrite(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn);
s32 InitCmd(XOspiPsv *OspiPsvInstancePtr);
int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable);
int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode);

/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
#ifdef __ICCARM__
#pragma data_alignment = 4
u8 ReadBfrPtr[8];
#else
u8 ReadBfrPtr[8]__attribute__ ((aligned(4)));
#endif
FlashInfo Flash_Config_Table[1] = {
	/* Micron */
	{0x20000, 0x200, 256, 0x40000, 0x4000000, MICRON_OCTAL_ID_BYTE0,
		MICRON_OCTAL_ID_BYTE2_512, 0xFFFF0000, 1, READ_CMD_OCTAL_IO_4B,
		(WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16}
};

u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;
static XScuGic IntcInstance;

static XOspiPsv_Msg FlashMsg;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile int TransferInProgress;

/*
 * The following variable tracks any errors that occur during interrupt
 * processing
 */
int Error;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 1;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
#ifdef __ICCARM__
#pragma data_alignment = 64
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#pragma data_alignment = 4
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#else
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(64)));
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(4)));
#endif

u8 CmdBfr[8];

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT*256;

/*****************************************************************************/
/**
*
* Main function to call the OSPIPSV Flash Interrupt example.
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

	xil_printf("OSPIPSV Flash Interrupt Example Test\r\n");

	/*
	 * Run the OspiPsv interrupt example.
	 */
	Status = OspiPsvInterruptFlashExample(&IntcInstance, &OspiPsvInstance,
					OSPIPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("OSPIPSV Flash Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran OSPIPSV Flash Interrupt Example \r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XOspiPsv
* device driver using OSPI flash devices greater than or equal to 128Mb.
* This function reads data in either DMA or DAC mode.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int OspiPsvInterruptFlashExample(XScuGic *IntcInstancePtr,
				XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page = 0;
	XOspiPsv_Config *OspiPsvConfig;
	int ReadBfrSize;
	ReadBfrSize = (PAGE_COUNT * MAX_PAGE_SIZE);

	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Initialize the OSPIPSV driver so that it's ready to use
	 */
	OspiPsvConfig = XOspiPsv_LookupConfig(OspiPsvDeviceId);
	if (NULL == OspiPsvConfig) {
		return XST_FAILURE;
	}
	/* To test, change connection mode here if not obtained from HDF */

	Status = XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiPsvConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = OspiPsvSetupIntrSystem(IntcInstancePtr, OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XOspiPsv_SetStatusHandler(OspiPsvInstancePtr, OspiPsvInstancePtr,
					 (XOspiPsv_StatusHandler) OspiPsvHandler);
	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);
	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_12);

	Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set Flash device and Controller modes */
	Status = FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
		Count < Flash_Config_Table[FCTIndex].PageSize * PAGE_COUNT;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);

	}

	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	Status = FlashErase(OspiPsvInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("WriteCmd: 0x%x\n\r", (u8)Flash_Config_Table[FCTIndex].WriteCmd);
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		Status = FlashIoWrite(OspiPsvInstancePtr,
		(Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
		((Flash_Config_Table[FCTIndex].PageSize)), WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}

	Status = FlashRead(OspiPsvInstancePtr, TEST_ADDRESS, MaxData,
			CmdBfr, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxData;
	     Count++, UniqueValue++) {
		if (ReadBuffer[Count] != (u8)(UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 0);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Set Extended SPI Mode in OSPI device and Controller */
	Status = FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	OspiPsvDisableIntrSystem(IntcInstancePtr, OSPIPSV_INTR_ID);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to determine the number of lines used
* for command, address and data
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Read is to tell whether a read or write
*
* @return	returns value to program the lines for command, address and data.
*
* @note		None.
*
*****************************************************************************/
u32 XOspiPsv_Get_Proto(XOspiPsv *OspiPsvInstancePtr, int Read)
{
	u32 Val;

	if(Read) {
		Val = XOSPIPSV_READ_1_8_8;
	} else {
		if (OspiPsvInstancePtr->OpMode == XOSPIPSV_IDAC_MODE) {
			Val = XOSPIPSV_WRITE_1_1_1;
		} else {
			Val = XOSPIPSV_WRITE_1_1_8;
		}
	}
	return Val;
}

/*****************************************************************************/
/**
*
* Reads the flash ID and identifies the flash in FCT table.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status;
	int ReadIdBytes = 8;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ReadIdBytes;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Dummy += 8;
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
	}
	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}


	xil_printf("FlashID = ");
	while(ReadIdBytes >= 0 ) {
		xil_printf("0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		if (ReadIdBytes >= 5) {
			OspiPsvPtr->DeviceIdData |= (ReadBfrPtr[FlashMsg.ByteCount -
				ReadIdBytes] << ((FlashMsg.ByteCount - ReadIdBytes) * 8));
		}
		ReadIdBytes--;
	}
	xil_printf("\n\r");
	/*
	 * Deduce flash make
	 */
	if (ReadBfrPtr[0] == MICRON_OCTAL_ID_BYTE0) {
		FlashMake = MICRON_OCTAL_ID_BYTE0;
	}

	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((FlashMake == MICRON_OCTAL_ID_BYTE0)) &&
		(ReadBfrPtr[2] == MICRON_OCTAL_ID_BYTE2_512)) {
		FCTIndex = FLASH_CFG_TBL_OCTAL_SINGLE_512_MC;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 Bytestowrite;

	while(ByteCount != 0) {
		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		if(ByteCount <= 8) {
			Bytestowrite = ByteCount;
			ByteCount = 0;
		} else {
			Bytestowrite = 8;
			ByteCount -= 8;
		}

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].WriteCmd;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = WriteBfrPtr;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = Bytestowrite;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
		FlashMsg.Dummy = 0;
		FlashMsg.Addrsize = 4;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Addr = Address;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		}
		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		WriteBfrPtr += 8;
		Address += 8;

		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while(TransferInProgress);
			if (Error) {
				return XST_FAILURE;
			}

			if ((FlashStatus[0] & 0x80) != 0)
				break;
		}
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial Flash connected to the
* OSPIPSV interface.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	int Sector;
	u32 NumSect;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
		(Flash_Config_Table[FCTIndex]).SectSize) ) {


		if(Flash_Config_Table[FCTIndex].NumDie == 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8));
			/*
			 * Call Bulk erase
			 */
			BulkErase(OspiPsvPtr, WriteBfrPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie > 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16));
			/*
			 * Call Die erase
			 */
			DieErase(OspiPsvPtr, WriteBfrPtr);
		}
		return 0;
	}

	xil_printf("EraseCmd 0x%x\n\r", (u8)Flash_Config_Table[FCTIndex].EraseCmd);
	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/(Flash_Config_Table[FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & Flash_Config_Table[FCTIndex].SectMask) ==
		((Address + (NumSect * Flash_Config_Table[FCTIndex].SectSize)) &
		Flash_Config_Table[FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {
		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */

		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].EraseCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Addr = Address;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}
			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while(TransferInProgress);
			if (Error) {
				return XST_FAILURE;
			}

			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		Address += Flash_Config_Table[FCTIndex].SectSize;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function performs read. DMA is the default setting.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u8 Status;

	xil_printf("ReadCmd 0x%x\r\n", Flash_Config_Table[FCTIndex].ReadCmd);
	FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
	FlashMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles +
			OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Dummy = 16 + OspiPsvPtr->Extra_DummyCycle;
	}
	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}
	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8);
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}
	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}
	while (1) {
		FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		if ((FlashStatus[0] & 0x80) != 0)
			break;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	u8 DieCnt;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;

	for(DieCnt = 0; DieCnt < Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16);
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while(TransferInProgress);
			if (Error) {
				return XST_FAILURE;
			}
			if ((FlashStatus[0] & 0x80) != 0)
				break;
		}
	}
	return 0;
}

/*****************************************************************************/
/**
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable)
{
	int Status;
	u8 Command;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif

	if(Enable)
		Command = ENTER_4B_ADDR_MODE;
	else
		Command = EXIT_4B_ADDR_MODE;

	switch (FlashMake) {
		case MICRON_OCTAL_ID_BYTE0:
			FlashMsg.Opcode = WRITE_ENABLE_CMD;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = NULL;
			FlashMsg.ByteCount = 0;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
			}
			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while(TransferInProgress);
			if (Error) {
				return XST_FAILURE;
			}
			break;

		default:
			break;
	}
	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = 0;
	FlashMsg.Addrsize = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 3;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	while (1) {
		FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}

		if ((FlashStatus[0] & 0x80) != 0)
			break;

	}

	switch (FlashMake) {
	case MICRON_OCTAL_ID_BYTE0:
		FlashMsg.Opcode = WRITE_DISABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while(TransferInProgress);
		if (Error) {
			return XST_FAILURE;
		}
		break;

		default:
			break;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system for a OspiPsv device.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc
 *		device.
 * @param	OspiPsvInstancePtr is a pointer to the instance of the
 *		OspiPsv device.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
static int OspiPsvSetupIntrSystem(XScuGic *IntcInstancePtr,
			       XOspiPsv *OspiPsvInstancePtr)
{
	int Status;

	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (IntcConfig == NULL) {
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
	Status = XScuGic_Connect(IntcInstancePtr, OSPIPSV_INTR_ID,
			(Xil_ExceptionHandler)XOspiPsv_IntrHandler,
			(void *)OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the OspiPsv device.
	 */
	XScuGic_Enable(IntcInstancePtr, OSPIPSV_INTR_ID);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function disables the interrupts that occur for the OspiPsv device.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc device.
 * @param	OspiPsvInstancePtr is a pointer to the instance of
 *		the OspiPsv device.
 * @param	OspiPsvIntrId is the interrupt Id for an OSPIPSV device.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
static void OspiPsvDisableIntrSystem(XScuGic *IntcInstancePtr,
		u16 OspiPsvIntrId)
{
	/*
	 * Disable the interrupt for the OSPIPSV device.
	 */
	XScuGic_Disable(IntcInstancePtr, OspiPsvIntrId);

	/*
	 * Disconnect and disable the interrupt for the OspiPsv device.
	 */
	XScuGic_Disconnect(IntcInstancePtr, OspiPsvIntrId);
}

/*****************************************************************************/
/**
 *
 * Callback handler.
 *
 * @param	None.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void OspiPsvHandler(void *CallBackRef, u32 StatusEvent)
{
	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		xil_printf("\n Error interrupt occurred %x\r\n", StatusEvent);
		Error = 1;
	} else {
		Error = 0;
	}

	/*
	 * Indicate the transfer on the OSPIPSV bus is no longer in progress
	 * regardless of the status event
	 */
	TransferInProgress = FALSE;
}

/*****************************************************************************/
/**
* This API enters the flash device into Octal DDR mode or exit from octal DDR
* mode (switches to Extended SPI mode).
*
* @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode)
{
	int Status;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));

#endif
	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Data[0] = 0xE7;
		Data[1] = 0xE7;
	} else {
		Data[0] = 0xFF;
		Data[1] = 0xFF;
	}


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION)
		XOspiPsv_ConfigureAutoPolling(OspiPsvPtr, Mode);

	FlashMsg.Opcode = WRITE_CONFIG_REG;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = 3;
	FlashMsg.Addr = 0x0;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		FlashMsg.Addrsize = 4;
		FlashMsg.ByteCount = 2;
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Read Configuration register */
	FlashMsg.Opcode = READ_CONFIG_REG;
	FlashMsg.Addrsize = 3;
	FlashMsg.Addr = 0x0;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = 8 + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		/* Read Configuration register */
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while(TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0])
		return XST_FAILURE;

	return Status;
}
