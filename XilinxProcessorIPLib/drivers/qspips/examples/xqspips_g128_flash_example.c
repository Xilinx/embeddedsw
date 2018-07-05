/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspips_g128_flash_example.c
*
*
* This file contains a design example using the QSPI driver (XQspiPs)
* with a serial Flash device greater than 128Mb. The example writes to flash
* and reads it back in I/O mode. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available. It is recommended to use Manual CS + Auto start for
* best performance.
* This example illustrates single, parallel and stacked modes.
* Both the flash devices have to be of the same make and size.
* The hardware which this example runs on, must have a serial Flash (Micron
* N25Q or Spansion S25FL) for it to run. This example has been
* tested with the Micron Serial Flash (N25Q256, N25Q512 & N25Q00AA) and
* Spansion (S25FL256 & S25FL512)
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
*       raw 12/10/15 Added support for Macronix 256Mb and 1Gb flash parts
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation.
* 		tjs	06/16/17 Added support for IS25LP256D flash part (PR-4650)
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xqspips.h"		/* QSPI device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD			0x02
#define READ_CMD			0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID				0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01
#define BANK_REG_RD			0x16
#define BANK_REG_WR			0x17
/* Bank register is called Extended Address Register in Micron */
#define EXTADD_REG_RD		0xC8
#define EXTADD_REG_WR		0xC5
#define	DIE_ERASE_CMD		0xC4
#define READ_FLAG_STATUS_CMD	0x70

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
				     reads */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
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
 * Flash on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

/*
 * Base address of Flash1
 */
#define FLASH1BASE 0x0000000

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000


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
 * Byte 1 is first byte of Device ID - 0xBB or 0xBA
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 * Spansion:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is Device ID - Memory Interface type - 0x20 or 0x02
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 */
#define MICRON_ID_BYTE0		0x20
#define MICRON_ID_BYTE2_128	0x18
#define MICRON_ID_BYTE2_256	0x19
#define MICRON_ID_BYTE2_512	0x20
#define MICRON_ID_BYTE2_1G	0x21

#define SPANSION_ID_BYTE0		0x01
#define SPANSION_ID_BYTE2_128	0x18
#define SPANSION_ID_BYTE2_256	0x19
#define SPANSION_ID_BYTE2_512	0x20

#define WINBOND_ID_BYTE0		0xEF
#define WINBOND_ID_BYTE2_128	0x18

#define MACRONIX_ID_BYTE0		0xC2
#define MACRONIX_ID_BYTE2_256	0x19
#define MACRONIX_ID_BYTE2_512	0x1A
#define MACRONIX_ID_BYTE2_1G	0x1B

#define ISSI_ID_BYTE0			0x9D
#define ISSI_ID_BYTE2_256		0x19
/*
 * The index for Flash config table
 */
/* Spansion*/
#define SPANSION_INDEX_START			0
#define FLASH_CFG_TBL_SINGLE_128_SP		SPANSION_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_SP	(SPANSION_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_SP	(SPANSION_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_SP		(SPANSION_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_SP	(SPANSION_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_SP	(SPANSION_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_SP		(SPANSION_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_SP	(SPANSION_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_SP	(SPANSION_INDEX_START + 8)

/* Micron */
#define MICRON_INDEX_START				(FLASH_CFG_TBL_PARALLEL_512_SP + 1)
#define FLASH_CFG_TBL_SINGLE_128_MC		MICRON_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_MC	(MICRON_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_MC	(MICRON_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_MC		(MICRON_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_MC	(MICRON_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_MC	(MICRON_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_MC		(MICRON_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_MC	(MICRON_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_MC	(MICRON_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_1GB_MC		(MICRON_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_1GB_MC	(MICRON_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_1GB_MC	(MICRON_INDEX_START + 11)

/* Winbond */
#define WINBOND_INDEX_START				(FLASH_CFG_TBL_PARALLEL_1GB_MC + 1)
#define FLASH_CFG_TBL_SINGLE_128_WB		WINBOND_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_WB	(WINBOND_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_WB	(WINBOND_INDEX_START + 2)

/* Macronix */
#define MACRONIX_INDEX_START			(FLASH_CFG_TBL_PARALLEL_128_WB + 1 - 3)
#define FLASH_CFG_TBL_SINGLE_256_MX		MACRONIX_INDEX_START
#define FLASH_CFG_TBL_STACKED_256_MX	(MACRONIX_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_256_MX	(MACRONIX_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_512_MX		(MACRONIX_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_512_MX	(MACRONIX_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_512_MX	(MACRONIX_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_1G_MX		(MACRONIX_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_1G_MX		(MACRONIX_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_1G_MX	(MACRONIX_INDEX_START + 8)
/* ISSI */
#define ISSI_INDEX_START				(FLASH_CFG_TBL_PARALLEL_1G_MX + 1)
#define FLASH_CFG_TBL_SINGLE_256_ISSI	ISSI_INDEX_START
#define FLASH_CFG_TBL_STACKED_256_ISSI	(ISSI_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_256_ISSI	(ISSI_INDEX_START + 2)
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID
/*
 * The following defines are for dual flash stacked mode interface.
 */
#define LQSPI_CR_FAST_QUAD_READ		0x0000006B /* Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		0x00000100 /* 1 Dummy Byte between
						     address and return data */

#define DUAL_STACK_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

#define DUAL_QSPI_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQSPIPS_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

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
#define TEST_ADDRESS		0x000000


#define UNIQUE_VALUE		0x06

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
	u8 NumDie;			/* No. of die forming a single flash */

}FlashInfo;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int QspiG128FlashExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId);

void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);

void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);

int FlashReadID(XQspiPs *QspiPtr, u8 *WriteBfrPtr, u8 *ReadBfrPtr);

void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);

int SendBankSelect(XQspiPs *QspiPtr, u8 *WriteBfrPtr, u32 BankSel);

void BulkErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr);

void DieErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr);

u32 GetRealAddr(XQspiPs *QspiPtr, u32 Address);


/************************** Variable Definitions *****************************/

FlashInfo Flash_Config_Table[34] = {
		/* Spansion */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x40000, 0x100, 512, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x40000, 0x200, 512, 0x40000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x80000, 0x100, 1024, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFF80000, 1},
		/* Spansion 1Gbit is handled as 512Mbit stacked */
		/* Micron */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFE0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x10000, 0x1000, 256, 0x100000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x20000, 0x800, 512, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFE0000, 4},
		/* Winbond */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFE0000, 1},
		/* Macronix */
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFE0000, 1},
		{0x2000, 0x4000, 256, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x2000, 0x8000, 256, 0x100000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x4000, 0x4000, 512, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFE0000, 1},
		/* ISSI */
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				ISSI_ID_BYTE0, ISSI_ID_BYTE2_256, 0xFFFF0000, 1}
};				/**< Flash Config Table */

u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */


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

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT*256;

/*****************************************************************************/
/**
*
* Main function to call the QSPI Flash example.
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

	xil_printf("QSPI Greater than 128Mb Flash Example Test \r\n");

	/*
	 * Run the Qspi Interrupt example.
	 */
	Status = QspiG128FlashExample(&QspiInstance, QSPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPI Greater than 128Mb Flash Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPI Greater than 128Mb Flash Ex Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XQspiPs
* device driver in single, parallel and stacked modes using
* flash devices greater than 128Mb.
* This function reads and writes data in I/O mode.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int QspiG128FlashExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPs_Config *QspiConfig;

	/*
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
	if (NULL == QspiConfig) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
					QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XQspiPs_SelfTest(QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}



	/*
	 * Set the pre-scaler for QSPI clock
	 */
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/*
	 * Set Manual Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
					     XQSPIPS_MANUAL_START_OPTION |
					     XQSPIPS_HOLD_B_DRIVE_OPTION);
	if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED) {
		/*
		 * Enable two flash memories, Shared bus (NOT separate bus),
		 * L_PAGE selected by default
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_STACK_CONFIG_WRITE);
	}

	if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_PARALLEL) {
		/*
		 * Enable two flash memories on separate buses
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_QSPI_CONFIG_WRITE);
	}

	/*
	 * Assert the Flash chip select.
	 */
	XQspiPs_SetSlaveSelect(QspiInstancePtr);

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	FlashReadID(QspiInstancePtr, WriteBuffer, ReadBuffer);

	/*
	 * Initialize MaxData according to page size.
	 */
	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);


	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0;
			Count < Flash_Config_Table[FCTIndex].PageSize;
			Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));


	/*
	 * Erase the flash.
	 */
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MaxData, WriteBuffer);

	/*
	 * Write the data in the write buffer to the serial Flash a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr,
			(Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
			Flash_Config_Table[FCTIndex].PageSize, WRITE_CMD, WriteBuffer);
	}

	/*
	 * I/O Read - for any flash size
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MaxData, QUAD_READ_CMD,
				WriteBuffer, ReadBuffer);

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

	/*
	 * Initialize the write buffer for a pattern to write to the Flash
	 * and the read buffer to zero so it can be verified after the read, the
	 * test value that is added to the unique value allows the value to be
	 * changed in a debug environment to guarantee
	 */
	for (UniqueValue = UNIQUE_VALUE, Count = 0;
			Count < Flash_Config_Table[FCTIndex].PageSize;
			Count++, UniqueValue++) {
		WriteBuffer[DATA_OFFSET + Count] = (u8)(UniqueValue + Test);
	}
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	/*
	 * Set Auto Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
					     XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Erase the flash.
	 */
	FlashErase(QspiInstancePtr, TEST_ADDRESS, MaxData, WriteBuffer);

	/*
	 * Write the data in the write buffer to the serial Flash a page at a
	 * time, starting from TEST_ADDRESS
	 */
	for (Page = 0; Page < PAGE_COUNT; Page++) {
		FlashWrite(QspiInstancePtr,
			(Page * Flash_Config_Table[FCTIndex].PageSize) + TEST_ADDRESS,
			Flash_Config_Table[FCTIndex].PageSize, WRITE_CMD, WriteBuffer);
	}

	/*
	 * I/O Read - for any flash size
	 */
	FlashRead(QspiInstancePtr, TEST_ADDRESS, MaxData, QUAD_READ_CMD,
				WriteBuffer, ReadBuffer);

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

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
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
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 RealAddr;
	u32 BankSel;
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};
	u8 FlagStatus[2];

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPtr, Address);
	/*
	 * Bank Select
	 */
	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		/*
		 * Calculate bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		SendBankSelect(QspiPtr, WriteBfrPtr, BankSel);
	}

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
	/*
	 * This will ensure a 3B address is transferred even when address
	 * is greater than 128Mb.
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = Command;
	WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

	/*
	 * Send the write command, address, and data to the Flash to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				ByteCount + OVERHEAD_SIZE);

	if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}
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

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}

}

/*****************************************************************************/
/**
*
*
* This function erases the sectors in the  serial Flash connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	int Sector;
	u32 RealAddr;
	u32 LqspiCr;
	u32 NumSect;
	u32 BankSel;
	u8 BankInitFlag = 1;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
			(Flash_Config_Table[FCTIndex]).SectSize) ) {

		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(QspiPtr, WriteBfrPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(QspiPtr, WriteBfrPtr);
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);

			if(Flash_Config_Table[FCTIndex].NumDie == 1) {
				/*
				 * Call Bulk erase
				 */
				BulkErase(QspiPtr, WriteBfrPtr);
			}

			if(Flash_Config_Table[FCTIndex].NumDie > 1) {
				/*
				 * Call Die erase
				 */
				DieErase(QspiPtr, WriteBfrPtr);
			}
		}

		return;
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
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Initial bank selection
		 */
		if((BankInitFlag) &&
				(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Reset initial bank select flag
			 */
			BankInitFlag = 0;
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, WriteBfrPtr, BankSel);
		}
		/*
		 * Check bank and send bank select if new bank
		 */
		if((BankSel != RealAddr/SIXTEENMB) &&
				(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, WriteBfrPtr, BankSel);
		}

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
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)(RealAddr >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)(RealAddr >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
					SEC_ERASE_SIZE);

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
				(FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}
		/*
		 * Wait for the sector erase command to the Flash to be completed
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

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
				(FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}

		Address += Flash_Config_Table[FCTIndex].SectSize;

	}
}

/*****************************************************************************/
/**
*
* This function reads serial Flash ID connected to the SPI interface.
* It then deduces the make and size of the flash and obtains the
* connection mode to point to corresponding parameters in the flash
* configuration table. The flash driver will function based on this and
* it presently supports Micron and Spansion - 128, 256 and 512Mbit and
* Winbond 128Mbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer (which is to be transmitted)
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(XQspiPs *QspiPtr, u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	int Status;
	int StartIndex;

	/*
	 * Read ID in Auto mode.
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = READ_ID;
	WriteBfrPtr[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBfrPtr[ADDRESS_2_OFFSET] = 0x08;
	WriteBfrPtr[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, ReadBfrPtr,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Deduce flash make
	 */
	if(ReadBfrPtr[1] == MICRON_ID_BYTE0) {
		FlashMake = MICRON_ID_BYTE0;
		StartIndex = MICRON_INDEX_START;
	}else if(ReadBfrPtr[1] == SPANSION_ID_BYTE0) {
		FlashMake = SPANSION_ID_BYTE0;
		StartIndex = SPANSION_INDEX_START;
	}else if(ReadBfrPtr[1] == WINBOND_ID_BYTE0) {
		FlashMake = WINBOND_ID_BYTE0;
		StartIndex = WINBOND_INDEX_START;
	} else if(ReadBfrPtr[1] == MACRONIX_ID_BYTE0) {
		FlashMake = MACRONIX_ID_BYTE0;
		StartIndex = MACRONIX_INDEX_START;
	} else if(ReadBfrPtr[0] == ISSI_ID_BYTE0) {
		FlashMake = ISSI_ID_BYTE0;
		StartIndex = ISSI_INDEX_START;
	}


	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)||
			(FlashMake == WINBOND_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_128)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_128_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	/* 256 and 512Mbit supported only for Micron and Spansion, not Winbond */
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)
			|| (FlashMake == MACRONIX_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_256)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_256_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	if((FlashMake == ISSI_ID_BYTE0) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_256)) {
		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_ISSI;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_ISSI;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_256_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	if ((((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)) &&
			(ReadBfrPtr[3] == MICRON_ID_BYTE2_512)) || ((FlashMake ==
			MACRONIX_ID_BYTE0) && (ReadBfrPtr[3] == MACRONIX_ID_BYTE2_512))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_512_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	/*
	 * 1Gbit Single connection supported for Spansion.
	 * The ConnectionMode will indicate stacked as this part has 2 SS
	 * The device ID will indicate 512Mbit.
	 * This configuration is handled as the above 512Mbit stacked configuration
	 */
	/* 1Gbit single, parallel and stacked supported for Micron */
	if(((FlashMake == MICRON_ID_BYTE0) &&
		(ReadBfrPtr[3] == MICRON_ID_BYTE2_1G)) ||
		((FlashMake == MACRONIX_ID_BYTE0) &&
		 (ReadBfrPtr[3] == MACRONIX_ID_BYTE2_1G))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBfrPtr[1], ReadBfrPtr[2],
		   ReadBfrPtr[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function performs an I/O read.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Command is the command used to read data from the flash. Supports
* 			normal, fast, dual and quad read commands.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	none.
*
* @note		None.
*
******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	u32 RealByteCnt;
	u32 BankSel;
	u32 BufferIndex;
	u32 TotalByteCnt;
	u8 ShiftSize;

	/*
	 * Retain the actual byte count
	 */
	TotalByteCnt = ByteCount;

	while(((signed long)(ByteCount)) > 0) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Select bank
		 */
		if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			BankSel = RealAddr/SIXTEENMB;
			SendBankSelect(QspiPtr, WriteBfrPtr, BankSel);
		}

		/*
		 * If data to be read spans beyond the current bank, then
		 * calculate RealByteCnt in current bank. Else
		 * RealByteCnt is the same as ByteCount
		 */
		if((Address & BANKMASK) != ((Address+ByteCount) & BANKMASK)) {
			RealByteCnt = (Address & BANKMASK) + SIXTEENMB - Address;
		}else {
			RealByteCnt = ByteCount;
		}


		/*
		 * Setup the write command with the specified address and data for the
		 * Flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			RealByteCnt += DUMMY_SIZE;
		}
		/*
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr,
				&(ReadBfrPtr[TotalByteCnt - ByteCount]),
				RealByteCnt + OVERHEAD_SIZE);

		/*
		 * To discard the first 5 dummy bytes, shift the data in read buffer
		 */
		if((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
			    (Command == QUAD_READ_CMD)){
			ShiftSize = OVERHEAD_SIZE + DUMMY_SIZE;
		}else{
			ShiftSize =  OVERHEAD_SIZE;
		}

		for(BufferIndex = (TotalByteCnt - ByteCount);
				BufferIndex < (TotalByteCnt - ByteCount) + RealByteCnt;
				BufferIndex++) {
			ReadBfrPtr[BufferIndex] = ReadBfrPtr[BufferIndex + ShiftSize];
		}

		/*
		 * Increase address to next bank
		 */
		Address = (Address & BANKMASK) + SIXTEENMB;
		/*
		 * Decrease byte count by bytes already read.
		 */
		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			ByteCount = ByteCount - (RealByteCnt - DUMMY_SIZE);
		}else {
			ByteCount = ByteCount - RealByteCnt;
		}

	}

}

/*****************************************************************************/
/**
*
* This functions selects the current bank
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	BankSel is the bank to be selected in the flash device(s).
*
* @return	XST_SUCCESS if bank selected, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SendBankSelect(XQspiPs *QspiPtr, u8 *WriteBfrPtr, u32 BankSel)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };

	/*
	 * Bank select commands for Micron and Spansion are different
	 */
	if(FlashMake == MICRON_ID_BYTE0) {
		/*
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		WriteBfrPtr[COMMAND_OFFSET]   = EXTADD_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);

	}
	if(FlashMake == SPANSION_ID_BYTE0) {
		WriteBfrPtr[COMMAND_OFFSET]   = BANK_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);
	}

	/* Winbond can be added here */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void BulkErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	/*
	 * Setup the bulk erase command
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;

	/*
	 * Send the bulk erase command; no receive buffer is specified
	 * since there is nothing to receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BULK_ERASE_SIZE);

	/*
	 * Wait for the erase command to the Flash to be completed
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

/*****************************************************************************/
/**
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void DieErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 DieCnt;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];

	for(DieCnt = 0; DieCnt < Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
		/*
		 * Select bank - the lower of the 2 banks in each die
		 * This is specific to Micron flash
		 */
		SendBankSelect(QspiPtr, WriteBfrPtr, DieCnt*2);

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
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 * The address is the start address of die - MSB bits will be
		 * derived from bank select by the flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_2_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_3_OFFSET] = 0x00;

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				DIE_ERASE_SIZE);

		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlagStatus[1] & 0x80) == 0x80) {
				break;
			}
		}

	}
}

/*****************************************************************************/
/**
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address which is to be accessed (for erase, write or read)
*
* @return	RealAddr is the translated address - for single it is unchanged;
* 			for stacked, the lower flash size is subtracted;
* 			for parallel the address is divided by 2.
*
* @note		None.
*
******************************************************************************/
u32 GetRealAddr(XQspiPs *QspiPtr, u32 Address)
{
	u32 LqspiCr;
	u32 RealAddr;

	switch(QspiPtr->Config.ConnectionMode) {
	case XQSPIPS_CONNECTION_MODE_SINGLE:
		RealAddr = Address;
		break;
	case XQSPIPS_CONNECTION_MODE_STACKED:
		/*
		 * Get the current LQSPI Config reg value
		 */
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);

		/* Select lower or upper Flash based on sector address */
		if(Address & Flash_Config_Table[FCTIndex].FlashDeviceSize) {
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Subtract first flash size when accessing second flash
			 */
			RealAddr = Address &
					(~Flash_Config_Table[FCTIndex].FlashDeviceSize);

		}else{

			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			RealAddr = Address;

		}

		/*
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(QspiPtr);
		break;
	case XQSPIPS_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(RealAddr);

}
