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
 * Definitions of PM slave SRAM structures and state transitions.
 *********************************************************************/

#include "pm_sram.h"
#include "pm_common.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"
#include "crf_apb.h"
#include "rpu.h"

/* TCM banks IDs (one hot encoded) */
#define PM_TCM_0A_BANK_ID	0x1U
#define PM_TCM_0B_BANK_ID	0x2U
#define PM_TCM_1A_BANK_ID	0x4U
#define PM_TCM_1B_BANK_ID	0x8U

/* Power states of SRAM */
#define PM_SRAM_STATE_OFF	0U
#define PM_SRAM_STATE_RET	1U
#define PM_SRAM_STATE_ON	2U
#define PM_SRAM_STATE_MAX	3U

/* Power consumptions for SRAM defined by its states */
#define DEFAULT_SRAM_POWER_ON		100U
#define DEFAULT_SRAM_POWER_RETENTION	50U
#define DEFAULT_SRAM_POWER_OFF		0U

/* SRAM state transition latency values */
#define PM_SRAM_ON_TO_RET_LATENCY	3
#define PM_SRAM_RET_TO_ON_LATENCY	130
#define PM_SRAM_ON_TO_OFF_LATENCY	3
#define PM_SRAM_OFF_TO_ON_LATENCY	3100

/* Sram states */
static const u32 pmSramStates[PM_SRAM_STATE_MAX] = {
	[PM_SRAM_STATE_OFF] = 0U,
	[PM_SRAM_STATE_RET] = PM_CAP_CONTEXT | PM_CAP_POWER,
	[PM_SRAM_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT | PM_CAP_POWER,
};

/* Sram transition table (from which to which state sram can transit) */
static const PmStateTran pmSramTransitions[] = {
	{
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_RET,
		.latency = PM_SRAM_ON_TO_RET_LATENCY,
	}, {
		.fromState = PM_SRAM_STATE_RET,
		.toState = PM_SRAM_STATE_ON,
		.latency = PM_SRAM_RET_TO_ON_LATENCY,
	}, {
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_OFF,
		.latency = PM_SRAM_ON_TO_OFF_LATENCY,
	}, {
		.fromState = PM_SRAM_STATE_OFF,
		.toState = PM_SRAM_STATE_ON,
		.latency = PM_SRAM_OFF_TO_ON_LATENCY,
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
			status = sram->PwrDn();
		} else if (PM_SRAM_STATE_OFF == nextState) {
			/* ON -> OFF*/
			XPfw_RMW32(sram->retCtrlAddr, sram->retCtrlMask,
				   ~sram->retCtrlMask);
			status = sram->PwrDn();
		} else {
			status = XST_NO_FEATURE;
		}
		break;
	case PM_SRAM_STATE_RET:
		if (PM_SRAM_STATE_ON == nextState) {
			/* RET -> ON */
			status = sram->PwrUp();
		} else if (PM_SRAM_STATE_OFF == nextState) {
			/* RET -> OFF */
			XPfw_RMW32(sram->retCtrlAddr, sram->retCtrlMask,
				   ~sram->retCtrlMask);
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
		PmDbg(DEBUG_DETAILED,"ERROR: Unknown SRAM state #%d\r\n",
				slave->node.currState);
		break;
	}

	return status;
}

/**
 * PmTcmFsmHandler() - TCM FSM handler, performs transition actions
 * @slave	TCM slave whose state should be changed
 * @nextState	State the TCM slave should enter
 *
 * @return	Status of performing transition action
 */
static int PmTcmFsmHandler(PmSlave* const slave, const PmStateId nextState)
{
	int status;
	PmSlaveTcm* tcm = (PmSlaveTcm*)slave->node.derived;

	if (PM_SRAM_STATE_ON == nextState) {
		status = PmPowerRequestRpu(tcm);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}

	status = PmSramFsmHandler(slave, nextState);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if ((PM_SRAM_STATE_OFF == slave->node.currState) &&
	    (PM_SRAM_STATE_ON == nextState)) {
		tcm->eccInit(tcm);
	}

	if (PM_SRAM_STATE_ON != nextState) {
		PmPowerReleaseRpu(tcm);
	}

done:
	return status;
}

/**
 * PmTcm0EccInit() - ECC initialization for TCM bank 0
 * @tcm		TCM slave node to initialize
 */
static void PmTcm0EccInit(const PmSlaveTcm* const tcm)
{
	(void)memset((void*)tcm->base, 0U, tcm->size);
}

/**
 * PmTcm1EccInit() - ECC initialization for TCM bank 1
 * @tcm		TCM slave node to initialize
 */
static void PmTcm1EccInit(const PmSlaveTcm* const tcm)
{
	u32 base = tcm->base;
	u32 ctrl = XPfw_Read32(RPU_RPU_GLBL_CNTL);

	if (0U != (ctrl & RPU_RPU_GLBL_CNTL_TCM_COMB_MASK)) {
		base -= 0x80000U;
	}
	(void)memset((void*)base, 0U, tcm->size);
}

/**
 * PmSlaveTcmInit() - Initialize the TCM slave
 * @slave	TCM slave node
 */
static int PmSlaveTcmInit(PmSlave* const slave)
{
	int status = XST_SUCCESS;
	PmSlaveTcm* tcm = (PmSlaveTcm*)slave->node.derived;

	if (PM_SRAM_STATE_ON == slave->node.currState) {
		status = PmPowerRequestRpu(tcm);
	}

	return status;
}

/**
 * PmSlaveTcmForceDown() - Force down the TCM slave
 * @slave	TCM slave node
 */
static int PmSlaveTcmForceDown(PmSlave* const slave)
{
	PmSlaveTcm* tcm = (PmSlaveTcm*)slave->node.derived;

	if (PM_SRAM_STATE_ON == slave->node.currState) {
		PmPowerReleaseRpu(tcm);
	}

	return XST_SUCCESS;
}

static PmSlaveClass pmSlaveClassTcm = {
	.init = PmSlaveTcmInit,
	.forceDown = PmSlaveTcmForceDown,
};

/* Sram FSM */
static const PmSlaveFsm slaveSramFsm = {
	.states = pmSramStates,
	.statesCnt = PM_SRAM_STATE_MAX,
	.trans = pmSramTransitions,
	.transCnt = ARRAY_SIZE(pmSramTransitions),
	.enterState = PmSramFsmHandler,
};

/* TCM FSM */
static const PmSlaveFsm pmSlaveTcmFsm = {
	DEFINE_SLAVE_STATES(pmSramStates),
	DEFINE_SLAVE_TRANS(pmSramTransitions),
	.enterState = PmTcmFsmHandler,
};

static u32 PmSramPowers[] = {
	DEFAULT_SRAM_POWER_OFF,
	DEFAULT_SRAM_POWER_RETENTION,
	DEFAULT_SRAM_POWER_ON,
};

/**
 * PmL2PwrDn() - Handler for powering down L2$
 *
 * @return	Status returned by PMU-ROM handler for powering down L2$
 */
static u32 PmL2PwrDn(void)
{
	int status;

	/* Now call PMU-ROM function to power down L2 RAM */
	status = XpbrPwrDnL2Bank0Handler();

	/*
	 * Assert L2 reset after the power down. Reset will be released by the
	 * PMU-ROM when the first APU core is woken-up.
	 */
	XPfw_RMW32(CRF_APB_RST_FPD_APU,
		   CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK,
		   CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK);

	return status;
}

PmSlaveSram pmSlaveL2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveL2_g,
			.nodeId = NODE_L2,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainFpd_g.power,
			.clocks = NULL,
			.currState = PM_SRAM_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmSramPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
		.flags = 0U,
	},
	.PwrDn = PmL2PwrDn,
	.PwrUp = XpbrPwrUpL2Bank0Handler,
	.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
	.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_L2_BANK0_MASK,
};

PmSlaveSram pmSlaveOcm0_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm0_g,
			.nodeId = NODE_OCM_BANK_0,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.currState = PM_SRAM_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmSramPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnOcmBank0Handler,
	.PwrUp = XpbrPwrUpOcmBank0Handler,
	.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
	.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_OCM_BANK0_MASK,
};

PmSlaveSram pmSlaveOcm1_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm1_g,
			.nodeId = NODE_OCM_BANK_1,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.currState = PM_SRAM_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmSramPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnOcmBank1Handler,
	.PwrUp = XpbrPwrUpOcmBank1Handler,
	.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
	.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_OCM_BANK1_MASK,
};

PmSlaveSram pmSlaveOcm2_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm2_g,
			.nodeId = NODE_OCM_BANK_2,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.currState = PM_SRAM_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmSramPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnOcmBank2Handler,
	.PwrUp = XpbrPwrUpOcmBank2Handler,
	.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
	.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_OCM_BANK2_MASK,
};

PmSlaveSram pmSlaveOcm3_g = {
	.slv = {
		.node = {
			.derived = &pmSlaveOcm3_g,
			.nodeId = NODE_OCM_BANK_3,
			.class = &pmNodeClassSlave_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.currState = PM_SRAM_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmSramPowers),
		},
		.class = NULL,
		.reqs = NULL,
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
		.flags = 0U,
	},
	.PwrDn = XpbrPwrDnOcmBank3Handler,
	.PwrUp = XpbrPwrUpOcmBank3Handler,
	.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
	.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_OCM_BANK3_MASK,
};

PmSlaveTcm pmSlaveTcm0A_g = {
	.sram = {
		.slv = {
			.node = {
				.derived = &pmSlaveTcm0A_g,
				.nodeId = NODE_TCM_0_A,
				.class = &pmNodeClassSlave_g,
				.parent = &pmPowerDomainLpd_g.power,
				.clocks = NULL,
				.currState = PM_SRAM_STATE_ON,
				.latencyMarg = MAX_LATENCY,
				.flags = 0U,
				DEFINE_PM_POWER_INFO(PmSramPowers),
			},
			.class = &pmSlaveClassTcm,
			.reqs = NULL,
			.wake = NULL,
			.slvFsm = &pmSlaveTcmFsm,
			.flags = 0U,
		},
		.PwrDn = XpbrPwrDnTcm0AHandler,
		.PwrUp = XpbrPwrUpTcm0AHandler,
		.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_TCM0A_MASK,
	},
	.size = 0x10000U,
	.base = 0xffe00000U,
	.eccInit = PmTcm0EccInit,
	.id = PM_TCM_0A_BANK_ID,
};

PmSlaveTcm pmSlaveTcm0B_g = {
	.sram = {
		.slv = {
			.node = {
				.derived = &pmSlaveTcm0B_g,
				.nodeId = NODE_TCM_0_B,
				.class = &pmNodeClassSlave_g,
				.parent = &pmPowerDomainLpd_g.power,
				.clocks = NULL,
				.currState = PM_SRAM_STATE_ON,
				.latencyMarg = MAX_LATENCY,
				.flags = 0U,
				DEFINE_PM_POWER_INFO(PmSramPowers),
			},
			.class = &pmSlaveClassTcm,
			.reqs = NULL,
			.wake = NULL,
			.slvFsm = &pmSlaveTcmFsm,
			.flags = 0U,
		},
		.PwrDn = XpbrPwrDnTcm0BHandler,
		.PwrUp = XpbrPwrUpTcm0BHandler,
		.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_TCM0B_MASK,
	},
	.size = 0x10000U,
	.base = 0xffe20000U,
	.eccInit = PmTcm0EccInit,
	.id = PM_TCM_0B_BANK_ID,
};

PmSlaveTcm pmSlaveTcm1A_g = {
	.sram = {
		.slv = {
			.node = {
				.derived = &pmSlaveTcm1A_g,
				.nodeId = NODE_TCM_1_A,
				.class = &pmNodeClassSlave_g,
				.parent = &pmPowerDomainLpd_g.power,
				.clocks = NULL,
				.currState = PM_SRAM_STATE_ON,
				.latencyMarg = MAX_LATENCY,
				.flags = 0U,
				DEFINE_PM_POWER_INFO(PmSramPowers),
			},
			.class = &pmSlaveClassTcm,
			.reqs = NULL,
			.wake = NULL,
			.slvFsm = &pmSlaveTcmFsm,
			.flags = 0U,
		},
		.PwrDn = XpbrPwrDnTcm1AHandler,
		.PwrUp = XpbrPwrUpTcm1AHandler,
		.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_TCM1A_MASK,
	},
	.size = 0x10000U,
	.base = 0xffe90000U,
	.eccInit = PmTcm1EccInit,
	.id = PM_TCM_1A_BANK_ID,
};

PmSlaveTcm pmSlaveTcm1B_g = {
	.sram = {
		.slv = {
			.node = {
				.derived = &pmSlaveTcm1B_g,
				.nodeId = NODE_TCM_1_B,
				.class = &pmNodeClassSlave_g,
				.parent = &pmPowerDomainLpd_g.power,
				.clocks = NULL,
				.currState = PM_SRAM_STATE_ON,
				.latencyMarg = MAX_LATENCY,
				.flags = 0U,
				DEFINE_PM_POWER_INFO(PmSramPowers),
			},
			.class = &pmSlaveClassTcm,
			.reqs = NULL,
			.wake = NULL,
			.slvFsm = &pmSlaveTcmFsm,
			.flags = 0U,
		},
		.PwrDn = XpbrPwrDnTcm1BHandler,
		.PwrUp = XpbrPwrUpTcm1BHandler,
		.retCtrlAddr = PMU_GLOBAL_RAM_RET_CNTRL,
		.retCtrlMask = PMU_GLOBAL_RAM_RET_CNTRL_TCM1B_MASK,
	},
	.size = 0x10000U,
	.base = 0xffeb0000U,
	.eccInit = PmTcm1EccInit,
	.id = PM_TCM_1B_BANK_ID,
};

#endif
