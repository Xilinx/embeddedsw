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

#ifndef XASU_KEYWRAPINFO_H
#define XASU_KEYWRAPINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* Key Wrap Unwrap module command IDs */
#define XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID	(0U) /**< Command ID for Key wrap using SHA2 for
							  RSA-OAEP*/
#define XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID	(1U) /**< Command ID for Key wrap using SHA3 for
							  RSA-OAEP*/
#define XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID	(2U) /**< Command ID for Key unwrap using SHA2 for
							  RSA-OAEP*/
#define XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID	(3U) /**< Command ID for Key unwrap using SHA3 for
							  RSA-OAEP*/
#define XASU_KEYWRAP_KAT_CMD_ID			(4U) /**< Command ID for Key wrap unwrap KAT
							  command */
#define XASU_KEYWRAP_GET_INFO_CMD_ID		(5U) /**< Command ID for Key wrap unwrap Get Info
							   command */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains Key wrap unwrap params info
 */
typedef struct {
	u64 InputDataAddr;	/**< plain key/wrapped key */
	u64 OutputDataAddr;	/**< wrapped key/unwrapped key */
	u64 ExpoCompAddr;	/**< RSA exponent data address */
	u64 KeyCompAddr;	/**< RSA key component address which contains public key components
					or private key components based on wrap or unwrap
					operation respectively*/
	u64 OptionalLabelAddr;	/**< RSA optional label address for OAEP padding */
	u32 InputDataLen;	/**< Data Len */
	u32 RsaKeySize;		/**< RSA Key Size */
	u32 OptionalLabelSize;	/**< RSA optional label size for OAEP padding */
	u32 OutuputDataLen;	/**< Output Data Len is an input from user which specifies the
					output wrapped key length in wrap operation and in unwrap
					operation it is the size of output buffer which should be
					equal or greater than the size of the unwrapped input */
	u8 AesKeySize;	/**< AES Key Size */
	u8 ShaType;	/**< SHA2/SHA3 type */
	u8 ShaMode;	/**< SHA digest mode */
	u8 Reserved;	/**<  Reserved */
} XAsu_KeyWrapParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYWRAPINFO_H */
/** @} */
