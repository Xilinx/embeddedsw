/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_ipi.h"

/****************************************************************************/
/**
 * @brief  Sends IPI request to the target module
 *
 * @param  Proc  Pointer to the processor who is initiating request
 * @param  Payload API id and call arguments to be written in IPI buffer
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_IpiSend(struct XPm_Proc *const Proc, u32 *Payload)
{
	XStatus Status;

	Status = XIpiPsu_PollForAck(Proc->Ipi, TARGET_IPI_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (Status != XST_SUCCESS) {
		XPm_Dbg("%s: ERROR: Timeout expired\n", __func__);
		goto done;
	}

	Status = XIpiPsu_WriteMessage(Proc->Ipi, TARGET_IPI_INT_MASK, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		XPm_Dbg("xilpm: ERROR writing to IPI request buffer\n");
		goto done;
	}

	Status = XIpiPsu_TriggerIpi(Proc->Ipi, TARGET_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Reads IPI Response after target module has handled interrupt
 *
 * @param  Proc Pointer to the processor who is waiting and reading Response
 * @param  Val1 Used to return value from 2nd IPI buffer element (optional)
 * @param  Val2 Used to return value from 3rd IPI buffer element (optional)
 * @param  Val3 Used to return value from 4th IPI buffer element (optional)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus Xpm_IpiReadBuff32(struct XPm_Proc *const Proc, u32 *Val1,
			  u32 *Val2, u32 *Val3)
{
	u32 Response[RESPONSE_ARG_CNT];
	XStatus Status;

	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(Proc->Ipi, TARGET_IPI_INT_MASK,
				    PM_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XPm_Dbg("%s: ERROR: Timeout expired\r\n", __func__);
		goto done;
	}

	Status = XIpiPsu_ReadMessage(Proc->Ipi, TARGET_IPI_INT_MASK, Response,
				     RESPONSE_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XPm_Dbg("%s: ERROR: Reading from IPI Response buffer\r\n", __func__);
		goto done;
	}

	/*
	 * Read Response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: Val1
	 * buf-2: Val2
	 * buf-3: Val3
	 */
	if (NULL != Val1)
		*Val1 = Response[1];
	if (NULL != Val2)
		*Val2 = Response[2];
	if (NULL != Val3)
		*Val3 = Response[3];

	Status = Response[0];

done:
	return Status;
}
