/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_hmacinfo.h
 *
 * This file contains the HMAC definitions which are common across the client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  01/02/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_HMACINFO_H
#define XASU_HMACINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* HMAC module command IDs */
#define XASU_HMAC_COMPUTE_SHA2_CMD_ID		(0U) /**< Command ID for HMAC compute for SHA2 command */
#define XASU_HMAC_COMPUTE_SHA3_CMD_ID		(1U) /**< Command ID for HMAC compute for SHA3 command */
#define XASU_HMAC_KAT_CMD_ID			(2U) /**< Command ID for HMAC KAT command */
#define XASU_HMAC_GET_INFO_CMD_ID		(3U) /**< Command ID for HMAC Get Info command */

/* HMAC operation mode */
#define XASU_HMAC_INIT				(0x1U) /**< HMAC init operation flag */
#define XASU_HMAC_UPDATE			(0x2U) /**< HMAC update operation flag */
#define XASU_HMAC_FINAL				(0x4U) /**< HMAC final operation flag */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains HMAC params info
 */
typedef struct {
	u8 ShaType; /**< Hash family type (SHA2/SHA3) */
	u8 ShaMode; /**< Digest type - 256/384/512 bytes */
	u8 IsLast; /**< Is last update */
	u8 OperationFlags; /**< HMAC operation flags */
	u64 KeyAddr; /**< Key address */
	u64 MsgBufferAddr; /**< Buffer holding the message */
	u64 HmacAddr; /**< Buffer address to hold the computed HMAC */
	u32 HmacLen; /**< Length of the HMAC */
	u32 KeyLen; /**< Length of the key */
	u32 MsgLen; /**< Length of the message */
} XAsu_HmacParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_HMACINFO_H */
/** @} */
