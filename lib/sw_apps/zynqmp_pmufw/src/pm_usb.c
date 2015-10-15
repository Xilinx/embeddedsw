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
 * Definitions of PM slave USB structures and state transitions.
 *********************************************************************/

#include "pm_usb.h"
#include "pm_common.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

/* USB states */
static const u32 pmUsbStates[PM_USB_STATE_MAX] = {
	[PM_USB_STATE_OFF] = PM_CAP_WAKEUP,
	[PM_USB_STATE_ON] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

/* USB transition table (from which to which state USB can transit) */
static const PmStateTran pmUsbTransitions[] = {
	{
		.fromState = PM_USB_STATE_ON,
		.toState = PM_USB_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_USB_STATE_OFF,
		.toState = PM_USB_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

/**
 * PmUsbFsmHandler() - Usb FSM handler, performs transition actions
 * @slave       Slave whose state should be changed
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static int PmUsbFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status = XST_PM_INTERNAL;
	PmSlaveUsb* usb = (PmSlaveUsb*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_USB_STATE_ON:
		if (PM_USB_STATE_OFF == nextState) {
			/* ON -> OFF*/
			status = usb->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_USB_STATE_OFF:
		if (PM_USB_STATE_ON == nextState) {
			/* OFF -> ON */
			status = usb->PwrUp();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: Unknown USB state #%d\n", slave->node.currState);
		break;
	}
	if (XST_SUCCESS == status) {
		slave->node.currState = nextState;
	}

	return status;
}

/* USB FSM */
static const PmSlaveFsm slaveUsbFsm = {
	.states = pmUsbStates,
	.statesCnt = ARRAY_SIZE(pmUsbStates),
	.trans = pmUsbTransitions,
	.transCnt = ARRAY_SIZE(pmUsbTransitions),
	.enterState = PmUsbFsmHandler,
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
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
		},
		.reqs = pmUsb0Reqs,
		.reqsCnt = ARRAY_SIZE(pmUsb0Reqs),
		.wake = &pmUsb0Wake,
		.slvFsm = &slaveUsbFsm,
	},
	.PwrDn = XpbrPwrDnUsb0Handler,
	.PwrUp = XpbrPwrUpUsb0Handler,
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
			.latencyMarg = MAX_LATENCY,
			.ops = NULL,
		},
		.reqs = pmUsb1Reqs,
		.reqsCnt = ARRAY_SIZE(pmUsb1Reqs),
		.wake = &pmUsb1Wake,
		.slvFsm = &slaveUsbFsm,
	},
	.PwrDn = XpbrPwrDnUsb1Handler,
	.PwrUp = XpbrPwrUpUsb1Handler,
};
