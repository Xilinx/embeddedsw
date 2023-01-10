/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xpsmfw_api.h"
#include "xpsmfw_ipi_manager.h"
#include "xpsmfw_power.h"
#include "xpsmfw_stl.h"
#include "xpsmfw_dvsec_common.h"

#define PACK_PAYLOAD(Payload, Arg0, Arg1)	\
	Payload[0] = (u32)Arg0;		\
	Payload[1] = (u32)Arg1;         \
	XPsmFw_Printf(DEBUG_DETAILED, "%s(%x)\r\n", __func__, Arg1);

#define XILPM_MODULE_ID			(0x06U)
#define HEADER(len, ApiId)		(u32)(((u32)len << 16) |		\
					      ((u32)XILPM_MODULE_ID << 8) |	\
					      (ApiId))

#define PACK_PAYLOAD0(Payload, ApiId)	\
	PACK_PAYLOAD(Payload, HEADER(0U, ApiId), 0U)

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
		XPsmFw_FpdPostHouseClean();
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_BISR:
		XPsmFw_FpdMbisr();
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_MBIST_CLEAR:
		XPsmFw_FpdMbistClear();
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_SECLOCKDOWN:
		XPsmFw_SecLockDown();
		Status = XST_SUCCESS;
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
 * @brief	Process keep alive event from PLM to indicate that PSM is alive
 * 		and healthy.
 *
 * @return	XST_SUCCESS
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus XPsmFw_KeepAliveEvent(void)
{
	XStatus Status = XST_FAILURE;
	u32 PsmKeepAliveCounter;

	/* Ack the IPI interrupt first */
	XPsmFw_Write32(IPI_PSM_ISR_ADDR, PMC_IPI_BIT);

	/* Read keep alive counter value from RTCA register */
	PsmKeepAliveCounter = XPsmFw_Read32(PSM_KEEP_ALIVE_COUNTER_ADDR);
	/* Increment keep alive counter value */
	PsmKeepAliveCounter++;
	/* Write incremented keep alive counter value in RTCA register */
	XPsmFw_Write32(PSM_KEEP_ALIVE_COUNTER_ADDR, PsmKeepAliveCounter);


#ifdef PSM_ENABLE_STL
	Status = XPsmFw_PeriodicStlHook();
#else
	Status = XST_SUCCESS;
#endif

	return Status;
}

/****************************************************************************/
/**
 * @brief	Enable/Disable Isolation
 *
 * @param IsolationId	Isolation index
 * @param Action	True - To enable Isolation
 * 			False - To disable Isolation
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus XPsmFw_DomainIso(u32 IsolationIdx, u32 Action)
{
	XStatus Status = XST_FAILURE;

	if (TRUE == Action) {
		if (XPSMFW_NODEIDX_ISO_CPM5_LPD_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD_DFX,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD_DFX);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_LPD == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_PL == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_PL_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL_DFX,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL_DFX);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_GT == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_GT_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT_DFX,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT_DFX);
		} else {
			XPsmFw_Printf(DEBUG_ERROR, "Iso Idx:0x%x not identified\n\r",
			   IsolationIdx);
			goto done;
		}
	} else if (FALSE == Action) {
		if (XPSMFW_NODEIDX_ISO_CPM5_LPD_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD_DFX, 0U);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_LPD == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_LPD, 0U);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_PL == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL, 0U);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_PL_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_PL_DFX, 0U);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_GT == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT, 0U);
		} else if (XPSMFW_NODEIDX_ISO_CPM5_GT_DFX == IsolationIdx) {
			XPsmFw_RMW32(PSM_LOCAL_MISC_CNTRL,
				     PSM_LOCAL_MISC_CNTRL_CPM5_GT_DFX, 0U);
		} else {
			XPsmFw_Printf(DEBUG_ERROR, "Iso Idx:0x%x not identified\n\r",
			   IsolationIdx);
			goto done;
		}
	} else {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Action: 0x%x not defined\r\n",
			      __func__, Action);
		goto done;
	}

	Status = XST_SUCCESS;
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
void XPsmFw_ProcessIpi(const u32 *Payload, u32 *Response)
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
		case PSM_API_CCIX_EN:
			Status = XPsmFw_DvsecEnable(Payload[1], Payload[2]);
			break;
		case PSM_API_KEEP_ALIVE:
			Status = XPsmFw_KeepAliveEvent();
			break;
		case PSM_API_DOMAIN_ISO:
			Status = XPsmFw_DomainIso(Payload[1], Payload[2]);
			break;
		case PSM_API_GET_PSM_TO_PLM_EVENT_ADDR:
			XPsmFw_GetPsmToPlmEventAddr(&Response[1]);
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	Response[0] = (u32)Status;

	return;
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
