/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keywrapinfo.h
 *
 * This file contains the Key wrap and unwrap definitions which are common across the client
 * and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   02/24/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_KEYWRAPINFO_H_
#define XASU_KEYWRAPINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* Key Wrap Unwrap module command IDs */
#define XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID	(0U) /**< Command ID for Key wrap using SHA2 */
#define XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID	(1U) /**< Command ID for Key wrap using SHA3 */
#define XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID	(2U) /**< Command ID for Key unwrap using SHA2 */
#define XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID	(3U) /**< Command ID for Key unwrap using SHA3 */
#define XASU_KEYWRAP_KAT_CMD_ID			(4U) /**< Command ID for Key wrap unwrap KAT */
#define XASU_KEYWRAP_GET_INFO_CMD_ID		(5U) /**< Command ID for Key wrap unwrap Get Info */

#define XASU_KEYWRAP_OUTPUT_LEN_SIZE_IN_BYTES	(4U)	/**< Key wrap unwrap output length size */
/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains Key wrap unwrap params info
 */
typedef struct {
	u64 InputDataAddr;	/**< Address of the input data buffer which holds: Plain key for
						 * key wrap or wrapped key for key unwrap */
	u64 OutputDataAddr;	/**< Address of the output buffer to store: the wrapped key for key wrap
						 * or unwrapped key for key unwrap */
	u64 ExpoCompAddr;	/**< Address of the RSA exponent data */
	u64 KeyCompAddr;	/**< RSA key component address of type: XAsu_RsaPubKeyComp for key wrap
					and XAsu_RsaPvtKeyComp  for key unwrap */
	u64 OptionalLabelAddr;	/**< RSA optional label address for OAEP padding */
	u64 ActualOutuputDataLenAddr;	/**< Actual output data length which is returned from
						key wrap/unwrap operation */
	u32 InputDataLen;	/**< Input data Len */
	u32 RsaKeySize;		/**< RSA Key Size */
	u32 OptionalLabelSize;	/**< RSA optional label size for OAEP padding */
	u32 OutuputDataLen;	/**< Output Data Len is an input from user which specifies the
					output wrapped key length in wrap operation and in unwrap
					operation it is the size of output buffer which should be
					equal or greater than the size of the unwrapped input */
	u8 AesKeySize;	/**< AES Key Size - 128 Bit: XASU_AES_KEY_SIZE_128_BITS or
					 256 Bit : XASU_AES_KEY_SIZE_256_BITS*/
	u8 ShaType;	/**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode;	/**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_SHA256 / XASU_SHA_MODE_SHA384 / XASU_SHA_MODE_SHA512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 Reserved;	/**< Reserved */
} XAsu_KeyWrapParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYWRAPINFO_H_ */
/** @} */
