/*
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM
#ifdef ENABLE_POS_QSPI

#include "pm_qspi.h"
#include "pm_common.h"
#include "xpfw_config.h"
#include "xparameters.h"
#ifdef XPAR_PSU_QSPI_0_DEVICE_ID
	#include <xqspipsu.h>
#else
	#error "ENABLE_POS_QSPI is defined but qspi device is not available"
#endif

#define IOU_SLCR_BASE		0XFF180000U
#define IOU_SCLR_MIO_PIN_0	( ( IOU_SLCR_BASE )  + 0X00000000U )
#define IOU_SCLR_MIO_PIN_1	( ( IOU_SLCR_BASE )  + 0X00000004U )
#define IOU_SCLR_MIO_PIN_2	( ( IOU_SLCR_BASE )  + 0X00000008U )
#define IOU_SCLR_MIO_PIN_3	( ( IOU_SLCR_BASE )  + 0X0000000CU )
#define IOU_SCLR_MIO_PIN_4	( ( IOU_SLCR_BASE )  + 0X00000010U )
#define IOU_SCLR_MIO_PIN_5	( ( IOU_SLCR_BASE )  + 0X00000014U )
#define IOU_SCLR_MIO_PIN_6	( ( IOU_SLCR_BASE )  + 0X00000018U )
#define IOU_SCLR_MIO_PIN_7	( ( IOU_SLCR_BASE )  + 0X0000001CU )
#define IOU_SCLR_MIO_PIN_8	( ( IOU_SLCR_BASE )  + 0X00000020U )
#define IOU_SCLR_MIO_PIN_9	( ( IOU_SLCR_BASE )  + 0X00000024U )
#define IOU_SCLR_MIO_PIN_10	( ( IOU_SLCR_BASE )  + 0X00000028U )
#define IOU_SCLR_MIO_PIN_11	( ( IOU_SLCR_BASE )  + 0X0000002CU )
#define IOU_SCLR_MIO_PIN_12	( ( IOU_SLCR_BASE )  + 0X00000030U )
#define IOU_SCLR_MIO_MST_TRI0	( ( IOU_SLCR_BASE )  + 0X00000204U )
#define IOU_SCLR_TAPDLY_BYPASS	( ( IOU_SLCR_BASE )  + 0X00000390U )

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

#define WRITE_CMD_4B		0x12
#define READ_CMD_4B		0x13
#define FAST_READ_CMD_4B	0x0C
#define DUAL_READ_CMD_4B	0x3C
#define QUAD_READ_CMD_4B	0x6C
#define	SEC_ERASE_CMD_4B	0xDC

#define BANK_REG_RD		0x16
#define BANK_REG_WR		0x17
/* Bank register is called Extended Address Register in Micron */
#define EXTADD_REG_RD		0xC8
#define EXTADD_REG_WR		0xC5
#define	DIE_ERASE_CMD		0xC4
#define READ_FLAG_STATUS_CMD	0x70

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
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define ADDRESS_4_OFFSET	4 /* LSB byte of address to read or write when
				     4 byte address */

#define DUMMY_CLOCKS		8 /* Number of dummy bytes for fast, dual and
				     quad reads */

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID

/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 	512
#define SECT_SIZE	0x10000
#define SECT_MASK	0xFFFF0000
#define FLASH_SIZE	0x4000000

/*
 * Flash address to which data is to be written.
 */
#define TEST_ADDRESS		0x000000
#define WRITE_ADDR		TEST_ADDRESS
#define READ_ADDR		TEST_ADDRESS

static u8 CmdBfr[8];

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPsu QspiPsuInstance;

static XQspiPsu_Msg FlashMsg[5];

int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
		u8 *WriteBfrPtr);
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
		u8 *WriteBfrPtr, u8 *ReadBfrPtr);
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr);
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address);

/*****************************************************************************/
/**
* This is wrapper function used to write data to the Flash. If number of bytes
* to be transmitted exceeds page size spilt transfer into smaller packets sized
* up to the page size.
*
* @param	Pointer to the write buffer (which is to be transmitted)
* @param	ByteCount contains the number of bytes to write.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int PmQspiWrite(u8 *WriteBufrPtr, u32 ByteCount)
{
	int Status = XST_SUCCESS;
	u32 length = ByteCount;
	u32 page = 0;

	FlashErase(&QspiPsuInstance, WRITE_ADDR, ByteCount, CmdBfr);

	while (length > 0) {
		if (length < MAX_PAGE_SIZE) {
			Status = FlashWrite(&QspiPsuInstance,
					page * MAX_PAGE_SIZE + WRITE_ADDR,
					length,
					WRITE_CMD_4B,
					WriteBufrPtr + page * MAX_PAGE_SIZE);
			length = 0;
		} else {
			Status = FlashWrite(&QspiPsuInstance,
					page * MAX_PAGE_SIZE + WRITE_ADDR,
					MAX_PAGE_SIZE,
					WRITE_CMD_4B,
					WriteBufrPtr + page * MAX_PAGE_SIZE);
			length -= MAX_PAGE_SIZE;
		}
		page++;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This is wrapper function used to read data from the Flash. Use READ_ADDR as
* as address to read from in the Flash. Use quad 4b read command.
*
* @param	ByteCount contains the number of bytes to read.
* @param	Pointer to the read buffer to which valid received data should be
* 		written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int PmQspiRead(u32 ByteCount, u8 *ReadBfrPtr)
{
	int Status;

	Status = FlashRead(&QspiPsuInstance, READ_ADDR, ByteCount,
		QUAD_READ_CMD_4B, CmdBfr, ReadBfrPtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This is function used to initialize QSPI Flash driver for communication with
* serial Flash memory device.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int PmQspiInit(void)
{
	int Status;
	XQspiPsu_Config *QspiPsuConfig;

	/* Initialize QSPIPSU driver so that it's ready to use */
	QspiPsuConfig = XQspiPsu_LookupConfig(QSPIPSU_DEVICE_ID);
	if (NULL == QspiPsuConfig) {
		return XST_FAILURE;
	}

	Status = XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiPsuConfig,
					QspiPsuConfig->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/* Set manual start option */
	XQspiPsu_SetOptions(&QspiPsuInstance, XQSPIPSU_MANUAL_START_OPTION);

	/* Set prescaler for QSPIPSU clock */
	XQspiPsu_SetClkPrescaler(&QspiPsuInstance, XQSPIPSU_CLK_PRESCALE_8);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the QSPIPSU interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries per memory chip.
*
* @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPIPSU
*		device supports only Page Program command to write data to the
*		flash.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	u8 WriteCmd[5];
	u32 RealAddr;
	int Status;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPsuPtr, Address);

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	WriteCmd[COMMAND_OFFSET] = Command;
	WriteCmd[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF000000) >> 24);
	WriteCmd[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteCmd[ADDRESS_3_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteCmd[ADDRESS_4_OFFSET] = (u8)(RealAddr & 0xFF);

	FlashMsg[0].TxBfrPtr = WriteCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 5;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = WriteBfrPtr;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = ByteCount;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_TX;
	if (QspiPsuPtr->Config.ConnectionMode ==
		XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		ReadStatusCmd = READ_FLAG_STATUS_CMD;
		FlashMsg[0].TxBfrPtr = &ReadStatusCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = FlashStatus;
		FlashMsg[1].ByteCount = 2;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;
		if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashStatus[1] &= FlashStatus[0];
		}

		if ((FlashStatus[1] & 0x80) == 0x80) {
			break;
		}
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This function performs read. DMA is the default setting.
*
* @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
* @param	Address contains the address in the Flash to read data from.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. Supports
* 		normal, fast, dual and quad read commands.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 		written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	int Status;

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPsuPtr, Address);

	WriteBfrPtr[COMMAND_OFFSET] = Command;
	WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF000000) >> 24);
	WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteBfrPtr[ADDRESS_4_OFFSET] = (u8)(RealAddr & 0xFF);

	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 5;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	/*
	 * It is recommended to have a separate entry for dummy
	 * It is recommended that Bus width value during dummy
	 * phase should be same as data phase
	 */
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = NULL;
	FlashMsg[1].ByteCount = DUMMY_CLOCKS;
	FlashMsg[1].Flags = 0;

	FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	FlashMsg[2].TxBfrPtr = NULL;
	FlashMsg[2].RxBfrPtr = ReadBfrPtr;
	FlashMsg[2].ByteCount = ByteCount;
	FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;
	if (QspiPsuPtr->Config.ConnectionMode ==
		XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[2].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
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
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	u32 Sector;
	u32 RealAddr;
	u32 NumSect;
	int Status;

	WriteEnableCmd = WRITE_ENABLE_CMD;

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/(SECT_SIZE) + 1;

	/*
	 * If ByteCount to k sectors, but the address range spans from
	 * N to N+k+1 sectors, then increment no. of sectors to be erased
	 */
	if (((Address + ByteCount) & SECT_MASK) ==
		((Address + (NumSect * SECT_SIZE)) & SECT_MASK) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		WriteBfrPtr[COMMAND_OFFSET] = SEC_ERASE_CMD_4B;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF000000)
								>> 24);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF0000)
								>> 16);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
		WriteBfrPtr[ADDRESS_4_OFFSET] = (u8)(RealAddr & 0xFF);
		FlashMsg[0].ByteCount = 5;

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait for the erase command to be completed
		 */
		while (1) {
			ReadStatusCmd = READ_FLAG_STATUS_CMD;
			FlashMsg[0].TxBfrPtr = &ReadStatusCmd;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].ByteCount = 1;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = FlashStatus;
			FlashMsg[1].ByteCount = 2;
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;
			if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
							 FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashStatus[1] &= FlashStatus[0];
			}

			if ((FlashStatus[1] & 0x80) != 0) {
				break;
			}
		}
		Address += SECT_SIZE;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
* @param	Address which is to be accessed (for erase, write or read)
*
* @return	RealAddr is the translated address - for single it is unchanged;
* 		for stacked, the lower flash size is subtracted;
* 		for parallel the address is divided by 2.
*
* @note		None.
*
******************************************************************************/
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address)
{
	u32 RealAddr;

	switch(QspiPsuPtr->Config.ConnectionMode) {
	case XQSPIPSU_CONNECTION_MODE_SINGLE:
		XQspiPsu_SelectFlash(QspiPsuPtr, XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		RealAddr = Address;
		break;
	case XQSPIPSU_CONNECTION_MODE_STACKED:
		/* Select lower or upper Flash based on sector address */
		if (Address & FLASH_SIZE) {
			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_UPPER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Address & (~FLASH_SIZE);
		} else {
			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Address;
		}
		break;
	case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual address / 2
		 */
		XQspiPsu_SelectFlash(QspiPsuPtr,
			XQSPIPSU_SELECT_FLASH_CS_BOTH,
			XQSPIPSU_SELECT_FLASH_BUS_BOTH);
		RealAddr = Address / 2;
		break;
	default:
		RealAddr = 0x0; /* Assign default value */
	break;
	}

	return(RealAddr);
}

/*****************************************************************************/
/**
*
* This function initialize hardware required for normal QSPI operation.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		By default this initialization is performed by FSBL, in case of
*		resume from Power Off Suspend because FSBL initialization is
*		skipped PMUFW must initialize hardware required for QSPI before
*		using it.
*
******************************************************************************/
int PmQspiHWInit(void)
{
	int Status;

	/* Initialize MIOs used by QSPI */
	XPfw_RMW32(IOU_SCLR_MIO_PIN_0, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_1, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_2, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_3, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_4, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_5, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_6, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_7, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_8, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_9, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_10, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_11, 0x000000FEU ,0x00000002U);
	XPfw_RMW32(IOU_SCLR_MIO_PIN_12, 0x000000FEU ,0x00000002U);

	/* Initialize MIO tri-state enables */
	XPfw_RMW32(IOU_SCLR_MIO_MST_TRI0, 0x00001FFFU ,0x00000000U);

	/* Initialize IOPLL used by QSPI clock */
	/* Configure IOPLL */
	XPfw_RMW32(CRL_APB_IOPLL_CFG, 0xFE7FEDEFU, 0x7E672C6CU);
	XPfw_RMW32(CRL_APB_IOPLL_CTRL, 0x00717F00U, 0x00002D00U);
	/* Bypass PLL */
	XPfw_RMW32(CRL_APB_IOPLL_CTRL, 0x00000008U, 0x00000008U);
	/* Assert PLL reset */
	XPfw_RMW32(CRL_APB_IOPLL_CTRL, 0x00000001U, 0x00000001U);
	/* Release PLL reset */
	XPfw_RMW32(CRL_APB_IOPLL_CTRL, 0x00000001U, 0x00000000U);
	/* Wait for PLL lock */
	Status = XPfw_UtilPollForMask(CRL_APB_PLL_STATUS, 0x00000001U, 32000U);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* Remove PLL bypass */
	XPfw_RMW32(CRL_APB_IOPLL_CTRL, 0x00000008U, 0x00000000U);
	/* Configure IOPLL */
	XPfw_RMW32(CRL_APB_IOPLL_TO_FPD_CTRL, 0x00003F00U, 0x00000300U);
	XPfw_RMW32(CRL_APB_IOPLL_FRAC_CFG, 0x8000FFFFU, 0x00000000U);

	/* Configure QSPI clock */
	XPfw_RMW32(CRL_APB_QSPI_REF_CTRL, 0x013F3F07U ,0x01010C00U);

	/* Release QSPI reset */
	XPfw_RMW32(CRL_APB_RST_LPD_IOU2, 0x00000001U ,0x00000000U);

	/* Configure QSPI tap delay */
	XPfw_RMW32(IOU_SCLR_TAPDLY_BYPASS, 0x00000004U ,0x00000004U);

done:
	return Status;
}

#endif
#endif
