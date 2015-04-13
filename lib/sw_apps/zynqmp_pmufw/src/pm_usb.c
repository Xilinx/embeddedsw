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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Definitions of PM slave USB structures and state transitions.
 *********************************************************************/

#include "pm_usb.h"
#include "pm_common.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

#define DEFTR(INST, TRAN) ((INST * PM_USB_TR_MAX) + TRAN)

static const PmTranHandler pmUsbActions_g[PM_USB_INST_MAX * PM_USB_TR_MAX] = {
	[ DEFTR(PM_USB_0, PM_USB_TR_ON_TO_OFF) ] = XpbrPwrDnUsb0Handler,
	[ DEFTR(PM_USB_0, PM_USB_TR_OFF_TO_ON) ] = XpbrPwrUpUsb0Handler,

	[ DEFTR(PM_USB_1, PM_USB_TR_ON_TO_OFF) ] = XpbrPwrDnUsb1Handler,
	[ DEFTR(PM_USB_1, PM_USB_TR_OFF_TO_ON) ] = XpbrPwrUpUsb1Handler,
};

/* USB states */
static const u32 pmUsbStates_g[PM_USB_STATE_MAX] = {
	[PM_USB_STATE_OFF] = PM_CAP_WAKEUP,
	[PM_USB_STATE_ON] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

/* USB transition table (from which to which state USB can transit) */
static const PmStateTran pmUsbTransitions_g[PM_USB_TR_MAX] = {
	[PM_USB_TR_ON_TO_OFF] = {
		.fromState = PM_USB_STATE_ON,
		.toState = PM_USB_STATE_OFF,
	},
	[PM_USB_TR_OFF_TO_ON] = {
		.fromState = PM_USB_STATE_OFF,
		.toState = PM_USB_STATE_ON,
	},
};

/* USB FSM */
static const PmSlaveFsm slaveUsbFsm = {
	.states = pmUsbStates_g,
	.statesCnt = PM_USB_STATE_MAX,
	.trans = pmUsbTransitions_g,
	.transCnt = PM_USB_TR_MAX,
	.actions = pmUsbActions_g,
};

static PmRequirement* const pmUsb0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_USB0],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_USB0],
};

static PmWakeProperties pmUsb0Wake = {
	.proxyIrqMask = FPD_GICP_USB0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP2],
};

PmSlaveUsb pmSlaveUsb0_g = {
	.slv = {
		.node = {
			.nodeId = NODE_USB_0,
			.typeId = PM_TYPE_USB,
			.currState = PM_USB_STATE_ON,
			.derived = &pmSlaveUsb0_g,
			.ops = NULL,
		},
		.instId = PM_USB_0,
		.reqs = pmUsb0Reqs,
		.reqsCnt = ARRAY_SIZE(pmUsb0Reqs),
		.wake = &pmUsb0Wake,
		.slvFsm = &slaveUsbFsm,
	},
};

static PmRequirement* const pmUsb1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_USB1],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_USB1],
};

static PmWakeProperties pmUsb1Wake = {
	.proxyIrqMask = FPD_GICP_USB1_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP2],
};

PmSlaveUsb pmSlaveUsb1_g = {
	.slv = {
		.node = {
			.nodeId = NODE_USB_1,
			.typeId = PM_TYPE_USB,
			.currState = PM_USB_STATE_ON,
			.derived = &pmSlaveUsb1_g,
			.ops = NULL,
		},
		.instId = PM_USB_1,
		.reqs = pmUsb1Reqs,
		.reqsCnt = ARRAY_SIZE(pmUsb1Reqs),
		.wake = &pmUsb1Wake,
		.slvFsm = &slaveUsbFsm,
	},
};
