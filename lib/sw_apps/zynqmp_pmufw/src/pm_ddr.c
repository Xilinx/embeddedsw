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
 * DDR slave definition
 *
 * Note: In ON state DDR depends on FPD (cannot be accessed if FPD is
 * not on). Therefore, power parent of DDR is FPD.
 *********************************************************************/

#include "pm_ddr.h"
#include "pm_common.h"
#include "pm_defs.h"
#include "pm_master.h"

/* Power states of DDR */
#define PM_DDR_STATE_OFF	0U
#define PM_DDR_STATE_SR		1U
#define PM_DDR_STATE_ON		2U
#define PM_DDR_STATE_MAX	3U

/* Power consumptions for DDR defined by its states */
#define DEFAULT_DDR_POWER_ON		100U
#define DEFAULT_DDR_POWER_SR		50U
#define DEFAULT_DDR_POWER_OFF		0U

/* DDR states */
static const u32 pmDdrStates[PM_DDR_STATE_MAX] = {
	[PM_DDR_STATE_OFF] = 0U,
	[PM_DDR_STATE_SR] = PM_CAP_CONTEXT,
	[PM_DDR_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* DDR transition table (from which to which state DDR can transit) */
static const PmStateTran pmDdrTransitions[] = {
	{
		.fromState = PM_DDR_STATE_ON,
		.toState = PM_DDR_STATE_SR,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_SR,
		.toState = PM_DDR_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_ON,
		.toState = PM_DDR_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_DDR_STATE_OFF,
		.toState = PM_DDR_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

/**
 * PmDdrFsmHandler() - DDR FSM handler, performs transition actions
 * @slave       Slave whose state should be changed (pointer to DDR object)
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static int PmDdrFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status = XST_PM_INTERNAL;

	/* Handle transition to OFF state here */
	if ((PM_DDR_STATE_OFF != slave->node.currState) &&
	    (PM_DDR_STATE_OFF == nextState)) {
		/* TODO : power down DDR here */
		status = XST_SUCCESS;
		goto done;
	}

	switch (slave->node.currState) {
	case PM_DDR_STATE_ON:
		if (PM_DDR_STATE_SR == nextState) {
			/* TODO : put DDR in self refresh here */
			status = XST_SUCCESS;
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_SR:
		if (PM_DDR_STATE_ON == nextState) {
			/* TODO : get DDR out of self refresh here */
			status = XST_SUCCESS;
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_DDR_STATE_OFF:
		if (PM_DDR_STATE_ON == nextState) {
			/* TODO : power up DDR here */
			status = XST_SUCCESS;
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: Unknown DDR state #%d\n", slave->node.currState);
		break;
	}

done:
	if (XST_SUCCESS == status) {
		PmNodeUpdateCurrState(&slave->node, nextState);
	}
	return status;
}

/* DDR FSM */
static const PmSlaveFsm pmSlaveDdrFsm = {
	.states = pmDdrStates,
	.statesCnt = PM_DDR_STATE_MAX,
	.trans = pmDdrTransitions,
	.transCnt = ARRAY_SIZE(pmDdrTransitions),
	.enterState = PmDdrFsmHandler,
};

static PmRequirement* const pmDdrReqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_DDR],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_DDR],
};

static u32 PmDdrPowerConsumptions[] = {
	DEFAULT_DDR_POWER_OFF,
	DEFAULT_DDR_POWER_SR,
	DEFAULT_DDR_POWER_ON,
};

PmSlave pmSlaveDdr_g = {
	.node = {
		.derived = &pmSlaveDdr_g,
		.nodeId = NODE_DDR,
		.typeId = PM_TYPE_DDR,
		.parent = &pmPowerDomainFpd_g,
		.currState = PM_DDR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.ops = NULL,
		.powerInfo = PmDdrPowerConsumptions,
		.powerInfoCnt = ARRAY_SIZE(PmDdrPowerConsumptions),
	},
	.reqs = pmDdrReqs,
	.reqsCnt = ARRAY_SIZE(pmDdrReqs),
	.wake = NULL,
	.slvFsm = &pmSlaveDdrFsm,
};
