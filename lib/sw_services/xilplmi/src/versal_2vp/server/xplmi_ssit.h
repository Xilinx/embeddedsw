/***************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xplmi_ssit.h
*
* This file contains declarations for the non-SSIT (mono) dummy SSIT
* PLM-to-PLM communication functions for versal_2vp.
* xplmi_plat.c provides no-op implementations (used when
* PLM_ENABLE_PLM_TO_PLM_COMM is undefined) so that precompiled
* libraries which call these functions unconditionally
* (e.g. XilSEM) can be linked.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   tvp  07/07/25 Initial release
* 2.4   rama 09/16/26 Added non-static dummy SSIT function
*                     declarations for XilSEM linkage
*
* </pre>
*
***************************************************************************************************/

#ifndef XPLMI_SSIT_H
#define XPLMI_SSIT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************ Constant Definitions ****************************************/
#include "xplmi.h"

/**
 * SSIT PLM-PLM communication related event handler definition
 */
typedef int (*XPlmi_EventHandler_t)(void *Data);

/***************************** Function Prototypes ************************************************/
u8 XPlmi_GetSlrIndex(void);
u32 XPlmi_GetSlavesSlrMask(void);
int XPlmi_SsitRegisterEvent(u32 EventIndex, XPlmi_EventHandler_t Handler, u8 EventOrigin);
int XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(u8 SlrIndex, u32* ReqBuf, u32 ReqBufSize);
int XPlmi_SsitReadEventBuffer(u32* ReqBuf, u32 ReqBufSize);
int XPlmi_SsitTriggerEvent(u8 SlrIndex, u32 EventIndex);
int XPlmi_SsitWaitForEvent(u8 SlrIndex, u32 EventIndex, u32 TimeOut);
int XPlmi_SsitReadResponse(u8 SlrIndex, u32* RespBuf, u32 RespBufSize);
int XPlmi_SsitWriteResponseAndAckMsgEvent(u32 *RespBuf, u32 RespBufSize);
int XPlmi_SsitAcknowledgeEvent(u8 SlrIndex, u32 EventIndex);
u64 XPlmi_SsitGetSlrAddr(u32 Address, u8 SlrIndex);
int XPlmi_SsitSendMsgEventAndGetResp(u8 SlrIndex, u32 *ReqBuf, u32 ReqBufSize,
		u32 *RespBuf, u32 RespBufSize, u32 WaitForEventCompletion);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_SSIT_H */
