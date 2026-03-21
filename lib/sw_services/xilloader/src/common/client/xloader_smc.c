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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_WORD_SHIFT	(32U)	/**< To shift 32 bits */
#define XLOADER_BYTE_SHIFT	(8U)	/**< To shift 8 bits */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function performs SMC call to communicate with PLM
 *
 * @param	PayloadBuf	Pointer to payload buffer containing ApiId and arguments
 * @param	PayloadLen	Length of payload buffer (number of u32 elements)
 * @param	Response	Pointer to store response values
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - Error on failure
 *
 **************************************************************************************************/
int XLoader_SmcCall(u32 *PayloadBuf, u32 PayloadLen, u32 *Response)
{
	volatile int Status = XST_FAILURE;
	XSmc_OutVar Out;
	u64 SmcArg1;
	u64 SmcArg2;
	u64 SmcArg3;
	u64 SmcArg4;
	u32 ApiId;
	u32 Args[SMC_REQUEST_LEN] = {0U};
	u32 Index;

	/** Validate input parameters */
	if ((PayloadBuf == NULL) || (PayloadLen == 0U) || (PayloadLen > (SMC_REQUEST_LEN + 1U))) {
		goto END;
	}

	/** Extract ApiId from first element */
	ApiId = PayloadBuf[XLOADER_SMC_PAYLOAD_INDEX_0];

	/** Copy remaining arguments to local args array */
	for (Index = XLOADER_SMC_PAYLOAD_INDEX_1; Index < PayloadLen; Index++) {
		Args[Index - XLOADER_SMC_PAYLOAD_INDEX_1] = PayloadBuf[Index];
	}

	/** Prepare payload using extended format */
	SmcArg1 = ((u64)Args[XLOADER_SMC_PAYLOAD_INDEX_0] << XLOADER_WORD_SHIFT) |
		(((u32)XILLOADER_MODULE_ID << XLOADER_BYTE_SHIFT) | ApiId);
	SmcArg2 = ((u64)Args[XLOADER_SMC_PAYLOAD_INDEX_2] << XLOADER_WORD_SHIFT) |
		Args[XLOADER_SMC_PAYLOAD_INDEX_1];
	SmcArg3 = ((u64)Args[XLOADER_SMC_PAYLOAD_INDEX_4] << XLOADER_WORD_SHIFT) |
		Args[XLOADER_SMC_PAYLOAD_INDEX_3];
	SmcArg4 = Args[XLOADER_SMC_PAYLOAD_INDEX_5];

	/** Perform SMC call */
	Out = Xil_Smc((u64)SMC_FID_EXT, SmcArg1, SmcArg2, SmcArg3, SmcArg4, 0U, 0U, 0U);

	/** Store response in provided buffer */
	if (Response != NULL) {
		Response[XLOADER_SMC_RESPONSE_INDEX_0] = (u32)(Out.Arg0 >> XLOADER_WORD_SHIFT);
		Response[XLOADER_SMC_RESPONSE_INDEX_1] = (u32)Out.Arg1;
		Response[XLOADER_SMC_RESPONSE_INDEX_2] = (u32)(Out.Arg1 >> XLOADER_WORD_SHIFT);
		Response[XLOADER_SMC_RESPONSE_INDEX_3] = (u32)Out.Arg2;
		Response[XLOADER_SMC_RESPONSE_INDEX_4] = (u32)(Out.Arg2 >> XLOADER_WORD_SHIFT);
		Response[XLOADER_SMC_RESPONSE_INDEX_5] = (u32)Out.Arg3;
	}

	Status = (int)Out.Arg0;

END:
	return Status;
}

/** @} end of xloader_client_apis group */

#endif /* defined (__aarch64__) && (EL1_NONSECURE == 1) */
