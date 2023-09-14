/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file net/server/xnvm_efuse_error_plat.h
 *
 * Header file for xnvm_efuse.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.0   kal  07/12/2022 Initial release
 * 3.2   har  02/22/2023 Added error code for ROM Rsvd bits
 *       kpt  09/11/2023 Added error code XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XNVM_EFUSE_ERROR_PLAT_H
#define XNVM_EFUSE_ERROR_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * @addtogroup xilnvm_versal_net_common_error_codes XilNvm Net Error Codes
 * @{
 */

/**
 The following table lists the Versal common eFuse library error codes.
 */
typedef enum {
	XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS = 0x8B00,/**< 0x8B00
						* Error in MiscCtrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS = 0x8C00, /**< 0x8C00
						* Error in SecCtrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS = 0x8D00, /**< 0x8D00
						* Error in Misc1Ctrl Bits
						* programming */
	XNVM_EFUSE_ERR_WRITE_UDS = 0x8E00, /**< 0x8E00
						* Error in Uds
						* programming */
	XNVM_EFUSE_ERR_WRITE_PLM_UPDATE = 0x8F00, /**< 0x8F00
						* Error in PLM_UPDATE
						* programming */
	XNVM_EFUSE_ERR_WRITE_BOOT_MODE_DISABLE = 0xA000, /**< 0xA000
						* Error in BootModeDisable
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_MODE = 0xA100, /**< 0xA100
						* Error in DmeMode
						* programming */
	XNVM_EFUSE_ERR_WRITE_RO_SWAP = 0xA200, /**< 0xA200
						* Error in RO_SWAP_EN
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 = 0xA300, /**< 0xA300
						* Error in DME_REVOKE_0
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_1 = 0xA400, /**< 0xA400
						* Error in DME_REVOKE_1
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_2 = 0xA500, /**< 0xA500
						* Error in DME_REVOKE_2
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_REVOKE_3 = 0xA600, /**< 0xA600
						* Error in DME_REVOKE_3
						* programming */
	XNVM_EFUSE_ERR_DME_MODE_SET = 0xA700, /**< 0xA700
						* Error in DME_MODE
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_KEY_0 = 0xA800, /**< 0xA800
						* Error in DME_KEY_0
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_KEY_1 = 0xA900, /**< 0xA900
						* Error in DME_KEY_1
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_KEY_2 = 0xAA00, /**< 0xAA00
						* Error in DME_KEY_2
						* programming */
	XNVM_EFUSE_ERR_WRITE_DME_KEY_3 = 0xAB00, /**< 0xAB00
						* Error in DME_KEY_3
						* programming */
	XNVM_EFUSE_ERR_WRITE_CRC_SALT = 0xAC00, /**< 0xAC00
						 * Error in CRC_SALT
						 * programming */
	XNVM_EFUSE_ERR_WRITE_CRC = 0xAD00, /**< 0xAD00
						* Error in CRC
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROM_RSVD_BITS = 0xAE00, /**< 0xAE00
						* Error in ROM Rsvd bits
						* programming */

	XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS = 0xAF00, /**< 0xAF00
					    * Error in PUF SEC CTRL bits
						* programming */

	XNVM_ERR_DME_KEY_ALREADY_PROGRAMMED = 0xBF00, /**< 0xBF00
						* DME_KEY is already
						* programmed */

	XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_CTRL_0_PROT = 0xC300, /**< 0xC300
								* Error in SEC_CTRL_0_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_CTRL_1_PROT = 0xC400, /**< 0xC400
								* Error in SEC_CTRL_1_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC0_0_PROT = 0xC500, /**< 0xC500
								* Error in SEC_MISC0_0_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC0_1_PROT = 0xC600, /**< 0xC600
								* Error in SEC_MISC0_1_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_PPK_HASH_0_PROT = 0xC700, /**< 0xC700
								* Error in PPK_HASH_0_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_PPK_HASH_1_PROT = 0xC800, /**< 0xC800
								* Error in PPK_HASH_1_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_METAHEADER_0_PROT = 0xC900, /**< 0xC900
								* Error in METAHEADER_0_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_METAHEADER_1_PROT = 0xCA00, /**< 0xCA00
								* Error in METAHEADER_1_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_CRC_0_PROT = 0xCB00, /**< 0xCB00
							* Error in CRC_0_PROT
							* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_CRC_1_PROT = 0xCC00, /**< 0xCC00
							* Error in CRC_1_PROT
							* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_PUF_CHASH_PROT = 0xCD00, /**< 0xCD00
								* Error in PUF_CHASH_PROT
								* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC1_PROT = 0xCE00, /**< 0xCE00
								* Error in SEC_MISC1_PROT
								* programming */

} XNvm_EfuseVersalNetErrorCodes;

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_ERROR_PLAT_H */
