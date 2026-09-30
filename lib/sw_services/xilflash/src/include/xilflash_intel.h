/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XFLASH_INTEL_H
#define XFLASH_INTEL_H

/*****************************************************************************/
/**
*
* @file xilflash_intel.h
* @brief Defines constants, types, and prototypes for Intel flash devices.
*
* This file consists of definitions, macros, and structures specific to the
* Intel flash devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  06/16/09 Added support for multiple banks in Intel flash.
*                     Added Reset Bank function.
*                     Added support for 0xF0 reset command.
*                     Added XFL_DEVCTL_SET_CONFIG_REG IOCTL to write to the
*                     Configuration Register of the Xilinx Platform Flash XL
*                     which can be used to set the Flash in Sync/Async mode.
*                     The Xilinx Platform Flash XL is set to Async mode during
*                     the initialization of the library.
*                     Added bank(s) reset function at the top of the read
*                     function.
*                     Updated Lock and Unlock operations for multiple blocks.
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* 4.14  vn   09/30/26 Align Doxygen comments with AMD standards
*
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

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#include "xilflash.h"

/************************** Constant Definitions *****************************/

/**
 * Supported manufacturer ID. Not all parts from this vendor are supported.
 */
#define XFL_MANUFACTURER_ID_INTEL		0x89

/**
 * @name RYBY options
 *
 * These options control the RYBY signal. They can be accessed with the
 * XFL_INTEL_DEVCTL_SET_RYBY command.
 * @{
 */
#define XFL_INTEL_RYBY_PULSE_OFF		0	/**< Do not pulse. */
/** Pulse on erase complete only. */
#define XFL_INTEL_RYBY_PULSE_ON_ERASE		1
/** Pulse on program complete only. */
#define XFL_INTEL_RYBY_PULSE_ON_PROG		2
/** Pulse on erase & program complete. */
#define XFL_INTEL_RYBY_PULSE_ON_ERASE_PROG	3
/** @} */

/**
 * @name Status register bits
 *
 * Status register bit definitions. Use these bitmaps to decipher the
 * return value of the XFL_INTEL_DEVCTL_GET_LAST_ERROR command.
 * @{
 */
/** Write state machine ready. */
#define XFL_INTEL_SR_WSM_READY			0x80
#define XFL_INTEL_SR_ERASE_SUSPENDED		0x40 /**< Erase suspended. */
/** Erase or unlock error. */
#define XFL_INTEL_SR_ERASE_OR_UNLOCK_ERROR	0x20
/** Program or lock error. */
#define XFL_INTEL_SR_PROG_OR_LOCK_ERROR		0x10
#define XFL_INTEL_SR_VOLTAGE_ERROR		0x08 /**< Voltage error. */
#define XFL_INTEL_SR_WRITE_SUSPENDED		0x04 /**< Write suspended. */
/** Block locked error. */
#define XFL_INTEL_SR_BLOCK_LOCKED_ERROR		0x02
/** @} */

/**
 * @name Extended capabilities list
 *
 * These bits can be read with the XFL_INTEL_DEVCTL_GET_SUPPORT1 command.
 * @{
 */
/** The part supports the chip erase command. */
#define XFL_INTEL_SUPPORT1_CHIP_ERASE		0x80000000
/** The part supports suspension and resumption of an erase operation. */
#define XFL_INTEL_SUPPORT1_SUSPEND_ERASE	0x40000000
/** The part supports suspension and resumption of a programming operation. */
#define XFL_INTEL_SUPPORT1_SUSPEND_PROG		0x20000000
/** The part supports legacy lock/unlock. */
#define XFL_INTEL_SUPPORT1_LEGACY_LOCK		0x10000000
/** The part supports the queuing up of erase blocks. */
#define XFL_INTEL_SUPPORT1_QUEUED_ERASE		0x08000000
/** The part supports instant individual block locking. */
#define XFL_INTEL_SUPPORT1_INSTANT_LOCK		0x04000000
/** The part supports protection bits. */
#define XFL_INTEL_SUPPORT1_PROTECTION_BIT	0x02000000
/** The part supports page-mode reads. */
#define XFL_INTEL_SUPPORT1_PAGE_MODE_READ	0x01000000
/** The part supports synchronous reads. */
#define XFL_INTEL_SUPPORT1_SYNC_READ		0x00800000
/** @} */

/**
 * Suspension capability: the part supports programming after suspending an
 * erase operation.
 */
#define XFL_INTEL_SUSPEND_SUPPORT_PROG_AFTER_ERASE	0x00000001

/**
 * @name BCS/SCS command codes
 *
 * Commands written to the devices are defined by the CMD_* constants below.
 * Each command contains 8 bits of significant data. For x16 or greater
 * devices, the command data should appear on the LSB. Other bytes may be
 * written as do not care values. The CMD_* constants are defined with the
 * command data in every nibble. This eliminates the need to know the bus
 * layout, the width of individual parts, or potential byte-swapping
 * requirements.
 * @{
 */
#define XFL_INTEL_CMD_READ_ARRAY		0xFFFFFFFF /**< Read array. */
/** Read ID codes. */
#define XFL_INTEL_CMD_READ_ID_CODES		0x90909090
/** Read status register. */
#define XFL_INTEL_CMD_READ_STATUS_REG		0x70707070
/** Clear status register. */
#define XFL_INTEL_CMD_CLEAR_STATUS_REG		0x50505050
/** Write to buffer. */
#define XFL_INTEL_CMD_WRITE_BUFFER		0xE8E8E8E8
#define XFL_INTEL_CMD_PROGRAM			0x40404040 /**< Program. */
#define XFL_INTEL_CMD_BLOCK_ERASE		0x20202020 /**< Block erase. */
#define XFL_INTEL_CMD_CONFIRM			0xD0D0D0D0 /**< Confirm. */
#define XFL_INTEL_CMD_SUSPEND			0xB0B0B0B0 /**< Suspend. */
#define XFL_INTEL_CMD_RESUME			0xD0D0D0D0 /**< Resume. */
#define XFL_INTEL_CMD_RESET_0xF0		0xF0F0F0F0 /**< Reset (0xF0). */
/** @} */

/** Status value that indicates the parts are ready. */
#define XFL_INTEL_STATUS_READY			0x00800080

/**
 * @name SCS command codes
 * @{
 */
#define XFL_INTEL_CMD_READ_QUERY		0x98989898 /**< Read query. */
/** RYBY configuration. */
#define XFL_INTEL_CMD_CONFIG			0xB8B8B8B8
/** Set block lock. */
#define XFL_INTEL_CMD_LOCK_BLOCK_SET		0x60606060
/** Set block lock confirm. */
#define XFL_INTEL_CMD_LOCK_BLOCK_SET_CONFIRM	0x01010101
/** Clear block lock. */
#define XFL_INTEL_CMD_LOCK_BLOCK_CLEAR		0x60606060
/** Clear block lock confirm. */
#define XFL_INTEL_CMD_LOCK_BLOCK_CLEAR_CONFIRM	0xD0D0D0D0
/** Set Configuration Register setup. */
#define XFL_INTEL_CMD_CONFIG_REG_SETUP		0x60606060
/** Set Configuration Register confirm. */
#define XFL_INTEL_CMD_CONFIG_REG_CONFIRM	0x03030303
/** @} */

/** Other command code: protection command. */
#define XFL_INTEL_CMD_PROTECTION		0xC0C0C0C0

/**
 * @name Configuration command codes
 * @{
 */
/** RYBY level mode, no pulse. */
#define XFL_INTEL_CONFIG_RYBY_LEVEL		0x00000000
/** RYBY pulse on erase. */
#define XFL_INTEL_CONFIG_RYBY_PULSE_ERASE	0x01010101
/** RYBY pulse on write. */
#define XFL_INTEL_CONFIG_RYBY_PULSE_WRITE	0x02020202
/** RYBY pulse on erase and write. */
#define XFL_INTEL_CONFIG_RYBY_PULSE_ALL		0x03030303
/** @} */

/**
 * @name Platform flash configuration register addresses
 *
 * Configuration register value/address for setting sync and async mode of the
 * platform flash.
 *
 * To put the memory in Asynchronous Read Mode, the CR15 bit of Configuration
 * Register is set to 1. The rest of the Configuration Register Bits are set to
 * default values. The Synchronous Read Mode is set by default with CR15 = 0.
 *
 * The commands to set Configuration register are "Set Configuration Register
 * SetUP" (BASE_ADDR + CONFIG_REG_ASYNC_ADDR -> 0x0060) and "Set Configuration
 * Register Confirm" (BASE_ADDR + CONFIG_REG_ASYNC_ADDR -> 0x0003).
 *
 * The Platform flash is a 16-bit flash, so the flash controller drops off the
 * least significant bit of the address bus. So the Configuration register
 * value seen at the flash device should be left shifted by 1.
 * @{
 */
/** Asynchronous read mode address. */
#define XFL_INTEL_CMD_CONFIG_REG_ASYNC_ADDR	0x17BBE
/** Synchronous read mode address. */
#define XFL_INTEL_CMD_CONFIG_REG_SYNC_ADDR	0x07BBE
/** @} */

/**
 * @name Platform flash IDs
 *
 * The Platform flash Manufacture ID and Device ID.
 * @{
 */
#define XFL_INTEL_PLATFORM_MANUFACTURE_ID	0x0049 /**< Manufacturer ID. */
#define XFL_INTEL_PLATFORM_DEVICE_ID		0x506B /**< Device ID. */
/** @} */

/**
 * @name Micron StrataFlash G18 command codes
 *
 * Micron StrataFlash Memory Device G18 (partially compatible with Intel CFI).
 * @{
 */
/** G18 write to buffer. */
#define XFL_INTEL_G18_CMD_WRITE_BUFFER		0xE9E9E9E9
#define XFL_INTEL_G18_CMD_PROGRAM 		0x41414141 /**< G18 program. */
/** @} */


/**************************** Type Definitions *******************************/

/**
 * @brief Holds the geometry, properties, and vendor data of an Intel part.
 */
typedef struct {
	XFlashGeometry Geometry;	/**< Part geometry. */
	XFlashProperties Properties;	/**< Part timing, programming, and
					 * identification properties. */
	XFlashVendorData VendorData;	/**< Part specific data. */
} XFlashIntel_Device;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XFlashIntel_Initialize(XFlash *InstancePtr);
int XFlashIntel_Reset(XFlash *InstancePtr);

int XFlashIntel_DeviceControl(XFlash *InstancePtr, u32 Command,
			      DeviceCtrlParam *Parameters);

int XFlashIntel_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes,
		     void *DestPtr);

int XFlashIntel_Write(XFlash *InstancePtr, u32 Offset, u32 Bytes,
		      void *SrcPtr);

int XFlashIntel_Erase(XFlash *InstancePtr, u32 Offset, u32 Bytes);

int XFlashIntel_Lock(XFlash *InstancePtr, u32 Offset, u32 Bytes);
int XFlashIntel_Unlock(XFlash *InstancePtr, u32 Offset, u32 Bytes);

/**
 * @brief Retrieves the status of the Intel flash device at an offset.
 *
 * @param[in] InstancePtr Pointer to the XFlash instance.
 * @param[in] Offset Offset address in the flash device.
 *
 * @return    Status of the flash device.
 *
 * @note      This function is declared but not implemented in the library.
 */
int XFlashIntel_GetStatus(XFlash *InstancePtr, u32 Offset);

#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
