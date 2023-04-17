/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse_error_plat.h
* @addtogroup xnvm_versal_efuse_apis XilNvm versal eFuse APIs
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
* 3.1   kpt  01/21/2023 Added error codes for Additional PPKs
* 3.2   kum  04/11/2023 Moved env error codes to common xnvm_efuse_error.h
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_ERROR_PLAT_H
#define XNVM_EFUSE_ERROR_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/

/*************************** Constant Definitions *****************************/

/***************************** Type Definitions *******************************/
/**
 * @addtogroup xilnvm_versal_error_codes XilNvm Versal Error Codes
 * @{
 */

/**
 The following table lists the Versal eFuse library error codes.
 */
typedef enum {
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
	XNVM_EFUSE_ERR_WRITE_PPK3_HASH = 0xC300,/**<0xC300 - Error in
						 * PPK3hash programming */
	XNVM_EFUSE_ERR_WRITE_PPK4_HASH = 0xC400,/**<0xC400 - Error in
						 * PPK4hash programming */
	XNVM_EFUSE_5_PPKS_FEATURE_NOT_SUPPORTED = 0xC500,/**<0xC500 - Error
							  * PPK3 and PPK4 are not
							  * enable */
	XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_0 = 0xC600,/**<0xC600 - Error in
							  * PPK3_INVALID_BIT_0
							  * efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK3_INVALID_BIT_1 = 0xC700,/**<0xC700 - Error in
							  * PPK3_INVALID_BIT_1
							  * efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_0 = 0xC800,/**<0xC800 - Error in
							  * PPK4_INVALID_BIT_0
							  * efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK4_INVALID_BIT_1 = 0xC900,/**<0xC900 - Error in
							  * PPK4_INVALID_BIT_1
							  * efuse programming */
	XNVM_EFUSE_ERR_PPK3_HASH_ALREADY_PRGMD = 0xCA00,/**<0xCA00 - Error PPK3 Hash
							  * already programmed */
	XNVM_EFUSE_ERR_PPK4_HASH_ALREADY_PRGMD = 0xCB00,/**<0xCB00 - Error PPK4 Hash
							  * already programmed */
	XNVM_EFUSE_ERR_WRITE_ADD_PPK_EN = 0xCC00,/**<0xCC00 - Error in ADD_PPK_EN
	                          * efuse programming */
	XNVM_EFUSE_ERR_USER_FUSE_PGM_NOT_ALLOWED = 0xCD00,/**<0xCD00 - Error while trying
	                          * to program user efuses 48 to 63 when ADD_PPK_EN bits are
							  * programmed */
	XNVM_EFUSE_ERR_ADD_PPK_PGM_NOT_ALLOWED = 0xCE00,/**<0xCE00 - Error while programming
	                          * additional PPK bits when PPK0 to PPK3 hashes are not
							  * programmed */
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
	XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS = 0xEE00,/**<0xEE00 - Error in
						* programming of Glitch ROM monitor
						* or glitch halt boot */
	XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS = 0xEF00,/**<0xEF00 - Error in
						* programming of ROM flow control halt boot
						* bits for generic and environment errors*/
} XNvm_EfuseVersalErrorCodes;

/*************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_ERROR_PLAT_H */

/* @} */
