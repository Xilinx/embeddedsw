/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_trng.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the implementation of the client interface functions for
 * TRNG driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/27/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_def.h"
#include "xasu_client.h"
#include "xasu_status.h"
#include "xasu_trnginfo.h"
#include "xasu_trng.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends request to ASU, to get random number from TRNG
 *
 * @param	ClientParamPtr  Pointer to the XAsu_ClientParams structure which holds
 *              the client input arguments.
 * @param	BufPtr          Pointer to the buffer to store the random number.
 * @param       Length          Length of the random number to be stored in buffer.
 *
 * @return
 *	        - XST_SUCCESS - upon successful request to asu
 *	        - XST_FAILURE - If there is a failure
 *
 *************************************************************************************************/
s32 XAsu_TrngGetRandomNum(XAsu_ClientParams *ClientParamPtr, u8 *BufPtr, u32 Length)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBuf;
	XAsu_QueueInfo *QueueInfo;

	/* Validatations of inputs */
	if ((ClientParamPtr->Priority != XASU_PRIORITY_HIGH) &&
	    (ClientParamPtr->Priority != XASU_PRIORITY_LOW)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}
	if ((BufPtr == NULL) || (Length == 0U) || (Length > XASU_TRNG_RANDOM_NUM_IN_BYTES)) {
		Status = XASU_INVALID_ARGUMENT;
		goto END;
	}

	QueueInfo = XAsu_GetQueueInfo(ClientParamPtr->Priority);
	if (QueueInfo == NULL) {
		goto END;
	}
	/* Get Queue memory */
	QueueBuf = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBuf == NULL) {
		goto END;
	}

	QueueBuf->ReqBuf.Header = XAsu_CreateHeader(XASU_TRNG_GET_RANDOM_BYTES_CMD_ID, 0U,
				  XASU_MODULE_TRNG_ID, 0U);

	Status = XAsu_UpdateQueueBufferNSendIpi(QueueInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Update requested buffer with the data */
	Status = Xil_SMemCpy(BufPtr, Length, &QueueBuf->RespBuf.Arg[1],
			     XASU_TRNG_RANDOM_NUM_IN_BYTES, XASU_TRNG_RANDOM_NUM_IN_BYTES);

END:
	return Status;
}
