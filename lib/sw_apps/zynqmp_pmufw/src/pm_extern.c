/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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

#include "pm_extern.h"
#include "pm_slave.h"

/**
 * PmWakeEventExtern - External wake event, derived from PmWakeEvent
 * @wake	Basic PmWakeEvent structure
 * @set		ORed IPI masks of masters that have set wake enabled
 */
typedef struct PmWakeEventExtern {
	PmWakeEvent wake;
	u32 set;
} PmWakeEventExtern;

/**
 * PmWakeEventExternSet() - Set external wake event as the wake source
 * @wake	Wake event
 * @ipiMask	IPI mask of the master which sets the wake source
 * @enable	Flag: for enable non-zero value, for disable value zero
 */
static void PmWakeEventExternSet(PmWakeEvent* const wake, const u32 ipiMask,
				 const u32 enable)
{
	PmWakeEventExtern* ext = (PmWakeEventExtern*)wake->derived;

	if (0U != enable) {
		ext->set |= ipiMask;
	} else {
		ext->set &= ~ipiMask;
	}
}

static PmWakeEventClass pmWakeEventClassExtern = {
	.set = PmWakeEventExternSet,
};

static PmWakeEventExtern pmExternWake = {
	.wake = {
		.derived = &pmExternWake,
		.class = &pmWakeEventClassExtern,
	},
	.set = 0U,
};

static const u32 pmExternDeviceFsmStates[] = {
	PM_CAP_WAKEUP,
};

static const PmSlaveFsm pmExternDeviceFsm = {
	DEFINE_SLAVE_STATES(pmExternDeviceFsmStates),
	.trans = NULL,
	.transCnt = 0U,
	.enterState = NULL,
};

PmSlave pmSlaveExternDevice_g = {
	.node = {
		.derived = &pmSlaveExternDevice_g,
		.nodeId = NODE_EXTERN,
		.class = &pmNodeClassSlave_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = 0U,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		.powerInfo = NULL,
		.powerInfoCnt = 0U,
	},
	.class = NULL,
	.reqs = NULL,
	.wake = &pmExternWake.wake,
	.slvFsm = &pmExternDeviceFsm,
	.flags = 0U,
};
