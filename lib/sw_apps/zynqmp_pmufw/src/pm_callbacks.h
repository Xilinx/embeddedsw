/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * PM callbacks interface.
 * Used by the power management to send a message to the PM master and
 * generate interrupt using IPI.
 *********************************************************************/

#ifndef PM_CALLBACKS_H_
#define PM_CALLBACKS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_master.h"
#include "xil_types.h"

#define IPI_REQUEST1(mask, arg0)				\
{	\
	u32 _ipi_req_data[] = {(arg0), 0U, 0U, 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteMessage(PmModPtr, (mask),		\
						&_ipi_req_data[0],		\
						ARRAY_SIZE(_ipi_req_data))) {	\
		PmWarn("Error in IPI write message\r\n");			\
	}									\
}

#define IPI_REQUEST2(mask, arg0, arg1)				\
{	\
	u32 _ipi_req_data[] = {(arg0), (arg1), 0U, 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteMessage(PmModPtr, (mask),		\
						&_ipi_req_data[0],		\
						ARRAY_SIZE(_ipi_req_data))) {	\
		PmWarn("Error in IPI write message\r\n");			\
	}									\
}

#define IPI_REQUEST3(mask, arg0, arg1, arg2)			\
{	\
	u32 _ipi_req_data[] = {(arg0), (arg1), (arg2), 0U, 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteMessage(PmModPtr, (mask),		\
						&_ipi_req_data[0],		\
						ARRAY_SIZE(_ipi_req_data))) {	\
		PmWarn("Error in IPI write message\r\n");			\
	}									\
}

#define IPI_REQUEST4(mask, arg0, arg1, arg2, arg3)		\
{	\
	u32 _ipi_req_data[] = {(arg0), (arg1), (arg2), (arg3), 0U, 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteMessage(PmModPtr, (mask),		\
						&_ipi_req_data[0],		\
						ARRAY_SIZE(_ipi_req_data))) {	\
		PmWarn("Error in IPI write message\r\n");			\
	}									\
}

#define IPI_REQUEST5(mask, arg0, arg1, arg2, arg3, arg4)	\
{	\
	u32 _ipi_req_data[] = {(arg0), (arg1), (arg2), (arg3), (arg4), 0U, 0U, 0U};	\
	if (XST_SUCCESS != XPfw_IpiWriteMessage(PmModPtr, (mask),		\
						&_ipi_req_data[0],		\
						ARRAY_SIZE(_ipi_req_data))) {	\
		PmWarn("Error in IPI write message\r\n");			\
	}									\
}

void PmAcknowledgeCb(const PmMaster* const master, const PmNodeId nodeId,
		     const u32 status, const u32 oppoint);

void PmNotifyCb(const PmMaster* const master, const PmNodeId nodeId,
		const u32 event, const u32 oppoint);

void PmInitSuspendCb(const PmMaster* const master, const u32 reason,
		     const u32 latency, const u32 state, const u32 timeout);


#ifdef __cplusplus
}
#endif

#endif /* PM_CALLBACKS_H_ */
