/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspipsu_generic_flash_non_blocking_read_example.c
*
*
** This file contains a design example using the QSPIPSU driver (XQspiPsu)
* The example writes to flash in IO mode and reads it back using non-blocking
* APIs in DMA mode.
* This examples runs with GENFIFO Manual start. It runs in polled mode.
* This example illustrates single, parallel and stacked modes.
* Both the flash devices have to be of the same make and size.
* The hardware which this example runs on, must have a serial Flash (Micron
* N25Q) for it to run. In order to test in single,
* parallel or stacked flash configurations the necessary HW must be present
* and QSPI_MODE (also reflected in ConnectionMode in the instance) has
* to be in sync with HW flash configuration being tested.
*
* This example has been tested with the Micron Serial Flash (N25Q512) and
* ISSI Serial Flash parts of IS25WP and IS25LP series flashes in
* single and parallel modes using A53 and R5 processors.
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
* 1.11   akm  02/19/20 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xqspipsu.h"		/* QSPIPSU device driver */
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
#define VOLATILE_WRITE_ENABLE_CMD	0x50
#define QUAD_MODE_ENABLE_BIT	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01
#define ENTER_4B_ADDR_MODE	0xB7
#define EXIT_4B_ADDR_MODE	0xE9
#define EXIT_4B_ADDR_MODE_ISSI	0x29

#define READ_CMD_4B		0x13
#define FAST_READ_CMD_4B	0x0C
#define DUAL_READ_CMD_4B	0x3C
#define QUAD_READ_CMD_4B	0x6C

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
#define ADDRESS_4_OFFSET	4 /* LSB byte of address to read or write
				   * when 4 byte address
				   */
#define DATA_OFFSET		5 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				   * reads
				   */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				   * quad reads
				   */
#define DUMMY_CLOCKS		8 /* Number of dummy bytes for fast, dual and
				   * quad reads
				   */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */
#define BANK_SEL_SIZE		2 /* BRWR or EARWR command + 1 byte bank
				   * value
				   */
#define RD_CFG_SIZE		2 /* 1 byte Configuration register + RD CFG
				   * command
				   */
#define WR_CFG_SIZE		3 /* WRR command + 1 byte each Status and
				   * Config Reg
				   */
#define DIE_ERASE_SIZE	4	/* Die Erase command + Die address */

/*
 * The following constants specify the extra bytes which are sent to the
 * Flash on the QSPIPSu interface, that are not data, but control information
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
#define MICRON_ID_BYTE2_2G	0x22

#define SPANSION_ID_BYTE0	0x01
#define SPANSION_ID_BYTE2_64	0x17
#define SPANSION_ID_BYTE2_128	0x18
#define SPANSION_ID_BYTE2_256	0x19
#define SPANSION_ID_BYTE2_512	0x20

#define WINBOND_ID_BYTE0	0xEF
#define WINBOND_ID_BYTE2_128	0x18

#define MACRONIX_ID_BYTE0	0xC2
#define MACRONIX_ID_BYTE2_1G	0x1B
#define MACRONIX_ID_BYTE2_1GU	0x3B

#define ISSI_ID_BYTE0		0x9D
#define ISSI_ID_BYTE2_08	0x14
#define ISSI_ID_BYTE2_16	0x15
#define ISSI_ID_BYTE2_32	0x16
#define ISSI_ID_BYTE2_64	0x17
#define ISSI_ID_BYTE2_128	0x18
#define ISSI_ID_BYTE2_256	0x19
#define ISSI_ID_BYTE2_512	0x1a

/*
 * The index for Flash config table
 */
/* Spansion*/
#define SPANSION_INDEX_START		0
#define FLASH_CFG_TBL_SINGLE_64_SP	SPANSION_INDEX_START
#define FLASH_CFG_TBL_STACKED_64_SP	(SPANSION_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_64_SP	(SPANSION_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_128_SP	(SPANSION_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_128_SP	(SPANSION_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_128_SP	(SPANSION_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_256_SP	(SPANSION_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_256_SP	(SPANSION_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_256_SP	(SPANSION_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_512_SP	(SPANSION_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_512_SP	(SPANSION_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_512_SP	(SPANSION_INDEX_START + 11)

/* Micron */
#define MICRON_INDEX_START		(FLASH_CFG_TBL_PARALLEL_512_SP + 1)
#define FLASH_CFG_TBL_SINGLE_128_MC	MICRON_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_MC	(MICRON_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_MC	(MICRON_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_MC	(MICRON_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_MC	(MICRON_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_MC	(MICRON_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_MC	(MICRON_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_MC	(MICRON_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_MC	(MICRON_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_1GB_MC	(MICRON_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_1GB_MC	(MICRON_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_1GB_MC	(MICRON_INDEX_START + 11)
#define FLASH_CFG_TBL_SINGLE_2GB_MC	(MICRON_INDEX_START + 12)
#define FLASH_CFG_TBL_STACKED_2GB_MC	(MICRON_INDEX_START + 13)
#define FLASH_CFG_TBL_PARALLEL_2GB_MC	(MICRON_INDEX_START + 14)

/* Winbond */
#define WINBOND_INDEX_START		(FLASH_CFG_TBL_PARALLEL_2GB_MC + 1)
#define FLASH_CFG_TBL_SINGLE_128_WB	WINBOND_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_WB	(WINBOND_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_WB	(WINBOND_INDEX_START + 2)

/* Macronix */
#define MACRONIX_INDEX_START		(FLASH_CFG_TBL_PARALLEL_128_WB + 1)
#define FLASH_CFG_TBL_SINGLE_1G_MX	MACRONIX_INDEX_START
#define FLASH_CFG_TBL_STACKED_1G_MX	(MACRONIX_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_1G_MX	(MACRONIX_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_1GU_MX	(MACRONIX_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_1GU_MX	(MACRONIX_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_1GU_MX	(MACRONIX_INDEX_START + 5)

/* ISSI */
#define ISSI_INDEX_START		(FLASH_CFG_TBL_PARALLEL_1GU_MX + 1)
#define FLASH_CFG_TBL_SINGLE_08_ISSI	ISSI_INDEX_START
#define FLASH_CFG_TBL_STACKED_08_ISSI	(ISSI_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_08_ISSI	(ISSI_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_16_ISSI	(ISSI_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_16_ISSI	(ISSI_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_16_ISSI	(ISSI_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_32_ISSI	(ISSI_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_32_ISSI	(ISSI_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_32_ISSI	(ISSI_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_64_ISSI	(ISSI_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_64_ISSI	(ISSI_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_64_ISSI	(ISSI_INDEX_START + 11)
#define FLASH_CFG_TBL_SINGLE_128_ISSI   (ISSI_INDEX_START + 12)
#define FLASH_CFG_TBL_STACKED_128_ISSI  (ISSI_INDEX_START + 13)
#define FLASH_CFG_TBL_PARALLEL_128_ISSI (ISSI_INDEX_START + 14)
#define FLASH_CFG_TBL_SINGLE_256_ISSI   (ISSI_INDEX_START + 15)
#define FLASH_CFG_TBL_STACKED_256_ISSI  (ISSI_INDEX_START + 16)
#define FLASH_CFG_TBL_PARALLEL_256_ISSI (ISSI_INDEX_START + 17)
#define FLASH_CFG_TBL_SINGLE_512_ISSI   (ISSI_INDEX_START + 18)
#define FLASH_CFG_TBL_STACKED_512_ISSI  (ISSI_INDEX_START + 19)
#define FLASH_CFG_TBL_PARALLEL_512_ISSI (ISSI_INDEX_START + 20)

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID

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


#define UNIQUE_VALUE		0x20

#define ENTER_4B	1
#define EXIT_4B		0


/**************************** Type Definitions *******************************/

typedef struct{
	u32 SectSize;		/* Individual sector size or combined sector
				 * size in case of parallel config
				 */
	u32 NumSect;		/* Total no. of sectors in one/two
				 * flash devices
				 */
	u32 PageSize;		/* Individual page size or
				 * combined page size in case of parallel
				 * config
				 */
	u32 NumPage;		/* Total no. of pages in one/two flash
				 * devices
				 */
	u32 FlashDeviceSize;	/* This is the size of one flash device
				 * NOT the combination of both devices,
				 * if present
				 */
	u8 ManufacturerID;	/* Manufacturer ID - used to identify make */
	u8 DeviceIDMemSize;	/* Byte of device ID indicating the
				 * memory size
				 */
	u32 SectMask;		/* Mask to get sector start address */
	u8 NumDie;		/* No. of die forming a single flash */
} FlashInfo;

u8 ReadCmd;
u8 WriteCmd;
u8 StatusCmd;
u8 SectorEraseCmd;
u8 FSRFlag;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int QspiPsuFlashNonBlockingReadExample(XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuDeviceId);

int FlashReadID(XQspiPsu *QspiPsuPtr);
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
int FlashWrite(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr);
int FlashRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
int MultiDieRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
                 u8 *WriteBfrPtr, u8 *ReadBfrPtr);
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address);
int BulkErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
int DieErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr);
int FlashRegisterRead(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
int FlashRegisterWrite(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn);
int FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr,unsigned int Enable);
int FlashEnableQuadMode(XQspiPsu *QspiPsuPtr);
/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
u8 ReadBfrPtr[3];
FlashInfo Flash_Config_Table[] = {
	/* Spansion */
	{SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE,
		0x8000, 0x800000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_64, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x800000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_64, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS128, BYTES512_PER_PAGE,
		0x8000, 0x800000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_64, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x1000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x1000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS256, BYTES512_PER_PAGE,
		0x10000, 0x1000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_128, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x2000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE,
		0x40000, 0x2000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS512, BYTES512_PER_PAGE,
		0x20000, 0x2000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_256, 0xFFFE0000, 1},
	{SECTOR_SIZE_256K, NUM_OF_SECTORS256, BYTES512_PER_PAGE,
		0x20000, 0x4000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
	{SECTOR_SIZE_256K, NUM_OF_SECTORS512, BYTES512_PER_PAGE,
		0x40000, 0x4000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
	{SECTOR_SIZE_512K, NUM_OF_SECTORS256, BYTES1024_PER_PAGE,
		0x20000, 0x4000000, SPANSION_ID_BYTE0,
		SPANSION_ID_BYTE2_512, 0xFFF80000, 1},
	/* Spansion 1Gbit is handled as 512Mbit stacked */
	/* Micron */
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x1000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x1000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS256, BYTES512_PER_PAGE,
		0x10000, 0x1000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_128, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x2000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE,
		0x40000, 0x2000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS512, BYTES512_PER_PAGE,
		0x20000, 0x2000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_256, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE,
		0x40000, 0x4000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE,
		0x80000, 0x4000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS1024, BYTES512_PER_PAGE,
		0x40000, 0x4000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_512, 0xFFFE0000, 2},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE,
		0x80000, 0x8000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE,
		0x100000, 0x8000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS2048, BYTES512_PER_PAGE,
		0x80000, 0x8000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_1G, 0xFFFE0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE,
		0x100000, 0x10000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_2G, 0xFFFF0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS8192, BYTES256_PER_PAGE,
		0x200000, 0x10000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_2G, 0xFFFF0000, 4},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS4096, BYTES512_PER_PAGE,
		0x100000, 0x10000000, MICRON_ID_BYTE0,
		MICRON_ID_BYTE2_2G, 0xFFFE0000, 4},
	/* Winbond */
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x1000000, WINBOND_ID_BYTE0,
		WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x1000000, WINBOND_ID_BYTE0,
		WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS256, BYTES512_PER_PAGE,
		0x10000, 0x1000000, WINBOND_ID_BYTE0,
		WINBOND_ID_BYTE2_128, 0xFFFE0000, 1},
	/* Macronix */
	{SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE,
		0x80000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE,
		0x100000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 4},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS2048, BYTES512_PER_PAGE,
		0x80000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1G, 0xFFFE0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE,
		0x80000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1GU, 0xFFFF0000, 4},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE,
		0x100000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1GU, 0xFFFF0000, 4},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS2048, BYTES512_PER_PAGE,
		0x80000, 0x8000000, MACRONIX_ID_BYTE0,
		MACRONIX_ID_BYTE2_1GU, 0xFFFE0000, 4},
	/* ISSI */
	{SECTOR_SIZE_64K, NUM_OF_SECTORS16, BYTES256_PER_PAGE,
		0x1000, 0x100000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_08, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS32, BYTES256_PER_PAGE,
		0x2000, 0x100000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_08, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS16, BYTES512_PER_PAGE,
		0x1000, 0x100000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_08, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS32, BYTES256_PER_PAGE,
		0x2000, 0x200000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_16, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS64, BYTES256_PER_PAGE,
		0x4000, 0x200000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_16, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS32, BYTES512_PER_PAGE,
		0x2000, 0x200000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_16, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS64, BYTES256_PER_PAGE,
		0x4000, 0x400000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_32, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE,
		0x8000, 0x400000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_32, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS64, BYTES512_PER_PAGE,
		0x4000, 0x400000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_32, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE,
		0x8000, 0x800000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_64, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x800000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_64, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS128, BYTES512_PER_PAGE,
		0x8000, 0x800000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_64, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE,
		0x10000, 0x1000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x1000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_128, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS256, BYTES512_PER_PAGE,
		0x10000, 0x1000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_128, 0xFFFE0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE,
		0x20000, 0x2000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE,
		0x40000, 0x2000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS512, BYTES512_PER_PAGE,
		0x20000, 0x2000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_256, 0xFFFF0000, 1},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE,
		0x40000, 0x4000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_512, 0xFFFF0000, 2},
	{SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE,
		0x80000, 0x4000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_512, 0xFFFF0000, 2},
	{SECTOR_SIZE_128K, NUM_OF_SECTORS1024, BYTES512_PER_PAGE,
		0x40000, 0x4000000, ISSI_ID_BYTE0,
		ISSI_ID_BYTE2_512, 0xFFFE0000, 2}
};

u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */


/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPsu QspiPsuInstance;

static XQspiPsu_Msg FlashMsg[5];

/*
 * The following variable allows a test value to be added to the values that
 * are written to the Flash such that unique values can be generated to
 * guarantee the writes to the Flash were successful
 */
int Test = 7;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 * The buffer size accounts for maximum page size and maximum banks -
 * for each bank separate read will be performed leading to that many
 * (overhead+dummy) bytes
 */
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8];
#else
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8] __attribute__ ((aligned(64)));
#endif
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

	xil_printf("QSPIPSU Generic Flash Non Blocking Read Example Test \r\n");

	/*
	 * Run the QspiPsu non blocking read example.
	 */
	Status = QspiPsuFlashNonBlockingReadExample(&QspiPsuInstance, QSPIPSU_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("QSPIPSU Generic Flash Non Blocking Read Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran QSPIPSU Generic Flash Non Blocking Read Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the XQspiPsu
 * device driver in single, parallel and stacked modes using
 * flash devices greater than or equal to 128Mb.
 * This function reads data in DMA mode.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int QspiPsuFlashNonBlockingReadExample(XQspiPsu *QspiPsuInstancePtr, u16 QspiPsuDeviceId)
{
	int Status;
	u8 UniqueValue;
	int Count;
	int Page;
	XQspiPsu_Config *QspiPsuConfig;
	int ReadBfrSize;

	ReadBfrSize = (PAGE_COUNT * MAX_PAGE_SIZE) +
			(DATA_OFFSET + DUMMY_SIZE)*8;

	/*
	 * Initialize the QSPIPSU driver so that it's ready to use
	 */
	QspiPsuConfig = XQspiPsu_LookupConfig(QspiPsuDeviceId);
	if (QspiPsuConfig == NULL) {
		return XST_FAILURE;
	}

	/* To test, change connection mode here if not obtained from HDF */

	Status = XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiPsuConfig,
					QspiPsuConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set Manual Start
	 */
	XQspiPsu_SetOptions(QspiPsuInstancePtr, XQSPIPSU_MANUAL_START_OPTION);

	/*
	 * Set the prescaler for QSPIPSU clock
	 */
	XQspiPsu_SetClkPrescaler(QspiPsuInstancePtr, XQSPIPSU_CLK_PRESCALE_8);

	XQspiPsu_SelectFlash(QspiPsuInstancePtr,
		XQSPIPSU_SELECT_FLASH_CS_LOWER,
		XQSPIPSU_SELECT_FLASH_BUS_LOWER);

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

	xil_printf("Flash connection mode : %d\n\r",
			QspiPsuConfig->ConnectionMode);
	xil_printf("where 0 - Single; 1 - Stacked; 2 - Parallel\n\r");
	xil_printf("FCTIndex: %d\n\r", FCTIndex);

	/*
	 * Initialize MaxData according to page size.
	 */
	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);

	/*
	 * Some flash needs to enable Quad mode before using
	 * quad commands.
	 */
	Status = FlashEnableQuadMode(QspiPsuInstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Address size and read command selection
	 * Micron flash on REMUS doesn't support this 4B write/erase cmd
	 */
	ReadCmd = QUAD_READ_CMD;
	WriteCmd = WRITE_CMD;
	SectorEraseCmd = SEC_ERASE_CMD;

	/* Status cmd - SR or FSR selection */
	if ((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FSRFlag = 0;
	}

	xil_printf("ReadCmd: 0x%x, WriteCmd: 0x%x,"
		   " StatusCmd: 0x%x, FSRFlag: %d\n\r",
		ReadCmd, WriteCmd, StatusCmd, FSRFlag);

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(QspiPsuInstancePtr, ENTER_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
			Count < Flash_Config_Table[FCTIndex].PageSize;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue + Test);
	}

	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}
	Status = FlashErase(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, CmdBfr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Page = 0; Page < PAGE_COUNT; Page++) {
		Status = FlashWrite(QspiPsuInstancePtr,
				(Page * Flash_Config_Table[FCTIndex].PageSize) +
				TEST_ADDRESS,
				Flash_Config_Table[FCTIndex].PageSize,
				WriteCmd, WriteBuffer);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	Status = FlashRead(QspiPsuInstancePtr, TEST_ADDRESS, MaxData, ReadCmd,
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

	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(QspiPsuInstancePtr, EXIT_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
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
 * @note	None.
 *
 *****************************************************************************/
int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status;
	int StartIndex;

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
	FlashMsg[1].RxBfrPtr = ReadBfrPtr;
	FlashMsg[1].ByteCount = 3;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBfrPtr[0], ReadBfrPtr[1],
		   ReadBfrPtr[2]);

	/* In case of dual, read both and ensure they are same make/size */

	/*
	 * Deduce flash make
	 */
	if (ReadBfrPtr[0] == MICRON_ID_BYTE0) {
		FlashMake = MICRON_ID_BYTE0;
		StartIndex = MICRON_INDEX_START;
	} else if (ReadBfrPtr[0] == SPANSION_ID_BYTE0) {
		FlashMake = SPANSION_ID_BYTE0;
		StartIndex = SPANSION_INDEX_START;
	} else if (ReadBfrPtr[0] == WINBOND_ID_BYTE0) {
		FlashMake = WINBOND_ID_BYTE0;
		StartIndex = WINBOND_INDEX_START;
	} else if (ReadBfrPtr[0] == MACRONIX_ID_BYTE0) {
		FlashMake = MACRONIX_ID_BYTE0;
		StartIndex = MACRONIX_INDEX_START;
	} else if (ReadBfrPtr[0] == ISSI_ID_BYTE0) {
		FlashMake = ISSI_ID_BYTE0;
		StartIndex = ISSI_INDEX_START;
	}



	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if((FlashMake == SPANSION_ID_BYTE0) ||
			(ReadBfrPtr[2] == SPANSION_ID_BYTE2_64)) {
		switch(QspiPsuPtr->Config.ConnectionMode)
		{
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_64_SP + StartIndex;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_64_SP + StartIndex;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_64_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}

	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)||
			(FlashMake == WINBOND_ID_BYTE0)) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_128)) {

		switch (QspiPsuPtr->Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_SP;
			} else if (FlashMake == MICRON_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_MC;
			} else {
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_WB;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_SP;
			} else if (FlashMake == MICRON_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_MC;
			} else {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_WB;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_STACKED_128_SP;
			} else if (FlashMake == MICRON_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_STACKED_128_MC;
			} else {
				FCTIndex = FLASH_CFG_TBL_STACKED_128_WB;
			}
			break;
		default:
			FCTIndex = 0;
			break;
		}
	}
	/* 256 and 512Mbit supported only for Micron and Spansion, not Winbond */
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_256)) {
		switch (QspiPsuPtr->Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_MC;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_MC;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_STACKED_256_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_STACKED_256_MC;
			}
			break;
		default:
			FCTIndex = 0;
			break;
		}
	}

	if (FlashMake == ISSI_ID_BYTE0) {
		if (ReadBfrPtr[2] == ISSI_ID_BYTE2_08) {
			switch (QspiPsuPtr->Config.ConnectionMode)
			{
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_08_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_08_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_08_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		} else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_16) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_16_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_16_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_16_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		} else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_32) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_32_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_32_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_32_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		} else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_64) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_64_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_64_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_64_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		}else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_128) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_128_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}

		}else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_256) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_256_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		}else if (ReadBfrPtr[2] == ISSI_ID_BYTE2_512) {
			switch (QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_512_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_512_ISSI;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_512_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
			}
		}
	}
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_512)) {
		switch (QspiPsuPtr->Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_SINGLE_512_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_SINGLE_512_MC;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_512_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_PARALLEL_512_MC;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			if (FlashMake == SPANSION_ID_BYTE0) {
				FCTIndex = FLASH_CFG_TBL_STACKED_512_SP;
			} else {
				FCTIndex = FLASH_CFG_TBL_STACKED_512_MC;
			}
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
	if((FlashMake == MICRON_ID_BYTE0) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_1G)) {

		switch(QspiPsuPtr->Config.ConnectionMode)
		{
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	/* 2Gbit single, parallel and stacked supported for Micron */
	if(((FlashMake == MICRON_ID_BYTE0) &&
			(ReadBfrPtr[2] == MICRON_ID_BYTE2_2G))) {

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_2GB_MC;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_2GB_MC;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_2GB_MC;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}

	/* 1Gbit single, parallel and stacked supported for Macronix */
	if(((FlashMake == MACRONIX_ID_BYTE0) &&
			((ReadBfrPtr[2] == MACRONIX_ID_BYTE2_1G) ||
					(ReadBfrPtr[2] == MACRONIX_ID_BYTE2_1GU)))) {

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
				if (ReadBfrPtr[2] == MACRONIX_ID_BYTE2_1GU)
					FCTIndex = FLASH_CFG_TBL_SINGLE_1GU_MX;
				else
					FCTIndex = FLASH_CFG_TBL_SINGLE_1G_MX;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				if (ReadBfrPtr[2] == MACRONIX_ID_BYTE2_1GU)
					FCTIndex = FLASH_CFG_TBL_PARALLEL_1GU_MX;
				else
					FCTIndex = FLASH_CFG_TBL_PARALLEL_1G_MX;
				break;
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				if (ReadBfrPtr[2] == MACRONIX_ID_BYTE2_1GU)
					FCTIndex = FLASH_CFG_TBL_STACKED_1GU_MX;
				else
					FCTIndex = FLASH_CFG_TBL_STACKED_1G_MX;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
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
 * @note	None.
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
	u32 CmdByteCount;
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

	WriteCmd[COMMAND_OFFSET]   = Command;

	/* To be used only if 4B address program cmd is supported by flash */
	if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		WriteCmd[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF000000) >> 24);
		WriteCmd[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF0000) >> 16);
		WriteCmd[ADDRESS_3_OFFSET] =
				(u8)((RealAddr & 0xFF00) >> 8);
		WriteCmd[ADDRESS_4_OFFSET] =
				(u8)(RealAddr & 0xFF);
		CmdByteCount = 5;
	} else {
		WriteCmd[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF0000) >> 16);
		WriteCmd[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF00) >> 8);
		WriteCmd[ADDRESS_3_OFFSET] =
				(u8)(RealAddr & 0xFF);
		CmdByteCount = 4;
	}

	FlashMsg[0].TxBfrPtr = WriteCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = CmdByteCount;
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
		ReadStatusCmd = StatusCmd;
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
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			} else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0) {
				break;
			}
		} else {
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}
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
 * @note	None.
 *
 ******************************************************************************/
int FlashErase(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Sector;
	u32 RealAddr;
	u32 NumSect;
	int Status;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
			(Flash_Config_Table[FCTIndex]).SectSize)) {

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_STACKED) {
			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		}

		if (Flash_Config_Table[FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(QspiPsuPtr, WriteBfrPtr);
		}

		if (Flash_Config_Table[FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(QspiPsuPtr, WriteBfrPtr);
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_STACKED) {

			XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_UPPER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);

			if (Flash_Config_Table[FCTIndex].NumDie == 1) {
				/*
				 * Call Bulk erase
				 */
				BulkErase(QspiPsuPtr, WriteBfrPtr);
			}

			if (Flash_Config_Table[FCTIndex].NumDie > 1) {
				/*
				 * Call Die erase
				 */
				DieErase(QspiPsuPtr, WriteBfrPtr);
			}
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

	if (((Address + ByteCount) & Flash_Config_Table[FCTIndex].SectMask) ==
		((Address + (NumSect * Flash_Config_Table[FCTIndex].SectSize)) &
		Flash_Config_Table[FCTIndex].SectMask)) {
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
		 * written to, this needs to be sent as a separate
		 * transfer before the write
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

		WriteBfrPtr[COMMAND_OFFSET]   = SectorEraseCmd;

		/*
		 * To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 4;
		}

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
			ReadStatusCmd = StatusCmd;
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
				if (FSRFlag) {
					FlashStatus[1] &= FlashStatus[0];
				} else {
					FlashStatus[1] |= FlashStatus[0];
				}
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
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
	u32 RealAddr;
	u32 DiscardByteCnt;
	u32 FlashMsgCnt;
	int Status;

	/* Check die boundary conditions if required for any flash */
	if (Flash_Config_Table[FCTIndex].NumDie > 1) {
		Status = MultiDieRead(QspiPsuPtr, Address, ByteCount, Command,
				      WriteBfrPtr, ReadBfrPtr);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	} else {
		/* For Dual Stacked, split and read for boundary crossing */
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsgCnt = 1;

		/* It is recommended to have a separate entry for dummy */
		if (Command == FAST_READ_CMD || Command == DUAL_READ_CMD ||
		    Command == QUAD_READ_CMD || Command == FAST_READ_CMD_4B ||
		    Command == DUAL_READ_CMD_4B ||
		    Command == QUAD_READ_CMD_4B) {
			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (Command == FAST_READ_CMD ||
			    Command == FAST_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_SPI;

			if (Command == DUAL_READ_CMD ||
			    Command == DUAL_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_DUALSPI;

			if (Command == QUAD_READ_CMD ||
			    Command == QUAD_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_QUADSPI;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0;

			FlashMsgCnt++;
		}

		if (Command == FAST_READ_CMD ||
		    Command == FAST_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_SPI;

		if (Command == DUAL_READ_CMD ||
		    Command == DUAL_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_DUALSPI;

		if (Command == QUAD_READ_CMD ||
		    Command == QUAD_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_QUADSPI;

		FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
		FlashMsg[FlashMsgCnt].RxBfrPtr = ReadBfrPtr;
		FlashMsg[FlashMsgCnt].ByteCount = ByteCount;
		FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL)
			FlashMsg[FlashMsgCnt].Flags |=
					XQSPIPSU_MSG_FLAG_STRIPE;

		Status = XQspiPsu_StartDmaTransfer(QspiPsuPtr, FlashMsg,
						 FlashMsgCnt + 1);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		do
			Status = XQspiPsu_CheckDmaDone(QspiPsuPtr);
		while(Status != XST_SUCCESS);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This functions performs a read operation for multi die flash devices.
 *
 * @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Command is the command used to read data from the flash.
 *			Supports normal, fast, dual and quad read commands.
 * @param	Pointer to the write buffer which contains data to be
 *			transmitted
 * @param	Pointer to the read buffer to which valid received data
 *			should be written.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int MultiDieRead(XQspiPsu *QspiPsuPtr, u32 Address, u32 ByteCount, u8 Command,
		 u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	u32 DiscardByteCnt;
	u32 FlashMsgCnt;
	int Status;
	u32 cur_bank = 0;
	u32 nxt_bank = 0;
	u32 bank_size;
	u32 remain_len = ByteCount;
	u32 data_len;
	u32 transfer_len;
	u8 *ReadBuffer = ReadBfrPtr;

	/*
	 * Some flash devices like N25Q512 have multiple dies
	 * in it. Read operation in these devices is bounded
	 * by its die segment. In a continuous read, across
	 * multiple dies, when the last byte of the selected
	 * die segment is read, the next byte read is the
	 * first byte of the same die segment. This is Die
	 * cross over issue. So to handle this issue, split
	 * a read transaction, that spans across multiple
	 * banks, into one read per bank. Bank size is 16MB
	 * for single and dual stacked mode and 32MB for dual
	 * parallel mode.
	 */
	if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL)

		bank_size = SIXTEENMB << 1;

	else if (QspiPsuPtr->Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_SINGLE)

		bank_size = SIXTEENMB;

	while (remain_len) {
		cur_bank = Address / bank_size;
		nxt_bank = (Address + remain_len) / bank_size;

		if (cur_bank != nxt_bank) {
			transfer_len = (bank_size * (cur_bank  + 1)) - Address;
			if (remain_len < transfer_len)
				data_len = remain_len;
			else
				data_len = transfer_len;
		} else {
			data_len = remain_len;
		}
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPsuPtr, Address);

		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			DiscardByteCnt = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsgCnt = 1;

		/* It is recommended to have a separate entry for dummy */
		if (Command == FAST_READ_CMD || Command == DUAL_READ_CMD ||
		    Command == QUAD_READ_CMD || Command == FAST_READ_CMD_4B ||
		    Command == DUAL_READ_CMD_4B ||
		    Command == QUAD_READ_CMD_4B) {
			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (Command == FAST_READ_CMD ||
			    Command == FAST_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_SPI;

			if (Command == DUAL_READ_CMD ||
			    Command == DUAL_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_DUALSPI;

			if (Command == QUAD_READ_CMD ||
			    Command == QUAD_READ_CMD_4B)
				FlashMsg[1].BusWidth =
						XQSPIPSU_SELECT_MODE_QUADSPI;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0;

			FlashMsgCnt++;
		}

		if (Command == FAST_READ_CMD ||
		    Command == FAST_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_SPI;

		if (Command == DUAL_READ_CMD ||
		    Command == DUAL_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_DUALSPI;

		if (Command == QUAD_READ_CMD ||
		    Command == QUAD_READ_CMD_4B)
			FlashMsg[FlashMsgCnt].BusWidth =
					XQSPIPSU_SELECT_MODE_QUADSPI;

		FlashMsg[FlashMsgCnt].TxBfrPtr = NULL;
		FlashMsg[FlashMsgCnt].RxBfrPtr = ReadBuffer;
		FlashMsg[FlashMsgCnt].ByteCount = data_len;
		FlashMsg[FlashMsgCnt].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL)
			FlashMsg[FlashMsgCnt].Flags |=
					XQSPIPSU_MSG_FLAG_STRIPE;

		Status = XQspiPsu_StartDmaTransfer(QspiPsuPtr, FlashMsg,
						 FlashMsgCnt + 1);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		do
			Status = XQspiPsu_CheckDmaDone(QspiPsuPtr);
		while(Status != XST_SUCCESS);

		ReadBuffer += data_len;
		Address += data_len;
		remain_len -= data_len;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	ByteCount is the number of bytes to write.
 * @param	Command is specific register write command.
 * @param	WriteBfrPtr is the pointer to value to be written.
 * @param	WrEn is a flag to mention if WREN has to be sent before write.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashRegisterWrite(XQspiPsu *QspiPsuPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;

	if (WrEn) {
		WriteEnableCmd = WRITE_ENABLE_CMD;

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
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
	}

	WriteBfrPtr[COMMAND_OFFSET]   = Command;
	/*
	 * Value(s) is(are) expected to be written to the
	 * write buffer by calling API ByteCount is the count
	 * of the value(s) excluding the command.
	 */

	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = ByteCount + 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the register write command to the Flash to be completed.
	 */
	while (1) {
		ReadStatusCmd = StatusCmd;
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

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0) {
				break;
			}
		} else {
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}
	}

	return 0;
}

/*****************************************************************************/
/**
 *
 * This API can be used to write to a flash register.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	ByteCount is the number of bytes to write.
 * @param	Command is specific register write command.
 * @param	WriteBfrPtr is the pointer to value to be written.
 * @param	WrEn is a flag to mention if WREN has to be sent before write.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	This API can only be used for one flash at a time.
 *
 ******************************************************************************/
int FlashRegisterRead(XQspiPsu *QspiPsuPtr, u32 ByteCount,
		u8 Command, u8 *ReadBfrPtr)
{
	u8 WriteCmd;
	int Status;

	WriteCmd = Command;
	FlashMsg[0].TxBfrPtr = &WriteCmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadBfrPtr;
	//FlashMsg[1].ByteCount = ByteCount;
	/* This is for DMA reasons; to be changed shortly */
	FlashMsg[1].ByteCount = 4;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
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
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	WritBfrPtr is the pointer to command+address to be sent
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int BulkErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;

	WriteEnableCmd = WRITE_ENABLE_CMD;
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

	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;
	FlashMsg[0].TxBfrPtr = WriteBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		ReadStatusCmd = StatusCmd;
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
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			} else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0) {
				break;
			}
		} else {
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}
	}

	return 0;
}

/*****************************************************************************/
/**
 *
 * This functions performs a die erase operation on all the die in
 * the flash device. This function uses the die erase command for
 * Micron 512Mbit and 1Gbit
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	WritBfrPtr is the pointer to command+address to be sent
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note	None.
 *
 ******************************************************************************/
int DieErase(XQspiPsu *QspiPsuPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd;
	u8 DieCnt;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	int Status;
	u32 DieSize = 0;
	u32 Address;
	u32 RealAddr;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	DieSize = ((Flash_Config_Table[FCTIndex]).NumSect *(Flash_Config_Table[FCTIndex]).SectSize) / Flash_Config_Table[FCTIndex].NumDie;
	for (DieCnt = 0;
		DieCnt < Flash_Config_Table[FCTIndex].NumDie;
		DieCnt++) {
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

		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		Address = DieSize * DieCnt;
		RealAddr = GetRealAddr(QspiPsuPtr, Address);
		/*
		 * To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		if (Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF000000) >> 24);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_4_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 5;
		} else {
			WriteBfrPtr[ADDRESS_1_OFFSET] =
					(u8)((RealAddr & 0xFF0000) >> 16);
			WriteBfrPtr[ADDRESS_2_OFFSET] =
					(u8)((RealAddr & 0xFF00) >> 8);
			WriteBfrPtr[ADDRESS_3_OFFSET] =
					(u8)(RealAddr & 0xFF);
			FlashMsg[0].ByteCount = 4;
		}

		FlashMsg[0].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait for the write command to the Flash to be completed,
		 * it takes some time for the data to be written
		 */
		while (1) {
			ReadStatusCmd = StatusCmd;
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
				if (FSRFlag) {
					FlashStatus[1] &= FlashStatus[0];
				} else {
					FlashStatus[1] |= FlashStatus[0];
				}
			}

			if (FSRFlag) {
				if ((FlashStatus[1] & 0x80) != 0) {
					break;
				}
			} else {
				if ((FlashStatus[1] & 0x01) == 0) {
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
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *		for stacked, the lower flash size is subtracted;
 *		for parallel the address is divided by 2.
 *
 * @note	In addition to get the actual address to work on flash this
 *		function also selects the CS and BUS based on the configuration
 *		detected.
 *
 ******************************************************************************/
u32 GetRealAddr(XQspiPsu *QspiPsuPtr, u32 Address)
{
	u32 RealAddr;

	switch (QspiPsuPtr->Config.ConnectionMode) {
	case XQSPIPSU_CONNECTION_MODE_SINGLE:
		XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
		RealAddr = Address;
		break;
	case XQSPIPSU_CONNECTION_MODE_STACKED:
		/* Select lower or upper Flash based on sector address */
		if (Address & Flash_Config_Table[FCTIndex].FlashDeviceSize) {

			XQspiPsu_SelectFlash(QspiPsuPtr,
					XQSPIPSU_SELECT_FLASH_CS_UPPER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			/*
			 * Subtract first flash size when accessing second flash
			 */
			RealAddr = Address &
				(~Flash_Config_Table[FCTIndex].FlashDeviceSize);
		} else {
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPsu_SelectFlash(QspiPsuPtr,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);

			RealAddr = Address;

		}
		break;
	case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		XQspiPsu_SelectFlash(QspiPsuPtr,
				XQSPIPSU_SELECT_FLASH_CS_BOTH,
				XQSPIPSU_SELECT_FLASH_BUS_BOTH);
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(RealAddr);
}

/*****************************************************************************/
/**
 * @brief
 * This API enters the flash device into 4 bytes addressing mode.
 * As per the Micron and ISSI spec, before issuing the command to enter
 * into 4 byte addr mode, a write enable command is issued.
 * For Macronix and Winbond flash parts write
 * enable is not required.
 *
 * @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 *
 ******************************************************************************/
int FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr, unsigned int Enable)
{
	int Status;
	u8 WriteEnableCmd;
	u8 Cmd;
	u8 WriteDisableCmd;
	u8 ReadStatusCmd;
	u8 WriteBuffer[2] = {0};
	u8 FlashStatus[2] = {0};

	if (Enable) {
		Cmd = ENTER_4B_ADDR_MODE;
	} else {
		if (FlashMake == ISSI_ID_BYTE0)
			Cmd = EXIT_4B_ADDR_MODE_ISSI;
		else
			Cmd = EXIT_4B_ADDR_MODE;
	}

	switch (FlashMake) {
	case ISSI_ID_BYTE0:
	case MICRON_ID_BYTE0:
		WriteEnableCmd = WRITE_ENABLE_CMD;
		GetRealAddr(QspiPsuPtr, TEST_ADDRESS);
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to, this
		 * needs to be sent as a separate transfer before
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
		break;

	case SPANSION_ID_BYTE0:

		if (Enable) {
			WriteBuffer[0] = BANK_REG_WR;
			WriteBuffer[1] = 1 << 7;
		} else {
			WriteBuffer[0] = BANK_REG_WR;
			WriteBuffer[1] = 0 << 7;
		}

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[0].ByteCount = 2;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		return Status;

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write enable command is not required.
		 */
		break;
	}

	GetRealAddr(QspiPsuPtr, TEST_ADDRESS);

	FlashMsg[0].TxBfrPtr = &Cmd;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (1) {
		ReadStatusCmd = StatusCmd;

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
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			} else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}

		if (FSRFlag) {
			if ((FlashStatus[1] & 0x80) != 0) {
				break;
			}
		} else {
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}
	}

	switch (FlashMake) {
	case ISSI_ID_BYTE0:
	case MICRON_ID_BYTE0:
		WriteDisableCmd = WRITE_DISABLE_CMD;
		GetRealAddr(QspiPsuPtr, TEST_ADDRESS);
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to,
		 * this needs to be sent as a separate
		 * transfer before
		 * the write
		 */
		FlashMsg[0].TxBfrPtr = &WriteDisableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write disable command is not required.
		 */
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This API enables Quad mode for the flash parts which require to enable quad
 * mode before using Quad commands.
 * For S25FL-L series flash parts this is required as the default configuration
 * is x1/x2 mode.
 *
 * @param	QspiPtr is a pointer to the QSPIPSU driver component to use.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 *
 ******************************************************************************/
int FlashEnableQuadMode(XQspiPsu *QspiPsuPtr)
{
	int Status;
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2];
	u8 StatusRegVal;
	u8 WriteBuffer[3] = {0};

	switch (FlashMake) {
	case SPANSION_ID_BYTE0:
		if (FCTIndex <= 2) {
			TxBfrPtr = READ_CONFIG_CMD;
			FlashMsg[0].TxBfrPtr = &TxBfrPtr;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].ByteCount = 1;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = &WriteBuffer[2];
			FlashMsg[1].ByteCount = 1;
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
					FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			WriteEnableCmd = VOLATILE_WRITE_ENABLE_CMD;
			/*
			 * Send the write enable command to the
			 * Flash so that it can be written to, this
			 * needs to be sent as a separate transfer before
			 * the write
			 */
			FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].ByteCount = 1;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
					FlashMsg, 1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			GetRealAddr(QspiPsuPtr, TEST_ADDRESS);

			WriteBuffer[0] = WRITE_CONFIG_CMD;
			WriteBuffer[1] |= 0;
			WriteBuffer[2] |= 1 << 1;

			FlashMsg[0].TxBfrPtr = WriteBuffer;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
			FlashMsg[0].ByteCount = 3;

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
					FlashMsg, 1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			TxBfrPtr = READ_CONFIG_CMD;
			FlashMsg[0].TxBfrPtr = &TxBfrPtr;
			FlashMsg[0].RxBfrPtr = NULL;
			FlashMsg[0].ByteCount = 1;
			FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = ReadBfrPtr;
			FlashMsg[1].ByteCount = 1;
			FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

			Status = XQspiPsu_PolledTransfer(QspiPsuPtr,
					FlashMsg, 2);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (ReadBfrPtr[0] & 0x02) {
				Status = XST_SUCCESS;
			} else {
				Status = XST_FAILURE;
			}
		}
		break;
	case ISSI_ID_BYTE0:
		/*
		 * Read Status register
		 */
		ReadStatusCmd = READ_STATUS_CMD;
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
			if (FSRFlag) {
				FlashStatus[1] &= FlashStatus[0];
			}else {
				FlashStatus[1] |= FlashStatus[0];
			}
		}
		/*
		 * Set Quad Enable Bit in the buffer
		 */
		StatusRegVal = FlashStatus[1];
		StatusRegVal |= 0x1 << QUAD_MODE_ENABLE_BIT;

		/*
		 * Write Enable
		 */
		WriteEnableCmd = WRITE_ENABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Write Status register
		 */
		WriteBuffer[COMMAND_OFFSET] = WRITE_STATUS_CMD;
		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = &StatusRegVal;
		FlashMsg[1].RxBfrPtr = NULL;
		FlashMsg[1].ByteCount = 1;
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
		 * Write Disable
		 */
		WriteEnableCmd = WRITE_DISABLE_CMD;
		FlashMsg[0].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	default:
		/*
		 * Currently only S25FL-L series requires the
		 * Quad enable bit to be set to 1.
		 */
		Status = XST_SUCCESS;
		break;
	}

	return Status;
}
