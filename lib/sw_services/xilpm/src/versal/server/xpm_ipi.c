/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_ipi.h"
#include "xpm_ipi.h"

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/****************************************************************************/
/**
 * @brief	Sends IPI request to the target module
 *
 * @param	IpiMask		IPI interrupt mask of target
 * @param	Payload		API id and call arguments to be written in IPI
 *				buffer
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPm_IpiSend(u32 IpiMask, u32 *Payload)
{
	XStatus Status = XST_FAILURE;

	Status = XPlmi_IpiPollForAck(IpiMask, PM_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmDbg("%s: ERROR: Timeout expired\n", __func__);
		goto done;
	}

	Status = XPlmi_IpiWrite(IpiMask, Payload,
				PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (XST_SUCCESS != Status) {
		PmDbg("%s: ERROR writing to IPI request buffer\n", __func__);
		goto done;
	}

	Status = XPlmi_IpiTrigger(IpiMask);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Reads IPI Response after target module has handled interrupt
 *
 * @param	IpiMask		IPI interrupt mask of target
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_IpiReadStatus(u32 IpiMask)
{
	u32 Response[RESPONSE_ARG_CNT] = {0};
	XStatus Status = XST_FAILURE;

	/* Wait until current IPI interrupt is handled by target module */
	Status = XPlmi_IpiPollForAck(IpiMask, PM_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		PmDbg("%s: ERROR: Timeout expired\r\n", __func__);
		goto done;
	}

	Status = XPlmi_IpiRead(IpiMask, Response,
			       RESPONSE_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		PmDbg("%s: ERROR: Reading from IPI Response buffer\r\n", __func__);
		goto done;
	}

	Status = (XStatus)Response[0];

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Check IPI Response
 *
 * @param	IpiMask		IPI interrupt mask of target
 * @param	TimeOutCount	Number of cycle to wait for response.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_IpiPollForAck(u32 IpiMask, u32 TimeOutCount)
{
	return XPlmi_IpiPollForAck(IpiMask, TimeOutCount);
}
#else
XStatus XPm_IpiSend(u32 IpiMask, u32 *Payload)
{
	(void)IpiMask;
	(void)Payload;

	return XST_FAILURE;
}

XStatus XPm_IpiReadStatus(u32 IpiMask)
{
	(void)IpiMask;

	return XST_FAILURE;
}

XStatus XPm_IpiPollForAck(u32 IpiMask, u32 TimeOutCount)
{
	(void)IpiMask;
	(void)TimeOutCount;

	return XST_FAILURE;
}
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
