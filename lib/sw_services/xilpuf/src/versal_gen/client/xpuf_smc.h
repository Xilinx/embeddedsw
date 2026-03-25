/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xpuf_smc.h
 *
 * This file Contains the SMC function prototypes, defines and macros for xilpuf client library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------------------------
 * 2.7   tbk  03/20/26 Initial release for SMC support
 *
 * </pre>
 *
*************************************************************************************************/

#ifndef XPUF_SMC_H
#define XPUF_SMC_H

#if defined (__aarch64__) && (EL1_NONSECURE == 1)

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xil_smc.h"
#include "xpuf_defs.h"
#include "xstatus.h"

/************************************ Constant Definitions ***************************************/
/** Payload buffer index macros */
#define XPUF_SMC_PAYLOAD_INDEX_0	(0U)	/**< Payload buffer index 0 */
#define XPUF_SMC_PAYLOAD_INDEX_1	(1U)	/**< Payload buffer index 1 */
#define XPUF_SMC_PAYLOAD_INDEX_2	(2U)	/**< Payload buffer index 2 */
#define XPUF_SMC_PAYLOAD_INDEX_3	(3U)	/**< Payload buffer index 3 */
#define XPUF_SMC_PAYLOAD_INDEX_4	(4U)	/**< Payload buffer index 4 */
#define XPUF_SMC_PAYLOAD_INDEX_5	(5U)	/**< Payload buffer index 5 */
#define XPUF_SMC_PAYLOAD_INDEX_6	(6U)	/**< Payload buffer index 6 */

/** Response array index macros */
#define XPUF_SMC_RESPONSE_INDEX_0	(0U)	/**< Response array index 0 */
#define XPUF_SMC_RESPONSE_INDEX_1	(1U)	/**< Response array index 1 */
#define XPUF_SMC_RESPONSE_INDEX_2	(2U)	/**< Response array index 2 */
#define XPUF_SMC_RESPONSE_INDEX_3	(3U)	/**< Response array index 3 */
#define XPUF_SMC_RESPONSE_INDEX_4	(4U)	/**< Response array index 4 */
#define XPUF_SMC_RESPONSE_INDEX_5	(5U)	/**< Response array index 5 */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

int XPuf_SmcCall(u32 *PayloadBuf, u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen);

#ifdef __cplusplus
}
#endif

#endif /* defined (__aarch64__) && (EL1_NONSECURE == 1) */

#endif  /* XPUF_SMC_H */
