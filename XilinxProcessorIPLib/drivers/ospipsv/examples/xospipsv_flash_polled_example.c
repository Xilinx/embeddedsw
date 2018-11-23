/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xospipsv_flash_polled_example.c
*
*
* This file contains a design example using the OSPIPS driver (xospipsv)
* The example writes to flash in IO mode and reads it back in DMA mode.
* It runs in polled mode.
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
* 1.0   nsk  02/19/17 First release
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xospipsv.h"		/* OSPIPS device driver */
#include "xil_printf.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

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
#define SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01
#define READ_CMD_OCTAL		0x8B
#define READ_CMD_OCTAL_IO	0xCB
#define WRITE_OCTAL_CMD		0x82
#define WRITE_OCTAL_IO_CMD	0xC2

#define WRITE_CMD_4B		0x12
#define READ_CMD_4B		0x13
#define FAST_READ_CMD_4B	0x0C
#define DUAL_READ_CMD_4B	0x3C
#define QUAD_READ_CMD_4B	0x6C
#define SEC_ERASE_CMD_4B	0xDC
#define READ_CMD_OCTAL_4B	0x7C
#define READ_CMD_OCTAL_IO_4B	0xCC
#define WRITE_CMD_OCTAL_4B	0x84
#define WRITE_CMD_OCTAL_IO_4B	0x8E
#define ENTER_4B_ADDR_MODE	0xB7
#define EXIT_4B_ADDR_MODE	0xE9

#define BANK_REG_RD		0x16
#define BANK_REG_WR		0x17
/* Bank register is called Extended Address Register in Micron */
#define EXTADD_REG_RD		0xC8
#define EXTADD_REG_WR		0xC5
#define DIE_ERASE_CMD		0xC4
#define READ_FLAG_STATUS_CMD	0x70

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the OSPIPS driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define ADDRESS_4_OFFSET	4 /* LSB byte of address to read or write when 4 byte address */
#define DATA_OFFSET		5 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				     reads */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				     quad reads */
#define DUMMY_CLOCKS		8 /* Number of dummy bytes for fast, dual and
				     quad reads */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */
#define BANK_SEL_SIZE	2 /* BRWR or EARWR command + 1 byte bank value */
#define RD_CFG_SIZE		2 /* 1 byte Configuration register + RD CFG command*/
#define WR_CFG_SIZE		3 /* WRR command + 1 byte each Status and Config Reg*/
#define DIE_ERASE_SIZE	4	/* Die Erase command + Die address */

/*
 * The following constants specify the extra bytes which are sent to the
 * Flash on the OSPIPS interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

/*
 * Base address of Flash1
 */
#define FLASH1BASE 0x000000

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000
#define ONEMB	0x100000
#define DEVSIZE 0x2000000


/*
 * Mask for quad enable bit in Flash configuration register
 */
#define FLASH_QUAD_EN_MASK 0x02

#define FLASH_SRWD_MASK 0x80

/*
 * Bank mask
 */
#define BANKMASK 0xF000000

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
#define OSPIPS_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID

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
}FlashInfo;

u8 ReadCmd;
u8 WriteCmd;
u8 StatusCmd;
u8 SectorEraseCmd;
u8 FSRFlag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int OspiPsvPolledFlashExample(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId);

int FlashReadID(XOspiPsv *OspiPsvPtr);
int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);
int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);

int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
u32 GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address);
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
int FlashRegisterRead(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
int FlashRegisterWrite(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn);
s32 InitCmd(XOspiPsv *OspiPsvInstancePtr);
int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable);

/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
u8 ReadBfrPtr[3];
FlashInfo Flash_Config_Table[28] = {
	/* Micron */
	{0x20000, 0x200, 256, 0x40000, 0x2000000,
		MICRON_OCTAL_ID_BYTE0, MICRON_OCTAL_ID_BYTE2_512, 0xFFFF0000, 1}
};

u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;

static XOspiPsv_Msg FlashMsg;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 1;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 * The buffer size accounts for maximum page size and maximum banks -
 * for each bank separate read will be performed leading to that many
 * (overhead+dummy) bytes
 */
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8];
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + DATA_OFFSET];
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
* Main function to call the OSPIPS Flash Polled example.
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

	xil_printf("OSPIPSV Flash Polled Example Test\r\n");

	/*
	 * Run the OspiPs Polled example.
	 */
	Status = OspiPsvPolledFlashExample(&OspiPsvInstance, OSPIPS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("OSPIPSV Flash Polled Ex Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran OSPIPSV Flash Polled Ex\r\n");
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
int OspiPsvPolledFlashExample(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page = 0;
	XOspiPsv_Config *OspiPsvConfig;
	int ReadBfrSize;
	ReadBfrSize = (PAGE_COUNT * MAX_PAGE_SIZE) +
			(DATA_OFFSET + DUMMY_SIZE)*8;

	/*
	 * Initialize the OSPIPS driver so that it's ready to use
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
	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);
	/*
	 * Set the prescaler for OSPIPS clock
	 */

	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_15);
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

	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);
	/*
	 * Address size and read command selection
	 * Micron flash on REMUS doesn't support this 4B write/erase cmd
	 */

	/* Status cmd - SR or FSR selection */
	if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
		(FlashMake == MICRON_OCTAL_ID_BYTE0)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FSRFlag = 0;
	}

	StatusCmd = READ_FLAG_STATUS_CMD;

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
		Count < Flash_Config_Table[FCTIndex].PageSize * 32;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);

	}

	InitCmd(OspiPsvInstancePtr);
	xil_printf("ReadCmd: 0x%x, WriteCmd: 0x%x, EraseCmd 0x%x\n\r", ReadCmd, WriteCmd,
		SectorEraseCmd) ;
	Status = FlashErase(OspiPsvInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_DAC_EN_OPTION) {
		for (Page = 0; Page < PAGE_COUNT; Page++) {
			Status = FlashLinearWrite(OspiPsvInstancePtr,
			(Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
			((Flash_Config_Table[FCTIndex].PageSize)), WriteCmd, WriteBuffer);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
		}
	} else {
		for (Page = 0; Page < PAGE_COUNT; Page++) {
			Status = FlashIoWrite(OspiPsvInstancePtr,
			(Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
			((Flash_Config_Table[FCTIndex].PageSize)), WriteCmd, WriteBuffer);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
		}
	}
	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}

	Status = FlashRead(OspiPsvInstancePtr, TEST_ADDRESS, MaxData, ReadCmd,
			CmdBfr, ReadBuffer);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

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
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to determine the number of lines used
* for command, address and data
*
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
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
		switch(ReadCmd) {
			case	READ_CMD_4B:
			case 	READ_CMD:
			case	FAST_READ_CMD:
				Val = XOSPIPSV_READ_1_1_1;
				break;
			case 	READ_CMD_OCTAL:
			case	READ_CMD_OCTAL_4B:
				Val = XOSPIPSV_READ_1_1_8;
				break;
			case	READ_CMD_OCTAL_IO_4B:
			case	READ_CMD_OCTAL_IO:
				Val = XOSPIPSV_READ_1_8_8;
				break;
		}
	} else {
		switch(WriteCmd) {
			case	WRITE_CMD:
			case 	WRITE_CMD_4B:
				Val = XOSPIPSV_WRITE_1_1_1;
				break;
			case	WRITE_OCTAL_CMD:
			case	WRITE_CMD_OCTAL_4B:
				Val = XOSPIPSV_WRITE_1_1_8;
				break;
			case 	WRITE_OCTAL_IO_CMD:
			case	WRITE_CMD_OCTAL_IO_4B:
				Val = XOSPIPSV_WRITE_1_8_8;
				break;
		}
	}
	return Val;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the commands used in IO, linear
* or DMA mode
*
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
s32 InitCmd(XOspiPsv *OspiPsvInstancePtr)
{
	u32 Status;

	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_IDAC_EN_OPTION) {
			WriteCmd = WRITE_CMD_4B;
			SectorEraseCmd = SEC_ERASE_CMD_4B;
			ReadCmd = READ_CMD_OCTAL_IO_4B;
		} else if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_DAC_EN_OPTION) {
			WriteCmd = WRITE_CMD_OCTAL_4B;
			SectorEraseCmd = SEC_ERASE_CMD_4B;
			ReadCmd = READ_CMD_OCTAL_IO_4B;
		} else {
			WriteCmd = WRITE_CMD_4B;
			SectorEraseCmd = SEC_ERASE_CMD_4B;
			ReadCmd = READ_CMD_4B;
		}
	} else {
		if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_IDAC_EN_OPTION) {
			WriteCmd = WRITE_CMD;
			SectorEraseCmd = SEC_ERASE_CMD;
			ReadCmd = READ_CMD_OCTAL;
		} else if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_DAC_EN_OPTION) {
			WriteCmd = WRITE_OCTAL_CMD;
			SectorEraseCmd = SEC_ERASE_CMD;
			ReadCmd = READ_CMD_OCTAL;
		} else  {
			WriteCmd = WRITE_CMD;
			SectorEraseCmd = SEC_ERASE_CMD;
			ReadCmd = READ_CMD;
		}
	}
	return XST_SUCCESS;
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
	int StartIndex;
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
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("FlashID = ");
	while(ReadIdBytes >= 0 ) {
		xil_printf("0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		ReadIdBytes--;
	}
	xil_printf("\n\r");
	/*
	 * Deduce flash make
	 */
	if (ReadBfrPtr[0] == MICRON_OCTAL_ID_BYTE0) {
		FlashMake = MICRON_OCTAL_ID_BYTE0;
		StartIndex = MICRON_INDEX_START;
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
* This function writes to the  serial Flash connected to the OSPIPS interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used when controller
* is in Linear mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. OSPIPS
*		device supports only Page Program command to write data to the
*		flash.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 Status;
	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = WriteBfrPtr;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	if((Command == WRITE_CMD_4B) || (Command == WRITE_CMD_OCTAL_4B))
		FlashMsg.Addrsize = 4;
	else
		FlashMsg.Addrsize = 3;

	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return 0;
}


/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPS interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. OSPIPS
*		device supports only Page Program command to write data to the
*		flash.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 FlashStatus;
	u8 Status;
	u32 Butestowrite;

	while(ByteCount != 0) {
		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if(ByteCount <= 8) {
			Butestowrite = ByteCount;
			ByteCount = 0;
		} else {
			Butestowrite = 8;
			ByteCount -= 8;
		}
		FlashMsg.Opcode = Command;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = WriteBfrPtr;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = Butestowrite;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		if((Command == WRITE_CMD_4B) || (Command == WRITE_CMD_OCTAL_4B))
			FlashMsg.Addrsize = 4;
		else
			FlashMsg.Addrsize = 3;

		FlashMsg.Addr = Address;
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		WriteBfrPtr += 8;
		Address += 8;

		while (1) {
			FlashMsg.Opcode = StatusCmd;
			FlashMsg.Addrsize = 3;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = &FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;

			if ((FlashStatus & 0x80) != 0)
				break;
		}
}
	return 0;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial Flash connected to the
* OSPIPS interface.
*
* @param	OspPsiPtr is a pointer to the OSPIPS driver component to use.
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
	u8 FlashStatus;
	u8 Status;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
		(Flash_Config_Table[FCTIndex]).SectSize) ) {


		if(Flash_Config_Table[FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(OspiPsvPtr, WriteBfrPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(OspiPsvPtr, WriteBfrPtr);
		}
		return 0;
	}

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

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		FlashMsg.Opcode = SectorEraseCmd;
		if(FlashMsg.Opcode == SEC_ERASE_CMD_4B)
			FlashMsg.Addrsize = 4;
		else
			FlashMsg.Addrsize = 3;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Addr = Address;
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (1) {
			FlashMsg.Opcode = StatusCmd;
			FlashMsg.Addrsize = 3;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = &FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if ((FlashStatus & 0x80) != 0) {
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
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Command is the command used to read data from the flash. Supports
* 			normal, fast, dual and quad read commands.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u8 Status;
	FlashMsg.Opcode = Command;
	if((Command == READ_CMD_OCTAL_4B) || (Command == READ_CMD_OCTAL_IO_4B)
			|| (Command == READ_CMD_4B))
		FlashMsg.Addrsize = 4;
	else
		FlashMsg.Addrsize = 3;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
	/* It is recommended to have a separate entry for dummy */
	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD) || (Command == FAST_READ_CMD_4B) ||
	    (Command == DUAL_READ_CMD_4B) || (Command == QUAD_READ_CMD_4B) ||
	    (Command == READ_CMD_OCTAL) ||
	    (Command == READ_CMD_OCTAL_4B)) {
		FlashMsg.Dummy = 8;
	} else if( (Command == READ_CMD_OCTAL_IO)|| (Command == READ_CMD_OCTAL_IO_4B)) {
		FlashMsg.Dummy = 16;
	}
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
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
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	u8 FlashStatus;
	int Status;


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

	XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg.Opcode = BULK_ERASE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}
	while (1) {
		FlashMsg.Opcode = StatusCmd;
		FlashMsg.Addrsize = 3;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = &FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if ((FlashStatus & 0x80) != 0)
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
* @param	OspiPsvPtr is a pointer to the OSPIPS driver component to use.
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
	u8 FlashStatus;
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

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		FlashMsg.Opcode = DIE_ERASE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		while (1) {
			FlashMsg.Opcode = StatusCmd;
			FlashMsg.Addrsize = 3;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = &FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
			if ((FlashStatus & 0x80) != 0)
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
* @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
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
	u8 FlashStatus;

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

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
			break;

		default:
			break;
	}
	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 3;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	while (1) {
		FlashMsg.Opcode = StatusCmd;
		FlashMsg.Addrsize = 3;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = &FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if ((FlashStatus & 0x80) != 0)
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

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		break;

		default:
			break;
	}
	return Status;
}
