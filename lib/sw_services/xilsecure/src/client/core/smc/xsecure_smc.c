/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xsecure_smc.c
 *
 * This file contains the implementation of the SMC interface APIs for
 * xilsecure client library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 5.7   tbk  03/16/26 Initial release for SMC support
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xsecure_client_apis XilSecure Client APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "bspconfig.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xsecure_defs.h"
#include "xsecure_smc.h"

/************************************ Constant Definitions ***************************************/
#define XSECURE_WORD_SHIFT	(32U)	/**< To shift 32 bits */
#define XSECURE_BYTE_SHIFT	(8U)	/**< To shift 8 bits */

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
 * @param	ResponseLen	Length of response buffer (number of u32 elements)
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - Error on failure
 *
 **************************************************************************************************/
int XSecure_SmcCall(u32 *PayloadBuf, u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen)
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
	SmcArg1 = ((u64)PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_1] << XSECURE_WORD_SHIFT) |
			PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_0];
	SmcArg2 = ((u64)PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_3] << XSECURE_WORD_SHIFT) |
			PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_2];
	SmcArg3 = ((u64)PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_5] << XSECURE_WORD_SHIFT) |
			PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_4];
	SmcArg4 = PayloadBuf[XSECURE_SMC_PAYLOAD_INDEX_6];

	/** Perform SMC call */
	Out = Xil_Smc((u64)SMC_FID_EXT, SmcArg1, SmcArg2, SmcArg3, SmcArg4, 0U, 0U, 0U);

	/** Store response in provided buffer based on ResponseLen */
	if ((ResponseBuf != NULL) && (ResponseLen > 0U)) {
		ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_0] = (u32)(Out.Arg0 >> XSECURE_WORD_SHIFT);
		if (ResponseLen > XSECURE_SMC_RESPONSE_INDEX_1) {
			ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_1] = (u32)Out.Arg1;
		}
		if (ResponseLen > XSECURE_SMC_RESPONSE_INDEX_2) {
			ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_2] =
					(u32)(Out.Arg1 >> XSECURE_WORD_SHIFT);
		}
		if (ResponseLen > XSECURE_SMC_RESPONSE_INDEX_3) {
			ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_3] = (u32)Out.Arg2;
		}
		if (ResponseLen > XSECURE_SMC_RESPONSE_INDEX_4) {
			ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_4] =
					(u32)(Out.Arg2 >> XSECURE_WORD_SHIFT);
		}
		if (ResponseLen > XSECURE_SMC_RESPONSE_INDEX_5) {
			ResponseBuf[XSECURE_SMC_RESPONSE_INDEX_5] = (u32)Out.Arg3;
		}
	}

	Status = (int)Out.Arg0;

END:
	return Status;
}

/** @} end of xsecure_client_apis group */

#endif /* defined (__aarch64__) && (EL1_NONSECURE == 1) */
