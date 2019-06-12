/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
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
	XStatus Status;

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
	XStatus Status;

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

	Status = Response[0];

done:
	return Status;
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
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
