/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspipsu_generic_nand flash_interrupt_example.c
*
*
* This file contains a design example using the QSPIPSU driver (XQspiPsu).
* This example checks bad block information of single block before erase,
* writes the data to flash and reads it back in DMA mode.
* This examples runs with GENFIFO Manual start. It runs in interrupt mode.
*
*
* This example has been tested with the Winbound 512MBits using A53
* and R5 processors.
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
* 1.13   akm  02/11/21 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"	/* SDK generated parameters */
#include "xqspipsu.h"		/* QSPIPSU device driver */
#include "xil_printf.h"
#include "xil_cache.h"
#include "xscugic.h"            /* Interrupt controller device driver */
#include "xil_exception.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID       XPAR_XQSPIPSU_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define QSPIPSU_INTR_ID         XPAR_XQSPIPS_0_INTR


/*
 * Number of flash pages to be written.
 * NOTE: This need to be updated as per the flash parameter.
 */
#define MAX_PAGE_COUNT		64

/*
 * Max page size to initialize write and read buffer
 * NOTE: This need to be updated as per the flash parameter.
 */
#define MAX_PAGE_SIZE 	2048

/*
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x000000

#define UNIQUE_VALUE		0x51

#define PARAMETER_PAGE_ADDRESS	0x01

#define OTP_E_BIT				0x06

#define STATUS_REG_1			0xA0
#define STATUS_REG_2			0xB0
#define STATUS_REG_3			0xC0
#define ENABLE_BUF_MODE			0x19
#define DISABLE_WP			0x00
#define DISABLE_BUF_MODE		0x11

#define ONFI_CRC_LEN			254

/*
 * Flash commands
 */
#define READ_ID				0x9F
#define PAGE_DATA_READ		0x13
#define PAGE_DATA_LOAD		0x02
#define QUAD_READ_CMD_4B	0x6C
#define READ_STATUS_CMD		0x05
#define WRITE_CMD			0x02
#define WRITE_ENABLE_CMD	0x06
#define WRITE_STATUS_CMD	0x01
#define BLOCK_ERASE_CMD 	0xD8
#define PROG_EXEC_CMD		0x10

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPIPSU driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */

#define PAGE_SIZE_0_OFFSET	80
#define PAGE_SIZE_1_OFFSET	81
#define PAGE_SIZE_2_OFFSET	82
#define PAGE_SIZE_3_OFFSET	83
#define SPARE_0_OFFSET		84
#define SPARE_1_OFFSET		85
#define PAGE_COUNT_0_OFFSET 	92
#define PAGE_COUNT_1_OFFSET	93
#define PAGE_COUNT_2_OFFSET 	94
#define PAGE_COUNT_3_OFFSET 	95

#define CRC_0_OFFSET		254
#define CRC_1_OFFSET		255
#define DUMMY_CLOCKS		8 /* Number of dummy cycles */

/**************************** Type Definitions *******************************/

u8 ReadCmd;
u8 WriteCmd;
u8 StatusCmd;
u8 EraseCmd;
u8 FSRFlag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int QspiPsuInterruptFlashExample(XScuGic *IntcInstancePtr,
                XQspiPsu *QspiPsuInstancePtr,
                u16 QspiPsuDeviceId, u16 QspiPsuIntrId);
int FlashReadID(XQspiPsu *QspiPsuPtr);
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address);
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
int FlashStatusRead(XQspiPsu *QspiPsuPtr, u8 RegAddr, u8 *ReadBfrPtr);
int FlashStatusWrite(XQspiPsu *QspiPsuPtr, u8 RegAddr, u8 RegVal);
int FlashCheckIsBadBlock(XQspiPsu *QspiPsuPtr, u32 Address);
int FlashIsNotBusy(XQspiPsu *QspiPsuPtr);
static int QspiPsuSetupIntrSystem(XScuGic *IntcInstancePtr,
                               XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuIntrId);
void QspiPsuHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount);
s32 XQspiPsu_ReadParamPage(XQspiPsu *QspiPsuPtr);
s32 XQspiPsu_ParamPageCrc(u8 *ParamBuf, u32 StartOff, u32 Length);
/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
u8 ReadBfrPtr[3];
u8 ReadSpareBfrPtr[100];
u8 ParameterPageData[2048];
u32 PageSize = 0;
u32 BlockSize = 0;
u32 PageCount = 0;

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XScuGic IntcInstance;
static XQspiPsu QspiPsuInstance;
static XQspiPsu_Msg FlashMsg[5];

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
 * The buffer size accounts for maximum page size and maximum banks -
 * for each bank separate read will be performed leading to that many
 * (overhead+dummy) bytes
 */
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 ReadBuffer[MAX_PAGE_COUNT * MAX_PAGE_SIZE];
#else
u8 ReadBuffer[MAX_PAGE_COUNT * MAX_PAGE_SIZE] __attribute__ ((aligned(64)));
#endif
u8 WriteBuffer[MAX_PAGE_COUNT * MAX_PAGE_SIZE];
u8 CmdBfr[8];

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData;

/*****************************************************************************/
/**
 *
 * Main function to call the QSPIPSU Flash Polled example.
 *
 * @param	None
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note	None
 *
 ******************************************************************************/
int main(void)
{
    int Status;

    xil_printf("QSPIPSU Generic NAND Flash Interrupt Example Test \r\n");

    /*
     * Run the QspiPsu Interrupt example.
     */
    Status = QspiPsuInterruptFlashExample(&IntcInstance, &QspiPsuInstance,
                                    QSPIPSU_DEVICE_ID, QSPIPSU_INTR_ID);
    if (Status != XST_SUCCESS) {
            xil_printf("QSPIPSU Generic NAND Flash Interrupt Example Failed\r\n");
            return XST_FAILURE;
    }

    xil_printf("Successfully ran QSPIPSU Generic NAND Flash Interrupt Example\r\n");
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the XQspiPsu
 * device driver using NAND flash devices.
 *
 * @param       None.
 *
 * @return      XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note        None.
 *
 *****************************************************************************/
int QspiPsuInterruptFlashExample(XScuGic *IntcInstancePtr,
                XQspiPsu *QspiPsuInstancePtr,
                u16 QspiPsuDeviceId, u16 QspiPsuIntrId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPsu_Config *QspiPsuConfig;
	int ReadBfrSize;
	u8 Status_Reg;
	u32 Crc = 0;

	/*
	 * Initialize the QSPIPSU driver so that it's ready to use
	 */
	QspiPsuConfig = XQspiPsu_LookupConfig(QspiPsuDeviceId);
	if (QspiPsuConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiPsuConfig,
					QspiPsuConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the QspiPsu device to the interrupt subsystem such that
	 * interrupts can occur. This function is application specific
	 */
	Status = QspiPsuSetupIntrSystem(IntcInstancePtr, QspiPsuInstancePtr,
				     QspiPsuIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the QSPIPSU that will be called from the
	 * interrupt context when an QSPIPSU status occurs, specify a pointer to
	 * the QSPIPSU driver instance as the callback reference
	 * so the handler is able to access the instance data
	 */
	XQspiPsu_SetStatusHandler(QspiPsuInstancePtr, QspiPsuInstancePtr,
				 (XQspiPsu_StatusHandler) QspiPsuHandler);

	/*
	 * Set Manual Start
	 */
	XQspiPsu_SetOptions(QspiPsuInstancePtr, XQSPIPSU_MANUAL_START_OPTION);

	/*
	 * Set the pre-scaler for QSPIPSU clock
	 */
	XQspiPsu_SetClkPrescaler(QspiPsuInstancePtr, XQSPIPSU_CLK_PRESCALE_8);

	/* Upper chip and upper bus selection */
	XQspiPsu_SelectFlash(QspiPsuInstancePtr,
		XQSPIPSU_SELECT_FLASH_CS_UPPER,
		XQSPIPSU_SELECT_FLASH_BUS_UPPER);

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(QspiPsuInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashStatusRead(QspiPsuInstancePtr, STATUS_REG_1, &Status_Reg);
	FlashStatusRead(QspiPsuInstancePtr, STATUS_REG_2, &Status_Reg);
	FlashStatusRead(QspiPsuInstancePtr, STATUS_REG_3, &Status_Reg);

	/* Un-protect flash device for erase and write */
	FlashStatusWrite(QspiPsuInstancePtr, STATUS_REG_1, DISABLE_WP);

	/* Enable buffer mode */
	FlashStatusWrite(QspiPsuInstancePtr, STATUS_REG_2, ENABLE_BUF_MODE);

	ReadCmd = QUAD_READ_CMD_4B;
	StatusCmd = READ_STATUS_CMD;
	WriteCmd = WRITE_CMD;
	EraseCmd = BLOCK_ERASE_CMD;

	Status = XQspiPsu_ReadParamPage(QspiPsuInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Crc = ((ParameterPageData[CRC_1_OFFSET] << 8) |
			ParameterPageData[CRC_0_OFFSET]);

	if (Crc != XQspiPsu_ParamPageCrc(ParameterPageData, 0x00, ONFI_CRC_LEN)) {
		xil_printf("Parameter page crc check failed\n\r");
		return XST_FAILURE;
	}

	PageSize = (ParameterPageData[PAGE_SIZE_3_OFFSET] << 24) |
			   (ParameterPageData[PAGE_SIZE_2_OFFSET] << 16) |
			   (ParameterPageData[PAGE_SIZE_1_OFFSET] << 8) |
			    ParameterPageData[PAGE_SIZE_0_OFFSET];

	PageCount = (ParameterPageData[PAGE_COUNT_3_OFFSET] << 24) |
			    (ParameterPageData[PAGE_COUNT_2_OFFSET] << 16) |
			    (ParameterPageData[PAGE_COUNT_1_OFFSET] << 8) |
			     ParameterPageData[PAGE_COUNT_0_OFFSET];

	if (PageSize > MAX_PAGE_SIZE) {
		xil_printf("Invalid Page Size\n\r");
		return XST_FAILURE;
	}

	if (PageCount > MAX_PAGE_COUNT) {
		xil_printf("Invalid Page Count\n\r");
		return XST_FAILURE;
	}

	xil_printf("ReadCmd: 0x%x, WriteCmd: 0x%x,"
		   " StatusCmd: 0x%x\n\r",
		ReadCmd, WriteCmd, StatusCmd);

	Status = FlashCheckIsBadBlock(QspiPsuInstancePtr, TEST_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ReadSpareBfrPtr[0] != 0xFF) {
		xil_printf("Block %d is bad block\r\n",
			   TEST_ADDRESS / (PageCount * PageSize));
		return XST_FAILURE;
	}

	ReadBfrSize = (PageCount * PageSize);

	MaxData = (PageCount * PageSize);

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < PageSize;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8) (UniqueValue + Test);
	}

	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}

	Status = FlashErase(QspiPsuInstancePtr, TEST_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Page = 0; Page < MAX_PAGE_COUNT; Page++) {
		Status = FlashWrite(QspiPsuInstancePtr,
				(Page * PageSize) + TEST_ADDRESS, PageSize, WriteCmd,
				WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Disable Buffer mode for continuous read mode */
	FlashStatusWrite(QspiPsuInstancePtr, STATUS_REG_2, DISABLE_BUF_MODE);

	Status = FlashRead(QspiPsuInstancePtr, TEST_ADDRESS,
			MaxData, ReadCmd, CmdBfr, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (UniqueValue = UNIQUE_VALUE, Count = 0; Count < MaxData;
				Count++, UniqueValue++) {
		if (ReadBuffer[Count] != (u8) (UniqueValue + Test)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system for a QspiPsu device.
 *
 * @param       IntcInstancePtr is a pointer to the instance of the Intc
 *              device.
 * @param       QspiPsuInstancePtr is a pointer to the instance of the
 *              QspiPsu device.
 * @param       QspiPsuIntrId is the interrupt Id for an QSPIPSU device.
 *
 * @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note        None.
 *
 ******************************************************************************/
static int QspiPsuSetupIntrSystem(XScuGic *IntcInstancePtr,
                               XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuIntrId)
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
        Status = XScuGic_Connect(IntcInstancePtr, QspiPsuIntrId,
                        (Xil_ExceptionHandler)XQspiPsu_InterruptHandler,
                        (void *)QspiPsuInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	/*
	 * Enable the interrupt for the QspiPsu device.
	 */
	XScuGic_Enable(IntcInstancePtr, QspiPsuIntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 *
 * Callback handler.
 *
 * @param       None.
 *
 * @return      None
 *
 * @note        None.
 *
 *****************************************************************************/
void QspiPsuHandler(void *CallBackRef, u32 StatusEvent, unsigned int ByteCount)
{
        /*
         * Indicate the transfer on the QSPIPSU bus is no longer in progress
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
 * Reads the flash ID information
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status;

	/*
	 * Read ID
	 */
	TxBfrPtr = READ_ID;
	FlashMsg[0].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = NULL;
	FlashMsg[2].RxBfrPtr = ReadBfrPtr;
	FlashMsg[2].ByteCount = 3;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);


	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBfrPtr[0], ReadBfrPtr[1],
		   ReadBfrPtr[2]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address to write data to in the Flash.
 * @param	ByteCount contains the number of bytes to write.
 * @param	Command is the command used to write data to the flash.
 * @param	Pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ProgExeCmd;
	u8 WriteBuf[5];
	u16 PageAddr;
	int Status;

	PageAddr = Address / PageSize;

	/* Enable write enable */
	WriteEnableCmd = WRITE_ENABLE_CMD;

	FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Load program data */
	WriteBuf[COMMAND_OFFSET]   = PAGE_DATA_LOAD;
	WriteBuf[ADDRESS_1_OFFSET] = 0x0;
	WriteBuf[ADDRESS_2_OFFSET] = 0x0;

	FlashMsg[0].TxBfrPtr = WriteBuf;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 3;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = WriteBfrPtr;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = ByteCount;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Program execute */
	ProgExeCmd = PROG_EXEC_CMD;

	WriteBuf[0] = (u8)((PageAddr & 0xFF00) >> 8);
	WriteBuf[1] = (u8)(PageAddr & 0xFF);


	FlashMsg[0].TxBfrPtr = &ProgExeCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = WriteBuf;
	FlashMsg[2].RxBfrPtr = NULL;
	FlashMsg[2].ByteCount = 2;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Check flash is busy */
	Status = FlashIsNotBusy(QspiPsuPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
 *
 * This function erases the sectors in the  serial Flash connected to the
 * QSPIPSU interface.
 *
 * @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *		be erased.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address)
{
	u8 WriteEnableCmd;
	u8 BlockEraseCmd;
	u32 RealAddr;
	u8 PageAddr[2];
	int Status;

	RealAddr = Address / PageSize;

	/* Enable write enable */
	WriteEnableCmd = WRITE_ENABLE_CMD;

	FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);


	/* Block erase */
	PageAddr[0] = (u8) ((RealAddr & 0xFF00) >> 8);
	PageAddr[1] = (u8) (RealAddr & 0xFF);

	BlockEraseCmd = EraseCmd;

	FlashMsg[0].TxBfrPtr = &BlockEraseCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = PageAddr;
	FlashMsg[2].RxBfrPtr = NULL;
	FlashMsg[2].ByteCount = 2;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Check flash is busy */
	Status = FlashIsNotBusy(QspiPsuPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}


/*****************************************************************************/
/**
 *
 * This function performs read. DMA is the default setting.
 *
 * @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Command is the command used to read data from the flash.
 *		Supports normal, fast, dual and quad read commands.
 * @param	Pointer to the write buffer which contains data to be
 *		transmitted
 * @param	Pointer to the read buffer to which valid received data
 *		should be written
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u16 PageAddr;
	u8 PageReadCmd;
	u8 WriteBuf[2];
	int Status;

	PageAddr = Address / PageSize;

	/* Set page address */
	WriteBuf[0] = (u8) ((PageAddr & 0xFF00) >> 8);
	WriteBuf[1] = (u8) (PageAddr & 0xFF);

	PageReadCmd = PAGE_DATA_READ;

	FlashMsg[0].TxBfrPtr = &PageReadCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = WriteBuf;
	FlashMsg[2].RxBfrPtr = NULL;
	FlashMsg[2].ByteCount = 2;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Check flash is busy */
	Status = FlashIsNotBusy(QspiPsuPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Continuous data read */
	WriteBfrPtr[COMMAND_OFFSET] = Command;

	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = 5 * DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = NULL;
	FlashMsg[2].RxBfrPtr = ReadBfrPtr;
	FlashMsg[2].ByteCount = ByteCount;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	RegAddr is the flash register address.
 * @param	Data to be written to register.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashStatusWrite(XQspiPsu *QspiPsuPtr, u8 RegAddr, u8 Data)
{
	u8 WriteBfr[5];
	int Status = XST_FAILURE;

	WriteBfr[COMMAND_OFFSET] = WRITE_STATUS_CMD;
	WriteBfr[ADDRESS_1_OFFSET] = RegAddr;
	WriteBfr[2] = Data;

	FlashMsg[0].TxBfrPtr = WriteBfr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 3;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	while (TransferInProgress);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	RegAddr is the flash register address
 * @param	ReadBfrPtr is the pointer to value to be read.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashStatusRead(XQspiPsu *QspiPsuPtr, u8 RegAddr, u8 *ReadBfrPtr)
{
	u8 WriteBfr[2];
	int Status = XST_FAILURE;


	WriteBfr[COMMAND_OFFSET] = READ_STATUS_CMD;
	WriteBfr[ADDRESS_1_OFFSET] = RegAddr;

	FlashMsg[0].TxBfrPtr = WriteBfr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 2;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadBfrPtr;
	FlashMsg[1].ByteCount = 1;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	while (TransferInProgress);

	return Status;
}

/*****************************************************************************/
/**
 *
 * This API is to get flash status.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashIsNotBusy(XQspiPsu *QspiPsuPtr)
{
	u8 FlashStatus[1];
	u8 WriteBfrPtr[2];
	int Status = XST_FAILURE;

	WriteBfrPtr[COMMAND_OFFSET] = StatusCmd;
	WriteBfrPtr[ADDRESS_1_OFFSET] = STATUS_REG_3;

	while (1) {
		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 2;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = FlashStatus;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		TransferInProgress = TRUE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		while (TransferInProgress);

		if ((FlashStatus[0] & 0x01) == 0x00) {
			return XST_SUCCESS;
		}
	}
}

/*****************************************************************************/
/**
 *
 * This API is to check bad block information.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 *
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 *
 ******************************************************************************/
int FlashCheckIsBadBlock(XQspiPsu *QspiPsuPtr, u32 Address)
{

	u32 RealAddr;
	u8 PageReadCmd;
	u8 WriteBfrPtr[3];
	u8 PageAddr[2];
	int Status;

	RealAddr = Address / PageSize;

	/* Page data read */
	PageAddr[0] = (u8) ((RealAddr & 0xFF00) >> 8);
	PageAddr[1] = (u8) (RealAddr & 0xFF);

	PageReadCmd = PAGE_DATA_READ;

	FlashMsg[0].TxBfrPtr = &PageReadCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = PageAddr;
	FlashMsg[2].RxBfrPtr = NULL;
	FlashMsg[2].ByteCount = 2;
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	/* Check flash is busy */
	Status = FlashIsNotBusy(QspiPsuPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Flash read */
	RealAddr = PageSize;

	WriteBfrPtr[COMMAND_OFFSET] = QUAD_READ_CMD_4B;
	WriteBfrPtr[ADDRESS_1_OFFSET] = (u8) ((RealAddr & 0xFF00) >> 8);
	WriteBfrPtr[ADDRESS_2_OFFSET] = (u8) (RealAddr & 0xFF);

	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 3;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = 3 * DUMMY_CLOCKS;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].TxBfrPtr = NULL;
	FlashMsg[2].RxBfrPtr = ReadSpareBfrPtr;
	FlashMsg[2].ByteCount = ((ParameterPageData[SPARE_1_OFFSET] << 8) |
							 ParameterPageData[SPARE_0_OFFSET]);
	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

	TransferInProgress = TRUE;
	Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (TransferInProgress);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads Parameter Page data from the flash.
*
* @param        QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
* @param        Buf is the parameter page information to return.
*
* @return
*               - XST_SUCCESS if successful.
*               - XST_FAILURE if failed.
*
* @note         None
*
******************************************************************************/
s32 XQspiPsu_ReadParamPage(XQspiPsu *QspiPsuPtr)
{
        s32 Status = XST_FAILURE;
        u16 PageAddr;
		u8 PageReadCmd;
		u8 WriteBuf[2];
		u8 Status_Reg;

		/*
		 * To access Parameter page data,
		 * the OTP-E bit in Status Register-2
		 * must be set to “1” first
		 */
		FlashStatusRead(QspiPsuPtr, STATUS_REG_2, &Status_Reg);
		Status_Reg |= 0x01 << OTP_E_BIT;
		FlashStatusWrite(QspiPsuPtr, STATUS_REG_2, Status_Reg);

		PageAddr = PARAMETER_PAGE_ADDRESS;

		/* Set page address */
		WriteBuf[0] = (u8) ((PageAddr & 0xFF00) >> 8);
		WriteBuf[1] = (u8) (PageAddr & 0xFF);

		PageReadCmd = PAGE_DATA_READ;

		FlashMsg[0].TxBfrPtr = &PageReadCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = DUMMY_CLOCKS;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = 0;

		FlashMsg[2].TxBfrPtr = WriteBuf;
		FlashMsg[2].RxBfrPtr = NULL;
		FlashMsg[2].ByteCount = 2;
		FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_TX;

		TransferInProgress = TRUE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);

		/* Check flash is busy */
		Status = FlashIsNotBusy(QspiPsuPtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Continuous data read */
		WriteBuf[COMMAND_OFFSET] = QUAD_READ_CMD_4B;

		FlashMsg[0].TxBfrPtr = WriteBuf;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 5 * DUMMY_CLOCKS;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = 0;

		FlashMsg[2].TxBfrPtr = NULL;
		FlashMsg[2].RxBfrPtr = ParameterPageData;
		FlashMsg[2].ByteCount = 2048;
		FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

		TransferInProgress = TRUE;
		Status = XQspiPsu_InterruptTransfer(QspiPsuPtr, FlashMsg, 3);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (TransferInProgress);

		/*
		 * To return to the main memory array operation,
		 * OTP-E bit in Status Register-2 needs to be to
		 * set to "0"
		 */
		FlashStatusRead(QspiPsuPtr, STATUS_REG_2, &Status_Reg);
		Status_Reg &= ~(0x01 << OTP_E_BIT);
		FlashStatusWrite(QspiPsuPtr, STATUS_REG_2, Status_Reg);

		return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calculates parameter page CRC.
*
* @param        Parambuf is a pointer to the ONFI parameter page buffer.
* @param        StartOff is the starting offset in buffer to calculate CRC.
* @param        Length is the number of bytes for which CRC is calculated.
*
* @return
*               CRC value.
* @note
*               None.
*
******************************************************************************/
s32 XQspiPsu_ParamPageCrc(u8 *ParamBuf, u32 StartOff, u32 Length)
{
        const u32 CrcInit = 0x4F4EU;
        const u32 Order = 16U;
        const u32 Polynom = 0x8005U;
        u32 i, j, c, Bit;
        u32 Crc = CrcInit;
        u32 DataIn;
        u32 DataByteCount = 0U;
        u32 CrcMask, CrcHighBit;

        CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) -(u32)1) << (u32)1) | (u32)1;
        CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
        /*
         * CRC covers the data bytes between byte 0 and byte 253
         * (ONFI 1.0, section 5.4.1.36)
         */
        for (i = StartOff; i < Length; i++) {
                DataIn = *(ParamBuf + i);
                c = (u32)DataIn;
                DataByteCount++;
                j = 0x80U;
                while (j != 0U) {
                        Bit = Crc & CrcHighBit;
                        Crc <<= 1U;
                        if ((c & j) != 0U) {
                                Bit ^= CrcHighBit;
                        }
                        if (Bit != 0U) {
                                Crc ^= Polynom;
                        }
                        j >>= 1U;
                }
                Crc &= CrcMask;
        }
        return Crc;
}
