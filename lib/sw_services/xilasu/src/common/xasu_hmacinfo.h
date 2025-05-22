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

/* HMAC operation mode */
#define XASU_HMAC_INIT				(0x1U) /**< HMAC init operation flag */
#define XASU_HMAC_UPDATE			(0x2U) /**< HMAC update operation flag */
#define XASU_HMAC_FINAL				(0x4U) /**< HMAC final operation flag */

#define XASU_HMAC_MAX_KEY_LENGTH		(0x1024U) /**< Max key length for HMAC. */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains HMAC params info. */
typedef struct {
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_SHA256 / XASU_SHA_MODE_SHA384 / XASU_SHA_MODE_SHA512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 IsLast; /**< Indicates whether it is the last update of data to HMAC.
				 * - FALSE: Not Last update.
				 * - TRUE: Last update.   */
	u8 OperationFlags; /**< Flags that determine the operation type. These can be a combination of
			XASU_HMAC_INIT, XASU_HMAC_UPDATE and XASU_HMAC_FINAL */
	u32 KeyLen; /**< Length of the key */
	u32 MsgLen; /**< Length of the message to be processed. MsgLen can be 0 <= MsgLen < ((2^B)-8B).
			 Where B is the block length of the selected SHA type and SHA mode. */
	u32 HmacLen; /**< Length of the HMAC to be generated */
	u64 KeyAddr; /**< Key address */
	u64 MsgBufferAddr; /**< Address of the message buffer */
	u64 HmacAddr; /**< Address of the output buffer to store the generated HMAC */
} XAsu_HmacParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_HMACINFO_H_ */
