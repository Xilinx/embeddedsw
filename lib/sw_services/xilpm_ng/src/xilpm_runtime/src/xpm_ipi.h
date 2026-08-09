/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_IPI_H_
#define XPM_IPI_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XPLMI_IPI_DEVICE_ID
#define PAYLOAD_ARG_CNT		XIPIPSU_MAX_MSG_LEN
#define RESPONSE_ARG_CNT	XIPIPSU_MAX_MSG_LEN
#else
#define PAYLOAD_ARG_CNT		(8U)
#define RESPONSE_ARG_CNT	(8U)
#endif

#define PM_IPI_TIMEOUT			(~0U)

/****************************************************************************/
/**
 * @brief	Sends IPI request to the target module.
 *
 * @param	IpiMask		IPI interrupt mask of target
 * @param	Payload		API id and call arguments to be written in IPI
 *				buffer
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 ****************************************************************************/
XStatus XPm_IpiSend(u32 IpiMask, const u32 *Payload);
/****************************************************************************/
/**
 * @brief	Waits for the target module to acknowledge the IPI.
 *
 * @param	IpiMask		IPI interrupt mask of target
 * @param	TimeOutCount	Number of cycles to wait for response.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 ****************************************************************************/
XStatus XPm_IpiPollForAck(u32 IpiMask, u32 TimeOutCount);

#ifdef __cplusplus
}
#endif

#endif /* XPM_IPI_H_ */
