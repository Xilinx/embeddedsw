/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#include "xpsmfw_api.h"
#include "xpsmfw_ipi_manager.h"
#include "xpsmfw_power.h"
#include "xpsmfw_update.h"

/****************************************************************************/
/**
 * @brief	Process keep alive event from PLM to indicate that PSM is alive
 *		and healthy.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
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


	Status = XST_SUCCESS;

	return Status;
}

/****************************************************************************/
/**
 * @brief	Process CDO blob which contains multiple CDO commands.
 *
 * @param CdoStartAddr	Address of the CDO blob
 * @param Length	Total length of CDO blob in word (32-bit)
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus XPsmFw_ProcessCdo(u32 CdoStartAddr, u32 Length)
{
	XStatus Status = XST_FAILURE;
	u32 CmdLength = 0U;
	u32 Len = Length;
	XPsmFw_PmCmdHeader CmdHeader;
	/*Sanity check boundary to make sure the CdoStart Address in bound of dedicated region from PSMX RAM*/
	if ((CdoStartAddr >= XPSMFW_PROC_LOCATION_ADDRESS) && \
		((CdoStartAddr + (Len * 4U)) <= (XPSMFW_PROC_LOCATION_ADDRESS + XPSMFW_PROC_LOCATION_LENGTH))) {
		u32* CmdPtr = (u32*)CdoStartAddr;
		while (0U != Len) {
			CmdHeader.Value = CmdPtr[0];
			if (XPSMFW_CDO_MAX_LENGTH == CmdHeader.Length) {
				/* This is the case of long command that beyond 255 words */
				CmdLength = CmdPtr[1];
			} else {
				CmdLength = CmdHeader.Length;
			}
			if (XPSMFW_PM_HANDLER_ID != CmdHeader.HandlerId) {
				Status = XST_FAILURE;
				break;
			}
			switch (CmdHeader.ApiId) {
			case XPSMFW_PROC_WRITE:
				if (2U > CmdLength) {
					Status = XST_FAILURE;
					Len = 0U;
					break;
				}
				XPsmFw_Write32(CmdPtr[1], CmdPtr[2]);
				Status = XST_SUCCESS;
				break;
			case XPSMFW_PROC_MASK_WRITE:
				if (3U > CmdLength) {
					Status = XST_FAILURE;
					Len = 0U;
					break;
				}
				XPsmFw_RMW32(CmdPtr[1], CmdPtr[2], CmdPtr[3]);
				Status = XST_SUCCESS;
				break;
			case XPSMFW_PROC_MASK_POLL:
				if (4U > CmdLength) {
					Status = XST_FAILURE;
					Len = 0U;
					break;
				}
				Status = XPsmFw_UtilPollForValue(CmdPtr[1], CmdPtr[2], CmdPtr[3], CmdPtr[4]);
				break;
			case XPSMFW_PROC_DELAY:
				if (1U > CmdLength) {
					Status = XST_FAILURE;
					Len = 0U;
					break;
				}
				XPsmFw_UtilWait(CmdPtr[1]);
				Status = XST_SUCCESS;
				break;
			case XPSMFW_PROC_MARKER:
				/* If command is "marker", ignore */
				Status = XST_SUCCESS;
				break;
			default:
				Status = XST_FAILURE;
				Len = 0U;
				break;
			}
			if (Len < (CmdLength + 1U)) {
				break;
			}
			Len -= (CmdLength + 1U);
			CmdPtr += (CmdLength + 1U);
		}
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	Process Isolation command that is sent from PMC
 *
 * @param Payload	IPI payload
 * @param Response	Output response to the source
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus XPsmFw_ProcessIsoCommand(const u32 *Payload, u32* Response)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = Payload[2U];
	if (0U == BaseAddress){
		goto done;
	}

	u32 Header = Payload[1U];
	u32 Mask = Payload[3U];
	u32 Value = Payload[4U];

	switch (Header){
	case PSM_API_DOMAIN_ISO_SETTER_HEADER:
		XPsmFw_RMW32(BaseAddress, Mask, Value);
		Status = XST_SUCCESS;
		break;
	case PSM_API_DOMAIN_ISO_GETTER_HEADER:
		*Response = XPsmFw_Read32(BaseAddress);
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
 * @brief	Process IPI commands
 *
 * @param Payload	API ID and call arguments
 * @param Response	Output response to the source
 *
 * @return	None
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
		case PSM_API_KEEP_ALIVE:
			Status = XPsmFw_KeepAliveEvent();
			break;
		case PSM_API_GET_PSM_TO_PLM_EVENT_ADDR:
			XPsmFw_GetPsmToPlmEventAddr(&Response[1]);
			Status = XST_SUCCESS;
			break;
		case PSM_API_CDO_PROC:
			Status = XPsmFw_ProcessCdo(Payload[1], Payload[2]);
			break;
		case PSM_API_DOMAIN_ISO:
			Status = XPsmFw_ProcessIsoCommand(Payload, &Response[1]);
			break;
		case PSM_API_SHUTDOWN_PSM:
			XPsmFw_StartShutdown();
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
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_NotifyPlmEvent(void)
{
	PsmToPlmEvent.EventInfo.PmEvent = 1U;

	return XPsmFw_IpiTrigger(IPI_PSM_IER_PMC_MASK);
}
