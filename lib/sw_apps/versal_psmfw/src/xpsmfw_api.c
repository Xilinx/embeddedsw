/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xpsmfw_api.h"
#include "xpsmfw_ipi_manager.h"
#include "xpsmfw_power.h"

#define PACK_PAYLOAD(Payload, Arg0, Arg1)	\
	Payload[0] = (u32)Arg0;		\
	Payload[1] = (u32)Arg1;         \
	XPsmFw_Printf(DEBUG_DETAILED, "%s(%x)\r\n", __func__, Arg1);

#define LIBPM_MODULE_ID			(0x06U)
#define HEADER(len, ApiId)		(u32)(((u32)len << 16) |		\
					      ((u32)LIBPM_MODULE_ID << 8) |	\
					      (ApiId))

#define PACK_PAYLOAD0(Payload, ApiId)	\
	PACK_PAYLOAD(Payload, HEADER(0U, ApiId), 0U)
#define PACK_PAYLOAD1(Payload, ApiId, Arg1)	\
	PACK_PAYLOAD(Payload, HEADER(1U, ApiId), Arg1)

static XStatus XPsmFw_FpHouseClean(u32 FunctionId)
{
	XStatus Status = XST_FAILURE;

	switch (FunctionId) {
	case (u32)FUNC_INIT_START:
		Status = XPsmFw_FpdPreHouseClean();
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)FUNC_INIT_FINISH:
		Status = XPsmFw_FpdPostHouseClean();
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)FUNC_SCAN_CLEAR:
		Status = XPsmFw_FpdScanClear();
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)FUNC_BISR:
		Status = XPsmFw_FpdMbisr();
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case (u32)FUNC_MBIST_CLEAR:
		Status = XPsmFw_FpdMbistClear();
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Process IPI commands
 *
 * @param Payload	API ID and call arguments
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_ProcessIpi(u32 *Payload)
{
	XStatus Status = XST_FAILURE;
	u32 ApiId = Payload[0];

	switch (ApiId) {
		case PSM_API_DIRECT_PWR_DWN:
			Status = XPsmFw_DirectPwrDwn(Payload[1]);
			break;
		case PSM_API_DIRECT_PWR_UP:
			Status = XPsmFw_DirectPwrUp(Payload[1]);
			break;
		case PSM_API_FPD_HOUSECLEAN:
			Status = XPsmFw_FpHouseClean(Payload[1]);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Trigger IPI of PLM to notify the event to PLM
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_NotifyPlmEvent(void)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD0(Payload, PM_PSM_TO_PLM_EVENT);

	Status = XPsmFw_IpiSend(IPI_PSM_IER_PMC_MASK, Payload);

	return Status;
}
