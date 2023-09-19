/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_temp.h
*
* TEMPORARY FILE....SHALL BE DELETED LATER
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   har  07/21/22 Initial release
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_TEMP_H
#define XNVM_TEMP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/


/*************************** Constant Definitions *****************************/
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

	XNVM_EFUSE_ERR_WRITE_AES_DIS = 0x8B00,/**<0x8B00 - Error in
						* AES_DIS efuse
						* programming */

	XNVM_EFUSE_ERR_WRITE_JTAG_ERROUT_DIS = 0x8C00,/**<0x8C00 - Error in
						* JTAG_ERROUT_DIS efuse
						* programming */

	XNVM_EFUSE_ERR_WRITE_JTAG_DIS = 0x8D00,/**<0x8D00 - Error in
						* JTAG_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_HWTSTBITS_DIS = 0x8E00,/**<0x8E00 - Error in
						* HWTSTBITS_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_IP_DIS_WR_LK = 0x8F00,/**<0x8F00 - Error in
						* IP_DIS_WR_LK efuse
						* programming */



	XNVM_EFUSE_ERR_WRITE_PPK0_WR_LK = 0x9000,/**<0x9000 - Error in
						* PPK0_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_PPK1_WR_LK = 0x9100,/**<0x9100 - Error in
						* PPK1_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_PPK2_WR_LK	= 0x9200,/**<0x9200 - Error in
						* PPK2_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_0 = 0x9300,/**<0x9300 - Error in
						* AES_CRC_LK_BIT0 efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_1 = 0x9400,/**<0x9400 - Error in
							 * AES_CRC_LK_BIT1 efuse
							 * programming */


	XNVM_EFUSE_ERR_WRITE_AES_WR_LK = 0x9500,/**<0x9500 - Error in
						* AES_WR_LK efuse
						* programming */




	XNVM_EFUSE_ERR_WRITE_USER_KEY0_CRC_LK = 0x9600,/**<0x9600 - Error in
							* USER_KEY0_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY0_WR_LK = 0x9700,/**<0x9700 - Error in
							* USER_KEY0_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY1_CRC_LK = 0x9800,/**<0x9800 - Error in
							* USER_KEY1_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY1_WR_LK = 0x9900,/**<0x9900 - Error in
							* USER_KEY1_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_0 = 0x9A00,/**<0x9600 - Error in
							* SECDBG_DIS_BIT_0
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_1 = 0x9B00,/**<0x9B00 - Error in
							* SECDBG_DIS_BIT_1
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_0 = 0x9C00,/**<0x9C00 - Error in
							* SECLOCKDBG_DIS
							* bit0
							* programming */
	XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_1 = 0x9D00,/**<0x9D00 - Error in
							* SECLOCKDBG_DIS
							* bit1 efuse
							* programming */
	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_0 = 0x9E00,/**<0x9E00 - Error in
							* PMC_SC_EN_BIT_0
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_1 = 0x9F00,/**<0x9F00 - Error in
							* PMC_SC_EN_BIT_1
							* efuse programming */

	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_2 = 0xA000,/**<0xA000 - Error in
							* PMC_SC_EN_BIT_2
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SVD_WR_LK = 0xA100,/**<0xA100 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_DNA_WR_LK = 0xA200,/**<0xA200 - Error in
						* DNA_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_BOOTENV_WR_LK = 0xA300,/**<0xA300 - Error in
						* BOOTENV_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_CACHE_WR_LK = 0xA400,/**<0xA400 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_0 = 0xA500,/**<0xA500 - Error in
						* REG_INIT_DIS_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_1 = 0xA600,/**<0xA600 - Error in
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
	XNVM_EFUSE_ERR_WRITE_SAFETY_MISSION_EN = 0xAD00,/**<0xAD00 - Error in
						* SAFETY_MISSION_EN
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_LBIST_EN = 0xAE00,/**<0xAE00 - Error in
						* LBIST_EN
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_CRYPTO_KAT_EN = 0xAF00,/**<0xAF00 - Error in
							 * CRYPTO_KAT_EN
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
	XNVM_ERR_WRITE_PUF_USER_DATA = 0x10000, /**< 0x10000
							* When user chooses PUF efuses as user efuses
							* data provided for last row
							* i.e row 255 is not valid */
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
	XNVM_EFUSE_ERR_RD_META_HEADER_IV_RANGE = 0xD800,/**<0xD800 - Error in
						* reading Meta IV efuses */
	XNVM_EFUSE_ERR_RD_BLACK_IV = 0xD900,/**<0xD900 - Error in
						* reading Blk IV efuses */
	XNVM_EFUSE_ERR_RD_PLM_IV_RANGE = 0xDA00,/**<0xDA00 - Error in
						* reading PLM Iv efuses */
	XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV_RANGE = 0xDB00,/**<0xDB00 - Error
						* in
						* reading Data Partition IV
						* efuses */
	XNVM_EFUSE_ERR_RD_DEC_ONLY = 0xDC00,/**<0xDC00 - Error in
						* reading in Dec_only efuses */
	XNVM_EFUSE_ERR_RD_USER_FUSES = 0xDD00,/**<0xDD00 - Error in
						* reading User efuses */
	XNVM_EFUSE_ERR_RD_PUF_FUSES = 0xDE00,/**<0xDE00 - Error in
						* reading PUF efuses */

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
	XNVM_EFUSE_ERROR_DME_MODE_SET = 0xF500,/**<0xF500 - Error
						* occured when DME Mode is set and
						* trying to read User fuses */
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

} XNvm_EfuseErrorCodes;


/**< EFUSE row count numbers */
#define XNVM_EFUSE_PPK_HASH_NUM_OF_CACHE_ROWS		(8U) /**<Number of cache rows for efuse ppk hash*/
#define XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS			(3U) /**<Number of cache rows for efuse IV*/
/** @} */

/**< EFUSE Row numbers */
#define XNVM_EFUSE_META_HEADER_IV_START_ROW		(90U) /**< Starting row of Meta Header IV eFuses*/
#define XNVM_EFUSE_BLACK_IV_START_ROW			(4U) /**< Starting Row of black IV eFuse*/
#define XNVM_EFUSE_PLM_IV_START_ROW			(4U) /**< Starting Row of PLM IV eFuse*/
#define XNVM_EFUSE_DATA_PARTITION_IV_START_ROW		(4U) /**< Starting Row of data partition IV eFuses*/
#define XNVM_EFUSE_PPK0_HASH_START_ROW			(160U) /**< Starting Row of ppk0 hash eFuse*/
#define XNVM_EFUSE_PPK1_HASH_START_ROW			(96U)  /**< Starting Row of ppk1 hash eFuse*/
#define XNVM_EFUSE_PPK2_HASH_START_ROW			(128U)  /**< Starting Row of ppk2 hash eFuse*/
#define XNVM_EFUSE_AES_KEY_0_TO_127_START_ROW		(16U)  /**< Starting Row of 0 to 127 eFuses of aes key*/
#define XNVM_EFUSE_AES_KEY_128_TO_255_START_ROW		(16U) /**< Starting Row of 128 to 255 eFuses of aes key*/
#define XNVM_EFUSE_USER_KEY0_0_TO_63_START_ROW		(56U) /**< Starting Row of 0 to 63 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY0_64_TO_191_START_ROW	(66U) /**< Starting Row of 64 to 191 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY0_192_TO_255_START_ROW	(74U) /**< Starting Row of 192 to 255 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY1_0_TO_63_START_ROW		(48U) /**< Starting Row of 0 to 63 eFuses of user key1*/
#define XNVM_EFUSE_USER_KEY1_64_TO_127_START_ROW	(66U) /**< Starting Row of 64 to 27 eFuses of user key1*/
#define XNVM_EFUSE_USER_KEY1_128_TO_255_START_ROW	(74U) /**< Starting Row of 128 to 255 eFuses of user key1*/
#define XNVM_EFUSE_AES_KEY_0_TO_255_END_ROW		(31U) /**< Ending row of 256 eFuse aes key*/
#define XNVM_EFUSE_USER_KEY0_0_TO_63_END_ROW		(63U) /**< Ending Row of 0 to 63 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY0_64_TO_191_END_ROW		(73U) /**< Ending Row of 64 to 191 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY0_192_TO_255_END_ROW		(81U) /**< Ending Row of 192 to 255 eFuses of user key0*/
#define XNVM_EFUSE_USER_KEY1_0_TO_63_END_ROW		(55U) /**< Ending Row of 0 to 63 eFuses of user key1*/
#define XNVM_EFUSE_USER_KEY1_64_TO_127_END_ROW		(73U) /**< Ending Row of 64 to 27 eFuses of user key1*/
#define XNVM_EFUSE_USER_KEY1_128_TO_255_END_ROW		(81U)/**< Ending Row of 128 to 255 eFuses of user key1*/

#define XNVM_EFUSE_AES_KEY_0_TO_127_COL_START_NUM	(8U) /**< Column starting number of 0 to 127 eFuses aes key*/
#define XNVM_EFUSE_AES_KEY_0_TO_127_COL_END_NUM		(15U) /**< Column ending number of 0 to 127 eFuses aes key*/
#define XNVM_EFUSE_AES_KEY_128_TO_255_COL_START_NUM	(16U) /**< Column starting number of 128 to 255 eFuses aes key*/
#define XNVM_EFUSE_AES_KEY_128_TO_255_COL_END_NUM	(23U) /**< Column ending number of 128 to 255 eFuses aes key*/
#define XNVM_EFUSE_USER_KEY0_0_TO_63_COL_START_NUM	(8U) /**< Column starting number of 0 to 63 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_0_TO_63_COL_END_NUM	(15U) /**< Column ending number of 0 to 63 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_64_TO_191_COL_START_NUM	(8U)  /**< Column starting number of 64 to 191 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_64_TO_191_COL_END_NUM	(23U) /**< Column ending number of 64 to 191 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_192_TO_255_COL_START_NUM	(8U) /**< Column starting number of 192 to 255 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_192_TO_255_COL_END_NUM	(15U) /**< Column ending number of 192 to 255 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY1_0_TO_63_START_COL_NUM	(16U) /**< Column starting number of 0 to 63 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_0_TO_63_END_COL_NUM	(23U) /**< Column ending number of 0 to 63 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_64_TO_127_START_COL_NUM	(24U) /**< Column starting number of 64 to 127 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_64_TO_127_END_COL_NUM	(31U)  /**< Column ending number of 64 to 127 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_128_TO_255_START_COL_NUM	(16U)  /**< Column starting number of 128 to 255 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_128_TO_255_END_COL_NUM	(31U) /**< Column ending number of 128 to 255 eFuses user key1*/

#define XNVM_EFUSE_AES_KEY_0_TO_127_NUM_OF_ROWS		(16U) /**< Number of rows of 0 to 127 eFuses aes key*/
#define XNVM_EFUSE_AES_KEY_128_TO_255_NUM_OF_ROWS	(16U) /**< Number of rows of 128 to 255 eFuses aes key*/
#define XNVM_EFUSE_USER_KEY0_0_TO_63_NUM_OF_ROWS	(8U) /**< Number of rows 0 to 63 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_64_TO_191_NUM_OF_ROWS	(8U) /**< Number of rows 64 to 191 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY0_192_TO_255_NUM_OF_ROWS	(8U) /**< Number of rows 192 to 255 eFuses user key0*/
#define XNVM_EFUSE_USER_KEY1_0_TO_63_NUM_OF_ROWS	(8U)  /**< Number of rows 0 to 63 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_64_TO_127_NUM_OF_ROWS	(8U) /**< Number of rows 64 to 127 eFuses user key1*/
#define XNVM_EFUSE_USER_KEY1_128_TO_255_NUM_OF_ROWS	(8U)  /**< Number of rows 128 to 255 eFusesuser key1*/

#define XNVM_EFUSE_PPK0_HASH_START_COL_NUM		(16U) /**<Starting column number of efuse ppk0 hash*/
#define XNVM_EFUSE_PPK0_HASH_END_COL_NUM		(23U) /**<Ending column number of efuse ppk0 hash*/
#define XNVM_EFUSE_PPK1_HASH_START_COL_NUM		(24U) /**<Starting column number of efuse ppk1 hash*/
#define XNVM_EFUSE_PPK1_HASH_END_COL_NUM		(31U) /**<Ending column number of efuse ppk1 hash*/
#define XNVM_EFUSE_PPK2_HASH_START_COL_NUM              (24U) /**<Starting column number of efuse ppk2 hash*/
#define XNVM_EFUSE_PPK2_HASH_END_COL_NUM                (31U) /**<Ending column number of efuse ppk2 hash*/

#define XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS			(32U) /**< Number of rows of efuse ppk hash*/

#define XNVM_EFUSE_METAHEADER_IV_RANGE_START_COL_NUM	(0U) /**<Starting column number of efuse metaheader IV range*/
#define XNVM_EFUSE_METAHEADER_IV_RANGE_END_COL_NUM	(31U) /**<Ending column number of efuse metaheader IV range*/
#define XNVM_EFUSE_BLACK_IV_START_COL_NUM		(8U) /**<Starting column number of black IV */
#define XNVM_EFUSE_BLACK_IV_END_COL_NUM			(15U)  /**<Ending column number of black IV */
#define XNVM_EFUSE_PLM_IV_RANGE_START_COL_NUM		(16U) /**<Starting column number of efuse plm IV range*/
#define XNVM_EFUSE_PLM_IV_RANGE_END_COL_NUM		(23U) /**<Ending column number of efuse plm IV range*/
#define XNVM_EFUSE_DATA_PARTITION_IV_START_COL_NUM	(24U) /**<Starting column number of efuse Data partition IV */
#define XNVM_EFUSE_DATA_PARTITION_IV_END_COL_NUM	(31U) /**<Ending column number of efuse Data partition IV */

#define XNVM_EFUSE_METAHEADER_IV_NUM_OF_ROWS		(3U) /**< No of rows of efuse metaheader IV*/
#define XNVM_EFUSE_BLACK_IV_NUM_OF_ROWS			(12U) /**< No of rows of efuse black IV*/
#define XNVM_EFUSE_PLM_IV_NUM_OF_ROWS			(12U) /**< No of rows of efuse plm IV*/
#define XNVM_EFUSE_DATA_PARTITION_IV_NUM_OF_ROWS	(12U) /**< No of rows of Data partition IV*/
/**
 *  @name eFUSE Cache Register Offsets
 */
/**< eFUSE Cache Register Offsets */
#define XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET	(0x00000180U) /**< MetaHeader IV cache offset */
#define XNVM_EFUSE_CACHE_BLACK_IV_OFFSET		(0x000001D0U) /**< Black IV cache offset */
#define XNVM_EFUSE_CACHE_PLM_IV_RANGE_OFFSET		(0x000001DCU) /**< PLM IV cache offset */
#define XNVM_EFUSE_CACHE_DATA_PARTITION_IV_OFFSET	(0x000001E8U) /**< Data partition IV cache offset */
#define XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET		(0x000000ACU) /**< Security control cache offset */
#define XNVM_EFUSE_CACHE_PPK0_HASH_OFFSET		(0x00000100U) /**< Ppk0 hash cache offset*/
#define XNVM_EFUSE_CACHE_PPK1_HASH_OFFSET		(0x00000120U) /**< Ppk1 hash cache offset */
#define XNVM_EFUSE_CACHE_PPK2_HASH_OFFSET		(0x00000140U) /**< Ppk2 hash cache offset*/
#define XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET		(0x000000E4U) /**< Security cache offset*/
#define XNVM_EFUSE_CACHE_IP_DISABLE_OFFSET		(0x00000018U) /**< IP disable cache offset */
#define XNVM_EFUSE_CACHE_DME_FIPS_OFFSET                (0x00000234U) /**< Dme fips cache offset */
/**
 * @name Register: EFUSE_CACHE_SECURITY_CONTROL_REG
 */
/**< eFUSE Cache Security Control Register Masks And Shifts */
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK	(0xc0000000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK	(0x10000000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_LOCK_DBG_DIS_MASK	(0x00600000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_SEC_DEBUG_DIS_MASK	(0x00180000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK		(0x00040000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK	(0x00020000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK	(0x00010000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK	(0x00008000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK	(0x00004000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK	(0x00002000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK	(0x00001000U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK	(0x00000800U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK	(0x00000600U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK	(0x00000100U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK	(0x00000080U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK	(0x00000040U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK		(0x00000004U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK	(0x00000002U)
#define XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK		(0x00000001U)

#define XNVM_EFUSE_FIPS_VERSION_0_MASK				(0x00000004U) /**< Mask for fips version 0*/
#define XNVM_EFUSE_FIPS_VERSION_2_1_MASK			(0xC0000000U) /**< Mask for FIPS_VERSION bits 1 and 2*/
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_MASK                (0xFF000000U) /**< Mask for dme fips fips mode*/
#define XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_SHIFT               (24U) /**<Shift value of efuse cache dme fips fips mode */

#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_2_1_SHIFT    (30U) /**<Shift value of efuse cache IP disable for FIPS_VERSION bits 1 and 2 */
#define XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_0_SHIFT      (2U) /**<Shift value of efuse cache IP disable fips version 0 */

/**
 * @name CRP base address definition
 */
/**< CRP Base Address */
#define XNVM_CRP_BASE_ADDR				(0xF1260000U)
/** @} */

/**
 * @name CRP eFUSE Clock Control Register
 */
/**< CRP REF_CLK offset and definition */
#define XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET		(0x00000134U)
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT		(2U)
#define XNVM_CRP_EFUSE_REF_CLK_IN			((u32)0x01U << \
					XNVM_CRP_EFUSE_REF_CLK_SELSRC_SHIFT)
#define XNVM_CRP_EFUSE_REF_CLK_SELSRC		(XNVM_CRP_EFUSE_REF_CLK_IN)
/** @} */

/**
 * @name eFUSE Controller base address
 */
/**< eFUSE Control Base Address */
#define XNVM_EFUSE_CTRL_BASEADDR			(0xF1240000U)
/** @} */

/**
 * @name eFUSE Controller Register Offsets
 */
/**< eFUSE CTRL Register Offsets */
#define XNVM_EFUSE_WR_LOCK_REG_OFFSET			(0x00000000U)
#define XNVM_EFUSE_CFG_REG_OFFSET			(0x00000004U)
#define XNVM_EFUSE_STATUS_REG_OFFSET			(0x00000008U)
#define XNVM_EFUSE_PGM_ADDR_REG_OFFSET			(0x0000000CU)
#define XNVM_EFUSE_RD_ADDR_REG_OFFSET			(0x00000010U)
#define XNVM_EFUSE_RD_DATA_REG_OFFSET			(0x00000014U)
#define XNVM_EFUSE_TPGM_REG_OFFSET			(0x00000018U)
#define XNVM_EFUSE_TRD_REG_OFFSET			(0x0000001CU)
#define XNVM_EFUSE_TSU_H_PS_REG_OFFSET			(0x00000020U)
#define XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET		(0x00000024U)
#define XNVM_EFUSE_TRDM_REG_OFFSET			(0x00000028U)
#define XNVM_EFUSE_TSU_H_CS_REG_OFFSET			(0x0000002CU)
#define XNVM_EFUSE_ISR_REG_OFFSET			(0x00000030U)
#define XNVM_EFUSE_CACHE_LOAD_REG_OFFSET		(0x00000040U)
#define XNVM_EFUSE_AES_CRC_REG_OFFSET			(0x00000048U)
#define XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET		(0x0000004CU)
#define XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET		(0x00000050U)
#define XNVM_EFUSE_PD_REG_OFFSET			(0x00000054U)
#define XNVM_EFUSE_TEST_CTRL_REG_OFFSET			(0x00000100U)
/** @} */

/**
 * @name Register: EFUSE_CTRL_CFG
 */
/**< eFUSE CTRL STATUS Register Masks */
#define XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK    		(0x00000004U)

/* access_type: ro  */
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK	(0x00000800U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK	(0x00000400U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK	(0x00000200U)
#define XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK	(0x00000100U)
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK		(0x00000080U)
#define XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK		(0x00000040U)
/** @} */

/**
 * @name  EFUSE_CACHE Base Address
 */
/**< eFUSE Cache Base Address */
#define XNVM_EFUSE_CACHE_BASEADDR				(0xF1250000U)
/** @} */

/**
 * @name  Register: EFUSE_CACHE_SECURITY_MISC_0
 */
/**< eFUSE Cache DEC_EFUSE_ONLY Mask */
#define XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)
/**
 * @name  WR_UNLOCK Code
 */
/**< eFUSE Write Unlock Passcode */
#define XNVM_EFUSE_WR_UNLOCK_PASSCODE			(0xDF0DU)
/** @} */

/**
 * @name eFUSE Controller CFG register
 */
/**< eFUSE CFG Modes */
#define XNVM_EFUSE_CFG_ENABLE_PGM			(0x01U << 1U)
#define XNVM_EFUSE_CFG_MARGIN_RD			(0x01U << 2U)
#define XNVM_EFUSE_CFG_NORMAL_RD			(0x00U << 2U)
/** @} */

/**
 * @name eFUSE STATUS register
 */
/**< eFUSE Status Register Masks */
#define XNVM_EFUSE_STATUS_TBIT_0			(0x01U << 0U)
#define XNVM_EFUSE_STATUS_TBIT_1			(0x01U << 1U)
#define XNVM_EFUSE_STATUS_TBIT_2			(0x01U << 2U)
#define XNVM_EFUSE_STATUS_CACHE_DONE			(0x01U << 5U)
/** @} */

/**
 * @name eFUSE Controller PGM_ADDR register
 */
/**< eFUSE Addres Shifts */
#define XNVM_EFUSE_ADDR_COLUMN_SHIFT			(0U)
#define XNVM_EFUSE_ADDR_ROW_SHIFT			(5U)
#define XNVM_EFUSE_ADDR_PAGE_SHIFT			(13U)
/** @} */

/**< eFUse Cache load mask */
#define XNVM_EFUSE_CACHE_LOAD_MASK			(0x01U)


#define XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK		(0x1FFFU)
/**< eFuse Protection Row Mask */

/**
 * @name eFUSE ISR Register
 */
/**< eFuse ISR registers masks */
#define XNVM_EFUSE_ISR_PGM_DONE				(0x01U << 0U)
#define XNVM_EFUSE_ISR_PGM_ERROR			(0x01U << 1U)
#define XNVM_EFUSE_ISR_RD_DONE				(0x01U << 2U)
#define XNVM_EFUSE_ISR_CACHE_ERROR			(0x01U << 4U)
/** @} */

/**< eFUSE Controller PD register definition */
#define XNVM_EFUSE_PD_ENABLE				(0x01U << 0U)


#define XNVM_PS_REF_CLK_FREQ			(XPAR_PSU_PSS_REF_CLK_FREQ_HZ)
					/**< PS Ref clock definition in Hz */

#define XNVM_NUM_OF_ROWS_PER_PAGE			(256U)
					/**< Number of Rows per Page */

/**
 * @name Timeout values
 */
/**< Timeout in term of number of times status register polled to check eFUSE
 * read operation complete
 */
#define XNVM_EFUSE_RD_TIMEOUT_VAL			(100U)

/**< Timeout in term of number of times status register polled to check eFUSE
 * programming operation complete
 */
#define XNVM_EFUSE_PGM_TIMEOUT_VAL			(100U)

/**< Timeout in term of number of times status register polled to check eFuse
 * Cache load is done
 */
#define XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL		(0x800U)

/**< Timeout in term of number of times status register polled to check eFuse
 * Crc check id done.
 */
#define XNVM_POLL_TIMEOUT				(0x400U)
/** @} */

#define XNVM_WORD_LEN					(4U) /**< No of bits in xnvm word*/
/**
 * @name XNVM_EFUSE_CTRL_WR
 */
#define XNVM_EFUSE_CTRL_WR_LOCKED	(0x01U) /**<efuse control is write locked*/
#define XNVM_EFUSE_CTRL_WR_UNLOCKED	(0x00U) /**<efuse control is write unlocked*/
/** @} */



#ifdef __cplusplus
}
#endif

#endif	/* XNVM_TEMP_H */
