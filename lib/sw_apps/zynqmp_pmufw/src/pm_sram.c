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
 * Definitions of PM slave SRAM structures and state transitions.
 *********************************************************************/

#include "pm_sram.h"
#include "pm_common.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

/* Sram states */
static const u32 pmSramStates[PM_SRAM_STATE_MAX] = {
	[PM_SRAM_STATE_OFF] = 0U,
	[PM_SRAM_STATE_RET] = PM_CAP_CONTEXT,
	[PM_SRAM_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

/* Sram transition table (from which to which state sram can transit) */
static const PmStateTran pmSramTransitions[] = {
	{
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_RET,
	}, {
		.fromState = PM_SRAM_STATE_RET,
		.toState = PM_SRAM_STATE_ON,
	}, {
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_OFF,
	}, {
		.fromState = PM_SRAM_STATE_OFF,
		.toState = PM_SRAM_STATE_ON,
	},
};

/**
 * PmSramFsmHandler() - Sram FSM handler, performs transition actions
 * @slave       Slave whose state should be changed
 * @nextState   State the slave should enter
 *
 * @return      Status of performing transition action
 */
static int PmSramFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status = XST_PM_INTERNAL;
	PmSlaveSram* sram = (PmSlaveSram*)slave->node.derived;

	switch (slave->node.currState) {
	case PM_SRAM_STATE_ON:
		if (PM_SRAM_STATE_RET == nextState) {
			/* ON -> RET */
			XPfw_RMW32(sram->retCtrlAddr, sram->retCtrlMask,
				   sram->retCtrlMask);
			status = XST_SUCCESS;
		} else if (PM_SRAM_STATE_OFF == nextState) {
			/* ON -> OFF*/
			status = sram->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_SRAM_STATE_RET:
		if (PM_SRAM_STATE_ON == nextState) {
			/* RET -> ON */
			XPfw_RMW32(sram->retCtrlAddr, sram->retCtrlMask,
				   ~sram->retCtrlMask);
			status = XST_SUCCESS;
		} else if (PM_SRAM_STATE_OFF == nextState) {
			/* RET -> OFF */
			status = sram->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_SRAM_STATE_OFF:
		if (PM_SRAM_STATE_ON == nextState) {
			/* OFF -> ON */
			status = sram->PwrUp();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	default:
		status = XST_PM_INTERNAL;
		PmDbg("ERROR: Unknown SRAM state #%d\n", slave->node.currState);
		break;
	}

	if (XST_SUCCESS == status) {
		slave->node.currState = nextState;
	}

	return status;
}

/* Sram FSM */
static const PmSlaveFsm slaveSramFsm = {
	.states = pmSramStates,
	.statesCnt = PM_SRAM_STATE_MAX,
	.trans = pmSramTransitions,
	.transCnt = ARRAY_SIZE(pmSramTransitions),
	.enterState = PmSramFsmHandler,
};

static PmRequirement* const pmL2Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_L2],
};

PmSlaveSram pmSlaveL2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveL2_g,
			.nodeId = NODE_L2,
			.typeId = PM_TYPE_SRAM,
			.parent = &pmPowerDomainFpd_g,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmL2Reqs,
		.reqsCnt = ARRAY_SIZE(pmL2Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnL2Bank0Handler,
	.PwrUp = XpbrPwrUpL2Bank0Handler,
	.retCtrlAddr = PMU_LOCAL_L2_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_L2_RET_CNTRL_BANK0_MASK,
};

static PmRequirement* const pmOcm0Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_OCM0],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_OCM0],
};

PmSlaveSram pmSlaveOcm0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm0_g,
			.nodeId = NODE_OCM_BANK_0,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmOcm0Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm0Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnOcmBank0Handler,
	.PwrUp = XpbrPwrUpOcmBank0Handler,
	.retCtrlAddr = PMU_LOCAL_OCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_OCM_RET_CNTRL_BANK0_MASK,
};

static PmRequirement* const pmOcm1Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_OCM1],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_OCM1],
};

PmSlaveSram pmSlaveOcm1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm1_g,
			.nodeId = NODE_OCM_BANK_1,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmOcm1Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm1Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnOcmBank1Handler,
	.PwrUp = XpbrPwrUpOcmBank1Handler,
	.retCtrlAddr = PMU_LOCAL_OCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_OCM_RET_CNTRL_BANK1_MASK,
};

static PmRequirement* const pmOcm2Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_OCM2],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_OCM2],
};

PmSlaveSram pmSlaveOcm2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm2_g,
			.nodeId = NODE_OCM_BANK_2,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmOcm2Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm2Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnOcmBank2Handler,
	.PwrUp = XpbrPwrUpOcmBank2Handler,
	.retCtrlAddr = PMU_LOCAL_OCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_OCM_RET_CNTRL_BANK2_MASK,
};

static PmRequirement* const pmOcm3Reqs[] = {
	&pmApuReq_g[PM_MASTER_APU_SLAVE_OCM3],
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_OCM3],
};

PmSlaveSram pmSlaveOcm3_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm3_g,
			.nodeId = NODE_OCM_BANK_3,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmOcm3Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm3Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnOcmBank3Handler,
	.PwrUp = XpbrPwrUpOcmBank3Handler,
	.retCtrlAddr = PMU_LOCAL_OCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_OCM_RET_CNTRL_BANK3_MASK,
};

static PmRequirement* const pmTcm0AReqs[] = {
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TCM0A],
};

PmSlaveSram pmSlaveTcm0A_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTcm0A_g,
			.nodeId = NODE_TCM_0_A,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmTcm0AReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm0AReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnTcm0AHandler,
	.PwrUp = XpbrPwrUpTcm0AHandler,
	.retCtrlAddr = PMU_LOCAL_TCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_TCM_RET_CNTRL_TCMA0_MASK,
};

static PmRequirement* const pmTcm0BReqs[] = {
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TCM0B],
};

PmSlaveSram pmSlaveTcm0B_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTcm0B_g,
			.nodeId = NODE_TCM_0_B,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmTcm0BReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm0BReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnTcm0BHandler,
	.PwrUp = XpbrPwrUpTcm0BHandler,
	.retCtrlAddr = PMU_LOCAL_TCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_TCM_RET_CNTRL_TCMB0_MASK,
};

static PmRequirement* const pmTcm1AReqs[] = {
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TCM1A],
};

PmSlaveSram pmSlaveTcm1A_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTcm1A_g,
			.nodeId = NODE_TCM_1_A,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmTcm1AReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm1AReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnTcm1AHandler,
	.PwrUp = XpbrPwrUpTcm1AHandler,
	.retCtrlAddr = PMU_LOCAL_TCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_TCM_RET_CNTRL_TCMA1_MASK,
};

static PmRequirement* const pmTcm1BReqs[] = {
	&pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_TCM1B],
};

PmSlaveSram pmSlaveTcm1B_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveTcm1B_g,
			.nodeId = NODE_TCM_1_B,
			.typeId = PM_TYPE_SRAM,
			.parent = NULL,
			.currState = PM_SRAM_STATE_ON,
			.ops = NULL,
		},
		.reqs = pmTcm1BReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm1BReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
	.PwrDn = XpbrPwrDnTcm1BHandler,
	.PwrUp = XpbrPwrUpTcm1BHandler,
	.retCtrlAddr = PMU_LOCAL_TCM_RET_CNTRL,
	.retCtrlMask = PMU_LOCAL_TCM_RET_CNTRL_TCMB1_MASK,
};
