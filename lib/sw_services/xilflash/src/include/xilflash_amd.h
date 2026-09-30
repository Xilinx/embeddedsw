/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XFLASH_AMD_H
#define XFLASH_AMD_H

/*****************************************************************************/
/**
*
* @file xilflash_amd.h
* @brief Defines constants and prototypes specific to AMD flash devices.
*
* This file consists of definitions, macros, and structures specific to the
* AMD flash devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ksu  04/10/08 First Release.
* 2.01a ktn  03/31/10 Updated the AMD code to support uniform sector WP modes.
*		      Added the new definitions XFL_AMD_TOP_WP_UNIFORM and
*		      XFL_AMD_BOTTOM_WP_UNIFORM.
* 4.1	nsk  06/06/12 Added New definitions as per AMD spec.
*		      (CR 781697).
* 4.7   akm  07/10/19 Updated XFlashAmd_Write() to use adjusted base address
*                     in write operation(CR-1029074).
* 4.14  vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifdef SDT
#include "xilflash_config.h"
#endif

#ifdef XPAR_XFL_DEVICE_FAMILY_AMD
#include "xilflash.h"

/************************** Constant Definitions *****************************/

/**
 * @name Command addresses
 *
 * The following constants define the addresses to be sent on the address bus
 * when sending a command on the data bus.
 * @{
 */
#define XFL_AMD_CMD1_ADDR		(0x0555) /**< First command address. */
#define XFL_AMD_CMD2_ADDR		(0x02AA) /**< Second command address. */
/** @} */

/**
 * @name Command data
 *
 * The following constants define the data to be sent on the data bus when
 * sending a command on the data bus.
 * @{
 */
#define XFL_AMD_CMD1_DATA		(0x00AA) /**< First command data. */
#define XFL_AMD_CMD2_DATA		(0x0055) /**< Second command data. */
/** @} */

/**
 * @name AMD flash command set
 *
 * The following constants define the command set of the AMD flash family.
 * @{
 */
#define XFL_AMD_CMD_RESET		(0x00F0) /**< Reset or read mode. */
/** Auto select command. */
#define XFL_AMD_CMD_AUTO_SELECT		(0x0090)
#define XFL_AMD_CMD_ERASE_RESUME	(0x0030) /**< Resume erase. */
#define XFL_AMD_CMD_ERASE_SUSPEND	(0x00B0) /**< Suspend erase. */
#define XFL_AMD_CMD_ERASE1		(0x0080) /**< Erase 1 command. */
/** Erase 2 command. */
#define XFL_AMD_CMD_ERASE_BLOCK		(0x0030)
#define XFL_AMD_CMD_UNLOCK_BYPASS	(0x0020) /**< Unlock bypass command. */
#define XFL_AMD_CMD_PROGRAM		(0x00A0) /**< Program command. */
/** Write to Buffer command. */
#define XFL_AMD_CMD_WRITE_BUFFER	(0x0025)
/** Program Buffer to Flash command. */
#define XFL_AMD_CMD_PROGRAM_BUFFER	(0x0029)
/** Unlock bypass 1 command. */
#define XFL_AMD_CMD_UNLOCK_BYPASS_RESET1 (0x0090)
/** Unlock bypass 1 command. */
#define XFL_AMD_CMD_UNLOCK_BYPASS_RESET2 (0x0000)
#define XFL_AMD_CMD_ERASE_CHIP		(0x0010) /**< Chip erase command. */
#define XFL_AMD_CMD_ENTER_EXT_MODE	(0x0088) /**< Enter extended mode. */
#define XFL_AMD_CMD_EXIT_EXT_MODE	(0x0090) /**< Exit extended mode. */
#define XFL_AMD_CMD_GROUP_PROTECT1	(0x0060) /**< Group protect command. */
#define XFL_AMD_CMD_GROUP_PROTECT2	(0x0040) /**< Group protect command. */
/** Status Register Read Command. */
#define XFL_AMD_CMD_STATUS_REG_READ	(0x0070)
/** Status Register Clear Command. */
#define XFL_AMD_CMD_STATUS_REG_CLEAR 	(0x0071)
/** @} */

/**
 * @name Status register bits
 *
 * The following constants define the Status Register Bits.
 * @{
 */
#define XFL_AMD_DEVICE_READY_MASK		0x80	/**< Device Ready. */
#define XFL_AMD_DEVICE_ERASE_INSUSPEND_MASK	0x40 /**< Erase in suspend. */
#define XFL_AMD_DEVICE_ERASE_FAIL_MASK		0x20	/**< Erase in Fail. */
#define XFL_AMD_DEVICE_PROGRAM_FAIL_MASK	0x10 /**< Program Write Fail. */
#define XFL_AMD_DEVICE_PROGRAM_ABORT_MASK	0x08	/**< Program aborted. */
#define XFL_AMD_DEVICE_PROGRAM_SUSPEND_MASK	0x04 /**< Program in Suspend. */
#define XFL_AMD_DEVICE_SECTOR_LOCKED_ERR_MASK	0x02 /**< Sector was locked. */
/** Status Register Mask. */
#define XFL_AMD_DEVICE_STATUS_REG_MASK		0xFF
/** @} */

/**
 * @name Status register erase masks
 *
 * The following constants define the erase status masks of the AMD flash
 * family.
 * @{
 */
#define XFL_AMD_SR_ERASE_START_MASK	(0x0008) /**< Erase start mask. */
/** Erase operation completed mask. */
#define XFL_AMD_SR_ERASE_COMPL_MASK	(0x0040)
/** Alternate erase operation completed mask. */
#define XFL_AMD_SR_ERASE_ERROR_MASK	(0x0020)
/** @} */

/**
 * @name Boot device types
 * @{
 */
#define XFL_AMD_TOP_BOOT		(0x03)	 /**< Top boot device. */
#define XFL_AMD_BOTTOM_BOOT		(0x02)	 /**< Bottom boot device. */
#define XFL_AMD_TOP_WP_UNIFORM		(0x05)	 /**< Top WP uniform sectors. */
/** Bottom WP uniform sectors. */
#define XFL_AMD_BOTTOM_UNIFORM		(0x04)
/** @} */

/**
 * @name Bank, protection, and timing definitions
 * @{
 */
#define XFL_MAX_BANKS			(0x0002) /**< Number of banks. */
/** Manufacturer ID offset when reading status. */
#define XFL_AMD_MANUFECTURE_ID_OFFSET	(0x0000)
/** Protection status offset when reading status. */
#define XFL_AMD_PROT_STATUS_OFFSET	(0x0002)
#define XFL_AMD_GROUP_UNPROTECTED	(0x0000) /**< Block is not protected. */
/** Block is protected. */
#define XFL_AMD_GROUP_PROTECTED		(0x0001)
/** Chip unprotect address bits. */
#define XFL_AMD_CHIP_UNPROTECT_ADDR	(0x042)
/** Number of clock pulses for one microsecond. */
#define XFL_COUNT_FOR_A_MICROSECOND	(63)
/** @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XFlashAmd_Initialize(XFlash *InstancePtr);
int XFlashAmd_Reset(XFlash *InstancePtr);

int XFlashAmd_DeviceControl(XFlash *InstancePtr, u32 Command,
			    DeviceCtrlParam *Parameters);

int XFlashAmd_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes,
		   void *DestPtr);

int XFlashAmd_Write(XFlash *InstancePtr, u32 Offset, u32 Bytes,
		    void *SrcPtr);

int XFlashAmd_Erase(XFlash *InstancePtr, u32 Offset, u32 Bytes);

int XFlashAmd_Lock(XFlash *InstancePtr, u32 Offset, u32 Bytes);
int XFlashAmd_Unlock(XFlash *InstancePtr, u32 Offset, u32 Bytes);

/**
 * @brief Erases the whole flash chip.
 *
 * @param[in] InstancePtr Pointer to the XFlash instance.
 *
 * @return
 *            - XST_SUCCESS if successful.
 *            - XST_FAILURE if the operation fails.
 */
int XFlashAmd_EraseChip(struct XFlashTag *InstancePtr);

#endif /* XPAR_XFL_DEVICE_FAMILY_AMD */

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
