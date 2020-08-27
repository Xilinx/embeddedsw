/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse.h
* @addtogroup xnvm_efuse_apis XilNvm eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains function declarations of eFUSE APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
* 2.0 	kal  02/27/2020	Added eFuse wrapper APIs to program AES keys, PPK hash,
*                       Revocation ID, SecCtrl eFuses, Puf HD and APIs to read
*                       eFuse Cache values.
*       kal  03/03/2020 Added protection eFuse row programming.
* 2.1   rpo  06/06/2020 Support added to write glitch configuration data.
*       rpo  06/08/2020 Support added to program eFUSE halt boot bits to stop
*                       at ROM stage.
* 		am 	 08/19/2020 Resolved MISRA C violations.
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_H
#define XNVM_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/
/**@cond xnvm_internal
 * @{
 */
/* Key and Iv length definitions for Versal eFuse */
#define XNVM_EFUSE_AES_KEY_LEN_IN_WORDS			(8U)
#define XNVM_EFUSE_IV_LEN_IN_WORDS                      (3U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS		(8U)
#define XNVM_EFUSE_DNA_IN_WORDS				(4U)

#define XNVM_EFUSE_IV_LEN_IN_BITS			(96U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS			(256U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS			(256U)

#define XNVM_EFUSE_IV_LEN_IN_BYES			(12U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES                 (32U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES		(32U)

/* PUF syndrome length definitions for Versal eFuse */
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)

#define XNVM_NUM_OF_REVOKE_ID_FUSES			(8U)
#define XNVM_USER_FUSE_START_NUM			(1U)
#define XNVM_USER_FUSE_END_NUM				(63U)
#define XNVM_NUM_OF_USER_FUSES				(63U)

/* Versal eFuse maximum bits in a row */
#define XNVM_EFUSE_MAX_BITS_IN_ROW			(32U)
#define XNVM_MAX_REVOKE_ID_FUSES			(XNVM_NUM_OF_REVOKE_ID_FUSES	\
											* XNVM_EFUSE_MAX_BITS_IN_ROW)

/*Macros for eFUSE CTRL WRITE LOCKED and UNLOCKED */
#define XNVM_EFUSE_CTRL_WR_LOCKED	(0x01U)
#define XNVM_EFUSE_CTRL_WR_UNLOCKED	(0x00U)

/**
* @}
* @endcond
*/

/***************************** Type Definitions *******************************/

/**
 * @addtogroup xilsecure_versal_error_codes List of Error Codes
 * @{
 */

/**
 The following table lists the Versal eFuse library error codes.
 */
typedef enum {
	XNVM_EFUSE_ERR_NONE = 0,/**< 0 - No error. */

	/* Error codes for EFuse */
	XNVM_EFUSE_ERR_INVALID_PARAM = 0x2,/**< 0x2 - Passed invalid param. */
	XNVM_EFUSE_ERR_RD,/**< 0x3 - Error while reading.*/
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

	XNVM_EFUSE_ERR_WRITE_META_HEADER_IV = 0x8700,/**<0x8700 - Error in
						* Meta header IV range
						* programming */

	XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV = 0x8800,/**<0x8800 - Error in
						* Blk Obfus IV
						* programming */


	XNVM_EFUSE_ERR_WRITE_PLM_IV = 0x8900,/**<0x8900 - Error in
						* PLM IV
						* programming */


	XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV = 0x8A00,/**<0x8A00 - Error in
						* Data Partition IV
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_DIS = 0x8B00,/**<0x8B00 - Error in
						* AES_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_JTAG_ERROUT_DIS = 0x8C00,/**<0x8B00 - Error in
						* JTAG_ERROUT_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_JTAG_DIS = 0x8D00,/**<0x8D00 - Error in
						* JTAG_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_HWTSTBITS_DIS = 0x8E00,/**<0x8E00 - Error in
						* HWTSTBITS_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_IP_DIS_WR_LK = 0x8F00,/**<0x8F00 - Error in
						* IP_DIS_WR_LK efuse
						* programming */



	XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK = 0x9000,/**<0x9000 - Error in
						* PPK0_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK = 0x9100,/**<0x9100 - Error in
						* PPK1_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK	= 0x9200,/**<0x9200 - Error in
						* PPK2_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_0 = 0x9300,/**<0x9300 - Error in
						* AES_CRC_LK_BIT0 efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1 = 0x9400,/**<0x9400 - Error in
							 * AES_CRC_LK_BIT1 efuse
							 * programming */


	XNVM_EFUSE_ERR_WRTIE_AES_WR_LK = 0x9500,/**<0x9500 - Error in
						* AES_WR_LK efuse
						* programming */




	XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK = 0x9600,/**<0x9600 - Error in
							* USER_KEY0_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK = 0x9700,/**<0x9700 - Error in
							* USER_KEY0_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK = 0x9800,/**<0x9800 - Error in
							* USER_KEY1_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK = 0x9900,/**<0x9900 - Error in
							* USER_KEY1_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_0 = 0x9A00,/**<0x9600 - Error in
							* SECDBG_DIS_BIT_0
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1 = 0x9B00,/**<0x9B00 - Error in
							* SECDBG_DIS_BIT_1
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_0 = 0x9C00,/**<0x9C00 - Error in
							* SECLOCKDBG_DIS
							* bit0
							* programming */
	XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_1 = 0x9D00,/**<0x9D00 - Error in
							* SECLOCKDBG_DIS
							* bit1 efuse
							* programming */
	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_0 = 0x9E00,/**<0x9E00 - Error in
							* PMC_SC_EN_BIT_0
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_1 = 0x9F00,/**<0x9F00 - Error in
							* PMC_SC_EN_BIT_1
							* efuse programming */

	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_2 = 0xA000,/**<0xA000 - Error in
							* PMC_SC_EN_BIT_2
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_SVD_WR_LK = 0xA100,/**<0xA100 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_DNA_WR_LK = 0xA200,/**<0xA200 - Error in
						* DNA_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK = 0xA300,/**<0xA300 - Error in
						* BOOTENV_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_CACHE_WR_LK = 0xA400,/**<0xA400 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_0 = 0xA500,/**<0xA500 - Error in
						* REG_INIT_DIS_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1 = 0xA600,/**<0xA600 - Error in
						* REG_INIT_DIS_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0 = 0xA700,/**<0xA700 - Error in
						* PPK0_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1 = 0xA800,/**<0xA800 - Error in
						* PPK0_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0 = 0xA900,/**<0xA900 - Error in
						* PPK1_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1 = 0xAA00,/**<0xAA00 - Error in
						* PPK1_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0 = 0xAB00,/**<0xAB00 - Error in
						* PPK2_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1 = 0xAC00,/**<0xAC00 - Error in
						* PPK2_INVALID_BIT_1
						* efuse programming */

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

	XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS = 0xC000,/**<0xC000 - Error in
						* REVOCATION_IDS
						* programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0 = 0xC100,/**<0xC100 - Error in
						* REVOCATION_ID_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1 = 0xC200,/**<0xC200 - Error in
						* REVOCATION_ID_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2 = 0xC300,/**<0xC300 - Error in
						* REVOCATION_ID_2
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3 = 0xC400,/**<0xC400 - Error in
						* REVOCATION_ID_3
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4 = 0xC500,/**<0xC500 - Error in
						* REVOCATION_ID_4
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5 = 0xC600,/**<0xC600 - Error in
						* REVOCATION_ID_5
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6 = 0xC700,/**<0xC700 - Error in
						* REVOCATION_ID_6
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7 = 0xC800,/**<0xC800 - Error in
						* REVOCATION_ID_7
						* efuse programming */

	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xD000,/**<0xD000 - Error in
						* reading Sec Ctrl efuses */
	XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS = 0xD100,/**<0xD100 - Error in
						* reading in Misc Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL = 0xD200,/**<0xD200 - Error in
						* reading in Puf Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SYN_DATA = 0xD300,/**<0xD300 - Error in
						* reading Puf syn efuses */
	XNVM_EFUSE_ERR_RD_PUF_CHASH = 0xD400,/**<0xD400 - Error in
						* reading Puf Chash efuses */
	XNVM_EFUSE_ERR_RD_PUF_AUX = 0xD500,/**<0xD500 - Error in
						* reading Puf Aux efuses */
	XNVM_EFUSE_ERR_RD_DNA = 0xD600,/**<0xD600 - Error in
						* reading DNA efuses */
	XNVM_EFUSE_ERR_RD_PPK_HASH = 0xD700,/**<0xD700 - Error in
						* reading PPK hash efuses */
	XNVM_EFUSE_ERR_RD_META_HEADER_IV = 0xD800,/**<0xD800 - Error in
						* reading Meta IV efuses */
	XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV = 0xD900,/**<0xD900 - Error in
						* reading Blk IV efuses */
	XNVM_EFUSE_ERR_RD_PLM_IV = 0xDA00,/**<0xDA00 - Error in
						* reading PLM Iv efuses */
	XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV = 0xDB00,/**<0xDB00 - Error in
						* reading Data Partition IV
						* efuses */
	XNVM_EFUSE_ERR_RD_DEC_ONLY = 0xDC00,/**<0xDC00 - Error in
						* reading in Dec_only efuses */
	XNVM_EFUSE_ERR_RD_USER_FUSES = 0xDD00,/**<0xDD00 - Error in
						* reading User efuses */

	XNVM_EFUSE_ERR_WRITE_ROW_37_PROT = 0xE000,/**<0xE000 - Error in
						* ROW_37_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_40_PROT = 0xE100,/**<0xE100 - Error in
						* ROW_40_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_42_PROT = 0xE200,/**<0xE200 - Error in
						* ROW_42_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT = 0xE300,/**<0xE300 - Error in
						* ROW_43_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT = 0xE400,/**<0xE400 - Error in
						* ROW_43_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT = 0xE500,/**<0xE500 - Error in
						* ROW_57_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT = 0xE600,/**<0xE600 - Error in
						* ROW_57_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_58_PROT = 0xE700,/**<0xE700 - Error in
						* ROW_58_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT = 0xE800,/**<0xE800 - Error in
						* ROW64_87_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT = 0xE900,/**<0xE900 - Error in
						* ROW64_87_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT = 0xEA00,/**<0xEA00 - Error in
						* ROW96_99_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT = 0xEB00,/**<0xEB00 - Error in
						* ROW96_99_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_GLITCH_CFG = 0xEC00,/**<0xEC00 - Error in
						* programming of glitch configuration */
	XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK = 0xED00,/**<0xED00 - Error in
						* programming of glitch write lock */
	XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS = 0xEE00,/**<0xEE00 - Error in
						* programming of Glitch ROM monitor
						* or glitch halt boot */
	XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS = 0xEF00,/**<0xEF00 - Error in
						* programming of ROM flow control halt boot
						* bits for generic and environment errors*/
	XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED = 0xF000,/**<0xF000 - Error in
						* Programming, no data is
						* provided for Programming.
						* All Data pointers are NULL */
	XNVM_EFUSE_ERR_FUSE_PROTECTED = 0x40000,/**< 0x40000
						* Requested eFUSE is write
						* protected. */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000/**< 0x80000
						* Error occurred before
						* programming. */

}XNvm_EfuseErrorCodes;

/**
* @}
*/

/**@cond xnvm_internal
 * @{
 */
typedef enum {
	XNVM_EFUSE_PAGE_0,
	XNVM_EFUSE_PAGE_1,
	XNVM_EFUSE_PAGE_2
}XNvm_EfuseType;

typedef enum {
	XNVM_EFUSE_META_HEADER_IV_TYPE,
	XNVM_EFUSE_BLACK_OBFUS_IV_TYPE,
	XNVM_EFUSE_PLM_IV_TYPE,
	XNVM_EFUSE_DATA_PARTITION_IV_TYPE
}XNvm_IvType;

typedef enum {
	XNVM_EFUSE_PPK0,
	XNVM_EFUSE_PPK1,
	XNVM_EFUSE_PPK2
}XNvm_PpkType;

typedef enum {
	XNVM_EFUSE_REVOCATION_ID_0,
	XNVM_EFUSE_REVOCATION_ID_1,
	XNVM_EFUSE_REVOCATION_ID_2,
	XNVM_EFUSE_REVOCATION_ID_3,
	XNVM_EFUSE_REVOCATION_ID_4,
	XNVM_EFUSE_REVOCATION_ID_5,
	XNVM_EFUSE_REVOCATION_ID_6,
	XNVM_EFUSE_REVOCATION_ID_7
}XNvm_RevocationId;

typedef enum {
	XNVM_EFUSE_SEC_AES_DIS,
	XNVM_EFUSE_SEC_JTAG_ERROUT_DIS,
	XNVM_EFUSE_SEC_JTAG_DIS,
	XNVM_EFUSE_SEC_HWTSTBITS_DIS,
	XNVM_EFUSE_SEC_IP_DIS_WRLK = 5,
	XNVM_EFUSE_SEC_PPK0_WRLK,
	XNVM_EFUSE_SEC_PPK1_WRLK,
	XNVM_EFUSE_SEC_PPK2_WRLK,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_0,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1,
	XNVM_EFUSE_SEC_AES_WRLK,
	XNVM_EFUSE_SEC_USER_KEY0_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY0_WRLK,
	XNVM_EFUSE_SEC_USER_KEY1_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY1_WRLK,
	XNVM_EFUSE_SEC_PUF_SYN_LK,
	XNVM_EFUSE_SEC_PUF_TEST2_DIS,
	XNVM_EFUSE_SEC_PUF_DIS,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_0,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_0,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_0,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2,
	XNVM_EFUSE_SEC_SVD_WRLK,
	XNVM_EFUSE_SEC_DNA_WRLK,
	XNVM_EFUSE_SEC_BOOTENV_WRLK,
	XNVM_EFUSE_SEC_CACHE_WRLK,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_0,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1
}XNvm_SecCtrlBitColumns;

typedef enum {
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_0 = 2,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1,
	XNVM_EFUSE_MISC_SAFETY_MISSION_EN,
	XNVM_EFUSE_LBIST_EN = 14,
	XNVM_EFUSE_CRYPTO_KAT_EN,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_0 = 19,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_1,
	XNVM_HALT_BOOT_ERROR_BIT_0,
	XNVM_HALT_BOOT_ERROR_BIT_1,
	XNVM_EFUSE_GD_ROM_MONITOR_EN = 29,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_0,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_1
}XNvm_MiscCtrlBitColumns;

typedef struct {
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
}XNvm_PpkHash;

typedef struct {
	u32 Iv[XNVM_EFUSE_IV_LEN_IN_WORDS];
}XNvm_Iv;

typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_IN_WORDS];
}XNvm_Dna;

typedef struct {
	u8 AesDis;
	u8 JtagErrOutDis;
	u8 JtagDis;
	u8 Ppk0WrLk;
	u8 Ppk1WrLk;
	u8 Ppk2WrLk;
	u8 AesCrcLk;
	u8 AesWrLk;
	u8 UserKey0CrcLk;
	u8 UserKey0WrLk;
	u8 UserKey1CrcLk;
	u8 UserKey1WrLk;
	u8 SecDbgDis;
	u8 SecLockDbgDis;
	u8 BootEnvWrLk;
	u8 RegInitDis;
}XNvm_EfuseSecCtrlBits;

typedef struct {
	u8 PufRegenDis;
	u8 PufHdInvalid;
	u8 PufTest2Dis;
	u8 PufDis;
	u8 PufSynLk;
}XNvm_EfusePufSecCtrlBits;

typedef struct {
	u8 GlitchDetHaltBootEn;
	u8 GlitchDetRomMonitorEn;
	u8 HaltBootError;
	u8 HaltBootEnv;
	u8 CryptoKatEn;
	u8 LbistEn;
	u8 SafetyMissionEn;
	u8 Ppk0Invalid;
	u8 Ppk1Invalid;
	u8 Ppk2Invalid;
}XNvm_EfuseMiscCtrlBits;

typedef struct {
	u8 SysmonTempMonEn;
        u8 SysmonVoltMonEn;
        u8 LpdNocScEn;
        u8 PmcMbistEn;
        u8 LpdMbistEn;
}XNvm_EfuseSecurityMisc1Bits;

typedef struct {
	u8 PrgmGlitch;
	u8 GlitchDetWrLk;
	u32 GlitchDetTrim;
	u8 GdRomMonitorEn;
	u8 GdHaltBootEn;
}XNvm_EfuseGlitchCfgBits;

typedef struct {
	u8 PrgmAesKey;
	u8 PrgmUserKey0;
	u8 PrgmUserKey1;
	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
}XNvm_EfuseAesKeys;

typedef struct {
	u8 PrgmPpk0Hash;
	u8 PrgmPpk1Hash;
	u8 PrgmPpk2Hash;
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
}XNvm_EfusePpkHash;

typedef struct {
	u8 PrgmDecOnly;
	u32 DecEfuseOnly;
}XNvm_EfuseDecOnly;

typedef struct {
	u8 PrgmRevokeId;
	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
}XNvm_EfuseRevokeIds;

typedef struct {
	u8 PrgmMetaHeaderIv;
	u8 PrgmBlkObfusIv;
	u8 PrgmPlmIv;
	u8 PrgmDataPartitionIv;
	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 PlmIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
}XNvm_EfuseIvs;

typedef struct {
	u32 StartUserFuseNum;
	u32 NumOfUserFuses;
	u32 *UserFuseData;
}XNvm_EfuseUserData;

typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	u8 PrgmPufHelperData;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}XNvm_EfusePufHd;

typedef struct {
	XNvm_EfuseAesKeys *AesKeys;
	XNvm_EfusePpkHash *PpkHash;
	XNvm_EfuseDecOnly *DecOnly;
	XNvm_EfuseSecCtrlBits *SecCtrlBits;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits;
	XNvm_EfuseRevokeIds *RevokeIds;
	XNvm_EfuseIvs *Ivs;
	XNvm_EfuseUserData *UserFuses;
	XNvm_EfuseGlitchCfgBits *GlitchCfgBits;
}XNvm_EfuseData;

/**
* @}
* @endcond
*/

/*************************** Function Prototypes ******************************/
int XNvm_EfuseWrite(const XNvm_EfuseData *WriteNvm);
int XNvm_EfuseWriteIVs(XNvm_EfuseIvs *EfuseIv);
int XNvm_EfuseRevokePpk(XNvm_PpkType PpkRevoke);
int XNvm_EfuseWriteRevocationId(u32 RevokeId);
int XNvm_EfuseWriteUserFuses(XNvm_EfuseUserData *WriteUserFuses);
int XNvm_EfuseReadIv(XNvm_Iv *EfuseIv, XNvm_IvType IvType);
int XNvm_EfuseReadRevocationId(u32 *RevokeFusePtr, u8 RevokeFuseNum);
int XNvm_EfuseReadUserFuses(const XNvm_EfuseUserData *UserFusesData);
int XNvm_EfuseReadMiscCtrlBits(XNvm_EfuseMiscCtrlBits *MiscCtrlBits);
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits);
int XNvm_EfuseReadPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType);
int XNvm_EfuseReadDecOnly(u32* DecOnly);
int XNvm_EfuseReadDna(XNvm_Dna *EfuseDna);
int XNvm_EfuseCheckAesKeyCrc(u32 Crc);
int XNvm_EfuseCheckAesUserKey0Crc(u32 Crc);
int XNvm_EfuseCheckAesUserKey1Crc(u32 Crc);
int XNvm_EfuseWritePuf(const XNvm_EfusePufHd *PufHelperData);
int XNvm_EfuseReadPuf(XNvm_EfusePufHd *PufHelperData);
int XNvm_EfuseReadPufSecCtrlBits(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits);

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
