/******************************************************************************
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_notifier_plat.h"
#include "xplmi_err_common.h"
#include "xplmi_err.h"
#include "xpm_subsystem.h"

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM

#include "xplmi_ssit.h"

/* Timeout for event completion (in microseconds) */
#define TIMEOUT_API_COMPL	(10000U)

static void XPmNotifier_SingleEamEventHandler(u32 ErrNodeId, u32 RegMask)
{
	int Status = XST_FAILURE;
	u32 SlrIndex = XPlmi_GetSlrIndex();

	XPlmi_ErrPrintToLog(ErrNodeId, RegMask);

	/* Trigger the SSIT Single EAM Event on secondary SLRs */
	Status = XPlmi_SsitTriggerEvent(XPLMI_SSIT_MASTER_SLR_INDEX,
			(u32)XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX);
	if (Status != XST_SUCCESS) {
		PmErr("SLR %u: 0x%x\r\n", SlrIndex, Status);
	}
}

XStatus XPmNotifier_PlatHandleSsit(u32 SubsystemId, u32 NodeId, u32 Event, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	(void)SubsystemId;

	if ((XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_ID != NodeId) ||
	    (XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK != (Event & XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK))) {
		/* N/A, so return success */
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * On primary SLR,
	 * Broadcast a registration of applicable EAM event to secondary SLRs
	 * and set an action for it
	 */
	if (XPLMI_SSIT_MASTER_SLR_INDEX == XPlmi_GetSlrIndex()) {
		u32 ReqBuf[6U] = { 0 };
		u32 RespBuf[2U] = { 0 };
		u32 SlrIndex = 0U;

		/*
		 * Event mask can contain more than one error masks,
		 * forward only XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK
		 */
		XPM_PACK_PAYLOAD4(ReqBuf, PM_REGISTER_NOTIFIER, NodeId,
				XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK, 0U, Enable);
		XPM_HEADER_SET_CMDTYPE(ReqBuf, XPLMI_CMD_SECURE);

		for (u32 SlvMask = XPlmi_GetSlavesSlrMask(); (SlvMask & 0x1U) != 0U; SlvMask >>= 1U) {
			++SlrIndex;
			/* Forward message event to secondary SLR */
			Status = XPlmi_SsitSendMsgEventAndGetResp((u8)SlrIndex,
					ReqBuf, ARRAY_SIZE(ReqBuf), RespBuf, ARRAY_SIZE(RespBuf),
					TIMEOUT_API_COMPL);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			Status = (XStatus)RespBuf[0];	/* First word contains status */
		}
	} else {
		/*
		 * On secondary SLR,
		 * Execute PM_REGISTER_NOTIFIER as EM_SET_ACTION
		 */
		if (0U != Enable) {
			/* Registration: enable action */
			Status = XPlmi_EmSetAction(NodeId, Event, XPLMI_EM_ACTION_CUSTOM,
						   XPmNotifier_SingleEamEventHandler, INVALID_SUBSYSID);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			/* De-registration: disable action */
			Status = XPlmi_EmDisable(NodeId, Event);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

done:
	return Status;
}

#else

XStatus XPmNotifier_PlatHandleSsit(u32 SubsystemId, u32 NodeId, u32 Event, u32 Enable)
{
	XStatus Status = XST_SUCCESS;
	(void)SubsystemId;
	(void)NodeId;
	(void)Event;
	(void)Enable;

	return Status;
}

#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */
