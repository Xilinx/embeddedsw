/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XILSFL_FLASHCONFIG_H
#define XILSFL_FLASHCONFIG_H

/*****************************************************************************/
/**
 * @file xilsfl_flashconfig.h
 * @brief Declares the flash configuration table and flash-related constants.
 *
 * This file contains the flash configuration table and flash-related defines.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.1   sb  02/11/25 Add support for x2/x4 operation.
 * 1.3   zm  03/09/26 Add support for W25Q128JW Winbond flash.
 * 1.4   vn  08/26/26 Fix Doxygen warnings.
 * 1.4   vn  09/30/26 Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xilsfl.h"

/************************** Constant Definitions *****************************/

/**
 * @name Flash dummy cycles
 *
 * Dummy cycles that the library can send to the flash device.
 * @{
 */
/** Zero dummy cycles. */
#define XSFL_FLASH_DUMMY_CYCLES_0   0x0
/** Eight dummy cycles. */
#define XSFL_FLASH_DUMMY_CYCLES_8   0x8
#define XSFL_FLASH_DUMMY_CYCLES_10  0xA		/**< 10 dummy cycles. */
#define XSFL_FLASH_DUMMY_CYCLES_16  0x10	/**< 16 dummy cycles. */
#define XSFL_FLASH_DUMMY_CYCLES_20  0x14	/**< 20 dummy cycles. */
#define XSFL_FLASH_DUMMY_CYCLES_21  0x15	/**< 21 dummy cycles. */
/** @} */

/**
 * @name Flash frequencies
 * @{
 */
/** Flash frequency of 166 MHz. */
#define XSFL_FLASH_FREQUENCY_166_MHZ   166000000U
/** Flash frequency of 150 MHz. */
#define XSFL_FLASH_FREQUENCY_150_MHZ   150000000U

/**
 * @def XSFL_FLASH_FREQUENCY_133_MHZ
 * @brief Flash frequency of 133 MHz.
 */
#define XSFL_FLASH_FREQUENCY_133_MHZ   133000000U
/** @} */

/**************************** Type Definitions *******************************/

/**
 * @brief Holds the configuration information for a flash device.
 */
typedef struct {
	u32 jedec_id;		/**< JEDEC ID. */
	u32 SectSize;		/**< Individual sector size, or combined sector
				 * size in case of parallel configuration. */
	u32 NumSect;		/**< Total number of sectors in one or two flash
				 * devices. */
	u32 PageSize;		/**< Individual page size, or combined page size
				 * in case of parallel configuration. */
	u32 NumPage;		/**< Total number of pages in one or two flash
				 * devices. */
	u32 FlashDeviceSize;	/**< Size of one flash device, NOT the
				 * combination of both devices, if present. */
	u32 SectMask;		/**< Mask to get the sector start address. */
	u32 ReadCmd;		/**< Read command used to read data from
				 * flash. */
	u32 WriteCmd;		/**< Write command used to write data to
				 * flash. */
	u32 EraseCmd;		/**< Erase command. */
	u32 Proto;		/**< Indicates the number of Cmd-Addr-Data
				 * lines. */
	u32 SdrMaxFreq;		/**< Maximum supported flash frequency in SDR
				 * mode. */
	u32 DummyCycles;	/**< Number of dummy cycles for read
				 * operation. */
	u8 StatusCmd;		/**< Status command. */
	u8 NumDie;		/**< Number of dies forming a single flash. */
	u8 FSRFlag;		/**< Indicates the Flag Status Register. */
	u8 ExtOpCodeType;	/**< Extended opcode in dual-byte opcode
				 * mode. */
	u8 FlashType;		/**< Indicates the type of the flash device. */
} XFlashInfo;

/************************** Function Prototypes ******************************/
u32 XSfl_CalculateFCTIndex(u32 ReadId, u32 *FCTIndex);

/************************** Variable Definitions *****************************/
/**
 * @brief Holds the configuration of each supported flash device.
 */
extern const XFlashInfo Flash_Config_Table[];

/***************** Macros (Inline Functions) Definitions *********************/

/** @} */
#endif /* XILSFL_FLASHCONFIG_H */
