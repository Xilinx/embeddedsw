/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xospipsv_flash_non_blocking_read_example.c
* @brief Implements a design example using the OSPIPSV driver (xospipsv).
*
* The example writes to flash in IO mode and reads it back using non-blocking
* APIs in DMA mode.
* It runs in polled mode.
* The hardware which this example runs on, must have an octal serial Flash
* (Micron) for it to run.
*
* The example has been tested with the Micron Octal Serial Flash
* (mt35xu01gbba).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sk  02/03/20 First release
* 1.3   sk  05/27/20 Added Stacked mode support.
* 1.4   sk  02/18/21 Added support for Macronix flash and DualByte commands.
* 1.9   sb  06/06/23 Added support for system device-tree flow.
* 1.10  akm 01/31/24 Use OSPI controller reset for resetting flash device.
* 1.10  sb  02/09/24 Add support for Infineon flash part S28HS02G.
* 1.11  sb  05/02/24 Add support for Macronix flash part mx66uw2g345gxrix0.
* 1.12  akm 01/01/25  Pass valid write command in DAC mode.
* 1.12  sb  03/12/25 Fixed gcc and g++ warnings.
* 1.14  vlt 12/15/25 Update Doxygen comments to include SDT flow details.
* 1.15  sb  08/11/26 Add support for pattern-based RX PHY tuning.
* 1.15  vn  09/03/26 Fix doxygen warnings.
* 1.15  tb  09/30/26 Align Doxygen comments with AMD documentation standards
*
*</pre>
*
******************************************************************************/

/** @addtogroup xospipsv_flash_non_blocking_read_example
 * @{
 */

/***************************** Include Files *********************************/

#include "xospipsv_flash_config.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
/** OSPIPSV device ID. */
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#endif

/**
 * Number of flash pages to be written.
 */
#define PAGE_COUNT		32

/**
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/**
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x0


#define UNIQUE_VALUE		0x09	/**< Initial value of the test data. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int OspiPsvFlashNonBlockingReadExample(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId);
#else
int OspiPsvFlashNonBlockingReadExample(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress);
#endif

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
/**
 * @brief Reads a flash register.
 *
 * @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver instance.
 * @param[in] ByteCount Number of bytes to read.
 * @param[in] Command Register read command.
 * @param[out] ReadBfrPtr Pointer to the buffer for the register data.
 *
 * @return XST_SUCCESS if successful, otherwise XST_FAILURE.
 */
int FlashRegisterRead(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
/**
 * @brief Writes to a flash register.
 *
 * @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver instance.
 * @param[in] ByteCount Number of bytes to write.
 * @param[in] Command Register write command.
 * @param[in] WriteBfrPtr Pointer to the value to write.
 * @param[in] WrEn Flag to send a write enable (WREN) before the write.
 *
 * @return XST_SUCCESS if successful, otherwise XST_FAILURE.
 */
int FlashRegisterWrite(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command,
		       u8 *WriteBfrPtr, u8 WrEn);
/**
 * @brief Initializes the flash commands used by the example.
 *
 * @param[in,out] OspiPsvInstancePtr Pointer to the OSPIPSV driver instance.
 *
 * @return XST_SUCCESS if successful, otherwise XST_FAILURE.
 */
s32 InitCmd(XOspiPsv *OspiPsvInstancePtr);
int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable);
int FlashWriteCfgReg(XOspiPsv *OspiPsvPtr, u8 Opcode, u8 AddrSize, u32 Address,
		     u8 *Data);
int FlashReadCfgReg(XOspiPsv *OspiPsvPtr, u8 Opcode, u8 AddrSize, u32 Address,
		    u8 Dummy, u8 *ConfigReg);
int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode);
#ifdef FLASH_SETUP_TUNING_PATTERN
int FlashSetupRxTuningPattern(XOspiPsv *OspiPsvPtr, int Mode);
#endif
int SpansionSetEccMode(XOspiPsv *OspiPsvPtr);
int MxConfigDummy(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
/** Transmit buffer. */
u8 TxBfrPtr;
/** Read buffer. */
#ifdef __ICCARM__
#pragma data_alignment = 4
u8 ReadBfrPtr[8];
#else
u8 ReadBfrPtr[8]__attribute__ ((aligned(4)));
#endif

/** Flash manufacturer ID (first byte of the flash ID). */
u32 FlashMake;
/** Flash device type (second byte of the flash ID). */
u32 FlashType;
u32 FCTIndex;	/**< Flash configuration table index */

/**
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;

/** Flash message used for the transfers. */
static XOspiPsv_Msg FlashMsg;

/**
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 1;

/**
 * The following variables hold the data read from and written to the flash
 * and they are global to avoid having large buffers on the stack
 */
#ifdef __ICCARM__
#pragma data_alignment = 64
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#pragma data_alignment = 4
/** Buffer holding the data written to the flash. */
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#else
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(64)));
/** Buffer holding the data written to the flash. */
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(4)));
#endif

u8 CmdBfr[8];	/**< Command buffer. */
u8 FSRFlag;	/**< Flag status register usage flag. */

/**
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT * 256;

/*****************************************************************************/
/**
*
* @brief Calls the OSPIPSV Flash non-blocking read example.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("OSPIPSV Flash Polled non-blocking read Example Test\r\n");

	/*
	 * Run the OspiPsv Polled example.
	 */
#ifndef SDT
	Status = OspiPsvFlashNonBlockingReadExample(&OspiPsvInstance, OSPIPSV_DEVICE_ID);
#else
	Status = OspiPsvFlashNonBlockingReadExample(&OspiPsvInstance, XPAR_XOSPIPSV_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("OSPIPSV non-blocking read Polled Ex Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran OSPIPSV Flash non-blocking read Ex\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Illustrates how to use the XOspiPsv device driver using OSPI flash
* devices greater than or equal to 128Mb.
*
* The function reads data in either DMA or DAC mode.
*
* @param[in]	OspiPsvInstancePtr Pointer to the OSPIPSV driver component
*               to use.
* @if SDT
* @param[in]        BaseAddress Base address of the device.
* @else
* @param[in]        OspiPsvDeviceId Device ID of the OSpi Device, the
*               XPAR_<OSPI_instance>_DEVICE_ID value from xparameters.h.
* @endif
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note	        In XSCT/classic flow, the device configuration is looked up
*               by DeviceId.
*
*****************************************************************************/
#ifndef SDT
int OspiPsvFlashNonBlockingReadExample(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId)
#else
int OspiPsvFlashNonBlockingReadExample(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u8 UniqueValue;
	u32 Count;
	int Page = 0;
	XOspiPsv_Config *OspiPsvConfig;
	u32 ReadBfrSize;
	ReadBfrSize = (PAGE_COUNT * MAX_PAGE_SIZE);

	/*
	 * Initialize the OSPIPSV driver so that it's ready to use
	 */
#ifndef SDT
	OspiPsvConfig = XOspiPsv_LookupConfig(OspiPsvDeviceId);
#else
	OspiPsvConfig = XOspiPsv_LookupConfig(BaseAddress);
#endif
	if (NULL == OspiPsvConfig) {
		return XST_FAILURE;
	}

	Status = XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiPsvConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal) && !defined (VERSAL_2VE_2VM)
	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
#else
	Status = XOspiPsv_DeviceResetViaOspi(OspiPsvInstancePtr, XOSPIPSV_HWPIN_RESET);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);
	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_12);

	Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

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

	/* Configure Flash Dummy cycles in flash device */
	if ((FlashMake == MACRONIX_OCTAL_ID_BYTE0) && (FlashType == MACRONIX_OCTAL_ID_BYTE1)){
		Status = MxConfigDummy(OspiPsvInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Configure Flash ECC mode */
	if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Status = SpansionSetEccMode(OspiPsvInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Set Flash device and Controller modes */
	Status = FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (OspiPsvInstancePtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		Flash_Config_Table[FCTIndex].NumPage *= 2;
		Flash_Config_Table[FCTIndex].NumSect *= 2;

		/*
		 * Both devices were switched to the required mode above, only
		 * the address mode has to be set on the upper device.
		 */
		Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}

		Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
	     Count < Flash_Config_Table[FCTIndex].PageSize * PAGE_COUNT;
	     Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);

	}

	Status = FlashErase(OspiPsvInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_DAC_EN_OPTION) {
		xil_printf("WriteCmd: 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].WriteCmd >> 8));
		Status = FlashLinearWrite(OspiPsvInstancePtr, TEST_ADDRESS,
					  (Flash_Config_Table[FCTIndex].PageSize * PAGE_COUNT), WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
	else {
		xil_printf("WriteCmd: 0x%x\n\r", (u8)Flash_Config_Table[FCTIndex].WriteCmd);
		for (Page = 0; Page < PAGE_COUNT; Page++) {
			Status = FlashIoWrite(OspiPsvInstancePtr,
					      (Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
					      ((Flash_Config_Table[FCTIndex].PageSize)), WriteBuffer);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
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

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Determines the number of lines used for command, address and data.
*
* @param[in] OspiPsvInstancePtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Read Nonzero for a read, 0 for a write.
*
* @return	Value to program the lines for command, address and data.
*
*****************************************************************************/
u32 XOspiPsv_Get_Proto(XOspiPsv *OspiPsvInstancePtr, int Read)
{
	u32 Val;

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Val = XOSPIPSV_READ_1_1_1;
		return Val;
	}

	if (Read) {
		Val = XOSPIPSV_READ_1_8_8;
	}
	else {
		if (OspiPsvInstancePtr->OpMode == XOSPIPSV_IDAC_MODE) {
			Val = XOSPIPSV_WRITE_1_1_1;
		}
		else {
			Val = XOSPIPSV_WRITE_1_1_8;
		}
	}
	return Val;
}

/*****************************************************************************/
/**
*
* @brief Reads the flash ID and identifies the flash in FCT table.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
*****************************************************************************/
int FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status;
	int ReadIdBytes = 8;
	u32 ReadId = 0;

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
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Dummy += 8;
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Addrsize = 4U;
		FlashMsg.Addrvalid = 1U;
		FlashMsg.Dummy = 4;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("FlashID = ");
	while (ReadIdBytes >= 0 ) {
		xil_printf("0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		ReadIdBytes--;
	}
	xil_printf("\n\r");

	ReadId = ((ReadBfrPtr[0] << 16) | (ReadBfrPtr[1] << 8) | ReadBfrPtr[2]);

	FlashMake = ReadBfrPtr[0];
	FlashType = ReadBfrPtr[1];

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		FSRFlag = 0;
	}
	else {
		FSRFlag = 1;
	}

	Status = CalculateFCTIndex(ReadId, &FCTIndex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Writes to the serial Flash connected to the OSPIPSV interface.
*
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used when controller
* is in Linear mode.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Address The address to write data to in the Flash.
* @param[in]	ByteCount The number of bytes to write.
* @param[in]	WriteBfrPtr Pointer to the write buffer
*		(which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
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
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].WriteCmd >> 8);
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = WriteBfrPtr;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
	}
	FlashMsg.IsDDROpCode = 0;
	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
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
* @brief Writes to the serial Flash connected to the OSPIPSV interface.
*
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Address The address to write data to in the Flash.
* @param[in]	ByteCount The number of bytes to write.
* @param[in] WriteBfrPtr Pointer to the write buffer (which is to be
*		transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
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
	u32 RealAddr;

	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (ByteCount <= 8) {
			Bytestowrite = ByteCount;
			ByteCount = 0;
		}
		else {
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
		FlashMsg.Addr = RealAddr;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
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
			FlashMsg.Addr = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
				if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
					FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
					FlashMsg.Opcode = READ_CONFIG_REG_SPN;
					FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
					FlashMsg.Addrsize = 4;
					FlashMsg.Addrvalid = 1;
				}
			}

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
	}
	return 0;
}

/*****************************************************************************/
/**
*
* @brief Erases the sectors in the serial Flash connected to the
* OSPIPSV interface.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Address The address of the first sector which needs to
*		be erased.
* @param[in]	ByteCount The total size to be erased.
* @param[in] WriteBfrPtr Pointer to the write buffer (which is to be
*		transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
	       u8 *WriteBfrPtr)
{
	u32 Sector;
	u32 NumSect;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 RealAddr;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
			  (Flash_Config_Table[FCTIndex]).SectSize) ) {

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS0);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		if (Flash_Config_Table[FCTIndex].NumDie == 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8));
			/*
			 * Call Bulk erase
			 */
			BulkErase(OspiPsvPtr, WriteBfrPtr);
		}

		if (Flash_Config_Table[FCTIndex].NumDie > 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16));
			/*
			 * Call Die erase
			 */
			DieErase(OspiPsvPtr, WriteBfrPtr);
		}

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (Flash_Config_Table[FCTIndex].NumDie == 1) {
				xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8));
				/*
				 * Call Bulk erase
				 */
				BulkErase(OspiPsvPtr, WriteBfrPtr);
			}

			if (Flash_Config_Table[FCTIndex].NumDie > 1) {
				xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16));
				/*
				 * Call Die erase
				 */
				DieErase(OspiPsvPtr, WriteBfrPtr);
			}
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
	NumSect = ByteCount / (Flash_Config_Table[FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if ( ((Address + ByteCount) & Flash_Config_Table[FCTIndex].SectMask) ==
	     ((Address + (NumSect * Flash_Config_Table[FCTIndex].SectSize)) &
	      Flash_Config_Table[FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].EraseCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
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
			FlashMsg.Addr = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
				if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
					FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
					FlashMsg.Opcode = READ_CONFIG_REG_SPN;
					FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
					FlashMsg.Addrsize = 4;
					FlashMsg.Addrvalid = 1;
				}
			}

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
		Address += Flash_Config_Table[FCTIndex].SectSize;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* @brief Performs a flash read.
*
* DMA is the default setting.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Address Flash address to read from.
* @param[in]	ByteCount Number of bytes to read.
* @param[in]	WriteBfrPtr Pointer to the write buffer which contains
*		data to be transmitted
* @param[out]	ReadBfrPtr Pointer to the read buffer to which valid
*		received data should be written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
	      u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	(void)WriteBfrPtr;
	u8 Status;
	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;

	if ((Address < Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
	    ((Address + ByteCount) >= Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
	    (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED)) {
		BytesToRead = (Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	}
	else {
		BytesToRead = ByteCount;
	}
	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBfrPtr;
		FlashMsg.ByteCount = BytesToRead;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
		FlashMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles +
				 OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
			if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
				FlashMsg.Dummy = 20 + OspiPsvPtr->Extra_DummyCycle;
				FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].ReadCmd >> 8);
			}
			else if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
				FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].ReadCmd >> 8);
				FlashMsg.Dummy = 21 + OspiPsvPtr->Extra_DummyCycle;
			}
		}

		xil_printf("ReadCmd 0x%x\r\n", FlashMsg.Opcode);

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_StartDmaTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		Status = XOspiPsv_CheckDmaDone(OspiPsvPtr);
		while (Status != XST_SUCCESS) {
			Status = XOspiPsv_CheckDmaDone(OspiPsvPtr);
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* @brief Performs a bulk erase operation when the flash device has a single die.
*
* Works for both Spansion and Micron
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	WriteBfrPtr Pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	(void)WriteBfrPtr;
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
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
			if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
				FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
				FlashMsg.Opcode = READ_CONFIG_REG_SPN;
				FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
				FlashMsg.Addrsize = 4;
				FlashMsg.Addrvalid = 1;
			}
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (FSRFlag) {
			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		else {
			if ((FlashStatus[0] & 0x01) == 0) {
				break;
			}
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Performs a die erase operation on all the die in the flash device.
*
* The function uses the die erase command for Micron 512Mbit and 1Gbit
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	WriteBfrPtr Pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
******************************************************************************/
int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	(void)WriteBfrPtr;
	u8 DieCnt;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;

	for (DieCnt = 0; DieCnt < Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
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

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
	}
	return 0;
}

/*****************************************************************************/
/**
* @brief Enters or exits the 4 bytes addressing mode of the flash device.
*
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Enable 1 to enter 4 byte addressing mode, 0 to exit it.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
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

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		return XST_SUCCESS;
	}

	if (Enable) {
		Command = ENTER_4B_ADDR_MODE;
	}
	else {
		Command = EXIT_4B_ADDR_MODE;
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (FSRFlag) {
			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		else {
			if ((FlashStatus[0] & 0x01) == 0) {
				break;
			}
		}
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

			if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			break;

		default:
			break;
	}

	return Status;
}

#ifdef FLASH_SETUP_TUNING_PATTERN
/*****************************************************************************/
/**
* @brief Programs the RX tuning pattern into the last sector of the flash,
* verifies it and gives it to the driver along with its location and how to
* read it back, so that the tuning algorithm can compare against it.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Mode The edge mode the controller is about to be switched to.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
* @note
*		The pattern is programmed while the controller and the flash are
*		still in SDR-NON-PHY mode, so this has to be called before
*		either of them is switched. Tuning reads the pattern from
*		whichever device is selected, so this also has to be called with
*		the lower flash selected. The read instruction handed to the
*		driver mirrors the one used by FlashRead() in the target mode,
*		except that Extra_DummyCycle is left out since tuning is what
*		determines it.
*
******************************************************************************/
int FlashSetupRxTuningPattern(XOspiPsv *OspiPsvPtr, int Mode)
{
	XOspiPsv_RxTuning *RxTuning = &OspiPsvPtr->RxTuning;
	int Status;
	u32 Count;
	/* The pattern is written to the last sector of the flash on CS0. */
	u32 PatternAddr = PATTERN_ADDRESS(FCTIndex);

	if ((Mode != XOSPIPSV_EDGE_MODE_DDR_PHY) &&
	    (Mode != XOSPIPSV_EDGE_MODE_SDR_PHY)) {
		return XST_SUCCESS;
	}

	Status = FlashErase(OspiPsvPtr, PatternAddr, TUNING_PATTERN_SIZE,
			    CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION) {
		Status = FlashLinearWrite(OspiPsvPtr, PatternAddr,
					  TUNING_PATTERN_SIZE, TuningPattern);
	}
	else {
		Status = FlashIoWrite(OspiPsvPtr, PatternAddr,
				      TUNING_PATTERN_SIZE, TuningPattern);
	}
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = FlashRead(OspiPsvPtr, PatternAddr, TUNING_PATTERN_SIZE,
			   CmdBfr, TuningReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Count = 0; Count < TUNING_PATTERN_SIZE; Count++) {
		if (TuningReadBuffer[Count] != TuningPattern[Count]) {
			return XST_FAILURE;
		}
	}

	RxTuning->Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
	RxTuning->Proto = (u8)XOspiPsv_Get_Proto(OspiPsvPtr, 1);
	RxTuning->Dummy = (u8)Flash_Config_Table[FCTIndex].DummyCycles;
	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		RxTuning->Proto = XOSPIPSV_READ_8_8_8;
		if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			RxTuning->Opcode = (u8)(Flash_Config_Table[FCTIndex].ReadCmd >> 8);
			RxTuning->Dummy = 20;
		}
		if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
			RxTuning->Opcode = (u8)(Flash_Config_Table[FCTIndex].ReadCmd >> 8);
			RxTuning->Dummy = 21;
		}
	}

	RxTuning->Addrsize = 4;
	RxTuning->PatternAddr = PatternAddr;
	RxTuning->Pattern = TuningPattern;
	RxTuning->PatternSize = TUNING_PATTERN_SIZE;
	RxTuning->ReadBfrPtr = TuningReadBuffer;

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* @brief Writes one byte into a configuration register of the flash device
* currently selected.
*
* A write enable is issued before the write.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Opcode The write register command of the flash device.
* @param[in]	AddrSize The size of the register address in bytes.
* @param[in]	Address The register address.
* @param[in]	Data The value to write, duplicated in both bytes as the DDR
*		modes transfer two bytes.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
******************************************************************************/
int FlashWriteCfgReg(XOspiPsv *OspiPsvPtr, u8 Opcode, u8 AddrSize, u32 Address,
		     u8 *Data)
{
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = Opcode;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		FlashMsg.Addrsize = 4;
		FlashMsg.ByteCount = 2;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Reads a configuration register of the flash device currently selected.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Opcode The read register command of the flash device.
* @param[in]	AddrSize The size of the register address in bytes.
* @param[in]	Address The register address.
* @param[in]	Dummy The number of dummy cycles of the read command.
* @param[out]	ConfigReg The two byte buffer the register is read into.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
******************************************************************************/
int FlashReadCfgReg(XOspiPsv *OspiPsvPtr, u8 Opcode, u8 AddrSize, u32 Address,
		    u8 Dummy, u8 *ConfigReg)
{
	int Status;

	FlashMsg.Opcode = Opcode;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	} else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Dummy += 8;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Enters the flash device into Octal DDR mode or exits from octal DDR
* mode (switches to Extended SPI mode).
*
* In a stacked configuration both devices are switched, the controller is
* switched once and RX tuning is run only against the lower device.
*
* @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
* @param[in]	Mode Either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
* @note
*		The lower flash has to be selected on entry.
*
******************************************************************************/
int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode)
{
	int Status;
	u8 Write_Reg_Opcode = WRITE_CONFIG_REG;
	u8 Read_Reg_Opcode = READ_CONFIG_REG;
	u8 AddrSize = 3;
	u8 Dummy = 8;
	u32 Address = 0;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
#endif

	if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Write_Reg_Opcode = WRITE_CONFIG_REG_SPN;
		Read_Reg_Opcode = READ_CONFIG_REG_SPN;
		AddrSize = 4;
		Address = CONFIG_REG_5_ADDR_SPN;
		Dummy = 0;
	}
	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		Write_Reg_Opcode = WRITE_CONFIG2_REG_MX;
		Read_Reg_Opcode = READ_CONFIG2_REG_MX;
		AddrSize = 4;
		if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			Dummy = 4;
		}
		else {
			Dummy = 0;
		}
	}

	/*
	 * Program the tuning pattern while everything is still in the working
	 * mode. It goes into the lower flash only, so the controller is tuned
	 * once against that device and the taps it settles on are used for both
	 * devices. Without it there is nothing to tune the PHY against, so the
	 * PHY modes are not available.
	 */
#ifdef FLASH_SETUP_TUNING_PATTERN
	Status = FlashSetupRxTuningPattern(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	if ((Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) ||
	    (Mode == XOSPIPSV_EDGE_MODE_SDR_PHY)) {
		return XST_FAILURE;
	}
#endif

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Data[0] = 0x2;
			Data[1] = 0x2;
		}
		else if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
			Data[0] = 0x43;
			Data[1] = 0x43;
		}
		else {
			Data[0] = 0xE7;
			Data[1] = 0xE7;
		}
	}
	else {
		if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Data[0] = 0x0;
			Data[1] = 0x0;
		}
		else if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
			Data[0] = 0x40;
			Data[1] = 0x40;
		}
		else {
			Data[0] = 0xFF;
			Data[1] = 0xFF;
		}
	}


	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION) {
		XOspiPsv_ConfigureAutoPolling(OspiPsvPtr, Mode);
	}

	/*
	 * Switch every flash device while the controller is still in its
	 * previous mode, so that each of them is reachable with the protocol it
	 * is currently using.
	 */
	Status = FlashWriteCfgReg(OspiPsvPtr, Write_Reg_Opcode, AddrSize,
				  Address, Data);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->Config.ConnectionMode ==
			XOSPIPSV_CONNECTION_MODE_STACKED) {
		Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = FlashWriteCfgReg(OspiPsvPtr, Write_Reg_Opcode, AddrSize,
					  Address, Data);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		if ((FlashMake == MACRONIX_OCTAL_ID_BYTE0) && (OspiPsvPtr->DualByteOpcodeEn == 0U)) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_ENABLE);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		else if ((FlashMake == SPANSION_OCTAL_ID_BYTE0) && (OspiPsvPtr->DualByteOpcodeEn == 0U)) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_SAME);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}
	else {
		if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_DISABLE);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = FlashReadCfgReg(OspiPsvPtr, Read_Reg_Opcode, AddrSize, Address,
				 Dummy, ConfigReg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0]) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->Config.ConnectionMode ==
			XOSPIPSV_CONNECTION_MODE_STACKED) {
		Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = FlashReadCfgReg(OspiPsvPtr, Read_Reg_Opcode, AddrSize,
					 Address, Dummy, ConfigReg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (ConfigReg[0] != Data[0]) {
			return XST_FAILURE;
		}

		Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Translates the address based on the type of interconnection.
 *
 * In case of stacked, the function asserts the corresponding slave select.
 *
 * @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
 * @param[in]	Address Which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *			for stacked, the lower flash size is subtracted;
 *
 * @note	In addition to get the actual address to work on flash the
 *			function also selects the CS based on the configuration detected.
 *
 ******************************************************************************/
u32 GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address)
{
	u32 RealAddr = Address;
	u8 Chip_Sel = XOSPIPSV_SELECT_FLASH_CS0;

	if ((OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) &&
	    (Address & Flash_Config_Table[FCTIndex].FlashDeviceSize)) {
		Chip_Sel = XOSPIPSV_SELECT_FLASH_CS1;
		RealAddr = Address & (~Flash_Config_Table[FCTIndex].FlashDeviceSize);
	}

	(void)XOspiPsv_SelectFlash(OspiPsvPtr, Chip_Sel);

	return RealAddr;
}

/*****************************************************************************/
/**
 *
 * @brief Configures the ECC mode in Flash device.
 *
 * In some flashes by default 2-bit ECC is enabled, which prevents byte-programming( <16-Bytes).
 * The function enables byte-programming by setting the ECC configuration to
 * 1-bit error detection/correction.
 *
 * @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
 *
 * @return	XST_SUCCESS or XST_FAILURE.
 *
 * @note	The function is specific to some spansion flashes(S28HS02GT).
 *
 ******************************************************************************/
int SpansionSetEccMode(XOspiPsv *OspiPsvPtr)
{
	int Status;
	u8 Write_Reg_Opcode = WRITE_CONFIG_REG_SPN;
	u8 Read_Reg_Opcode = READ_CONFIG_REG_SPN;
	u8 AddrSize = 4;
	u8 Dummy = 8;
	u32 Address = CONFIG_REG_4_ADDR_SPN;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
#endif

	Data[0] = 0xA0;
	Data[1] = 0xA0;

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
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
	}

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = Write_Reg_Opcode;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
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

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read Configuration register */
	FlashMsg.Opcode = Read_Reg_Opcode;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		/* Read Configuration register */
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0]) {
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Configures the Dummy cycles in Flash device.
 *
 * @param[in,out] OspiPsvPtr Pointer to the OSPIPSV driver component to use.
 *
 * @return	XST_SUCCESS or XST_FAILURE.
 *
 ******************************************************************************/
int MxConfigDummy(XOspiPsv *OspiPsvPtr){

	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
	u32 RegAddr=CONFIG_REG2_VOLATILE_ADDR_MX;
	u32 Status;
	Data[0]=0x00;
	Data[1]=0x00;

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
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = WRITE_CONFIG2_REG_MX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Addr = RegAddr;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.ByteCount = 2;
	}
	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read back the configuration register 2 */
	FlashMsg.Opcode = READ_CONFIG2_REG_MX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = 0;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Addr = RegAddr;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.ByteCount = 2;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("ConfigReg:0x%x \n",ConfigReg[0]);
	if (ConfigReg[0] != Data[0]) {
			return XST_FAILURE;
	}
	return Status;
}
/** @} */
