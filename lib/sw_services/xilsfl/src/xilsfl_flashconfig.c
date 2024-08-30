/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_flashconfig.c
 * @addtogroup xilsfl overview
 * @{
 *
 * The xilsfl_flashconfig.c file contains the definitions of flash configuration table.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xilsfl_flashconfig.h"

/************************** Variable Definitions *****************************/
const XFlashInfo Flash_Config_Table[] = {
	/* Micron */
	/*MT35XU02GCBA*/
	{
		0x2c5b1c, XSFL_FLASH_SECTOR_SIZE_128KB, 0x800, XSFL_FLASH_PAGE_SIZE_256, 0x100000,
		XSFL_FLASH_DEVICE_SIZE_2G, 0xFFFE0000, XSFL_READ_CMD_OCTAL_IO_4B << 16 | XSFL_READ_CMD_OCTAL_4B,
		(XSFL_WRITE_CMD_OCTAL_4B << 8) | XSFL_WRITE_CMD_4B,
		(XSFL_DIE_ERASE_CMD << 16) | (XSFL_BULK_ERASE_CMD << 8) | XSFL_SEC_ERASE_CMD_4B,
		(XSFL_FLASH_PROTO_1_1_8 << 16) | (XSFL_FLASH_PROTO_8_8_8 << 8) | XSFL_FLASH_PROTO_1_1_1,
		XSFL_FLASH_FREQUENCY_166_MHZ, (XSFL_FLASH_DUMMY_CYCLES_16 << 8) | XSFL_FLASH_DUMMY_CYCLES_8 ,
		XSFL_READ_FLAG_STATUS_CMD, 1, 1, XSFL_DUAL_BYTE_OP_DISABLE, XSFL_OSPI_FLASH
	},
	/*n25q00a*/
	{
		0x20bb20, XSFL_FLASH_SECTOR_SIZE_64KB, 0x400, XSFL_FLASH_PAGE_SIZE_256,
		0x80000, XSFL_FLASH_DEVICE_SIZE_512M, 0xFFFF0000,  XSFL_QUAD_READ_CMD_4B,
		XSFL_WRITE_CMD_4B, XSFL_SEC_ERASE_CMD_4B, XSFL_FLASH_PROTO_1_1_4 << 16 | XSFL_FLASH_PROTO_1_1_1,
		XSFL_FLASH_FREQUENCY_150_MHZ, XSFL_FLASH_DUMMY_CYCLES_8,
		XSFL_READ_FLAG_STATUS_CMD, 1, 1, XSFL_DUAL_BYTE_OP_DISABLE, XSFL_QSPI_FLASH
	},

};

/*****************************************************************************/
/**
 *
 * This function used to Identifies the flash in FCT table.
 *
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 ******************************************************************************/
u32 XSfl_CalculateFCTIndex(u32 ReadId, u32 *FCTIndex)
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
