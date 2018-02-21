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
#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * PM API calls definitions: api ids, arguments and functions used for
 * checking correctness of API calls received through IPI.
 * Every implemented PM API must have its entry in pm_api_table.
 * Whenever a PM request is made, PMU PM compares the arguments read
 * from IPI buffer, against the definition in PM API table defined in
 * this file.
 *********************************************************************/

#include "pm_api.h"
#include "pm_defs.h"

typedef struct {
	const u8 apiId;
	const u8 argTypes[PAYLOAD_API_ARGS_CNT];
} PmApiEntry;

static const PmApiEntry pmApiTable[] = {
	{
		.apiId = PM_SELF_SUSPEND,
		.argTypes = { ARG_NODE, ARG_LATENCY, ARG_STATE, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_REQUEST_SUSPEND,
		.argTypes = { ARG_NODE, ARG_ACK, ARG_LATENCY, ARG_STATE,
			      ARG_UNDEF }
	}, {
		.apiId = PM_FORCE_POWERDOWN,
		.argTypes = { ARG_NODE, ARG_ACK, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_ABORT_SUSPEND,
		.argTypes = { ARG_ABORT_REASON, ARG_NODE, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_REQUEST_WAKEUP,
		.argTypes = { ARG_NODE, ARG_UINT32, ARG_UINT32, ARG_ACK,
			      ARG_UNDEF }
	}, {
		.apiId = PM_SET_WAKEUP_SOURCE,
		.argTypes = { ARG_NODE, ARG_NODE, ARG_ENABLE, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_SYSTEM_SHUTDOWN,
		.argTypes = { ARG_SHUTDOWN_TYPE, ARG_SHUTDOWN_SUBTYPE,
			      ARG_UNDEF, ARG_UNDEF, ARG_UNDEF }
	}, {
		.apiId = PM_REQUEST_NODE,
		.argTypes = { ARG_NODE, ARG_CAPABILITIES, ARG_QOS, ARG_ACK,
			      ARG_UNDEF }
	}, {
		.apiId = PM_RELEASE_NODE,
		.argTypes = { ARG_NODE, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_SET_REQUIREMENT,
		.argTypes = {ARG_NODE, ARG_CAPABILITIES, ARG_QOS, ARG_ACK,
			     ARG_UNDEF }
	}, {
		.apiId = PM_SET_MAX_LATENCY,
		.argTypes = {ARG_NODE, ARG_LATENCY, ARG_UNDEF, ARG_UNDEF,
			     ARG_UNDEF }
	}, {
		.apiId = PM_GET_API_VERSION,
		.argTypes = { ARG_UNDEF, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_SET_CONFIGURATION,
		.argTypes = { ARG_UINT32, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_GET_NODE_STATUS,
		.argTypes = { ARG_NODE, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_GET_OP_CHARACTERISTIC,
		.argTypes = { ARG_NODE, ARG_OP_CH_TYPE, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_REGISTER_NOTIFIER,
		.argTypes = { ARG_NODE, ARG_EVENT_ID, ARG_WAKE, ARG_ENABLE,
			      ARG_UNDEF }
	}, {
		.apiId = PM_RESET_ASSERT,
		.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_RESET_GET_STATUS,
		.argTypes = { ARG_UINT32, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_MMIO_WRITE,
		.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UINT32, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_MMIO_READ,
		.argTypes = { ARG_UINT32, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_INIT_FINALIZE,
		.argTypes = { ARG_UNDEF, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_FPGA_LOAD,
		.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UINT32, ARG_UINT32,
			      ARG_UNDEF }
	}, {
		.apiId = PM_FPGA_GET_STATUS,
		.argTypes = { ARG_UNDEF, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF }
	}, {
		.apiId = PM_GET_CHIPID,
		.argTypes = { ARG_UNDEF, ARG_UNDEF, ARG_UNDEF, ARG_UNDEF,
			      ARG_UNDEF },
	}, {
			.apiId = PM_SECURE_RSA_AES,
			.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UINT32, ARG_UINT32,
				      ARG_UNDEF }
	}, {
			.apiId = PM_SECURE_SHA,
			.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UINT32, ARG_UINT32,
					      ARG_UNDEF }
	}, {
			.apiId = PM_SECURE_RSA,
			.argTypes = { ARG_UINT32, ARG_UINT32, ARG_UINT32, ARG_UINT32,
					      ARG_UNDEF }
	},{
			.apiId = PM_SECURE_IMAGE,
			.argTypes = { ARG_UINT32, ARG_UINT32,  ARG_UINT32, ARG_UINT32,
						  ARG_UNDEF }
	},
};

/**
 * PmIsApiIdValid() - Check whether the given api id is within regular range
 * @api     PM API call id
 *
 * @return  True if api id is within supported api ids range, false otherwise
 */
bool PmIsApiIdValid(const u32 api)
{
	return (api >= PM_API_MIN) && (api <= PM_API_MAX);
}

/**
 * PmCheckArgument() - API argument checking
 * @argType     Argument type
 * @arg         Argument to be checked
 *
 * @return      Status, either OK or ERR_<reason>
 */
static PmPayloadStatus PmCheckArgument(const u8 argType,
				       const u32 arg)
{
	PmPayloadStatus status = PM_PAYLOAD_OK;

	switch (argType) {
	case ARG_NODE:
		if ((arg < NODE_MIN) || (arg > NODE_MAX)) {
			status = PM_PAYLOAD_ERR_NODE;
		}
		break;
	case ARG_ACK:
		if ((arg < REQUEST_ACK_MIN) || (arg > REQUEST_ACK_MAX)) {
			status = PM_PAYLOAD_ERR_ACK;
		}
		break;
	case ARG_ABORT_REASON:
		if ((arg < ABORT_REASON_MIN) || (arg > ABORT_REASON_MAX)) {
			status = PM_PAYLOAD_ERR_ABORT_REASON;
		}
		break;
	case ARG_SUSPEND_REASON:
		if ((arg < SUSPEND_REASON_MIN) || (arg > SUSPEND_REASON_MAX)) {
			status = PM_PAYLOAD_ERR_SUSPEND_REASON;
		}
		break;
	case ARG_QOS:
		if (arg > MAX_QOS) {
			status = PM_PAYLOAD_ERR_QOS;
		}
		break;
	case ARG_WAKE:
		if (arg != 1U && arg != 0U) {
			status = PM_PAYLOAD_ERR_WAKE;
		}
		break;
	case ARG_ENABLE:
	case ARG_SHUTDOWN_TYPE:
		if (arg != PMF_SHUTDOWN_TYPE_SHUTDOWN &&
		    arg != PMF_SHUTDOWN_TYPE_RESET) {
			status = PM_PAYLOAD_ERR_SHUTDOWN_TYPE;
		}
		break;
	case ARG_SHUTDOWN_SUBTYPE:
		if (arg != PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM &&
		    arg != PMF_SHUTDOWN_SUBTYPE_PS_ONLY &&
		    arg != PMF_SHUTDOWN_SUBTYPE_SYSTEM) {
			status = PM_PAYLOAD_ERR_SHUTDOWN_TYPE;
		}
		break;
	case ARG_CAPABILITIES:
	case ARG_OP_CH_TYPE:
	case ARG_STATE:
	case ARG_EVENT_ID:
	case ARG_RESET:
	case ARG_LATENCY:
	case ARG_UINT32:
		break;
	default:
		status = PM_PAYLOAD_ERR_UNKNOWN;
		break;
	}

	return status;
}

/**
 * PmCheckPayloadArgs() - Check for payload correctness (are provided values
 * within expected range)
 * @entry   PM API call entry with the expected argument definitions
 * @args    PM API call arguments
 *
 * @return  Status, either OK or ERR_<reason>
 */
static PmPayloadStatus PmCheckPayloadArgs(const PmApiEntry* entry,
					  const u32 *args)
{
	u8 i;
	PmPayloadStatus status;

	for (i = 0U; i < PAYLOAD_API_ARGS_CNT; i++) {
		/* if not an argument don't check anymore, return ok */
		if (ARG_UNDEF == entry->argTypes[i]) {
			status = PM_PAYLOAD_OK;
			goto done;
		}

		/* check i-th argument */
		status = PmCheckArgument(entry->argTypes[i], args[i]);
		if (PM_PAYLOAD_OK != status) {
			/* return when argument is invalid */
			goto done;
		}
	}

done:
	/* all arguments are ok */
	return status;
}

/**
 * PmCheckPayload() - Check payload read from IPI buffer
 * @args    Array of argument values read from IPI buffer (the payload)
 *
 * @return  Status of the performed check: OK or ERR_<reason>
 */
PmPayloadStatus PmCheckPayload(const u32 *args)
{
	u32 i;
	PmPayloadStatus ret;
	const PmApiEntry* entry = NULL;
	bool status = PmIsApiIdValid(args[0]);

	if (false == status) {
		ret = PM_PAYLOAD_ERR_API_ID;
		goto done;
	}

	for (i = 0U; i < ARRAY_SIZE(pmApiTable); i++) {
		if (args[0] == pmApiTable[i].apiId) {
			entry = &pmApiTable[i];
			break;
		}
	}

	if (NULL == entry) {
		ret = PM_PAYLOAD_ERR_API_ID;
		goto done;
	}

	ret = PmCheckPayloadArgs(entry, &args[1]);

done:
	return ret;
}

/**
 * PmRequestAcknowledge() - check whether this API call requires acknowledge
 * @args    Pointer to array of argument values read from IPI buffer
 *
 * @return  Extracted acknowledge argument from payload, if payload does not
 *          contain acknowledge argument (because of APIs call declaration),
 *          return REQUEST_ACK_BLOCKING
 */
u32 PmRequestAcknowledge(const u32 *args)
{
	u32 i;
	u32 ack = REQUEST_ACK_BLOCKING;
	const PmApiEntry* entry = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmApiTable); i++) {
		if (args[0] == pmApiTable[i].apiId) {
			entry = &pmApiTable[i];
			break;
		}
	}

	if (NULL == entry) {
		goto done;
	}

	for (i = 1U; i < PAYLOAD_ELEM_CNT; i++) {
		if (ARG_ACK == entry->argTypes[i - 1]) {
			ack = args[i];
			break;
		}
	}

done:
	return ack;
}

#endif
