/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef PM_DEFS_H_
#define PM_DEFS_H_

#define PM_VERSION_MAJOR	0
#define PM_VERSION_MINOR	1

#define PM_VERSION	((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)

/* Capabilities for RAM */
#define PM_CAP_ACCESS	0x1U
#define PM_CAP_CONTEXT	0x2U

#define MAX_LATENCY	(~0U)
#define MAX_QOS		100U

enum XPmApiId {
	/* Miscellaneous API functions: */
	PM_GET_API_VERSION = 1, /* Do not change or move */
	PM_SET_CONFIGURATION = 2,
	PM_GET_NODE_STATUS = 3,
	PM_GET_OP_CHARACTERISTIC = 4,
	PM_REGISTER_NOTIFIER = 5,
	/* API for suspending of PUs: */
	PM_REQ_SUSPEND = 6,
	PM_SELF_SUSPEND = 7,
	PM_FORCE_POWERDOWN = 8,
	PM_ABORT_SUSPEND = 9,
	PM_REQ_WAKEUP = 10,
	PM_SET_WAKEUP_SOURCE = 11,
	PM_SYSTEM_SHUTDOWN = 12,
	/* API for managing PM slaves: */
	PM_REQ_NODE = 13,
	PM_RELEASE_NODE = 14,
	PM_SET_REQUIREMENT = 15,
	PM_SET_MAX_LATENCY = 16,
	/* Direct control API functions: */
	PM_CLOCK_REQUEST = 17,
	PM_CLOCK_RELEASE = 18,
	PM_CLOCK_SET_RATE = 19,
	PM_CLOCK_GET_RATE = 20,
	PM_CLOCK_GET_RATE_INFO = 21,
	PM_RESET_ASSERT = 22,
	PM_RESET_GET_STATUS = 23,
	PM_MMIO_WRITE = 24,
	PM_MMIO_READ = 25,
};

#define PM_API_MIN	1
#define PM_API_MAX	25

enum XPmApiCbId {
	PM_INIT_SUSPEND_CB = 30,
	PM_ACKNOWLEDGE_CB,
	PM_NOTIFY_CB,
};

enum XPmNodeId {
	NODE_UNKNOWN = 0,
	NODE_APU,
	NODE_APU_0,
	NODE_APU_1,
	NODE_APU_2,
	NODE_APU_3,
	NODE_RPU,
	NODE_RPU_0,
	NODE_RPU_1,
	NODE_PL,
	NODE_FPD,
	NODE_OCM_BANK_0,
	NODE_OCM_BANK_1,
	NODE_OCM_BANK_2,
	NODE_OCM_BANK_3,
	NODE_TCM_0_A,
	NODE_TCM_0_B,
	NODE_TCM_1_A,
	NODE_TCM_1_B,
	NODE_L2,
	NODE_GPU_PP_0,
	NODE_GPU_PP_1,
	NODE_USB_0,
	NODE_USB_1,
	NODE_TTC_0,
};

enum XPmRequestAck {
	REQ_ACK_NO = 1,
	REQ_ACK_BLOCKING = 2,
	REQ_ACK_CB_STANDARD = 3,
	REQ_ACK_CB_ERROR = 4,
};

enum XPmAbortReason {
	ABORT_REASON_WKUP_EVENT = 100,
	ABORT_REASON_PU_BUSY,
	ABORT_REASON_NO_PWRDN,
	ABORT_REASON_UNKNOWN,
};

enum XPmSuspendReason {
	SUSPEND_REASON_PU_REQ = 201,
	SUSPEND_REASON_ALERT,
	SUSPEND_REASON_SYS_SHUTDOWN,
};

enum XPmRamState {
	PM_RAM_STATE_OFF = 1,
	PM_RAM_STATE_RETENTION,
	PM_RAM_STATE_ON,
};

enum XPmOpCharType {
	PM_OPCHAR_TYPE_POWER = 1,
	PM_OPCHAR_TYPE_ENERGY = 2,
	PM_OPCHAR_TYPE_TEMP = 3,
};

/**
 * @PM_RET_SUCCESS:		success
 * @PM_RET_ERROR_ARGS:		illegal arguments provided
 * @PM_RET_ERROR_ACCESS:	access rights violation
 * @PM_RET_ERROR_TIMEOUT:	timeout in communication with PMU
 * @PM_RET_ERROR_NOTSUPPORTED:	feature not supported
 * @PM_RET_ERROR_PROC:		node is not a processor node
 * @PM_RET_ERROR_API_ID:	illegal API ID
 * @PM_RET_ERROR_OTHER:		other error
 */
enum XPmStatus {
	PM_RET_SUCCESS = 0,
	PM_RET_ERROR_ARGS = 1,
	PM_RET_ERROR_ACCESS = 2,
	PM_RET_ERROR_TIMEOUT = 3,
	PM_RET_ERROR_NOTSUPPORTED = 4,
	PM_RET_ERROR_PROC = 5,
	PM_RET_ERROR_API_ID = 6,
	PM_RET_ERROR_FAILURE = 7,
	PM_RET_ERROR_COMMUNIC = 8,
	PM_RET_ERROR_DOUBLEREQ = 9,
	PM_RET_ERROR_OTHER = 25,
};

/**
 * @PM_INITIAL_BOOT:	boot is a fresh system startup
 * @PM_RESUME:		boot is a resume
 * @PM_BOOT_ERROR:	error, boot cause cannot be identified
 */
enum XPmBootStatus {
	PM_INITIAL_BOOT,
	PM_RESUME,
	PM_BOOT_ERROR,
};

#endif /* PM_DEFS_H_ */
