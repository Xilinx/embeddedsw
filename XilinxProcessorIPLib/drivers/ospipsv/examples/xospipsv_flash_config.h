/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xospipsv_flash_config.h
*
*
* This file contains flash configuration table and flash related defines.
* This file should be included in the example files and compiled along with
* the examples (*.c).
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
* 1.3   sk  05/13/20 First release
*       sk  05/18/20 Added support for gigadevice and ISSI parts.
*       sk  06/22/20 Fixed Sector Mask value for Micron and ISSI.
*       sk  02/18/21 Added support for Macronix flash.
* 1.7   sk  06/28/22 Added Block Protection test for Micron flash.
*
*</pre>
*
******************************************************************************/
#ifndef XOSPIPSV_FLASH_CONFIG_H
#define XOSPIPSV_FLASH_CONFIG_H
/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xil_printf.h"
#include "xil_cache.h"

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
#define READ_STATUS_CMD		0x05
#define READ_FLAG_STATUS_CMD	0x70
#define READ_CMD_4B			0x13
#define WRITE_CMD_4B		0x12
#define SEC_ERASE_CMD_4B	0xDC
#define SEC_ERASE_CMD_MX	0x21
#define READ_CMD_OPI_MX		0xEE
#define READ_CMD_OCTAL_IO_4B	0xCC
#define READ_CMD_OCTAL_DDR	0x9D
#define WRITE_CMD_OCTAL_4B	0x84
#define ENTER_4B_ADDR_MODE	0xB7
#define EXIT_4B_ADDR_MODE	0xE9
#define WRITE_CONFIG_REG	0x81
#define READ_CONFIG_REG		0x85
#define WRITE_CONFIG2_REG_MX	0x72
#define READ_CONFIG2_REG_MX		0x71

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000

#define FLASH_PAGE_SIZE_256		256
#define FLASH_SECTOR_SIZE_4KB		0x1000
#define FLASH_SECTOR_SIZE_64KB		0x10000
#define FLASH_SECTOR_SIZE_128KB		0x20000
#define FLASH_DEVICE_SIZE_256M		0x2000000
#define FLASH_DEVICE_SIZE_512M		0x4000000
#define FLASH_DEVICE_SIZE_1G		0x8000000
#define FLASH_DEVICE_SIZE_2G		0x10000000

#define	MICRON_OCTAL_ID_BYTE0		0x2c
#define GIGADEVICE_OCTAL_ID_BYTE0	0xc8
#define ISSI_OCTAL_ID_BYTE0		0x9d
#define MACRONIX_OCTAL_ID_BYTE0		0xc2

#define MICRON_BP_BITS_MASK		0x7C

/**************************** Type Definitions *******************************/

typedef struct{
	u32 jedec_id;	/* JEDEC ID */
	u32 SectSize;		/* Individual sector size or
						 * combined sector size in case of parallel config*/
	u32 NumSect;		/* Total no. of sectors in one/two flash devices */
	u32 PageSize;		/* Individual page size or
				 * combined page size in case of parallel config*/
	u32 NumPage;		/* Total no. of pages in one/two flash devices */
	u32 FlashDeviceSize;	/* This is the size of one flash device
				 * NOT the combination of both devices, if present
				 */
	u32 SectMask;		/* Mask to get sector start address */
	u8 NumDie;		/* No. of die forming a single flash */
	u32 ReadCmd;		/* Read command used to read data from flash */
	u32 WriteCmd;	/* Write command used to write data to flash */
	u32 EraseCmd;	/* Erase Command */
	u8 StatusCmd;	/* Status Command */
	u8 DummyCycles;	/* Number of Dummy cycles for Read operation */
}FlashInfo;

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
FlashInfo Flash_Config_Table[] = {
	/* Micron */
	{0x2c5b1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M,0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16},
	{0x2c5b1b, FLASH_SECTOR_SIZE_128KB, 0x400, FLASH_PAGE_SIZE_256, 0x80000,
		FLASH_DEVICE_SIZE_1G,0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16},
	{0x2c5b1c, FLASH_SECTOR_SIZE_128KB, 0x800, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G,0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16},
	/* GIGADEVICE */
	{0xc86819, FLASH_SECTOR_SIZE_64KB, 0x200, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFF0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16},
	/* ISSI */
	{0x9d5b19, FLASH_SECTOR_SIZE_128KB, 0x100, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16},
	/* MACRONIX */
	{0xc2813a, FLASH_SECTOR_SIZE_4KB, 0x4000, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) |READ_CMD_4B, WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0},
};

/***************** Macros (Inline Functions) Definitions *********************/

static INLINE u32 CalculateFCTIndex(u32 ReadId, u32 *FCTIndex)
{
	u32 Index;

	for (Index = 0; Index < sizeof(Flash_Config_Table)/sizeof(Flash_Config_Table[0]);
				Index++) {
		if (ReadId == Flash_Config_Table[Index].jedec_id) {
			*FCTIndex = Index;
			return XST_SUCCESS;
		}
	}

	return XST_FAILURE;
}

#endif /* XOSPIPSV_FLASH_CONFIG_H */
