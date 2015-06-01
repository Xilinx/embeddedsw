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

#include "pm_periph.h"
#include "pm_common.h"
#include "pm_node.h"
#include "pm_master.h"

/*
 * Without clock/reset control, from PM perspective ttc has only one state.
 * It is in LPD, which is never turned off, does not sit in power island,
 * therefore has no off state.
 */
static const u32 pmAonFsmStates[] = {
	[PM_AON_SLAVE_STATE] = PM_CAP_WAKEUP | PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

static const PmSlaveFsm slaveAonFsm = {
	.states = pmAonFsmStates,
	.statesCnt = ARRAY_SIZE(pmAonFsmStates),
	.trans = NULL,
	.transCnt = 0U,
	.enterState = NULL,
};

static PmWakeProperties pmTtc0Wake = {
	.proxyIrqMask = FPD_GICP_TTC0_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP1],
};

static PmRequirement* const pmTtc0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_TTC0],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TTC0],
};

PmSlaveTtc pmSlaveTtc0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTtc0_g,
			.nodeId = NODE_TTC_0,
			.typeId = PM_TYPE_TTC,
			.parent = NULL,
			.currState = PM_AON_SLAVE_STATE,
			.ops = NULL,
		},
		.reqs = pmTtc0Reqs,
		.reqsCnt = ARRAY_SIZE(pmTtc0Reqs),
		.wake = &pmTtc0Wake,
		.slvFsm = &slaveAonFsm,
	},
};

/*
 * Standard slave with no private PM properties to be controlled.
 * It can be powered down with the power parent.
 */
static const u32 pmStdStates[] = {
	[PM_STD_SLAVE_STATE_OFF] = 0U,
	[PM_STD_SLAVE_STATE_ON] = PM_CAP_WAKEUP | PM_CAP_ACCESS |
				  PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* Standard slave transitions (from which to which state Std slave transits) */
static const PmStateTran pmStdTransitions[] = {
	{
		.fromState = PM_STD_SLAVE_STATE_ON,
		.toState = PM_STD_SLAVE_STATE_OFF,
	}, {
		.fromState = PM_STD_SLAVE_STATE_OFF,
		.toState = PM_STD_SLAVE_STATE_ON,
	},
};

static const PmSlaveFsm slaveStdFsm = {
	.states = pmStdStates,
	.statesCnt = ARRAY_SIZE(pmStdStates),
	.trans = pmStdTransitions,
	.transCnt = ARRAY_SIZE(pmStdTransitions),
	.enterState = NULL,
};

static PmWakeProperties pmSataWake = {
	.proxyIrqMask = FPD_GICP_SATA_WAKE_IRQ_MASK,
	.proxyGroup = &gicProxyGroups_g[FPD_GICP_GROUP4],
};

static PmRequirement* const pmSataReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_SATA],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_SATA],
};

PmSlaveSata pmSlaveSata_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveSata_g,
			.nodeId = NODE_SATA,
			.typeId = PM_TYPE_SATA,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_STD_SLAVE_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmSataReqs,
		.reqsCnt = ARRAY_SIZE(pmSataReqs),
		.wake = &pmSataWake,
		.slvFsm = &slaveStdFsm,
	},
};
