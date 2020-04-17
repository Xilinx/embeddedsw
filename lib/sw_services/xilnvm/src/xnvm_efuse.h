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
* 2.0 	kal  02/27/2020	Added Wrapper APIs for eFuse.
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
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES			(32U)
#define XNVM_EFUSE_IV_LEN_IN_BITS			(96U)

/* PPK definition for Versal eFuse */
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES		(32U)

/* PUF syndrome length definitions for Versal eFuse */
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)
#define XNVM_EFUSE_PUF_AUX_LEN_IN_BITS			(24U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS			(256U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS			(256U)

/* Versal eFuse maximum bits in a row */
#define XNVM_EFUSE_MAX_BITS_IN_ROW			(32U)

#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS		(8U)
#define XNVM_EFUSE_DNA_IN_WORDS				(4U)
#define XNVM_EFUSE_IV_LEN_IN_WORDS			(3U)


#define XNVM_NUM_OF_REVOKE_ID_FUSES			(8U)

#define XNVM_USER_FUSE_START_NUM			(1U)
#define XNVM_USER_FUSE_END_NUM				(63U)
#define XNVM_NUM_OF_USER_FUSES				(63U)

/***************************** Type Definitions *******************************/

typedef enum {
	XNVM_EFUSE_ERR_NONE = 0U,/**< 0 - No error. */

	/* Error codes for EFuse */
	XNVM_EFUSE_ERR_INVALID_PARAM = 0x2U,/**< 0x2 - Passed invalid param. */
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


	XNVM_EFUSE_ERR_AES_ALREADY_PRGMD = 0x10U,/**<0x10U - Aes key is
						* already programmed */
	XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD = 0x20U,/**<0x20U - User key 0 is
						* already programmed */
	XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD = 0x30U,/**<0x30U - User key 1 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD = 0x40U,/**<0x40U - Ppk hash 0 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD = 0x50U,/**<0x50U - Ppk hash 1 is
						* already programmed */

	XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD = 0x60U,/**<0x60U - Ppk hash 2 is
						* already programmed */

	XNVM_EFUSE_ERR_DEC_EFUSE_ONLY_ALREADY_PRGMD = 0x70U,/**<0x70U -
						* DEC_ONLY is
						* already programmed */

	XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD = 0x80U,/**<0x80U - Puf syndrome is
						* already programmed */

	XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD = 0x90U,/**<0x90U - Puf chash is
						* already programmed */

	XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD = 0xA0U,/**<0xA0U - Puf Aux is
						* already programmed */

	XNVM_EFUSE_ERR_BIT_CANT_REVERT = 0xF0U,/**<0xF0U - Already programmed eFuse
						* Bit, cant be reverted */


	XNVM_EFUSE_ERR_WRITE_AES_KEY = 0x8000U,/**<0x8000U - Error in Aes key
						* programming */

	XNVM_EFUSE_ERR_WRITE_USER0_KEY = 0x8100U,/**<0x8100U - Error in Userkey0
						* programming */

	XNVM_EFUSE_ERR_WRITE_USER1_KEY = 0x8200U,/**<0x8200U - Error in Userkey1
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK0_HASH = 0x8300U,/**<0x8300U - Error in PPK0hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK1_HASH = 0x8400U,/**<0x8400U - Error in PPK1hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_PPK2_HASH = 0x8500U,/**<0x8500U - Error in PPK2hash
						* programming */

	XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY = 0x8600U,/**<0x8600U - Error in
						* DEC_ONLY programming */

	XNVM_EFUSE_ERR_WRITE_META_HEADER_IV = 0x8700U,/**<0x8700U - Error in
						* Meta header IV range
						* programming */

	XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV = 0x8800U,/**<0x8800U - Error in
						* Blk Obfus IV
						* programming */


	XNVM_EFUSE_ERR_WRITE_PLML_IV = 0x8900U,/**<0x8900U - Error in
						* PLML IV
						* programming */


	XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV = 0x8A00U,/**<0x8A00U - Error in
						* Data Partition IV
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_DIS = 0x8B00U,/**<0x8B00U - Error in
						* AES_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_JTAG_ERROUT_DIS = 0x8C00U,/**<0x8B00U - Error in
						* JTAG_ERROUT_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_JTAG_DIS = 0x8D00U,/**<0x8D00U - Error in
						* JTAG_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_HWTSTBITS_DIS = 0x8E00U,/**<0x8E00U - Error in
						* HWTSTBITS_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_IP_DIS_WR_LK = 0x8F00U,/**<0x8F00U - Error in
						* IP_DIS_WR_LK efuse
						* programming */



	XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK = 0x9000U,/**<0x9000U - Error in
						* PPK0_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK = 0x9100U,/**<0x9100U - Error in
						* PPK1_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK	= 0x9200U,/**<0x9200U - Error in
						* PPK2_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1 = 0x9300U,/**<0x9300U - Error in
						* AES_CRC_LK_BIT1 efuse
						* programming */


	XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_2 = 0x9400U,/**<0x9400U - Error in
							 * AES_CRC_LK_BIT2 efuse
							 * programming */


	XNVM_EFUSE_ERR_WRTIE_AES_WR_LK = 0x9500U,/**<0x9500U - Error in
						* AES_WR_LK efuse
						* programming */




	XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK = 0x9600U,/**<0x9600U - Error in
							* USER_KEY0_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK = 0x9700U,/**<0x9700U - Error in
							* USER_KEY0_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK = 0x9800U,/**<0x9800U - Error in
							* USER_KEY1_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK = 0x9900U,/**<0x9900U - Error in
							* USER_KEY1_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1 = 0x9A00U,/**<0x9600U - Error in
							* SECDBG_DIS_BIT_1
							* efuse programming */


	XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_2 = 0x9B00U,/**<0x9B00U - Error in
							* SECDBG_DIS_BIT_2
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_1 = 0x9C00U,/**<0x9C00U - Error in
							* SECLOCKDBG_DIS
							* bit1
							* programming */
	XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_2 = 0x9D00U,/**<0x9D00U - Error in
							* SECLOCKDBG_DIS
							* bit2 efuse
							* programming */
	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_1 = 0x9E00U,/**<0x9E00U - Error in
							* PMC_SC_EN_BIT_1
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_2 = 0x9F00U,/**<0x9F00U - Error in
							* PMC_SC_EN_BIT_2
							* efuse programming */

	XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_3 = 0xA000U,/**<0xA000U - Error in
							* PMC_SC_EN_BIT_3
							* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_SVD_WR_LK = 0xA100U,/**<0xA100U - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_DNA_WR_LK = 0xA200U,/**<0xA200U - Error in
						* DNA_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK = 0xA300U,/**<0xA300U - Error in
						* BOOTENV_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_CACHE_WR_LK = 0xA400U,/**<0xA400U - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1 = 0xA500U,/**<0xA500U - Error in
						* REG_INIT_DIS_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_2 = 0xA600U,/**<0xA600U - Error in
						* REG_INIT_DIS_BIT_2
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1 = 0xA700U,/**<0xA700U - Error in
						* PPK0_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_2 = 0xA800U,/**<0xA800U - Error in
						* PPK0_INVALID_BIT_2
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1 = 0xA900U,/**<0xA900U - Error in
						* PPK1_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_2 = 0xAA00U,/**<0xAA00U - Error in
						* PPK1_INVALID_BIT_2
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1 = 0xAB00U,/**<0xAB00U - Error in
						* PPK2_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_2 = 0xAC00U,/**<0xAC00U - Error in
						* PPK2_INVALID_BIT_2
						* efuse programming */

	XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA = 0xB000U,/**<0xB000U - Error in
						* Writing Puf helper data
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA = 0xB100U,/**<0xB100U - Error in
						* Puf syndrome data
						* programming */
	XNVM_EFUSE_ERR_WRITE_PUF_CHASH = 0xB200U,/**<0xB200U - Error in
						* Puf Chash
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_AUX = 0xB300U,/**<0xB300U - Error in
						* Puf Aux
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS = 0xB400U,/**<0xB400U - Error in
						* PUF_REGEN_DIS
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD = 0xB500U,/**<0xB500U - Error in
						* PUF_HD_INVLD
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_TEST2_DIS = 0xB600U,/**<0xB600U - Error in
						* PUF_TEST2_DIS
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK = 0xB700U,/**<0xB700U - Error in
						* PUF_SYN_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PUF_DIS = 0xB800U,/**<0xB800U - Error in
						* PUF_DIS
						* efuse programming */

	XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS = 0xC000U,/**<0xC000U - Error in
						* REVOCATION_IDS
						* programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0 = 0xC100U,/**<0xC100U - Error in
						* REVOCATION_ID_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1 = 0xC200U,/**<0xC200U - Error in
						* REVOCATION_ID_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2 = 0xC300U,/**<0xC300U - Error in
						* REVOCATION_ID_2
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3 = 0xC400U,/**<0xC400U - Error in
						* REVOCATION_ID_3
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4 = 0xC500U,/**<0xC500U - Error in
						* REVOCATION_ID_4
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5 = 0xC600U,/**<0xC600U - Error in
						* REVOCATION_ID_5
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6 = 0xC700U,/**<0xC700U - Error in
						* REVOCATION_ID_6
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7 = 0xC800U,/**<0xC800U - Error in
						* REVOCATION_ID_7
						* efuse programming */

	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xD000U,/**<0xD000U - Error in
						* reading Sec Ctrl efuses */
	XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS = 0xD100U,/**<0xD100U - Error in
						* reading in Misc Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL = 0xD200U,/**<0xD200U - Error in
						* reading in Puf Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SYN_DATA = 0xD300U,/**<0xD300U - Error in
						* reading Puf syn efuses */
	XNVM_EFUSE_ERR_RD_PUF_CHASH = 0xD400U,/**<0xD400U - Error in
						* reading Puf Chash efuses */
	XNVM_EFUSE_ERR_RD_PUF_AUX = 0xD500U,/**<0xD500U - Error in
						* reading Puf Aux efuses */
	XNVM_EFUSE_ERR_RD_DNA = 0xD600U,/**<0xD600U - Error in
						* reading DNA efuses */
	XNVM_EFUSE_ERR_RD_PPK_HASH = 0xD700U,/**<0xD700U - Error in
						* reading PPK hash efuses */
	XNVM_EFUSE_ERR_RD_META_HEADER_IV = 0xD800U,/**<0xD800U - Error in
						* reading Meta IV efuses */
	XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV = 0xD900U,/**<0xD900U - Error in
						* reading Blk IV efuses */
	XNVM_EFUSE_ERR_RD_PLML_IV = 0xDA00U,/**<0xDA00U - Error in
						* reading PLML Iv efuses */
	XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV = 0xDB00U,/**<0xDB00U - Error in
						* reading Data Partition IV
						* efuses */
	XNVM_EFUSE_ERR_RD_DEC_ONLY = 0xDC00U,/**<0xDC00U - Error in
						* reading in Dec_only efuses */
	XNVM_EFUSE_ERR_RD_USER_FUSES = 0xDD00U,/**<0xDD00U - Error in
						* reading User efuses */

	XNVM_EFUSE_ERR_WRITE_ROW_37_PROT = 0xE000U,/**<0xE000U - Error in
						* ROW_37_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_40_PROT = 0xE100U,/**<0xE100U - Error in
						* ROW_40_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_42_PROT = 0xE200U,/**<0xE200U - Error in
						* ROW_42_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT = 0xE300U,/**<0xE300U - Error in
						* ROW_43_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT = 0xE400U,/**<0xE400U - Error in
						* ROW_43_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT = 0xE500U,/**<0xE500U - Error in
						* ROW_57_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT = 0xE600U,/**<0xE600U - Error in
						* ROW_57_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_58_PROT = 0xE700U,/**<0xE700U - Error in
						* ROW_58_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT = 0xE800U,/**<0xE800U - Error in
						* ROW64_87_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT = 0xE900U,/**<0xE900U - Error in
						* ROW64_87_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT = 0xEA00U,/**<0xEA00U - Error in
						* ROW96_99_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT = 0xEB00U,/**<0xEB00U - Error in
						* ROW96_99_1_PROT
						* programming */

	XNVM_EFUSE_ERR_FUSE_PROTECTED = 0x40000U,/**< 0x40000U
						* Requested eFUSE is write
						* protected. */
	XNVM_EFUSE_ERR_BEFORE_PROGRAMMING = 0x80000U/**< 0x80000U
						* Error occurred before
						* programming. */

}XNvm_EfuseErrorCodes;

/**
* @}
* @endcond
*/
typedef enum {
	XNVM_EFUSE_PAGE_0,
	XNVM_EFUSE_PAGE_1,
	XNVM_EFUSE_PAGE_2
}XNvm_EfuseType;

typedef enum {
	XNVM_EFUSE_META_HEADER_IV_TYPE,
	XNVM_EFUSE_BLACK_OBFUS_IV_TYPE,
	XNVM_EFUSE_PLML_IV_TYPE,
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
	XNVM_EFUSE_SEC_IP_DIS_WRLK = 5U,
	XNVM_EFUSE_SEC_PPK0_WRLK,
	XNVM_EFUSE_SEC_PPK1_WRLK,
	XNVM_EFUSE_SEC_PPK2_WRLK,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_2,
	XNVM_EFUSE_SEC_AES_WRLK,
	XNVM_EFUSE_SEC_USER_KEY0_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY0_WRLK,
	XNVM_EFUSE_SEC_USER_KEY1_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY1_WRLK,
	XNVM_EFUSE_SEC_PUF_SYN_LK,
	XNVM_EFUSE_SEC_PUF_TEST2_DIS,
	XNVM_EFUSE_SEC_PUF_DIS,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_2,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_2,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_3,
	XNVM_EFUSE_SEC_SVD_WRLK,
	XNVM_EFUSE_SEC_DNA_WRLK,
	XNVM_EFUSE_SEC_BOOTENV_WRLK,
	XNVM_EFUSE_SEC_CACHE_WRLK,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_2
}XNvm_SecCtrlBitColumns;

typedef enum {
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1 = 2U,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_2,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_2,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_2,
	XNVM_EFUSE_MISC_SAFETY_MISSION_EN,
	XNVM_EFUSE_LBIST_EN = 14U,
	XNVM_EFUSE_CRYPTO_KAT_EN,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_1 = 19U,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_2,
	XNVM_HALT_BOOT_ERROR_BIT_1,
	XNVM_HALT_BOOT_ERROR_BIT_2,
	XNVM_EFUSE_GD_ROM_MONITOR_EN = 29U,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_1,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_2
}XNvm_MiscCtrlBitColumns;

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
}XNvm_SecCtrlBits;

typedef struct {
	u8 PufRegenDis;
	u8 PufHdInvalid;
	u8 PufTest2Dis;
	u8 PufDis;
	u8 PufSynLk;
}XNvm_PufSecCtrlBits;

typedef struct {
	u8 SafetyMissionEn;
	u8 LbistEn;
	u8 CrytoKatEn;
	u8 HaltBootEnv;
	u8 HaltBootError;
	u8 GdRomMonitorEn;
	u8 GdHaltBootEn;
	u8 Ppk0Invalid;
	u8 Ppk1Invalid;
	u8 Ppk2Invalid;
}XNvm_MiscCtrlBits;

typedef struct {
	u8 SysmonTempMonEn;
	u8 SysmonVoltMonEn;
	u8 LpdNocScEn;
	u8 PmcMbistEn;
	u8 LpdMbistEn;
	u16 DpaMaskDis;
}Xnvm_SecurityMisc1Bits;

typedef struct {
	u8 AesKey;
	u8 UserKey0;
	u8 UserKey1;
	u8 Ppk0Hash;
	u8 Ppk1Hash;
	u8 Ppk2Hash;
	u8 DecOnly;
}Xnvm_WriteFlags;

typedef struct {
	XNvm_SecCtrlBits PrgmSecCtrlFlags;
	Xnvm_WriteFlags	CheckWriteFlags;
	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 DecEfuseOnly;
	XNvm_SecCtrlBits ReadBackSecCtrlBits;
}XNvm_EfuseWriteData;

typedef struct {
	u8 RevokeId0;
	u8 RevokeId1;
	u8 RevokeId2;
	u8 RevokeId3;
	u8 RevokeId4;
	u8 RevokeId5;
	u8 RevokeId6;
	u8 RevokeId7;
}XNvm_RevokeIdFlags;

typedef struct {
	u8 PrgmRevokeId0;
	u8 PrgmRevokeId1;
	u8 PrgmRevokeId2;
	u8 PrgmRevokeId3;
	u8 PrgmRevokeId4;
	u8 PrgmRevokeId5;
	u8 PrgmRevokeId6;
	u8 PrgmRevokeId7;
	u32 RevokeId0;
	u32 RevokeId1;
	u32 RevokeId2;
	u32 RevokeId3;
	u32 RevokeId4;
	u32 RevokeId5;
	u32 RevokeId6;
	u32 RevokeId7;
}XNvm_RevokeIdEfuse;

typedef struct {
	u8 Ppk0;
	u8 Ppk1;
	u8 Ppk2;
}XNvm_RevokePpkFlags;

typedef struct {
	XNvm_PufSecCtrlBits PrgmPufSecCtrlBits;
	u8 PrgmChash;
	u8 PrgmAux;
	u8 PrgmSynData;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
	XNvm_PufSecCtrlBits ReadPufSecCtrlBits;
}XNvm_PufHelperData;

typedef struct {
	u8 PgrmMetaHeaderIv;
	u8 PgrmBlkObfusIv;
	u8 PgmPlmlIv;
	u8 PgmDataPartitionIv;
	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 PlmlIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
}XNvm_Iv;

typedef struct {
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
}XNvm_PpkHash;

typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_IN_WORDS];
}XNvm_Dna;

/*************************** Function Prototypes ******************************/
u32 XNvm_EfuseWrite(XNvm_EfuseWriteData *WriteNvm);
u32 XNvm_EfuseReadSecCtrlBits(XNvm_SecCtrlBits *ReadbackSecCtrlBits);
u32 XNvm_EfuseWritePuf(XNvm_PufHelperData *PrgmPufHelperData);
u32 XNvm_EfuseReadPufSecCtrlBits(XNvm_PufSecCtrlBits *ReadBackPufSecCtrlBits);
u32 XNvm_EfuseWriteIVs(XNvm_Iv *EfuseIv);
u32 XNvm_EfuseReadIVs(XNvm_Iv *EfuseIv);
u32 XNvm_EfuseReadPufHelperData(XNvm_PufHelperData *PrgmPufHelperData);
u32 XNvm_EfuseReadDna(XNvm_Dna *EfuseDna);
u32 XNvm_EfuseReadPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType);
u32 XNvm_EfuseRevokePpk(XNvm_RevokePpkFlags *PpkRevoke);
u32 XNvm_EfuseWriteRevocationId(XNvm_RevokeIdEfuse *WriteRevokeId);
u32 XNvm_EfuseReadMiscCtrlBits(XNvm_MiscCtrlBits *ReadMiscCtrlBits);
u32 XNvm_EfuseReadDecOnly(u32* DecOnly);
u32 XNvm_EfuseReadRevocationId(u32 *RevokeFusePtr, u8 RevokeFuseNum);
u32 XNvm_EfuseCheckAesKeyCrc(u32 Crc);
u32 XNvm_EfuseCheckAesUserKey0Crc(u32 Crc);
u32 XNvm_EfuseCheckAesUserKey1Crc(u32 Crc);
u32 XNvm_EfuseWriteUserFuses(u32 StartUserFuseNum, u8 NumOfUserFuses, const u32* UserFuseData);
u32 XNvm_EfuseReadUserFuses(u32 *UserFuseData);

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
