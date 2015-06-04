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
#define PM_CAP_WAKEUP	0x4U

#define MAX_LATENCY	(~0U)
#define MAX_QOS		100U

/* System shutdown macros */
#define PM_SHUTDOWN	0U
#define PM_RESTART	1U

enum XPmApiId {
	/* Miscellaneous API functions: */
	PM_GET_API_VERSION = 1, /* Do not change or move */
	PM_SET_CONFIGURATION,
	PM_GET_NODE_STATUS,
	PM_GET_OP_CHARACTERISTIC,
	PM_REGISTER_NOTIFIER,
	/* API for suspending of PUs: */
	PM_REQUEST_SUSPEND,
	PM_SELF_SUSPEND,
	PM_FORCE_POWERDOWN,
	PM_ABORT_SUSPEND,
	PM_REQUEST_WAKEUP,
	PM_SET_WAKEUP_SOURCE,
	PM_SYSTEM_SHUTDOWN,
	/* API for managing PM slaves: */
	PM_REQUEST_NODE,
	PM_RELEASE_NODE,
	PM_SET_REQUIREMENT,
	PM_SET_MAX_LATENCY,
	/* Direct control API functions: */
	PM_RESET_ASSERT,
	PM_RESET_GET_STATUS,
	PM_MMIO_WRITE,
	PM_MMIO_READ,
};

#define PM_API_MIN	PM_GET_API_VERSION
#define PM_API_MAX	PM_MMIO_READ

enum XPmApiCbId {
	PM_INIT_SUSPEND_CB = 30,
	PM_ACKNOWLEDGE_CB,
	PM_NOTIFY_CB,
};

enum XPmNodeId {
	NODE_UNKNOWN,
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
	NODE_TTC_1,
	NODE_TTC_2,
	NODE_TTC_3,
	NODE_SATA,
	NODE_ETH_0,
	NODE_ETH_1,
	NODE_ETH_2,
	NODE_ETH_3,
	NODE_UART_0,
	NODE_UART_1,
	NODE_SPI_0,
	NODE_SPI_1,
	NODE_I2C_0,
	NODE_I2C_1,
	NODE_SD_0,
	NODE_SD_1,
	NODE_DP,
	NODE_GDMA,
	NODE_ADMA,
	NODE_NAND,
	NODE_QSPI,
	NODE_GPIO,
	NODE_CAN_0,
	NODE_CAN_1,
	NODE_AFI,
	NODE_APLL,
	NODE_VPLL,
	NODE_DPLL,
	NODE_RPLL,
	NODE_IOPLL,
};

enum XPmRequestAck {
	REQUEST_ACK_NO = 1,
	REQUEST_ACK_BLOCKING,
	REQUEST_ACK_CB_STANDARD,
	REQUEST_ACK_CB_CERROR,
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
	PM_OPCHAR_TYPE_ENERGY,
	PM_OPCHAR_TYPE_TEMP,
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
	PM_RET_SUCCESS,
	PM_RET_ERROR_ARGS,
	PM_RET_ERROR_ACCESS,
	PM_RET_ERROR_TIMEOUT,
	PM_RET_ERROR_NOTSUPPORTED,
	PM_RET_ERROR_PROC,
	PM_RET_ERROR_API_ID,
	PM_RET_ERROR_FAILURE,
	PM_RET_ERROR_COMMUNIC,
	PM_RET_ERROR_DOUBLEREQ,
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
