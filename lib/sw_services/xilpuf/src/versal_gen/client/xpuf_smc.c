/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xpuf_smc.c
 *
 * This file contains the implementation of the SMC interface APIs for
 * xilpuf client library.
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

/**
 * @addtogroup xpuf_client_apis XilPuf Client APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "bspconfig.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xpuf_smc.h"
#include "xpuf_mailbox.h"

/************************************ Constant Definitions ***************************************/
#define XPUF_WORD_SHIFT		(32U)	/**< To shift 32 bits */
#define XPUF_BYTE_SHIFT		(8U)	/**< To shift 8 bits */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs SMC call to communicate with PLM
 *
 * @param	PayloadBuf	Pointer to payload buffer containing ApiId and arguments
 * @param	PayloadLen	Length of payload buffer (number of u32 elements)
 * @param	ResponseBuf	Pointer to store response values
 * @param	ResponseLen	Length of response buffer (pass 0U when ResponseBuf is NULL)
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - Error on failure
 *
*************************************************************************************************/
int XPuf_SmcCall(u32 *PayloadBuf, u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen)
{
	volatile int Status = XST_FAILURE;
	XSmc_OutVar Out;
	u64 SmcArg1;
	u64 SmcArg2;
	u64 SmcArg3;
	u64 SmcArg4;

	/** Validate input parameters */
	if ((PayloadBuf == NULL) || (PayloadLen == 0U)) {
		goto END;
	}

	/** Prepare payload using extended format */
	SmcArg1 = ((u64)PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_1] << XPUF_WORD_SHIFT) |
		 PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_0];
	SmcArg2 = ((u64)PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_3] << XPUF_WORD_SHIFT) |
		PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_2];
	SmcArg3 = ((u64)PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_5] << XPUF_WORD_SHIFT) |
		PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_4];
	SmcArg4 = PayloadBuf[XPUF_SMC_PAYLOAD_INDEX_6];

	/** Perform SMC call */
	Out = Xil_Smc((u64)SMC_FID_EXT, SmcArg1, SmcArg2, SmcArg3, SmcArg4, 0U, 0U, 0U);

	/** Store response in provided buffer based on ResponseLen */
	if ((ResponseBuf != NULL) && (ResponseLen > 0U)) {
		ResponseBuf[XPUF_SMC_RESPONSE_INDEX_0] =
					(u32)(Out.Arg0 >> XPUF_WORD_SHIFT);
		if (ResponseLen > XPUF_SMC_RESPONSE_INDEX_1) {
			ResponseBuf[XPUF_SMC_RESPONSE_INDEX_1] = (u32)Out.Arg1;
		}
		if (ResponseLen > XPUF_SMC_RESPONSE_INDEX_2) {
			ResponseBuf[XPUF_SMC_RESPONSE_INDEX_2] =
					(u32)(Out.Arg1 >> XPUF_WORD_SHIFT);
		}
		if (ResponseLen > XPUF_SMC_RESPONSE_INDEX_3) {
			ResponseBuf[XPUF_SMC_RESPONSE_INDEX_3] = (u32)Out.Arg2;
		}
		if (ResponseLen > XPUF_SMC_RESPONSE_INDEX_4) {
			ResponseBuf[XPUF_SMC_RESPONSE_INDEX_4] =
					(u32)(Out.Arg2 >> XPUF_WORD_SHIFT);
		}
		if (ResponseLen > XPUF_SMC_RESPONSE_INDEX_5) {
			ResponseBuf[XPUF_SMC_RESPONSE_INDEX_5] = (u32)Out.Arg3;
		}
	}

	Status = (int)Out.Arg0;

END:
	return Status;
}

/** @} end of xpuf_client_apis group */

#endif /* defined (__aarch64__) && (EL1_NONSECURE == 1) */
