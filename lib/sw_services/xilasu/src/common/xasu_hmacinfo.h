/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       kp   03/24/26 Embedded XAsu_KdfHmacKeyObject in XAsu_HmacParams
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_HMACINFO_H_
#define XASU_HMACINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* HMAC module command IDs */
#define XASU_HMAC_COMPUTE_SHA2_CMD_ID		(0U) /**< Command ID for HMAC compute for SHA2 command */
#define XASU_HMAC_COMPUTE_SHA3_CMD_ID		(1U) /**< Command ID for HMAC compute for SHA3 command */
#define XASU_HMAC_KAT_CMD_ID			(2U) /**< Command ID for HMAC KAT command */
#define XASU_HMAC_MAX_CMDS			(3U) /**< Maximum number of commands supported by HMAC module */
#define XASU_HMAC_MAX_KEY_LENGTH			(1024U) /**< Maximum key length for HMAC */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains KDF/HMAC key object for vault resolution. */
typedef struct {
	u64 KeyInAddr; /**< Key address */
	u32 KeyInLen; /**< Key length */
	u32 KeyId; /**< Key identifier for key vault resolution */
} XAsu_KdfHmacKeyObject;

/** This structure contains HMAC params info. */
typedef struct {
	XAsu_KdfHmacKeyObject KeyObject; /**< Key object for input key */
	u64 MsgBufferAddr; /**< Address of the message buffer */
	u64 HmacAddr; /**< Address of the output buffer to store the generated HMAC */
	u32 MsgLen; /**< Length of the message to be processed. MsgLen can be 0 <= MsgLen < ((2^B)-8B).
			 Where B is the block length of the selected SHA type and SHA mode. */
	u32 HmacLen; /**< Length of the HMAC to be generated */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 IsLast; /**< Indicates whether it is the last update of data to HMAC.
				 * - FALSE: Not Last update.
				 * - TRUE: Last update.   */
	u8 OperationFlags; /**< Flags that determine the operation type. These can be a combination of
			XASU_INIT, XASU_UPDATE and XASU_FINISH */
	u8 Reserved[4]; /**< Reserved for 8-byte alignment */
} XAsu_HmacParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_HMACINFO_H_ */
