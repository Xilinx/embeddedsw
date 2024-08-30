/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xilsfl_flashconfig.h
 * @addtogroup xilsfl overview
 * @{
 *
 * This file contains flash configuration table and flash related defines.
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
#ifndef XILSFL_FLASHCONFIG_H
#define XILSFL_FLASHCONFIG_H

/***************************** Include Files *********************************/
#include "xilsfl.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants define the Dummy cycles which may be sent to the Flash
 * device.
 */
#define XSFL_FLASH_DUMMY_CYCLES_0   0x0
#define XSFL_FLASH_DUMMY_CYCLES_8   0x8
#define XSFL_FLASH_DUMMY_CYCLES_16  0x10
#define XSFL_FLASH_DUMMY_CYCLES_20  0x14
#define XSFL_FLASH_DUMMY_CYCLES_21  0x15

/*
 * The following constants define the Dummy cycles which may be sent to the Flash
 * device.
 */
#define XSFL_FLASH_FREQUENCY_166_MHZ   166000000U
#define XSFL_FLASH_FREQUENCY_150_MHZ   150000000U

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a flash Device.
 */
typedef struct {
	u32 jedec_id;		/* JEDEC ID */
	u32 SectSize;		/* Individual sector size or
				 * combined sector size in case of parallel config*/
	u32 NumSect;		/* Total no. of sectors in one/two flash devices */
	u32 PageSize;		/* Individual page size or
				 * combined page size in case of parallel config*/
	u32 NumPage;		/* Total no. of pages in one/two flash devices */
	u32 FlashDeviceSize;    /* This is the size of one flash device
			         * NOT the combination of both devices, if present
			         */
	u32 SectMask;		/* Mask to get sector start address */
	u32 ReadCmd;		/* Read command used to read data from flash */
	u32 WriteCmd;		/* Write command used to write data to flash */
	u32 EraseCmd;		/* Erase Command */
	u32 Proto;              /**< Indicate number of Cmd-Addr-Data lines */
	u32 SdrMaxFreq;         /**< Max support frequency of flash in SDR mode*/
	u16 DummyCycles;	/* Number of Dummy cycles for Read operation
				 *  Upper 8-bits for ddr and Lower 8-bits for sdr
				 */
	u8 StatusCmd;		/* Status Command */
	u8 NumDie;              /* No. of die forming a single flash */
	u8 FSRFlag;             /**< Indicates Flag Status Register */
	u8 ExtOpCodeType;	/**< Extended opcode in dual-byte opcode mode */
	u8 FlashType;           /**< Indicates the type of the flash device */
} XFlashInfo;

/************************** Function Prototypes ******************************/
u32 XSfl_CalculateFCTIndex(u32 ReadId, u32 *FCTIndex);

/************************** Variable Definitions *****************************/
const extern XFlashInfo Flash_Config_Table[];

/***************** Macros (Inline Functions) Definitions *********************/

#endif /* XILSFL_FLASHCONFIG_H */
