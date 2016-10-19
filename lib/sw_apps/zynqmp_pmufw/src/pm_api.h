/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Definitions needed to check correctness of PM API call
 * (payload against api id, ranges of arguments, etc.)
 * Every new API call must have its pm_api_entry with the definition
 * of argument types, otherwise it will be considered irregular.
 *********************************************************************/

#ifndef PM_API_H_
#define PM_API_H_

#include "pm_common.h"

/*********************************************************************
 * Macro definitions
 ********************************************************************/
/* One (first) u32 is used for API call id coding */
#define PAYLOAD_API_ID		1U

/* Each API can have up to 5 arguments */
#define PAYLOAD_API_ARGS_CNT	5U

/* Number of payload elements (api id and api's arguments) */
#define PAYLOAD_ELEM_CNT	(PAYLOAD_API_ID + PAYLOAD_API_ARGS_CNT)

/* Payload element size in bytes */
#define PAYLOAD_ELEM_SIZE	4U

#define ARG_UNDEF              0U
#define ARG_NODE               1U
#define ARG_ACK                2U
#define ARG_ABORT_REASON       3U
#define ARG_SUSPEND_REASON     4U
#define ARG_CAPABILITIES       5U
#define ARG_OP_CH_TYPE         6U
#define ARG_STATE              7U
#define ARG_QOS                8U
#define ARG_EVENT_ID           9U
#define ARG_RESET              11U
#define ARG_LATENCY            12U
#define ARG_UINT32             13U
#define ARG_WAKE               14U
#define ARG_ENABLE             15U
#define ARG_SHUTDOWN_TYPE      16U
#define ARG_SHUTDOWN_SUBTYPE   17U

/*********************************************************************
 * Enum definitions
 ********************************************************************/
typedef enum {
	PM_PAYLOAD_OK,
	PM_PAYLOAD_ERR_API_ID,
	PM_PAYLOAD_ERR_NODE,
	PM_PAYLOAD_ERR_ACK,
	PM_PAYLOAD_ERR_ABORT_REASON,
	PM_PAYLOAD_ERR_SUSPEND_REASON,
	PM_PAYLOAD_ERR_CAPABILITIES,
	PM_PAYLOAD_ERR_OP_CH_TYPE,
	PM_PAYLOAD_ERR_STATE,
	PM_PAYLOAD_ERR_QOS,
	PM_PAYLOAD_ERR_EVENT_ID,
	PM_PAYLOAD_ERR_RESET,
	PM_PAYLOAD_ERR_LATENCY,
	PM_PAYLOAD_ERR_WAKE,
	PM_PAYLOAD_ERR_ENABLE,
	PM_PAYLOAD_ERR_SHUTDOWN_TYPE,
	PM_PAYLOAD_ERR_SHUTDOWN_SUBTYPE,
	PM_PAYLOAD_ERR_UNKNOWN,
} PmPayloadStatus;

/*********************************************************************
 * Function declarations
 ********************************************************************/
bool PmIsApiIdValid(const u32 api);
u32 PmRequestAcknowledge(const u32 *args);
PmPayloadStatus PmCheckPayload(const u32 *args);

#endif
