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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*********************************************************************
 * Definitions of PM slave SRAM structures and state transitions.
 *********************************************************************/

#include "pm_sram.h"
#include "pm_common.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

#define DEFTR(INST, TRAN) ((INST * PM_SRAM_TR_MAX) + TRAN)

/* Ocm bank 0 */
static u32 PmOcm0RetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK0_MASK,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmOcm0RetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK0_MASK,
		   ~PMU_LOCAL_OCM_RET_CNTRL_BANK0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

/* Ocm bank 1 */
static u32 PmOcm1RetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK1_MASK,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmOcm1RetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK1_MASK,
		   ~PMU_LOCAL_OCM_RET_CNTRL_BANK1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

/* Ocm bank 2 */
static u32 PmOcm2RetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK2_MASK,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK2_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmOcm2RetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK2_MASK,
		   ~PMU_LOCAL_OCM_RET_CNTRL_BANK2_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

/* Ocm bank 3 */
static u32 PmOcm3RetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK3_MASK,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK3_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmOcm3RetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_OCM_RET_CNTRL,
		   PMU_LOCAL_OCM_RET_CNTRL_BANK3_MASK,
		   ~PMU_LOCAL_OCM_RET_CNTRL_BANK3_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm0ARetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA0_MASK,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm0ARetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA0_MASK,
		   ~PMU_LOCAL_TCM_RET_CNTRL_TCMA0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm0BRetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB0_MASK,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm0BRetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB0_MASK,
		   ~PMU_LOCAL_TCM_RET_CNTRL_TCMB0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm1ARetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA1_MASK,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm1ARetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMA1_MASK,
		   ~PMU_LOCAL_TCM_RET_CNTRL_TCMA1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm1BRetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB1_MASK,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmTcm1BRetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_TCM_RET_CNTRL,
		   PMU_LOCAL_TCM_RET_CNTRL_TCMB1_MASK,
		   ~PMU_LOCAL_TCM_RET_CNTRL_TCMB1_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmL2RetEntry(void)
{
	XPfw_RMW32(PMU_LOCAL_L2_RET_CNTRL,
		   PMU_LOCAL_L2_RET_CNTRL_BANK0_MASK,
		   PMU_LOCAL_L2_RET_CNTRL_BANK0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static u32 PmL2RetExit(void)
{
	XPfw_RMW32(PMU_LOCAL_L2_RET_CNTRL,
		   PMU_LOCAL_L2_RET_CNTRL_BANK0_MASK,
		   ~PMU_LOCAL_L2_RET_CNTRL_BANK0_MASK);

	PmDbg("%s\n", __func__);

	return XST_SUCCESS;
}

static const PmTranHandler pmSramActions_g[PM_SRAM_INST_MAX * PM_SRAM_TR_MAX] = {
	[ DEFTR(PM_SRAM_OCM0, PM_SRAM_TR_ON_TO_RET) ] = PmOcm0RetEntry,
	[ DEFTR(PM_SRAM_OCM0, PM_SRAM_TR_RET_TO_ON) ] = PmOcm0RetExit,
	[ DEFTR(PM_SRAM_OCM0, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnOcmBank0Handler,
	[ DEFTR(PM_SRAM_OCM0, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpOcmBank0Handler,

	[ DEFTR(PM_SRAM_OCM1, PM_SRAM_TR_ON_TO_RET) ] = PmOcm1RetEntry,
	[ DEFTR(PM_SRAM_OCM1, PM_SRAM_TR_RET_TO_ON) ] = PmOcm1RetExit,
	[ DEFTR(PM_SRAM_OCM1, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnOcmBank1Handler,
	[ DEFTR(PM_SRAM_OCM1, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpOcmBank1Handler,

	[ DEFTR(PM_SRAM_OCM2, PM_SRAM_TR_ON_TO_RET) ] = PmOcm2RetEntry,
	[ DEFTR(PM_SRAM_OCM2, PM_SRAM_TR_RET_TO_ON) ] = PmOcm2RetExit,
	[ DEFTR(PM_SRAM_OCM2, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnOcmBank2Handler,
	[ DEFTR(PM_SRAM_OCM2, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpOcmBank2Handler,

	[ DEFTR(PM_SRAM_OCM3, PM_SRAM_TR_ON_TO_RET) ] = PmOcm3RetEntry,
	[ DEFTR(PM_SRAM_OCM3, PM_SRAM_TR_RET_TO_ON) ] = PmOcm3RetExit,
	[ DEFTR(PM_SRAM_OCM3, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnOcmBank3Handler,
	[ DEFTR(PM_SRAM_OCM3, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpOcmBank3Handler,

	[ DEFTR(PM_SRAM_TCM0A, PM_SRAM_TR_ON_TO_RET) ] = PmTcm0ARetEntry,
	[ DEFTR(PM_SRAM_TCM0A, PM_SRAM_TR_RET_TO_ON) ] = PmTcm0ARetExit,
	[ DEFTR(PM_SRAM_TCM0A, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnTcm0AHandler,
	[ DEFTR(PM_SRAM_TCM0A, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpTcm0AHandler,

	[ DEFTR(PM_SRAM_TCM0B, PM_SRAM_TR_ON_TO_RET) ] = PmTcm0BRetEntry,
	[ DEFTR(PM_SRAM_TCM0B, PM_SRAM_TR_RET_TO_ON) ] = PmTcm0BRetExit,
	[ DEFTR(PM_SRAM_TCM0B, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnTcm0BHandler,
	[ DEFTR(PM_SRAM_TCM0B, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpTcm0BHandler,

	[ DEFTR(PM_SRAM_TCM1A, PM_SRAM_TR_ON_TO_RET) ] = PmTcm1ARetEntry,
	[ DEFTR(PM_SRAM_TCM1A, PM_SRAM_TR_RET_TO_ON) ] = PmTcm1ARetExit,
	[ DEFTR(PM_SRAM_TCM1A, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnTcm1AHandler,
	[ DEFTR(PM_SRAM_TCM1A, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpTcm1AHandler,

	[ DEFTR(PM_SRAM_TCM1B, PM_SRAM_TR_ON_TO_RET) ] = PmTcm1BRetEntry,
	[ DEFTR(PM_SRAM_TCM1B, PM_SRAM_TR_RET_TO_ON) ] = PmTcm1BRetExit,
	[ DEFTR(PM_SRAM_TCM1B, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnTcm1BHandler,
	[ DEFTR(PM_SRAM_TCM1B, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpTcm1BHandler,

	[ DEFTR(PM_SRAM_L2, PM_SRAM_TR_ON_TO_RET) ] = PmL2RetEntry,
	[ DEFTR(PM_SRAM_L2, PM_SRAM_TR_RET_TO_ON) ] = PmL2RetExit,
	[ DEFTR(PM_SRAM_L2, PM_SRAM_TR_ON_TO_OFF) ] = XpbrPwrDnL2Bank0Handler,
	[ DEFTR(PM_SRAM_L2, PM_SRAM_TR_OFF_TO_ON) ] = XpbrPwrUpL2Bank0Handler,
};

/* Sram states */
static const u32 pmSramStates_g[PM_SRAM_STATE_MAX] = {
	[PM_SRAM_STATE_OFF] = 0U,
	[PM_SRAM_STATE_RET] = PM_CAP_CONTEXT,
	[PM_SRAM_STATE_ON] = PM_CAP_ACCESS | PM_CAP_CONTEXT,
};

/* Sram transition table (from which to which state sram can transit) */
static const PmStateTran pmSramTransitions_g[PM_SRAM_TR_MAX] = {
	[PM_SRAM_TR_ON_TO_RET] = {
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_RET,
	},
	[PM_SRAM_TR_RET_TO_ON] = {
		.fromState = PM_SRAM_STATE_RET,
		.toState = PM_SRAM_STATE_ON,
	},
	[PM_SRAM_TR_ON_TO_OFF] = {
		.fromState = PM_SRAM_STATE_ON,
		.toState = PM_SRAM_STATE_OFF,
	},
	[PM_SRAM_TR_OFF_TO_ON] = {
		.fromState = PM_SRAM_STATE_OFF,
		.toState = PM_SRAM_STATE_ON,
	},
};

/* Sram FSM */
static const PmSlaveFsm slaveSramFsm = {
	.states = pmSramStates_g,
	.statesCnt = PM_SRAM_STATE_MAX,
	.trans = pmSramTransitions_g,
	.transCnt = PM_SRAM_TR_MAX,
	.actions = pmSramActions_g,
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
		.instId = PM_SRAM_L2,
		.reqs = pmL2Reqs,
		.reqsCnt = ARRAY_SIZE(pmL2Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_OCM0,
		.reqs = pmOcm0Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm0Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_OCM1,
		.reqs = pmOcm1Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm1Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_OCM2,
		.reqs = pmOcm2Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm2Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_OCM3,
		.reqs = pmOcm3Reqs,
		.reqsCnt = ARRAY_SIZE(pmOcm3Reqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_TCM0A,
		.reqs = pmTcm0AReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm0AReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_TCM0B,
		.reqs = pmTcm0BReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm0BReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_TCM1A,
		.reqs = pmTcm1AReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm1AReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
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
		.instId = PM_SRAM_TCM1B,
		.reqs = pmTcm1BReqs,
		.reqsCnt = ARRAY_SIZE(pmTcm1BReqs),
		.wake = NULL,
		.slvFsm = &slaveSramFsm,
	},
};
