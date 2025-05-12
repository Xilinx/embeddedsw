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
 * ----- ---- ---------- --------------------------------------------------------------------------
 * 1.0   pre  03/02/2025 Initial release
 *       pre  05/10/2025 Added AES and SHA events queuing mechanism under XPLMI_IPI_DEVICE_ID macro
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
#include "xplmi_config.h"

/************************** Constant Definitions *****************************/
#define XSECURE_IPI_FIRST_PACKET_MASK		(0x40000000U)
					/**< IPI First packet Mask */
#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET)\
     && defined(XPLMI_IPI_DEVICE_ID))
#define XSECURE_EVENT_PENDING (1U) /**< Indicates event pending */
#define XSECURE_EVENT_CLEAR (0U) /**< Indicates no event  */
#define XSECURE_2SEC_INTREMSOF_10MSEC (200U) /*< 2seconds Timeout value in terms of 10msec */
#define XSECURE_TIMEOUT_CLEAR         (0U) /**< Timeout clear */

/**************************** Type Definitions *******************************/
typedef enum {
	XSECURE_RES_FREE = 0, /**< Resource free */
	XSECURE_RES_BUSY, /**< Resource busy */
} XSecure_ResourceSts;

/************************** Function Prototypes ******************************/
int XSecure_QueuesAndTaskInit(XSecure_PartialPdiEventParams *PpdiEventParamsPtr);
int XSecure_NotifyIpiEvent(u32 BufIndex, XPlmi_CoreType Core);
int XSecure_TriggerIpiEvent(XPlmi_CoreType Core);
int XSecure_GetShaAndAesSts(XSecure_ResourceSts *ResourceSts);
#endif
int XSecure_MakeResFree(XPlmi_CoreType Core);
int XSecure_IpiEventHandling(XPlmi_Cmd *Cmd, XPlmi_CoreType Core);
int XSecure_SetDataContextLost(XPlmi_CoreType Core);
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RESOURCEHANDLING_H_ */
/** @} */
