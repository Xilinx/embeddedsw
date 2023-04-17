/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuse_error.h
* @addtogroup xnvm_efuse_api_ids XilNvm Efuse API IDs
* @{
*
* This file contains the xilnvm Versal_Net API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  01/05/22 Initial release
* 3.1   skg  10/25/22 Added comments for macros and enums
*       skg  12/07/22 Added error codes for Additional PPKs
*       kpt  01/21/23 Removed error codes for Additional PPKs
* 3.2   kum  04/11/2023 Moved Env error codes to common to make use for both versal and versalnet
*
* </pre>
* @note
*
*
******************************************************************************/

#ifndef XNVM_EFUSE_ERROR_H
#define XNVM_EFUSE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnvm_efuse_error_plat.h"

/************************** Constant Definitions ****************************/

/**
 * @addtogroup xilnvm_versal_common_error_codes XilNvm Error Codes
 * @{
 */

/**
 *  The following table lists the Versal common eFuse library error codes.
 */
typedef enum {
	XNVM_EFUSE_ERR_NONE = 0,/**< 0 - No error. */

	/* Error codes for EFuse */
	XNVM_EFUSE_ERR_INVALID_PARAM = 0x2,/**< 0x2 - Passed invalid param. */
	XNVM_EFUSE_ERR_IN_ZEROIZATION,/**< 0x3 - Error while reading.*/
	XNVM_EFUSE_ERR_RD_TIMEOUT,/**< 0x4 - Read Timeout.*/
	XNVM_EFUSE_ERR_CACHE_PARITY,/**< 0x5 - Error in Cache reload.*/
	XNVM_EFUSE_ERR_LOCK,/**< 0x6 - Error while Locking the controller */
	XNVM_EFUSE_ERR_UNLOCK,/**<0x7 - Error while Unlocking the controller */
	XNVM_EFUSE_ERR_PGM_VERIFY,/**<0x8 - Error in verifying the bit */
	XNVM_EFUSE_ERR_PGM,/**<0x9 - Error in Programming the bit */
	XNVM_EFUSE_ERR_PGM_TIMEOUT,/**<0xA - Programming timeout happened */
	XNVM_EFUSE_ERR_PGM_TBIT_PATTERN,/**<0xB - Error in T-Bit pattern */
	XNVM_EFUSE_ERR_CACHE_LOAD,/**<0xC - Error while loading the cache */
	XNVM_EFUSE_ERR_CRC_VERIFICATION,/**<0xD - Error in Crc Verification */
	XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN,/**<0xE - Error in Anchor bits pattern */
	XNVM_EFUSE_ERR_IN_PROTECTION_CHECK,/**<0xF - Error in Protection row
					* configuration */

	XNVM_EFUSE_ERR_AES_ALREADY_PRGMD = 0x10,/**<0x10 - Aes key is
						* already programmed */
	XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD = 0x20,/**<0x20 - User key 0 is
						* already programmed */
	XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD = 0x30,/**<0x30 - User key 1 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD = 0x40,/**<0x40 - Ppk hash 0 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD = 0x50,/**<0x50 - Ppk hash 1 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD = 0x60,/**<0x60 - Ppk hash 2 is
						* already programmed */

	XNVM_EFUSE_ERR_BLK_OBFUS_IV_ALREADY_PRGMD = 0x70,/**<0x70 -
						* BLK_OBFUS_IV is
						* already programmed */

	XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD = 0x80,/**<0x80 - Puf syndrome is
						* already programmed */
	XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD = 0x90,/**<0x90 - Puf chash is
						* already programmed */

	XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD = 0xA0,/**<0xA0 - Puf Aux is
						* already programmed */

	XNVM_EFUSE_ERR_DEC_ONLY_KEY_MUST_BE_PRGMD = 0xB0,/**<0xB0 - Aes key
						* should be programmed for
						* DEC_ONLY eFuse programming */

	XNVM_EFUSE_ERR_DEC_ONLY_IV_MUST_BE_PRGMD = 0xC0,/**<0xC0 - Blk obfus
						* IV should be programmed for
						* DEC_ONLY eFuse programming */

	XNVM_EFUSE_ERR_DEC_ONLY_ALREADY_PRGMD = 0xD0,/**<0xD0 - Dec only eFuse
						* is already programmed */

	XNVM_EFUSE_ERR_BOOT_ENV_CTRL_ALREADY_PRGMD = 0xE0,/**<0xE0 - BootEnvCtrl
						* is already programmed */

	XNVM_EFUSE_ERR_BIT_CANT_REVERT = 0xF0,/**<0xF0 - Already programmed eFuse
						* Bit, can't be reverted */


	XNVM_EFUSE_ERR_WRITE_AES_KEY = 0x8000,/**<0x8000 - Error in Aes key
						* programming */

	XNVM_EFUSE_ERR_WRITE_USER_KEY0 = 0x8100,/**<0x8100 - Error in Userkey0
						* programming */

	XNVM_EFUSE_ERR_WRITE_USER_KEY1 = 0x8200,/**<0x8200 - Error in Userkey1
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK0_HASH = 0x8300,/**<0x8300 - Error in PPK0hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK1_HASH = 0x8400,/**<0x8400 - Error in PPK1hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK2_HASH = 0x8500,/**<0x8500 - Error in PPK2hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY = 0x8600,/**<0x8600 - Error in
						* DEC_ONLY programming */

	XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE = 0x8700,/**<0x8700 - Error in
						* Meta header IV range
						* programming */

	XNVM_EFUSE_ERR_WRITE_BLK_IV = 0x8800,/**<0x8800 - Error in
						* Blk Obfus IV
						* programming */

	XNVM_EFUSE_ERR_WRITE_PLM_IV_RANGE = 0x8900,/**<0x8900 - Error in
						* PLM IV
						* programming */

	XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV_RANGE = 0x8A00,/**<0x8A00 -
						* Error in
						* Data Partition IV
						* programming */
	XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA = 0xB000,/**<0xB000 - Error in
						* Writing Puf helper data
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA = 0xB100,/**<0xB100 - Error in
						* Puf syndrome data
						* programming */
	XNVM_EFUSE_ERR_WRITE_PUF_CHASH = 0xB200,/**<0xB200 - Error in
						* Puf Chash
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_AUX = 0xB300,/**<0xB300 - Error in
						* Puf Aux
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS = 0xB400,/**<0xB400 - Error in
						* PUF_REGEN_DIS
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD = 0xB500,/**<0xB500 - Error in
						* PUF_HD_INVLD
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS = 0xB600,/**<0xB600 - Error in
						* PUF_TEST2_DIS
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK = 0xB700,/**<0xB700 - Error in
						* PUF_SYN_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_DIS = 0xB800,/**<0xB800 - Error in
						* PUF_DIS
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SECURITY_MISC_1 = 0xBD00,/**<0xBD00 - Error in
						* SYSMON_MISC_1
						* efuses programming */
	XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL = 0xBE00,/**<0xBE00 - Error in
						* BOOT_ENV_CTRL
						* efuses programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS = 0xC000,/**<0xC000 - Error in
						* REVOCATION_IDS
						* programming */
	XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS = 0xC100,/**<0xC100 - Error in
							* OFFCHIP_REVOKE
							* programming */
	XNVM_EFUSE_ERR_WRITE_PUF_FUSES = 0xC200, /**<0xC200 -  Error in
							* Write PUF
							* efuses */
	XNVM_EFUSE_ERR_WRITE_GLITCH_CFG = 0xEC00,/**<0xEC00 - Error in
						* programming of glitch configuration */
	XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK = 0xED00,/**<0xED00 - Error in
						* programming of glitch write lock */

	XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED = 0xF000,/**<0xF000 - Error in
						* Programming, no data is
						* provided for Programming.
						* All Data pointers are NULL */
	XNVM_EFUSE_ERROR_READ_TMEPERATURE_OUT_OF_RANGE = 0xF100,/**<0xF100 - Error
						* before programming eFuse,
						* Temparature is out of range */
	XNVM_EFUSE_ERROR_READ_VOLTAGE_OUT_OF_RANGE = 0xF200,/**<0xF200 - Error
						* before programming eFuse,
						* Voltage is out of range */
	XNVM_EFUSE_ERROR_NO_SUPPLIES_ENABLED = 0xF300,/**<0xF200 - Error
						* before programming eFuse,
						* no supplies are enabled */
	XNVM_EFUSE_ERROR_SYSMON_NO_NEW_DATA = 0xF400,/**<0xF400 - Error
						* before programming eFuse,
						* new data is not available */
	XNVM_EFUSE_ERR_GLITCH_DETECTED = 0x20000,/**<0x20000
						 * Glitch detected, due to which
						 * requested eFuses may be
						 * partially programmed */
	XNVM_EFUSE_ERR_FUSE_PROTECTED = 0x40000,/**< 0x40000
						* Requested eFUSE is write
						* protected. */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000,/**< 0x80000
						* Error occurred before
						* programming. */

} XNvm_EfuseCommonErrorCodes;

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_ERROR_H */
/**
* @}
*/
