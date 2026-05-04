/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_smc.c
 *
 * This file contains the implementation of the SMC interface APIs for
 * xilloader client library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 2.4   tbk  02/10/26 Initial release for SMC support
 * 2.4   sms  04/16/26 Updated the Payload and Response buffer length parameters in the function
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_apis XilLoader Client APIs
 * @{
 */
/*************************************** Include Files *******************************************/
#include "bspconfig.h"
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xloader_smc.h"
#include "xloader_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_WORD_SHIFT	(32U)	/**< To shift 32 bits */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs SMC call to communicate with PLM
 *
 * @param	PayloadBuf	Pointer to payload buffer containing packed header and arguments
 * @param	PayloadLen	Length of payload buffer (number of u32 elements)
 * @param	Response	Pointer to store response values
 * @param	ResponseLen	Length of response buffer (number of u32 elements)
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - Error on failure
 *
 **************************************************************************************************/
int XLoader_SmcCall(u32 *PayloadBuf, u32 PayloadLen, u32 *ResponseBuf, u32 ResponseLen)
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
	SmcArg1 = ((u64)PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_1] << XLOADER_WORD_SHIFT) |
		PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_0];
	SmcArg2 = ((u64)PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_3] << XLOADER_WORD_SHIFT) |
		PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_2];
	SmcArg3 = ((u64)PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_5] << XLOADER_WORD_SHIFT) |
		PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_4];
	SmcArg4 = PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_6];

	/** Perform SMC call */
	Out = Xil_Smc((u64)SMC_FID_EXT, SmcArg1, SmcArg2, SmcArg3, SmcArg4, 0U, 0U, 0U);

	/** Store response in provided buffer based on ResponseLen */
	if ((ResponseBuf != NULL) && (ResponseLen > 0U)) {
		ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_0] = (u32)(Out.Arg0 >> XLOADER_WORD_SHIFT);
		if (ResponseLen > XLOADER_SMC_RESPONSE_INDEX_1) {
			ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_1] = (u32)Out.Arg1;
		}
		if (ResponseLen > XLOADER_SMC_RESPONSE_INDEX_2) {
			ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_2] = (u32)(Out.Arg1 >> XLOADER_WORD_SHIFT);
		}
		if (ResponseLen > XLOADER_SMC_RESPONSE_INDEX_3) {
			ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_3] = (u32)Out.Arg2;
		}
		if (ResponseLen > XLOADER_SMC_RESPONSE_INDEX_4) {
			ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_4] = (u32)(Out.Arg2 >> XLOADER_WORD_SHIFT);
		}
		if (ResponseLen > XLOADER_SMC_RESPONSE_INDEX_5) {
			ResponseBuf[XLOADER_SMC_RESPONSE_INDEX_5] = (u32)Out.Arg3;
		}
	}

	Status = (int)Out.Arg0;

END:
	return Status;
}

/** @} end of xloader_client_apis group */

#endif /* defined (__aarch64__) && (EL1_NONSECURE == 1) */
