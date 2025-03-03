/******************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_resourcehandling.h
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   pre  03/02/2025 Initial release
 *
 * </pre>
 *
 ******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
#ifndef XSECURE_RESOURCEHANDLING_H_
#define XSECURE_RESOURCEHANDLING_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes.h"
#include "xsecure_sha_common.h"
#include "xplmi_scheduler.h"
#include "xsecure_sha_ipihandler.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/
#define XSECURE_IPI_FIRST_PACKET_MASK		(0x40000000U)
					/**< IPI First packet Mask */
#define XSECURE_EVENT_PENDING (1U) /**< Indicates event pending */
#define XSECURE_EVENT_CLEAR (0U) /**< Indicates no event  */
#define XSECURE_2SEC_INTREMSOF_10MSEC (200U) /*< 2seconds Timeout value in terms of 10msec */
#define XSECURE_TIMEOUT_CLEAR         (0U) /**< Timeout clear */

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_GetAesState(XSecure_AesState *AesState);
int XSecure_GetShaState(XSecure_ShaState *ShaState, XPlmi_CoreType Core);
#ifndef PLM_SECURE_EXCLUDE
int XSecure_MakeAesFree(void);
int XSecure_AesIpiEventHandling(XPlmi_Cmd *Cmd);
#endif
int XSecure_MakeShaFree(XPlmi_CoreType Core);
int XSecure_QueuesAndTaskInit(XSecure_PartialPdiEventParams *PpdiEventParamsPtr);
int XSecure_ShaIpiEventHandling(XPlmi_Cmd *Cmd, XPlmi_CoreType Core);
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RESOURCEHANDLING_H_ */
/** @} */
