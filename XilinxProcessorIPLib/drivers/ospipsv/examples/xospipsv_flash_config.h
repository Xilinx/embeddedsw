/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef XOSPIPSV_FLASH_CONFIG_H
#define XOSPIPSV_FLASH_CONFIG_H

/******************************************************************************/
/**
*
* @file xospipsv_flash_config.h
* @brief Contains the flash configuration table and flash related defines.
*
* The file should be included in the example files and compiled along with
* the examples (*.c).
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
* 1.9   sb  06/06/23 Added support for system device-tree flow.
* 1.10  sb  11/21/23 Added support for Macronix flash mx66um2g45g.
* 1.10  sb  01/04/24 Added support for ISSI flash is25lx512m.
* 1.10  sb  02/09/24 Add support for Infineon flash part S28HS02G.
* 1.11  sb  05/02/24 Add support for Macronix flash part mx66uw2g345gxrix0.
* 1.15  tb  09/30/26 Align Doxygen comments with AMD documentation standards
*
*</pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"	/* SDK generated parameters */
#endif
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xil_printf.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */

#define WRITE_STATUS_CMD	0x01	/**< Write status register command. */
#define WRITE_DISABLE_CMD	0x04	/**< Write disable command. */
#define WRITE_ENABLE_CMD	0x06	/**< Write enable command. */
#define BULK_ERASE_CMD		0xC7	/**< Bulk erase command. */
#define DIE_ERASE_CMD		0xC4	/**< Die erase command. */
#define READ_ID			0x9F	/**< Read ID command. */
#define READ_CONFIG_CMD		0x35	/**< Read config register command. */
#define WRITE_CONFIG_CMD	0x01	/**< Write config register command. */
#define READ_STATUS_CMD		0x05	/**< Read status register command. */
#define READ_FLAG_STATUS_CMD	0x70	/**< Read flag status command. */
#define READ_CMD_4B			0x13	/**< 4-byte read command. */
#define WRITE_CMD_4B		0x12	/**< 4-byte write command. */
#define SEC_ERASE_CMD_4B	0xDC	/**< 4-byte sector erase command. */
#define SEC_ERASE_CMD_MX	0x21	/**< Macronix sector erase command. */
#define READ_CMD_OPI_MX		0xEE	/**< Macronix OPI read command. */
#define READ_CMD_OCTAL_IO_4B	0xCC	/**< 4-byte octal IO read command. */
#define READ_CMD_OCTAL_DDR	0x9D	/**< Octal DDR read command. */
#define WRITE_CMD_OCTAL_4B	0x84	/**< 4-byte octal write command. */
#define ENTER_4B_ADDR_MODE	0xB7	/**< Enter 4-byte address mode. */
#define EXIT_4B_ADDR_MODE	0xE9	/**< Exit 4-byte address mode. */
#define WRITE_CONFIG_REG	0x81	/**< Write config register command. */
#define READ_CONFIG_REG		0x85	/**< Read config register command. */
#define WRITE_CONFIG2_REG_MX	0x72	/**< Macronix write CR2 command. */
/** Macronix read configuration register 2 command. */
#define READ_CONFIG2_REG_MX		0x71
#define WRITE_CONFIG_REG_SPN	0x71	/**< Spansion write config command. */
#define READ_CONFIG_REG_SPN	0x65	/**< Spansion read config command. */
/** Spansion status register 1 address. */
#define STATUS_REG_1_ADDR_SPN	0x800000
/** Spansion configuration register 4 address. */
#define CONFIG_REG_4_ADDR_SPN	0x800005
/** Spansion configuration register 5 address. */
#define CONFIG_REG_5_ADDR_SPN	0x800006
#define DDR_READ_CMD_4B_SPN	0xEE	/**< Spansion DDR read command. */

/**
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000

#define FLASH_PAGE_SIZE_256		256	/**< 256-byte page size. */
#define FLASH_SECTOR_SIZE_4KB		0x1000	/**< 4 KB sector size. */
#define FLASH_SECTOR_SIZE_256KB 	0x40000	/**< 256 KB sector size. */
#define FLASH_SECTOR_SIZE_64KB		0x10000	/**< 64 KB sector size. */
#define FLASH_SECTOR_SIZE_128KB		0x20000	/**< 128 KB sector size. */
/** 256 Mbit flash device size. */
#define FLASH_DEVICE_SIZE_256M		0x2000000
/** 512 Mbit flash device size. */
#define FLASH_DEVICE_SIZE_512M		0x4000000
/** 1 Gbit flash device size. */
#define FLASH_DEVICE_SIZE_1G		0x8000000
/** 2 Gbit flash device size. */
#define FLASH_DEVICE_SIZE_2G		0x10000000

#define	MICRON_OCTAL_ID_BYTE0		0x2c	/**< Micron ID byte 0. */
#define GIGADEVICE_OCTAL_ID_BYTE0	0xc8	/**< GigaDevice ID byte 0. */
#define ISSI_OCTAL_ID_BYTE0		0x9d	/**< ISSI ID byte 0. */
#define MACRONIX_OCTAL_ID_BYTE0		0xc2	/**< Macronix ID byte 0. */
#define SPANSION_OCTAL_ID_BYTE0		0x34	/**< Spansion ID byte 0. */

#define MACRONIX_OCTAL_ID_BYTE1		0x94	/**< Macronix ID byte 1. */

#define MICRON_BP_BITS_MASK		0x7C	/**< Micron BP bits mask. */
/** Macronix volatile configuration register 2 address. */
#define CONFIG_REG2_VOLATILE_ADDR_MX 	0x00000300

/*
 * Define this to run the example in SDR-PHY or DDR-PHY mode. While still in
 * SDR-NON-PHY mode the example then programs the RX tuning pattern below into
 * the last sector of the flash, verifies it and hands it to the driver, which
 * reads it back at every RX tap to calibrate the PHY. It is undefined by
 * default, in which case the example runs in SDR-NON-PHY mode where no tuning
 * is required.
 */
#undef FLASH_SETUP_TUNING_PATTERN

/**************************** Type Definitions *******************************/

/**
 * @brief Holds the flash device information.
 */
typedef struct {
	u32 jedec_id;	/**< JEDEC ID. */
	u32 SectSize;		/**< Individual sector size or
						 * combined sector size in case of parallel config*/
	/** Total no. of sectors in one/two flash devices. */
	u32 NumSect;
	u32 PageSize;		/**< Individual page size or
				 * combined page size in case of parallel config*/
	/** Total no. of pages in one/two flash devices. */
	u32 NumPage;
	u32 FlashDeviceSize;	/**< Size of one flash device
				 * NOT the combination of both devices, if present
				 */
	u32 SectMask;		/**< Mask to get sector start address. */
	u8 NumDie;		/**< No. of die forming a single flash. */
	/** Read command used to read data from flash. */
	u32 ReadCmd;
	u32 WriteCmd;	/**< Write command used to write data to flash. */
	u32 EraseCmd;	/**< Erase Command. */
	u8 StatusCmd;	/**< Status Command. */
	u8 DummyCycles;	/**< Number of Dummy cycles for Read operation. */
} FlashInfo;

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/** Flash configuration table of the supported flash devices. */
FlashInfo Flash_Config_Table[] = {
	/* Micron */
	{
		0x2c5b1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	{
		0x2c5b1b, FLASH_SECTOR_SIZE_128KB, 0x400, FLASH_PAGE_SIZE_256, 0x80000,
		FLASH_DEVICE_SIZE_1G, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	{
		0x2c5b1c, FLASH_SECTOR_SIZE_128KB, 0x800, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* GIGADEVICE */
	{
		0xc86819, FLASH_SECTOR_SIZE_64KB, 0x200, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFF0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* ISSI */
	{
		0x9d5b19, FLASH_SECTOR_SIZE_128KB, 0x100, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/*is25lx512m*/
	{
		0x9d5a1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFE0000, 2,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* MACRONIX */
	{
		0xc2813a, FLASH_SECTOR_SIZE_4KB, 0x4000, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, (WRITE_CMD_4B << 8) | WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/*mx66um2g45g*/
	{
		0xc2803c, FLASH_SECTOR_SIZE_4KB, 0x10000, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, (WRITE_CMD_4B << 8) | WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/*mx66uw2g345gxrix0*/
	{
		0xc2943c, FLASH_SECTOR_SIZE_4KB, 0x10000, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, (WRITE_CMD_4B << 8) | WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/* Infineon */
	/*s28hs02gt*/
	{
		0x345b1c, FLASH_SECTOR_SIZE_256KB, 0x400, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFC0000, 2,
		(DDR_READ_CMD_4B_SPN  << 8) | READ_CMD_4B, (WRITE_CMD_4B << 8) | WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B, READ_STATUS_CMD, 0
	},
};

#ifdef FLASH_SETUP_TUNING_PATTERN
/**
 * Size in bytes of the RX tuning pattern. The driver reads this many bytes at
 * every RX tap, so it has to be a multiple of four.
 */
#define TUNING_PATTERN_SIZE	128U

/****************************************************************************/
/**
*
* @brief Returns the address at which the RX tuning pattern is programmed.
*
* The address is the start of the last sector of the flash on CS0.
*
* @param[in]	Index Index of the flash in the flash configuration table.
*
* @return	Flash address of the RX tuning pattern.
*
* @note
* C-style signature
*	u32 PATTERN_ADDRESS(u32 Index)
*
******************************************************************************/
#define PATTERN_ADDRESS(Index)	(Flash_Config_Table[Index].FlashDeviceSize - \
				 Flash_Config_Table[Index].SectSize)

/**
 * Reference pattern the RX tuning algorithm compares against. The example
 * programs it into the flash and passes it to the driver.
 */
#ifdef __ICCARM__
#pragma data_alignment = 4
u8 TuningPattern[TUNING_PATTERN_SIZE] = {
#else
u8 TuningPattern[TUNING_PATTERN_SIZE] __attribute__ ((aligned(4))) = {
#endif
	0xFEU, 0xFFU, 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x00U,
	0x00U, 0xFEU, 0xFEU, 0x01U, 0x01U, 0x01U, 0x01U, 0x00U,
	0x00U, 0xFEU, 0xFEU, 0x01U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
	0xFFU, 0x00U, 0x00U, 0xFEU, 0xFEU, 0xFFU, 0xFFU, 0xFFU,
	0xFFU, 0x00U, 0x00U, 0xFEU, 0xFEU, 0xFFU, 0x01U, 0x01U,
	0x01U, 0x01U, 0x01U, 0xFEU, 0x00U, 0xFEU, 0xFEU, 0x01U,
	0x01U, 0x01U, 0x01U, 0xFEU, 0x00U, 0xFEU, 0xFEU, 0x01U,
	0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFEU, 0x00U, 0xFEU,
	0xFEU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFEU, 0x00U, 0xFEU,
	0xFEU, 0xFFU, 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x00U,
	0xFEU, 0xFEU, 0xFEU, 0x01U, 0x01U, 0x01U, 0x01U, 0x00U,
	0xFEU, 0xFEU, 0xFEU, 0x01U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
	0xFFU, 0x00U, 0xFEU, 0xFEU, 0xFEU, 0xFFU, 0xFFU, 0xFFU,
	0xFFU, 0x00U, 0xFEU, 0xFEU, 0xFEU, 0xFFU, 0x01U, 0x01U,
	0x01U, 0x01U, 0x01U, 0xFEU, 0xFEU, 0xFEU, 0xFEU, 0x01U,
	0x01U, 0x01U, 0x01U, 0xFEU, 0xFEU, 0xFEU, 0xFEU, 0x01U
};

/**
 * Buffer the pattern is read back into, both by the example when it verifies
 * what it programmed and by the driver at every RX tap. It is filled using DMA,
 * hence the cache line alignment.
 */
#ifdef __ICCARM__
#pragma data_alignment = 64
u8 TuningReadBuffer[TUNING_PATTERN_SIZE];
#else
u8 TuningReadBuffer[TUNING_PATTERN_SIZE] __attribute__ ((aligned(64)));
#endif
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief Finds the flash configuration table index for a flash ID.
*
* @param[in]	ReadId JEDEC ID read from the flash.
* @param[out]	FCTIndex Pointer to store the matching table index.
*
* @return
*		- XST_SUCCESS if a matching entry is found.
*		- XST_FAILURE otherwise.
*
******************************************************************************/
static INLINE u32 CalculateFCTIndex(u32 ReadId, u32 *FCTIndex)
{
	u32 Index;

	for (Index = 0; Index < sizeof(Flash_Config_Table) / sizeof(Flash_Config_Table[0]);
	     Index++) {
		if (ReadId == Flash_Config_Table[Index].jedec_id) {
			*FCTIndex = Index;
			return XST_SUCCESS;
		}
	}

	return XST_FAILURE;
}

#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_FLASH_CONFIG_H */
