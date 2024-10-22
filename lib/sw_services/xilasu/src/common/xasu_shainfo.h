/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_shainfo.h
 *
 * This file contains the SHA definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       ma   06/14/24 Updated XAsufw_ShaOperationCmd structure to have 64-bit hash address
 *       vns  08/22/24 Updated sha command structure
 *       am   10/22/24 Added macros for hash lengths.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_SHAINFO_H
#define XASU_SHAINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/* SHA module command IDs */
#define XASU_SHA_OPERATION_CMD_ID		(0U) /**< Command ID for SHA3 operation command */
#define XASU_SHA_KAT_CMD_ID			(1U) /**< Command ID for SHA3 KAT command */
#define XASU_SHA_GET_INFO_CMD_ID		(2U) /**< Command ID for SHA3 Get Info command */

/* SHA modes */
#define XASU_SHA_MODE_SHA256			(0U) /**< SHA mode 256 */
#define XASU_SHA_MODE_SHA384			(1U) /**< SHA mode 384 */
#define XASU_SHA_MODE_SHA512			(2U) /**< SHA mode 512 */
#define XASU_SHA_MODE_SHAKE256			(4U) /**< SHA mode SHAKE */

/* SHA operation mode */
#define XASU_SHA_START				(0x1U) /**< SHA start operation flag */
#define XASU_SHA_UPDATE				(0x2U) /**< SHA update operation flag */
#define XASU_SHA_FINISH				(0x4U) /**< SHA finish operation flag */

/* SHA hash lengths */
#define XASU_SHA_256_HASH_LEN			(32U) /**< SHA2/3 256 hash length */
#define XASU_SHA_384_HASH_LEN			(48U) /**< SHA2/3 384 hash length */
#define XASU_SHA_512_HASH_LEN			(64U) /**< SHA2/3 512 hash length */
#define XASU_SHAKE_256_HASH_LEN			(32U) /**< SHAKE 256 hash length */
#define XASU_SHAKE_256_MAX_HASH_LEN		(136U) /**< SHAKE 256 maximum hash length */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains SHA params info
 */
typedef struct {
	u64 DataAddr; /**< SHA2/3 data address */
	u64 HashAddr; /**< SHA2/3 hash address */
	u32 DataSize; /**< SHA2/3 data size */
	u32 HashBufSize; /**< SHA2/3 hash buffer size */
	u8 ShaMode; /**< SHA2/3 mode */
	u8 IsLast; /**< Is last update */
	u8 OperationFlags; /**< SHA2/3 operation flags */
	u8 ShakeReserved; /**< SHA3 SHAKE256 next xof enable flag. NA for client. ASUFW internal use */
} XAsu_ShaOperationCmd;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_SHAINFO_H */
/** @} */
