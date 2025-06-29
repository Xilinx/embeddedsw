/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.3   sk  05/27/20 Added Stacked mode support.
* 1.4   sk  02/18/21 Added support for Macronix flash and DualByte commands.
* 1.9   sb  06/06/23 Added support for system device-tree flow.
* 1.10  akm 01/31/24 Use OSPI controller reset for resetting flash device.
* 1.10  sb  02/09/24 Add support for Infineon flash part S28HS02G.
* 1.11  sb  05/02/24 Add support for Macronix flash part mx66uw2g345gxrix0.
* 1.12  sb  03/12/25 Fixed gcc and g++ warnings.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv_flash_config.h"
#include "xscugic.h"
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
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#endif
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

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifndef SDT
int OspiPsvInterruptFlashExample(XScuGic *IntcInstancePtr, XOspiPsv *OspiPsvInstancePtr,
				 u16 OspiPsvDeviceId);
#else
int OspiPsvInterruptFlashExample(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress);
#endif
#ifndef SDT
static int OspiPsvSetupIntrSystem(XScuGic *IntcInstancePtr,
				  XOspiPsv *OspiPsvInstancePtr);
static void OspiPsvDisableIntrSystem(XScuGic *IntcInstancePtr, u16 OspiPsvIntrId);
#endif
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
int SpansionSetEccMode(XOspiPsv *OspiPsvPtr);
int MxConfigDummy(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
#ifdef __ICCARM__
#pragma data_alignment = 4
u8 ReadBfrPtr[8];
#else
u8 ReadBfrPtr[8]__attribute__ ((aligned(4)));
#endif

u32 FlashMake;
u32 FlashType;
u32 FCTIndex;	/* Flash configuration table index */

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;
#ifndef SDT
static XScuGic IntcInstance;
#endif

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
u8 FSRFlag;

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT * 256;

/*****************************************************************************/
/**
*
* Main function to call the OSPIPSV Flash Interrupt example.
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

	xil_printf("OSPIPSV Flash Interrupt Example Test\r\n");

	/*
	 * Run the OspiPsv interrupt example.
	 */
#ifndef SDT
	Status = OspiPsvInterruptFlashExample(&IntcInstance, &OspiPsvInstance,
					      OSPIPSV_DEVICE_ID);
#else
	Status = OspiPsvInterruptFlashExample(&OspiPsvInstance, XPAR_XOSPIPSV_0_BASEADDR);
#endif
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
* @param        IntcInstancePtr is a pointer to the instance of the Intc device.
* @param        OspiPsvInstancePtr is a pointer to the instance of the Ospi device.
* @param        OspiPsvDeviceId is the Device ID of the Ospi Device and is the
*               XPAR_<OSPI_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
int OspiPsvInterruptFlashExample(XScuGic *IntcInstancePtr,
				 XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId)
#else
int OspiPsvInterruptFlashExample(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress)
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

	/* To test, change connection mode here if not obtained from HDF */

	Status = XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiPsvConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal) && !defined (VERSAL_NET)
	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
#else
	Status = XOspiPsv_DeviceResetViaOspi(OspiPsvInstancePtr, XOSPIPSV_HWPIN_RESET);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	Status = OspiPsvSetupIntrSystem(IntcInstancePtr, OspiPsvInstancePtr);
#else
	Status = XSetupInterruptSystem(OspiPsvInstancePtr, &XOspiPsv_IntrHandler,
				       OspiPsvConfig->IntrId,
				       OspiPsvConfig->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
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

	/* Configure Flash ECC mode */
	if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Status = SpansionSetEccMode(OspiPsvInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Configure Flash Dummy cycles in flash device */
	if ((FlashMake == MACRONIX_OCTAL_ID_BYTE0) && (FlashType == MACRONIX_OCTAL_ID_BYTE1)){
		Status = MxConfigDummy(OspiPsvInstancePtr);
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

		/* Reset the controller mode to NON-PHY */
		Status = XOspiPsv_SetSdrDdrMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
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

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

#ifndef SDT
	OspiPsvDisableIntrSystem(IntcInstancePtr, OSPIPSV_INTR_ID);
#else
	XDisconnectInterruptCntrl(OspiPsvConfig->IntrId, OspiPsvConfig->IntrParent);
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to determine the number of lines used
* for command, address and data
*
* @param	OspiPsvInstancePtr is a pointer to the OSPIPSV driver component to use.
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
* Reads the flash ID and identifies the flash in FCT table.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}


	xil_printf("FlashID = ");
	while (ReadIdBytes >= 0 ) {
		xil_printf("0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		ReadIdBytes--;
	}
	xil_printf("\n\r");

	OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[3] << 24) | (ReadBfrPtr[2] << 16) |
				    (ReadBfrPtr[1] << 8) | ReadBfrPtr[0]);
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
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	WriteBfrPtr is Pointer to the write buffer
*		(which is to be transmitted)
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
		if (Error) {
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
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

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while (TransferInProgress);
			if (Error) {
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
* This function erases the sectors in the  serial Flash connected to the
* OSPIPSV interface.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	WriteBfrPtr is Pointer to the write buffer
*		(which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
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

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while (TransferInProgress);
			if (Error) {
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
* This function performs read. DMA is the default setting.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	WriteBfrPtr is Pointer to the write buffer which contains data to be transmitted
* @param	ReadBfrPtr isPointer to the read buffer to which valid received data should be
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
			if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
		if (Error) {
			return XST_FAILURE;
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
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WriteBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
		if (Error) {
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
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WriteBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
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

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
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

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while (TransferInProgress);
			if (Error) {
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
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
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

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		TransferInProgress = TRUE;
		Error = 0;
		Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);
		if (Error) {
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

			TransferInProgress = TRUE;
			Error = 0;
			Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			while (TransferInProgress);
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
#ifndef SDT
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
#endif

/*****************************************************************************/
/**
 *
 * Callback handler.
 *
 * @param        CallBackRef is the upper layer callback reference passed back
 *               when the callback function is invoked.
 * @param        StatusEvent is the event that just occurred.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void OspiPsvHandler(void *CallBackRef, u32 StatusEvent)
{
	(void)CallBackRef;
	/*
	 * If the event was not transfer done, then track it as an error
	 */
	if (StatusEvent != XST_SPI_TRANSFER_DONE) {
		xil_printf("\n Error interrupt occurred %x\r\n", StatusEvent);
		Error = 1;
	}
	else {
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
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Mode is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
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
			OspiPsvPtr->DeviceIdData = 0;
			OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[0] << 8) | ReadBfrPtr[0]);
			OspiPsvPtr->DeviceIdData |= ((ReadBfrPtr[1] << 24) | (ReadBfrPtr[1] << 16));
		}
		else {
			Dummy = 0;
			OspiPsvPtr->DeviceIdData = 0;
			OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[3] << 24) | (ReadBfrPtr[2] << 16) |
						    (ReadBfrPtr[1] << 8) | ReadBfrPtr[0]);
		}
	}

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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION) {
		XOspiPsv_ConfigureAutoPolling(OspiPsvPtr, Mode);
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
		return XST_FAILURE;
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

	/* Disable the interrupt as tuning is always done in polled mode */
#ifndef SDT
	OspiPsvDisableIntrSystem(&IntcInstance, OSPIPSV_INTR_ID);
#else
	XDisconnectInterruptCntrl(OspiPsvInstance.Config.IntrId, OspiPsvInstance.Config.IntrParent);
#endif

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	Status = OspiPsvSetupIntrSystem(&IntcInstance, OspiPsvPtr);
#else
	Status = XSetupInterruptSystem(OspiPsvPtr, &XOspiPsv_IntrHandler,
				       OspiPsvInstance.Config.IntrId, OspiPsvInstance.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	} else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Dummy += 8;
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *			for stacked, the lower flash size is subtracted;
 *
 * @note	In addition to get the actual address to work on flash this
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
 * This function configures the ECC mode in Flash device.
 * In some flashes by default 2-bit ECC is enabled, which prevents byte-programming( <16-Bytes).
 * This function enables byte-programming by setting the ECC configuration to 1-bit error
 * detection/correction.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 *
 * @return	XST_SUCCESS or XST_FAILURE.
 *
 * @note	This function is specific to some spansion flashes(S28HS02GT).
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.Dummy = 8;
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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
 * This function configures the Dummy cycles in Flash device.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
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

	TransferInProgress = TRUE;
	Error = 0;
	Status = XOspiPsv_IntrTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);
	if (Error) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0]) {
		return XST_FAILURE;
	}
	return Status;
}
