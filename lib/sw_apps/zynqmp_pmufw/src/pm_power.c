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
 * Power nodes (power islands and power domains) related structures,
 * transition actions, and FSM definition.
 *********************************************************************/

#include "pm_power.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_master.h"
#include "pm_reset.h"
#include "pm_sram.h"
#include "pm_periph.h"
#include "pm_pll.h"
#include "pm_usb.h"
#include "xpfw_rom_interface.h"
#include "crf_apb.h"
#include "pm_system.h"
#include "pm_ddr.h"
#include "apu.h"
#include "pm_clock.h"
#include "rpu.h"
#include "xpfw_util.h"
#include "pm_gpp.h"

#define DEFINE_PM_POWER_CHILDREN(c)	.children = (c), \
					.childCnt = ARRAY_SIZE(c)

#define PM_POWER_SUPPLYCHECK_TIMEOUT	100000U

/**
 * PmPowerStack() - Used to construct stack for implementing non-recursive
 *		depth-first search in power graph
 * @power	Power node pushed/popped from stack
 * @index	Index of the child to be visited when the power node gets popped
 */
typedef struct PmPowerStack {
	PmPower* power;
	u8 index;
} PmPowerStack;

/**
 * PmPowerDfs() - Captures current state of depth-first search in power graph
 * @power	Currently visited power node
 * @it		Iterator/index of the power's child that is currently visited
 * @sp		Power stack pointer
 */
typedef struct PmPowerDfs {
	PmPower* power;
	u8 it;
	u8 sp;
} PmPowerDfs;

/*
 * Stack size is equal to the number of levels in power hierarchy. Power
 * hierarchy has only 2 levels: power islands and domains, therefore the stack
 * size is 2. If this changes in future the stack size should be incremented.
 */
static PmPowerStack pmPowerStack[2];
static PmPowerDfs pmDfs;

/**
 * PmPowerStackPush() - Push power/index on stack
 * @power	Power node pushed on stack
 * @index	Index of a child which should be visited upon pop
 */
void PmPowerStackPush(PmPower* const power, const u8 index)
{
	/* Stack overflow should never happen */
	if (ARRAY_SIZE(pmPowerStack) == pmDfs.sp) {
		PmDbg("ERROR: stack overflow!\r\n");
		goto done;
	}
	pmPowerStack[pmDfs.sp].power = power;
	pmPowerStack[pmDfs.sp].index = index;
	pmDfs.sp++;
done:
	return;
}

/**
 * PmPowerStackPop() - Pop power/index from stack
 * @power	Pointer to the location where to store popped power node pointer
 * @index	Pointer to the location where to store popped index
 */
void PmPowerStackPop(PmPower** const power, u8* const index)
{
	if (0U == pmDfs.sp) {
		/* This should never happen */
		PmDbg("ERROR: empty stack!\r\n");
		goto done;
	}
	pmDfs.sp--;
	*power = pmPowerStack[pmDfs.sp].power;
	*index = pmPowerStack[pmDfs.sp].index;

	/* Clearing is not needed, but it's nice for debugging */
	(void)memset(&pmPowerStack[pmDfs.sp], 0U, sizeof(PmPowerStack));

done:
	return;
}

/**
 * PmPowerStackIsEmpty() - Check if power stack is empty
 * @return	True if empty, false otherwise
 */
static inline bool PmPowerStackIsEmpty(void)
{
	return 0U == pmDfs.sp;
}

/**
 * PmPowerDfsBegin() - Prepare for the power graph search
 * @power	Power node which is the root of the searched graph
 */
static void PmPowerDfsBegin(PmPower* const power)
{
	/* Clearing stack is not needed, but it's nice for debugging */
	(void)memset(pmPowerStack, 0U, sizeof(pmPowerStack));
	pmDfs.sp = 0U;
	pmDfs.it = 0U;
	pmDfs.power = NULL;
	PmPowerStackPush(power, 0U);
}

/**
 * PmPowerDfsGetNext() - Get next node (DFS)
 * @return	Pointer to the next node or NULL if all nodes are visited
 */
static PmNode* PmPowerDfsGetNext(void)
{
	PmNode* node = NULL;

	while ((NULL != pmDfs.power) || (false == PmPowerStackIsEmpty())) {
		if (NULL == pmDfs.power) {
			PmPowerStackPop(&pmDfs.power, &pmDfs.it);
		}
		if (pmDfs.power->childCnt == pmDfs.it) {
			node = &pmDfs.power->node;
			pmDfs.power = NULL;
			goto done;
		}
		if (NODE_IS_POWER(pmDfs.power->children[pmDfs.it])) {
			PmPowerStackPush(pmDfs.power, pmDfs.it + 1U);
			PmPowerStackPush(pmDfs.power->children[pmDfs.it]->derived, 0U);
			pmDfs.power = NULL;
		} else {
			node = pmDfs.power->children[pmDfs.it];
			pmDfs.it++;
			goto done;
		}
	}

done:
	return node;
}

/*
 * Note: PLL registers will never be saved/restored as part of CRF_APB module
 * context. PLLs have separate logic, which is part of PmSlavePll (pm_pll.h/c)
 */
static PmRegisterContext pmFpdContext[] = {
	{ .addr = CRF_APB_ERR_CTRL },
	{ .addr = CRF_APB_CRF_WPROT },
	{ .addr = CRF_APB_ACPU_CTRL, },
	{ .addr = CRF_APB_DBG_TRACE_CTRL },
	{ .addr = CRF_APB_DBG_FPD_CTRL },
	{ .addr = CRF_APB_DP_VIDEO_REF_CTRL },
	{ .addr = CRF_APB_DP_AUDIO_REF_CTRL },
	{ .addr = CRF_APB_DP_STC_REF_CTRL },
	{ .addr = CRF_APB_DDR_CTRL },
	{ .addr = CRF_APB_GPU_REF_CTRL },
	{ .addr = CRF_APB_SATA_REF_CTRL },
	{ .addr = CRF_APB_PCIE_REF_CTRL },
	{ .addr = CRF_APB_GDMA_REF_CTRL },
	{ .addr = CRF_APB_DPDMA_REF_CTRL },
	{ .addr = CRF_APB_TOPSW_MAIN_CTRL },
	{ .addr = CRF_APB_TOPSW_LSBUS_CTRL },
	{ .addr = CRF_APB_GTGREF0_REF_CTRL },
	{ .addr = CRF_APB_DBG_TSTMP_CTRL },
	{ .addr = CRF_APB_RST_FPD_TOP },
	{ .addr = CRF_APB_RST_FPD_APU },
	{ .addr = APU_PWRCTL },
};

/**
 * PmPowerSupplyCheck() - Wrapper for PMU-ROM power supply check handler
 * @RomHandler  Default PMU-ROM handler for power supply check
 *
 * @return      The PMU-ROM handler's return value
 *
 * @note        The wrapper just introduces a timeout based on counting.
 *              This function should be replaced by either Sysmon-based check
 *              or custom/board specific implementation.
 */
static u32 PmPowerSupplyCheck(XpbrServHndlr_t RomHandler)
{
	int status;
	u32 var = 0U;

	/* Cheat compiler to not optimize timeout based on counting */
	XPfw_UtilPollForMask((u32)&var, ~var, PM_POWER_SUPPLYCHECK_TIMEOUT);

	status = RomHandler();

	return status;
}

/**
 * PmPowerInit() - Initialize power (call only upon boot)
 */
void PmPowerInit(void)
{
	XpbrServExtTbl[XPBR_SERV_EXT_FPD_SUPPLYCHECK] = PmPowerSupplyCheck;
	XpbrServExtTbl[XPBR_SERV_EXT_PLD_SUPPLYCHECK] = PmPowerSupplyCheck;
}

/**
 * PmFpdSaveContext() - Save context of CRF_APB module due to powering down FPD
 */
static void PmFpdSaveContext(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmFpdContext); i++) {
		pmFpdContext[i].value = XPfw_Read32(pmFpdContext[i].addr);
	}
}

/**
 * PmFpdRestoreContext() - Restore context of CRF_APB module (FPD has been
 *                         powered up)
 */
static void PmFpdRestoreContext(void)
{
	u32 i;

	for (i = 0U; i < ARRAY_SIZE(pmFpdContext); i++) {
		XPfw_Write32(pmFpdContext[i].addr, pmFpdContext[i].value);
	}
}

/**
 * PmPowerDownFpd() - Power down FPD domain
 *
 * @return      Status of the pmu-rom operations
 */
static int PmPowerDownFpd(void)
{
	int status;

	PmFpdSaveContext();

	ddr_io_retention_set(true);

	PmResetAssertInt(PM_RESET_FPD, PM_RESET_ACTION_ASSERT);

	status = XpbrPwrDnFpdHandler();
	if (XST_SUCCESS != status) {
		goto err;
	}

	/*
	 * When FPD is powered off, the APU-GIC will be affected too.
	 * GIC Proxy has to take over for all wake-up sources for
	 * the APU.
	 */
	pmMasterApu_g.gic->enable();

err:
	return status;
}

/**
 * PmPowerDownLpd() - Power down LPD domain
 *
 * @return      XST_SUCCESS always (not implemented)
 */
static int PmPowerDownLpd(void)
{
	return XST_SUCCESS;
}

/**
 * PmPowerUpRpu() - Power up RPU island
 *
 * @return      Status returned by the PMU-ROM handler
 */
static int PmPowerUpRpu(void)
{
	int status;
	u32 resetMask = CRL_APB_RST_LPD_TOP_RPU_PGE_RESET_MASK |
			CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK |
			CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK |
			CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK;

	status = XpbrPwrUpRpuHandler();

	if (XST_SUCCESS != status) {
		goto done;
	}

	/* Put both the R5s in HALT */
	XPfw_RMW32(RPU_RPU_0_CFG,
		   RPU_RPU_0_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_0_CFG_NCPUHALT_MASK);
	XPfw_RMW32(RPU_RPU_1_CFG,
		   RPU_RPU_1_CFG_NCPUHALT_MASK,
		  ~RPU_RPU_1_CFG_NCPUHALT_MASK);

	/* Release RPU island reset */
	XPfw_RMW32(CRL_APB_RST_LPD_TOP, resetMask, ~resetMask);

done:
	return status;
}

/**
 * PmPowerUpFpd() - Power up FPD domain
 *
 * @return      Status of the pmu-rom operations
 */
static int PmPowerUpFpd(void)
{
	int status = XpbrPwrUpFpdHandler();
	if (XST_SUCCESS != status) {
		goto err;
	}

	status = PmResetAssertInt(PM_RESET_FPD, PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != status) {
		goto err;
	}

	PmFpdRestoreContext();

err:
	return status;
}

/**
 * PmPowerDownRpu() - Wrapper for powering down RPU (due to the return cast)
 * @return      Return value of PMU-ROM handler
 */
static int PmPowerDownRpu(void)
{
	return XpbrPwrDnRpuHandler();
}

/**
 * PmPowerDownPld() - Wrapper for powering down PLD (due to the return cast)
 * @return      Return value of PMU-ROM handler
 */
static int PmPowerDownPld(void)
{
	return XpbrPwrDnPldHandler();
}

/**
 * PmPowerUpPld() - Wrapper for powering up PLD (due to the return cast)
 * @return      Return value of PMU-ROM handler
 */
static int PmPowerUpPld(void)
{
	return XpbrPwrUpPldHandler();
}

/**
 * PmPowerDown() - Power down the power node
 * @power       Power node in question
 *
 * @return      Status of powering down (what powerDown handler returns or
 *              XST_SUCCESS)
 */
static int PmPowerDown(PmPower* const power)
{
	int status = XST_SUCCESS;

	if (PM_PWR_STATE_OFF == power->node.currState) {
		goto done;
	}

	if (NULL != power->powerDown) {
		status = power->powerDown();
	}

	if (XST_SUCCESS != status) {
		goto done;
	}

	PmNodeUpdateCurrState(&power->node, PM_PWR_STATE_OFF);

done:
	PmDbg("%s\r\n", PmStrNode(power->node.nodeId));
	return status;
}

/**
 * PmPowerUp() - Power up island/domain
 * @power      Power node to be powered up
 *
 * @return  Operation status of power up procedure (node specific) or
 *          XST_SUCCESS
 */
static int PmPowerUp(PmPower* const power)
{
	int status = XST_SUCCESS;

	PmDbg("%s\r\n", PmStrNode(power->node.nodeId));

	if (PM_PWR_STATE_ON == power->node.currState) {
		goto done;
	}

	if (NULL != power->powerUp) {
		status = power->powerUp();
	}

	if (XST_SUCCESS != status) {
		goto done;
	}

	PmNodeUpdateCurrState(&power->node, PM_PWR_STATE_ON);

done:
	return status;
}

/* Children array definitions */
static PmNode* pmApuChildren[] = {
	&pmProcApu0_g.node,
	&pmProcApu1_g.node,
	&pmProcApu2_g.node,
	&pmProcApu3_g.node,
};

static PmNode* pmRpuChildren[] = {
	&pmProcRpu0_g.node,
	&pmProcRpu1_g.node,
	&pmSlaveTcm0A_g.slv.node,
	&pmSlaveTcm0B_g.slv.node,
	&pmSlaveTcm1A_g.slv.node,
	&pmSlaveTcm1B_g.slv.node,
};

static PmNode* pmFpdChildren[] = {
	&pmPowerIslandApu_g.node,
	&pmSlaveL2_g.slv.node,
	&pmSlaveSata_g.node,
	&pmSlaveApll_g.slv.node,
	&pmSlaveVpll_g.slv.node,
	&pmSlaveDpll_g.slv.node,
	&pmSlaveGpu_g.node,
	&pmSlaveGpuPP0_g.slv.node,
	&pmSlaveGpuPP1_g.slv.node,
	&pmSlaveGdma_g.node,
	&pmSlaveDP_g.node,
	&pmSlaveDdr_g.node,
	&pmSlavePcie_g.node,
};

static PmNode* pmLpdChildren[] = {
	&pmPowerIslandRpu_g.node,
	&pmSlaveOcm0_g.slv.node,
	&pmSlaveOcm1_g.slv.node,
	&pmSlaveOcm2_g.slv.node,
	&pmSlaveOcm3_g.slv.node,
	&pmSlaveUsb0_g.slv.node,
	&pmSlaveUsb1_g.slv.node,
	&pmSlaveTtc0_g.node,
	&pmSlaveTtc1_g.node,
	&pmSlaveTtc2_g.node,
	&pmSlaveTtc3_g.node,
	&pmSlaveSata_g.node,
	&pmSlaveRpll_g.slv.node,
	&pmSlaveIOpll_g.slv.node,
	&pmSlaveUart0_g.node,
	&pmSlaveUart1_g.node,
	&pmSlaveSpi0_g.node,
	&pmSlaveSpi1_g.node,
	&pmSlaveI2C0_g.node,
	&pmSlaveI2C1_g.node,
	&pmSlaveSD0_g.node,
	&pmSlaveSD1_g.node,
	&pmSlaveCan0_g.node,
	&pmSlaveCan1_g.node,
	&pmSlaveEth0_g.node,
	&pmSlaveEth1_g.node,
	&pmSlaveEth2_g.node,
	&pmSlaveEth3_g.node,
	&pmSlaveAdma_g.node,
	&pmSlaveNand_g.node,
	&pmSlaveQSpi_g.node,
	&pmSlaveGpio_g.node,
	&pmSlaveAFI_g.node,
	&pmSlaveIpiApu_g.node,
	&pmSlaveIpiRpu0_g.node,
	&pmSlavePcap_g.node,
};

/* Dummy consumption for the power domains/islands */
static u32 PmDomainPowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_OFF,
};

/*
 * Power Island and Power Domain definitions
 *
 * We only define those islands and domains containing more than 1 node.
 * For optimization reasons private power islands, such as APU0-island or
 * USB0-island are modeled as a feature of the node itself and are therefore
 * not described here.
 */
PmPower pmPowerIslandRpu_g = {
	.node = {
		.derived = &pmPowerIslandRpu_g,
		.nodeId = NODE_RPU,
		.class = &pmNodeClassPower_g,
		.parent = &pmPowerDomainLpd_g,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDomainPowers),
	},
	DEFINE_PM_POWER_CHILDREN(pmRpuChildren),
	.powerUp = PmPowerUpRpu,
	.powerDown = PmPowerDownRpu,
	.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
	.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
	.forcePerms = 0U,
	.reqPerms = 0U,
	.requests = 0U,
};

/*
 * @Note: The APU power island does not physically exist, therefore it has
 * no operations and no latencies. The individual APU cores have their own
 * dedicated power islands, the transition latency is hence accounted for
 * in PmProc
 */
PmPower pmPowerIslandApu_g = {
	.node = {
		.derived = &pmPowerIslandApu_g,
		.nodeId = NODE_APU,
		.class = &pmNodeClassPower_g,
		.parent = &pmPowerDomainFpd_g,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDomainPowers),
	},
	DEFINE_PM_POWER_CHILDREN(pmApuChildren),
	.powerUp = NULL,
	.powerDown = NULL,
	.pwrDnLatency = 0,
	.pwrUpLatency = 0,
	.forcePerms = 0U,
	.reqPerms = 0U,
	.requests = 0U,
};

PmPower pmPowerDomainFpd_g = {
	.node = {
		.derived = &pmPowerDomainFpd_g,
		.nodeId = NODE_FPD,
		.class = &pmNodeClassPower_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDomainPowers),
	},
	DEFINE_PM_POWER_CHILDREN(pmFpdChildren),
	.powerUp = PmPowerUpFpd,
	.powerDown = PmPowerDownFpd,
	.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
	.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
	.forcePerms = 0U,
	.reqPerms = 0U,
	.requests = 0U,
};

PmPower pmPowerDomainLpd_g = {
	.node = {
		.derived = &pmPowerDomainLpd_g,
		.nodeId = NODE_LPD,
		.class = &pmNodeClassPower_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDomainPowers),
	},
	DEFINE_PM_POWER_CHILDREN(pmLpdChildren),
	.powerUp = NULL,
	.powerDown = PmPowerDownLpd,
	.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
	.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
	.forcePerms = 0U,
	.reqPerms = 0U,
	.requests = 0U,
};

PmPower pmPowerDomainPld_g = {
	.node = {
		.derived = &pmPowerDomainPld_g,
		.nodeId = NODE_PL,
		.class = &pmNodeClassPower_g,
		.parent = NULL,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmDomainPowers),
	},
	.children = NULL,
	.childCnt = 0U,
	.powerUp = PmPowerUpPld,
	.powerDown = PmPowerDownPld,
	.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
	.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
	.forcePerms = 0U,
	.reqPerms = 0U,
	.requests = 0U,
};

/**
 * PmChildIsInLowestPowerState() - Checked whether the child node is in lowest
 *                                 power state
 * @nodePtr     Pointer to a node whose state should be checked
 */
static bool PmChildIsInLowestPowerState(const PmNode* const nodePtr)
{
	bool status = false;

	if (NODE_IS_PROC(nodePtr) || NODE_IS_POWER(nodePtr)) {
		if (true == NODE_IS_OFF(nodePtr)) {
			status = true;
		}
	} else {
		/* Node is a slave */
		if (false == PmSlaveRequiresPower((PmSlave*)nodePtr->derived)) {
			status = true;
		}
	}

	return status;
}

/**
 * PmHasAwakeChild() - Check whether power node has awake children
 * @power       Pointer to PmPower object to be checked
 *
 * Used during opportunistic suspend:
 * Function checks whether any child of the power provided as argument stops
 * power from being turned off. In the case of processor or power child, that
 * can be checked by inspecting currState value. For slaves, that is not the
 * case, as slave can be in non-off state just because the off state is entered
 * when power is turned off. This is the case when power parent is common for
 * multiple nodes. Therefore, slave does not block power from turning off if
 * it is unused and not in lowest power state.
 *
 * Latency accounting: determine the lowest latency requirement of any child
 * and pass it up to the power island/domain node.
 *
 * @return      True if it has a child that is not off
 */
static bool PmHasAwakeChild(PmPower* const power)
{
	u32 i;
	u32 minLatencyMargin = MAX_LATENCY;
	bool hasAwakeChild = false;

	for (i = 0U; i < power->childCnt; i++) {
		/* Determine the lowest latency requirement of any child */
		if (power->children[i]->latencyMarg < minLatencyMargin) {
			minLatencyMargin = power->children[i]->latencyMarg;
		}

		if (false == PmChildIsInLowestPowerState(power->children[i])) {
			hasAwakeChild = true;
			PmDbg("%s\r\n", PmStrNode(power->children[i]->nodeId));
			break;
		}
	}

	/* Pass the lowest latency margin to the power island/domain node */
	power->node.latencyMarg = minLatencyMargin;

	return hasAwakeChild;
}

/**
 * PmOpportunisticSuspend() - After a node goes to sleep, try to power off
 *                            parents
 * @powerParent Pointer to the power node which should try to suspend, as well
 *              its parents.
 */
void PmOpportunisticSuspend(PmPower* const powerParent)
{
	u32 worstCaseLatency;
	PmPower* power = powerParent;

	if ((NULL == power) || NODE_IS_OFF(&power->node)) {
		goto done;
	}

	do {
		PmDbg("Opportunistic suspend attempt for %s\r\n",
		      PmStrNode(power->node.nodeId));

		worstCaseLatency = power->pwrUpLatency + power->pwrDnLatency;

		if ((false == PmHasAwakeChild(power)) &&
		    (0U == power->requests)) {
			/* Note: latencyMarg field updated by PmHasAwakeChild */
			if (worstCaseLatency < power->node.latencyMarg) {
				(void)PmPowerDown(power);
				power = power->node.parent;
				continue;
			}
		}
		power = NULL;

	} while (NULL != power);

done:
	return;
}

/**
 * PmPowerUpTopParent() - Power up top parent in hierarchy that's currently off
 * @powerChild  Power child whose power parent has to be powered up
 *
 * @return      Status of the power up operation (XST_SUCCESS if all power
 *              parents are already powered on)
 *
 * This function turns on exactly one power parent, starting with the highest
 * level parent that's currently off. If all power parents are on, it will
 * turn on "powerChild", which was passed as an argument.
 *
 * Since MISRA-C doesn't allow recursion, there's an iterative algorithm in
 * PmTriggerPowerUp that calls this function iteratively until all power
 * nodes in the hierarchy are powered up.
 */
static int PmPowerUpTopParent(PmPower* const powerChild)
{
	int status = XST_SUCCESS;
	PmPower* powerParent = powerChild;

	if (NULL == powerParent) {
		status = XST_PM_INTERNAL;
		goto done;
	}

	/*
	 * Powering up needs to happen from the top down, so find the highest
	 * level parent that's currently still off and turn it on.
	 */
	while ((NULL != powerParent->node.parent) &&
	       (true == NODE_IS_OFF(&powerParent->node.parent->node))) {
		powerParent = powerChild->node.parent;
	}

	status = PmPowerUp(powerParent);

done:
	return status;
}

/**
 * PmTriggerPowerUp() - Triggered by child node (processor or slave) when it
 *                      needs its power islands/domains to be powered up
 * @power       Power node that needs to be powered up
 *
 * @return      Status of the power up operation.
 */
int PmTriggerPowerUp(PmPower* const power)
{
	int status = XST_SUCCESS;

	if (NULL == power) {
		goto done;
	}

	/*
	 * Multiple hierarchy levels of power islands/domains may need to be
	 * turned on (always top-down).
	 * Use iterative approach for MISRA-C compliance
	 */
	while ((true == NODE_IS_OFF(&power->node)) && (XST_SUCCESS == status)) {
		status = PmPowerUpTopParent(power);
	}

done:
#ifdef DEBUG_PM
	if (XST_SUCCESS != status) {
		PmDbg("ERROR #%d failed to power up\r\n", status);
	}
#endif

	return status;
}

/**
 * PmPowerMasterRequest() - Explicit request for power node (power up)
 * @master	Master which has requested power node
 * @power	The requested node
 * @return	Status of processing request
 * @note	If the request is processed successfully the power node ON
 *		state is granted to the caller
 */
int PmPowerMasterRequest(const PmMaster* const master, PmPower* const power)
{
	int status = XST_SUCCESS;

	/* Check whether the master is allowed to request the power node */
	if (0U == (master->ipiMask & power->reqPerms)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check whether the master has already requested the power node */
	if (0U != (master->ipiMask & power->requests)) {
		status = XST_PM_DOUBLE_REQ;
		goto done;
	}

	/* Power up the whole power parent hierarchy if needed */
	if (true == NODE_IS_OFF(&power->node)) {
		status = PmTriggerPowerUp(power);
	}

	/* Remember master's mask if request is processed successfully */
	if (XST_SUCCESS == status) {
		power->requests |= master->ipiMask;
	}

done:
	return status;
}

/**
 * PmPowerMasterRelease() - Explicit release for power node
 * @master	Master which has requested power node
 * @power	The requested node
 * @return	Status of processing request
 * @note	If the master has previously had the grant for the power node
 *		state, after this call is performed the power node may be
 *		turned off. Whether it will be turned off depends on other
 *		requests (related to slaves, latencies, etc.)
 */
int PmPowerMasterRelease(const PmMaster* const master, PmPower* const power)
{
	int status = XST_SUCCESS;

	/* Check whether the master has permissions for the power node ops */
	if (0U == (master->ipiMask & power->reqPerms)) {
		status = XST_PM_NO_ACCESS;
		goto done;
	}

	/* Check whether the master has previously requested the power node */
	if (0U == (master->ipiMask & power->requests)) {
		status = XST_FAILURE;
		goto done;
	}

	/* Clear the request flag */
	power->requests &= ~master->ipiMask;

	/*
	 * If no other master has explicitely requested power node we call
	 * opportunistic suspend. It will take care of all dependencies that
	 * might exist with respect to the slaves or latencies. If there are
	 * no dependencies the power node will be powered down.
	 */
	if (0U == power->requests) {
		PmOpportunisticSuspend(power);
	}

done:
	return status;
}

/**
 * PmPowerClearConfig() - Clear configuration of the power node
 * @powerNode	Pointer to the power node
 */
static void PmPowerClearConfig(PmNode* const powerNode)
{
	PmPower* const power = (PmPower*)powerNode->derived;

	power->reqPerms = 0U;
	power->forcePerms = 0U;
	power->requests = 0U;
}

/**
 * PmPowerGetWakeUpLatency() - Get wake-up latency of the power node
 * @node	Power node whose wake-up latency should be get
 * @lat		Pointer to the location where the latency value should be stored
 *
 * @return	XST_SUCCESS always
 */
static int PmPowerGetWakeUpLatency(const PmNode* const node, u32* const lat)
{
	PmPower* const power = (PmPower*)node->derived;
	PmPower* parent = power->node.parent;

	*lat = 0U;
	if (PM_PWR_STATE_ON == node->currState) {
		goto done;
	}

	*lat = power->pwrUpLatency;

	/* Account latencies of powered down parents (if a parent is down) */
	while (NULL != parent) {
		if (PM_PWR_STATE_ON == parent->node.currState) {
			break;
		}
		*lat += parent->pwrUpLatency;
		parent = parent->node.parent;
	}

done:
	return XST_SUCCESS;
}

/**
 * PmPowerGetPowerData() - Get power consumption of the node
 * @powerNode	Power node whose power consumption should be get
 * @data	Pointer to the location where the power data should be stored
 *
 * @return	XST_SUCCESS if power consumption data is stored in *data
 *		XST_NO_FEATURE otherwise
 * @note	Power consumption of power node is a sum of consumptions of the
 *		children.
 */
static int PmPowerGetPowerData(const PmNode* const powerNode, u32* const data)
{
	PmNode* node;
	u32 val;
	int status = XST_NO_FEATURE;

	*data = 0U;
	if (PM_PWR_STATE_OFF == powerNode->currState) {
		status = XST_SUCCESS;
		goto done;
	}

	PmPowerDfsBegin((PmPower*)powerNode->derived);
	node = PmPowerDfsGetNext();
	while (NULL != node) {
		if (NODE_IS_POWER(node)) {
			status = PmNodeGetPowerInfo(node, &val);
		} else {
			if (NULL != node->class->getPowerData) {
				status = node->class->getPowerData(node, &val);
			} else {
				status = XST_NO_FEATURE;
			}
		}
		if (XST_SUCCESS != status) {
			goto done;
		}
		*data += val;
		node = PmPowerDfsGetNext();
	}

done:
	return status;
}

/**
 * PmPowerForceDown() - Force down the power node and all of its children
 * @powerNode	Power node to force down
 *
 * @return	Status of performing force power down
 */
static int PmPowerForceDown(PmNode* const powerNode)
{
	PmNode* node;
	int status = XST_FAILURE;

	PmPowerDfsBegin((PmPower*)powerNode->derived);
	node = PmPowerDfsGetNext();
	while (NULL != node) {
		if (NODE_IS_POWER(node)) {
			status = PmPowerDown((PmPower*)node->derived);
		} else {
			status = PmNodeForceDown(node);
		}
		if (XST_SUCCESS != status) {
			goto done;
		}
		node = PmPowerDfsGetNext();
	}

done:
	return status;
}

/* Collection of power nodes */
static PmNode* pmNodePowerBucket[] = {
	&pmPowerIslandRpu_g.node,
	&pmPowerIslandApu_g.node,
	&pmPowerDomainFpd_g.node,
	&pmPowerDomainPld_g.node,
	&pmPowerDomainLpd_g.node,
};

PmNodeClass pmNodeClassPower_g = {
	DEFINE_NODE_BUCKET(pmNodePowerBucket),
	.id = NODE_CLASS_POWER,
	.clearConfig = PmPowerClearConfig,
	.getWakeUpLatency = PmPowerGetWakeUpLatency,
	.getPowerData = PmPowerGetPowerData,
	.forceDown = PmPowerForceDown,
};
