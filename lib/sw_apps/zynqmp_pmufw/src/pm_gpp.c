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
#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * GPU Pixel Processors slaves FSM implementation
 *********************************************************************/

#include "pm_gpp.h"
#include "pm_common.h"
#include "pm_power.h"
#include "xpfw_rom_interface.h"

/* A GPP has its own power islands and dependencies to the FPD power parent */
#define PM_GPP_SLAVE_STATE_OFF	0U
#define PM_GPP_SLAVE_STATE_ON	1U

static const u32 pmGppStates[] = {
	[PM_GPP_SLAVE_STATE_OFF] = 0U,
	[PM_GPP_SLAVE_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* GPP slave transitions (from which to which state slave can transits) */
static const PmStateTran pmGppTransitions[] = {
	{
		.fromState = PM_GPP_SLAVE_STATE_ON,
		.toState = PM_GPP_SLAVE_STATE_OFF,
		.latency = PM_DEFAULT_LATENCY,
	}, {
		.fromState = PM_GPP_SLAVE_STATE_OFF,
		.toState = PM_GPP_SLAVE_STATE_ON,
		.latency = PM_DEFAULT_LATENCY,
	},
};

/**
 * PmGppFsmHandler() - FSM handler of a GPP slave
 * @slave	Slave whose state should be changed
 * @nextState	State the slave should enter
 *
 * @return	Status of performing transition action
 */
static int PmGppFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status = XST_PM_INTERNAL;
	PmSlaveGpp* gpp = (PmSlaveGpp*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_GPP_SLAVE_STATE_ON:
		if (PM_GPP_SLAVE_STATE_OFF == nextState) {
			/* ON -> OFF*/
			status = gpp->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_GPP_SLAVE_STATE_OFF:
		if (PM_GPP_SLAVE_STATE_ON == nextState) {
			/* OFF -> ON */
			status = gpp->PwrUp();
			if ((XST_SUCCESS == status) && (NULL != gpp->reset)) {
				status = gpp->reset();
			}
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		PmDbg(DEBUG_DETAILED,"ERROR: Unknown state #%d\r\n",
				slave->node.currState);
		break;
	}

	return status;
}

static const PmSlaveFsm pmSlaveGppFsm = {
	DEFINE_SLAVE_STATES(pmGppStates),
	DEFINE_SLAVE_TRANS(pmGppTransitions),
	.enterState = PmGppFsmHandler,
};

static u32 pmGppSlavePowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

PmSlaveGpp pmSlaveGpuPP0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP0_g,
			.nodeId = NODE_GPU_PP_0,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainFpd_g.power,
			.clocks = NULL,
			.currState = PM_GPP_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(pmGppSlavePowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &pmSlaveGppFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnPp0Handler,
	.PwrUp = XpbrPwrUpPp0Handler,
	.reset = XpbrRstPp0Handler,
};

PmSlaveGpp pmSlaveGpuPP1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveGpuPP1_g,
			.nodeId = NODE_GPU_PP_1,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainFpd_g.power,
			.clocks = NULL,
			.currState = PM_GPP_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(pmGppSlavePowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &pmSlaveGppFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnPp1Handler,
	.PwrUp = XpbrPwrUpPp1Handler,
	.reset = XpbrRstPp1Handler,
};

#pragma weak pmUserHookVcuPwrDn
u32 pmUserHookVcuPwrDn(void)
{
	return XST_SUCCESS;
}

#pragma weak pmUserHookVcuPwrUp
u32 pmUserHookVcuPwrUp(void)
{
	return XST_SUCCESS;
}

static u32 pmSlvVcuPwrDn(void)
{
	return pmUserHookVcuPwrDn();
}

static u32 pmSlvVcuPwrUp(void)
{
	return pmUserHookVcuPwrUp();
}

PmSlaveGpp pmSlaveVcu_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveVcu_g,
			.nodeId = NODE_VCU,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainPld_g.power,
			.clocks = NULL,
			.currState = PM_GPP_SLAVE_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			DEFINE_PM_POWER_INFO(pmGppSlavePowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &pmSlaveGppFsm,
		.flags = 0U,
	},
	.PwrDn = pmSlvVcuPwrDn,
	.PwrUp = pmSlvVcuPwrUp,
	.reset = NULL,
};

#endif
