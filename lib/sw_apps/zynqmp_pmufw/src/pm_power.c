/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

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
#include "pm_requirement.h"
#include "xpfw_rom_interface.h"
#include "crf_apb.h"
#include "pm_system.h"
#include "pm_ddr.h"
#include "apu.h"
#include "pm_clock.h"
#include "rpu.h"
#include "xpfw_util.h"
#include "pm_gpp.h"
#include "xpfw_aib.h"
#include "pm_hooks.h"
#include "xpfw_default.h"

#define DEFINE_PM_POWER_CHILDREN(c)	.children = ((c)), \
					.childCnt = ARRAY_SIZE((c))

#define PM_FPD_POWER_SUPPLYCHECK_TIMEOUT	727273U		/* Delay of 40ms */
#define PM_PLD_POWER_SUPPLYCHECK_TIMEOUT	100000U		/* Delay of 5ms */

#define AMS_PSSYSMON_CONFIG_REG2	0XFFA50908U

#define A53_DBG_0_EDPRCR_REG			(0xFEC10310U)	/* APU_0 external debug control */
#define A53_DBG_1_EDPRCR_REG			(0xFED10310U)	/* APU_1 external debug control */
#define A53_DBG_2_EDPRCR_REG			(0xFEE10310U)	/* APU_2 external debug control */
#define A53_DBG_3_EDPRCR_REG			(0xFEF10310U)	/* APU_3 external debug control */
#define A53_DBG_EDPRCR_REG_MASK			(0x00000009U)	/* COREPURQ and CORENPDRQ bit mask */

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
static void PmPowerStackPush(PmPower* const power, const u8 index)
{
	/* Stack overflow should never happen */
	if (ARRAY_SIZE(pmPowerStack) == pmDfs.sp) {
		PmAlert("Power stack overflow\r\n");
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
static void PmPowerStackPop(PmPower** const power, u8* const index)
{
	if (0U == pmDfs.sp) {
		/* This should never happen */
		PmAlert("Power stack empty\r\n");
		goto done;
	}
	pmDfs.sp--;
	*power = pmPowerStack[pmDfs.sp].power;
	*index = pmPowerStack[pmDfs.sp].index;

	/* Clearing is not needed, but it's nice for debugging */
	(void)memset(&pmPowerStack[pmDfs.sp], (s32)0U, sizeof(PmPowerStack));

done:
	return;
}

/**
 * PmPowerStackIsEmpty() - Check if power stack is empty
 * @return	True if empty, false otherwise
 */
static inline bool PmPowerStackIsEmpty(void)
{
	return (0U == pmDfs.sp);
}

/**
 * PmPowerDfsBegin() - Prepare for the power graph search
 * @power	Power node which is the root of the searched graph
 */
static void PmPowerDfsBegin(PmPower* const power)
{
	/* Clearing stack is not needed, but it's nice for debugging */
	(void)memset(pmPowerStack, (s32)0U, sizeof(pmPowerStack));
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
 * context. PLLs have separate logic, which is part of the PLL management
 * (see pm_pll.h/c)
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
 * PmFpdPowerSupplyCheck() - Wrapper for PMU-ROM FPD power supply check handler
 * @RomHandler  Default PMU-ROM handler for FPD power supply check
 *
 * @return      The PMU-ROM handler's return value
 *
 * @note        The wrapper just introduces a timeout based on counting.
 *              This function should be replaced by either Sysmon-based check
 *              or custom/board specific implementation.
 */
static u32 PmFpdPowerSupplyCheck(XpbrServHndlr_t RomHandler)
{
	u32 status;
	u32 var = 0U;

	/* Cheat compiler to not optimize timeout based on counting */
	(void)XPfw_UtilPollForMask((u32)&var, ~var,
				   PM_FPD_POWER_SUPPLYCHECK_TIMEOUT);

	status = RomHandler();

	return status;
}

/**
 * PmPldPowerSupplyCheck() - Wrapper for PMU-ROM PLD power supply check handler
 * @RomHandler  Default PMU-ROM handler for PLD power supply check
 *
 * @return      The PMU-ROM handler's return value
 *
 * @note        The wrapper just introduces a timeout based on counting.
 *              This function should be replaced by either Sysmon-based check
 *              or custom/board specific implementation.
 */
static u32 PmPldPowerSupplyCheck(XpbrServHndlr_t RomHandler)
{
	u32 status;
	u32 var = 0U;

	/* Cheat compiler to not optimize timeout based on counting */
	(void)XPfw_UtilPollForMask((u32)&var, ~var,
				   PM_PLD_POWER_SUPPLYCHECK_TIMEOUT);

	status = RomHandler();

	return status;
}

 /*
 * PmPowerDomainConstruct() - Constructor method to call for power domain node
 * @power	Power node of a domain
 */
static void PmPowerDomainConstruct(PmPower* const power)
{
	PmPowerDomain* pd = (PmPowerDomain*)power->node.derived;

	if (NULL != pd->supplyCheckHook) {
		XpbrServExtTbl[pd->supplyCheckHookId] = pd->supplyCheckHook;
	}
}

static PmPowerClass pmPowerClassDomain_g = {
	.construct = PmPowerDomainConstruct,
	.forceDown = NULL,
};

/**
 * PmFpdSaveContext() - Save context of CRF_APB module due to powering down FPD
 */
void PmFpdSaveContext(void)
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
void PmFpdRestoreContext(void)
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
static s32 PmPowerDownFpd(void)
{
	s32 status;
	u32 dbg0, dbg1, dbg2, dbg3;

	dbg0 = XPfw_Read32(A53_DBG_0_EDPRCR_REG);
	dbg1 = XPfw_Read32(A53_DBG_1_EDPRCR_REG);
	dbg2 = XPfw_Read32(A53_DBG_2_EDPRCR_REG);
	dbg3 = XPfw_Read32(A53_DBG_3_EDPRCR_REG);

	if (0U != (A53_DBG_EDPRCR_REG_MASK & (dbg0 | dbg1 | dbg2 | dbg3 ))) {
		PmInfo("Skipped FPD pwrdn (debugger connected)\r\n");
		return XST_SUCCESS;
	}

/* Block FPD power down if any of the LPD peripherals uses CCI path which is in FPD */
#ifdef XPAR_LPD_IS_CACHE_COHERENT
	PmErr("Blocked FPD pwrdn (CCI used by LPD)\r\n");
	status = XST_FAILURE;
	goto err;
#endif /* XPAR_LPD_IS_CACHE_COHERENT */

	PmFpdSaveContext();

	ddr_io_prepare();

	status = PmResetAssertInt(PM_RESET_FPD, PM_RESET_ACTION_ASSERT);
	if (XST_SUCCESS != status) {
		goto err;
	}

	XPfw_AibEnable(XPFW_AIB_LPD_TO_DDR);
	XPfw_AibEnable(XPFW_AIB_LPD_TO_FPD);

	status = (s32)XpbrPwrDnFpdHandler();
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
 * @return      Function doesn't return because LPD is powered down
 */
static s32 __attribute__((noreturn)) PmPowerDownLpd(void)
{
#ifdef ENABLE_POS
	/* Call user hook for finishing Power Off Suspend */
	PmHookFinalizePowerOffSuspend();
#endif

	/* Call user hook for powering down LPD */
	PmHookPowerDownLpd();

	while (true) {
		; /* For MISRA compliance */
	}
}

/**
 * PmPowerUpRpu() - Power up RPU island and disable AIBs
 *
 * @return      Status returned by the PMU-ROM handler
 */
static s32 PmPowerUpRpu(void)
{
	s32 status;

	status = (s32)XpbrPwrUpRpuHandler();

	if (XST_SUCCESS != status) {
		goto done;
	}

	XPfw_AibDisable(XPFW_AIB_RPU0_TO_LPD);
	XPfw_AibDisable(XPFW_AIB_RPU1_TO_LPD);
	XPfw_AibDisable(XPFW_AIB_LPD_TO_RPU0);
	XPfw_AibDisable(XPFW_AIB_LPD_TO_RPU1);

done:
	return status;
}

/**
 * PmPowerUpFpd() - Power up FPD domain
 *
 * @return      Status of the pmu-rom operations
 */
static s32 PmPowerUpFpd(void)
{
	if (0U != (XPfw_Read32(PMU_GLOBAL_PWR_STATE) &
				PMU_GLOBAL_PWR_STATE_FP_MASK)) {
		PmInfo("Skipped FPD pwrup (FPD is on)\r\n");
		return XST_SUCCESS;
	}

	s32 status = (s32)XpbrPwrUpFpdHandler();
	if (XST_SUCCESS != status) {
		goto err;
	}

	status = PmResetAssertInt(PM_RESET_FPD, PM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != status) {
		goto err;
	}

	XPfw_AibDisable(XPFW_AIB_LPD_TO_DDR);
	XPfw_AibDisable(XPFW_AIB_LPD_TO_FPD);

	PmFpdRestoreContext();

err:
	return status;
}

/**
 * PmPowerDownRpu() - Wrapper for powering down RPU (due to the return cast)
 *                    Enables AIBs at RPU interfaces to gracefully respond to
 *                    AXI transactions when RPU is powered down
 * @return      Return value of PMU-ROM handler
 */
static s32 PmPowerDownRpu(void)
{
	XPfw_AibEnable(XPFW_AIB_RPU0_TO_LPD);
	XPfw_AibEnable(XPFW_AIB_RPU1_TO_LPD);
	XPfw_AibEnable(XPFW_AIB_LPD_TO_RPU0);
	XPfw_AibEnable(XPFW_AIB_LPD_TO_RPU1);

	return (s32)XpbrPwrDnRpuHandler();
}

/**
 * PmPowerForceDownRpu() - Force down RPU island
 * @power	RPU power island
 */
static void PmPowerForceDownRpu(PmPower* const power)
{
	/*
	 * To access TCM, RPU island needs to be on. If RPU island if getting
	 * turned off, TCM also needs to be turned off. Otherwise, user may not
	 * get correct state of TCM whether TCM can be accessed or not.
	 */
	(void)PmNodeForceDown(&pmSlaveTcm0A_g.sram.slv.node);
	(void)PmNodeForceDown(&pmSlaveTcm0B_g.sram.slv.node);
	(void)PmNodeForceDown(&pmSlaveTcm1A_g.sram.slv.node);
	(void)PmNodeForceDown(&pmSlaveTcm1B_g.sram.slv.node);

	/* Reset RPU AMBA */
	(void)PmResetAssertInt(PM_RESET_RPU_AMBA, PM_RESET_ACTION_ASSERT);
}

/**
 * PmPowerDownPld() - Wrapper for powering down PLD (due to the return cast)
 * @return      Return value of PMU-ROM handler
 */
static s32 PmPowerDownPld(void)
{
	XPfw_AibEnable(XPFW_AIB_LPD_TO_AFI_FS2);
	XPfw_AibEnable(XPFW_AIB_FPD_TO_AFI_FS0);
	XPfw_AibEnable(XPFW_AIB_FPD_TO_AFI_FS1);

	return (s32)XpbrPwrDnPldHandler();
}

/**
 * PmPowerUpPld() - Wrapper for powering up PLD (due to the return cast)
 * @return      Return value of PMU-ROM handler
 */
static s32 PmPowerUpPld(void)
{
	s32 status;
	status = (s32)XpbrPwrUpPldHandler();
	if (XST_SUCCESS != status) {
		goto done;
	}

	XPfw_AibDisable(XPFW_AIB_LPD_TO_AFI_FS2);
	XPfw_AibDisable(XPFW_AIB_FPD_TO_AFI_FS0);
	XPfw_AibDisable(XPFW_AIB_FPD_TO_AFI_FS1);
done:
	return status;
}

/**
 * PmPowerPowerDownSysOsc() - Wrapper to put SysOsc in sleep mode.
 */
static void PmPowerDownSysOsc(void)
{
        u32 val;

        /* Put Sysosc in sleep mode */
        val = Xil_In32(AMS_PSSYSMON_CONFIG_REG2);
        val |= 0x30U;
        Xil_Out32(AMS_PSSYSMON_CONFIG_REG2, val);
}

/**
 * PmPowerPowerUpSysOsc() - Wrapper to put SysOsc in normal operation mode.
 */
static void PmPowerUpSysOsc(void)
{
        u32 val;

        /* Wake up SysOsc */
        val = Xil_In32(AMS_PSSYSMON_CONFIG_REG2);
        val &= 0xFF0FU;
        Xil_Out32(AMS_PSSYSMON_CONFIG_REG2, val);
}

/**
 * PmPowerDown() - Power down the power node
 * @power       Power node in question
 *
 * @return      Status of powering down (what powerDown handler returns or
 *              XST_SUCCESS)
 */
s32 PmPowerDown(PmPower* const power)
{
	s32 status = XST_SUCCESS;

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

	if (NULL != power->node.clocks) {
		PmClockRelease(&power->node);
	}
	PmInfo("%s 1->0\r\n", power->node.name);
#ifdef DEBUG_MODE
	if ((pmPowerIslandRpu_g.power.node.currState == PM_PWR_STATE_OFF) &&
                        (pmPowerDomainFpd_g.power.node.currState == PM_PWR_STATE_OFF)) {
		PmRequirement* req;
#if (STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR)
		req = PmRequirementGetNoMaster(&pmSlaveUart0_g);
#elif (STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR)
		req = PmRequirementGetNoMaster(&pmSlaveUart1_g);
#endif
		status = PmRequirementUpdate(req, 0U);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}
#endif

	/* Put SysOsc in sleep mode while going to deep sleep mode. */
	if ((pmPowerIslandRpu_g.power.node.currState == PM_PWR_STATE_OFF) &&
			(pmPowerDomainFpd_g.power.node.currState == PM_PWR_STATE_OFF) &&
			(pmPowerDomainPld_g.power.node.currState == PM_PWR_STATE_OFF) &&
			(pmIOpll_g.node.currState == PM_PLL_STATE_RESET) &&
			(pmRpll_g.node.currState == PM_PLL_STATE_RESET)) {
		PmPowerDownSysOsc();
	}

done:
	return status;
}

/**
 * PmPowerUp() - Power up island/domain
 * @power      Power node to be powered up
 *
 * @return  Operation status of power up procedure (node specific) or
 *          XST_SUCCESS
 */
static s32 PmPowerUp(PmPower* const power)
{
	s32 status = XST_SUCCESS;

	/* Enable SysOsc for normal operation if it is in sleep mode. */
	if ((Xil_In32(AMS_PSSYSMON_CONFIG_REG2) & 0xF0U) == 0x30U) {
		PmPowerUpSysOsc();
	}

#ifdef DEBUG_MODE
	PmRequirement* req;
#if (STDOUT_BASEADDRESS == XPMU_UART_0_BASEADDR)
	req = PmRequirementGetNoMaster(&pmSlaveUart0_g);
#elif (STDOUT_BASEADDRESS == XPMU_UART_1_BASEADDR)
	req = PmRequirementGetNoMaster(&pmSlaveUart1_g);
#endif
	status = PmRequirementUpdate(req, PM_CAP_ACCESS);
	if (XST_SUCCESS != status) {
		goto done;
	}
#endif

	PmInfo("%s 0->1\r\n", power->node.name);

	if (PM_PWR_STATE_ON == power->node.currState) {
		goto done;
	}

	if (NULL != power->powerUp) {
		status = power->powerUp();
	}

	if (XST_SUCCESS != status) {
		goto done;
	}

	if (NULL != power->node.clocks) {
		status = PmClockRequest(&power->node);
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
};

static PmNode* pmFpdChildren[] = {
	&pmPowerIslandApu_g.node,
	&pmApll_g.node,
	&pmVpll_g.node,
	&pmDpll_g.node,
	&pmSlaveL2_g.slv.node,
	&pmSlaveSata_g.node,
	&pmSlaveGpu_g.node,
	&pmSlaveGpuPP0_g.slv.node,
	&pmSlaveGpuPP1_g.slv.node,
	&pmSlaveGdma_g.node,
	&pmSlaveDP_g.node,
	&pmSlaveDdr_g.node,
	&pmSlavePcie_g.node,
};

static PmNode* pmLpdChildren[] = {
	&pmPowerIslandRpu_g.power.node,
	&pmRpll_g.node,
	&pmIOpll_g.node,
	&pmSlaveOcm0_g.slv.node,
	&pmSlaveOcm1_g.slv.node,
	&pmSlaveOcm2_g.slv.node,
	&pmSlaveOcm3_g.slv.node,
	&pmSlaveTcm0A_g.sram.slv.node,
	&pmSlaveTcm0B_g.sram.slv.node,
	&pmSlaveTcm1A_g.sram.slv.node,
	&pmSlaveTcm1B_g.sram.slv.node,
	&pmSlaveUsb0_g.slv.node,
	&pmSlaveUsb1_g.slv.node,
	&pmSlaveTtc0_g.node,
	&pmSlaveTtc1_g.node,
	&pmSlaveTtc2_g.node,
	&pmSlaveTtc3_g.node,
	&pmSlaveSata_g.node,
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
	&pmSlaveIpiApu_g.node,
	&pmSlaveIpiRpu0_g.node,
	&pmSlaveIpiRpu1_g.node,
	&pmSlaveIpiPl0_g.node,
	&pmSlaveIpiPl1_g.node,
	&pmSlaveIpiPl2_g.node,
	&pmSlaveIpiPl3_g.node,
	&pmSlavePcap_g.node,
};

static PmNode* pmPldChildren[] = {
	&pmSlaveVcu_g.slv.node,
	&pmSlavePl_g.node,
};

/* Dummy consumption for the power domains/islands */
static u8 PmDomainPowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_ON,
};

static u8 PmApuDomainPowers[] = {
	DEFAULT_POWER_OFF,
	DEFAULT_POWER_OFF,
};

static PmPowerClass pmPowerClassRpuIsland_g = {
	.construct = NULL,
	.forceDown = PmPowerForceDownRpu,
};

/*
 * Power Island and Power Domain definitions
 *
 * We only define those islands and domains containing more than 1 node.
 * For optimization reasons private power islands, such as APU0-island or
 * USB0-island are modeled as a feature of the node itself and are therefore
 * not described here.
 */
PmPowerIslandRpu pmPowerIslandRpu_g = {
	.power = {
		.node = {
			.derived = &pmPowerIslandRpu_g,
			.nodeId = NODE_RPU,
			.class = &pmNodeClassPower_g,
			.parent = &pmPowerDomainLpd_g.power,
			.clocks = NULL,
			.currState = PM_PWR_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmDomainPowers),
			DEFINE_NODE_NAME("rpu"),
		},
		DEFINE_PM_POWER_CHILDREN(pmRpuChildren),
		.class = &pmPowerClassRpuIsland_g,
		.powerUp = PmPowerUpRpu,
		.powerDown = PmPowerDownRpu,
		.pwrDnLatency = PM_POWER_ISLAND_LATENCY,
		.pwrUpLatency = PM_POWER_ISLAND_LATENCY,
		.forcePerms = 0U,
	},
	.deps = 0U,
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
		.parent = &pmPowerDomainFpd_g.power,
		.clocks = NULL,
		.currState = PM_PWR_STATE_ON,
		.latencyMarg = MAX_LATENCY,
		.flags = 0U,
		DEFINE_PM_POWER_INFO(PmApuDomainPowers),
		DEFINE_NODE_NAME("apu"),
	},
	DEFINE_PM_POWER_CHILDREN(pmApuChildren),
	.class = NULL,
	.powerUp = NULL,
	.powerDown = NULL,
	.pwrDnLatency = 0,
	.pwrUpLatency = 0,
	.forcePerms = 0U,
};

PmPowerDomain pmPowerDomainFpd_g = {
	.power = {
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
			DEFINE_NODE_NAME("fpd"),
		},
		DEFINE_PM_POWER_CHILDREN(pmFpdChildren),
		.class = &pmPowerClassDomain_g,
		.powerUp = PmPowerUpFpd,
		.powerDown = PmPowerDownFpd,
		.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
		.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
		.forcePerms = 0U,
		.useCount = 0U,
	},
	.supplyCheckHook = PmFpdPowerSupplyCheck,
	.supplyCheckHookId = XPBR_SERV_EXT_FPD_SUPPLYCHECK,
};

PmPowerDomain pmPowerDomainLpd_g = {
	.power = {
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
			DEFINE_NODE_NAME("lpd"),
		},
		DEFINE_PM_POWER_CHILDREN(pmLpdChildren),
		.class = &pmPowerClassDomain_g,
		.powerUp = NULL,
		.powerDown = PmPowerDownLpd,
		.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
		.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
		.forcePerms = 0U,
		.useCount = 0U,
	},
	.supplyCheckHook = NULL,
	.supplyCheckHookId = XPBR_SERV_EXT_TBL_BASE,
};

PmPowerDomain pmPowerDomainPld_g = {
	.power = {
		.node = {
			.derived = &pmPowerDomainPld_g,
			.nodeId = NODE_PLD,
			.class = &pmNodeClassPower_g,
			.parent = NULL,
			.clocks = NULL,
			.currState = PM_PWR_STATE_ON,
			.latencyMarg = MAX_LATENCY,
			.flags = 0U,
			DEFINE_PM_POWER_INFO(PmDomainPowers),
			DEFINE_NODE_NAME("pld"),
		},
		DEFINE_PM_POWER_CHILDREN(pmPldChildren),
		.class = &pmPowerClassDomain_g,
		.powerUp = PmPowerUpPld,
		.powerDown = PmPowerDownPld,
		.pwrDnLatency = PM_POWER_DOMAIN_LATENCY,
		.pwrUpLatency = PM_POWER_DOMAIN_LATENCY,
		.forcePerms = 0U,
		.useCount = 0U,
	},
	.supplyCheckHook = PmPldPowerSupplyCheck,
	.supplyCheckHookId = XPBR_SERV_EXT_PLD_SUPPLYCHECK,
};

/**
 * PmPowerUpdateLatencyMargin() - Update latency margin for the power node
 * @power	Power node to update
 */
static void PmPowerUpdateLatencyMargin(PmPower* const power)
{
	u32 i;

	/* Find minimum latency margin of all children */
	power->node.latencyMarg = MAX_LATENCY;
	for (i = 0U; i < power->childCnt; i++) {
		if (power->children[i]->latencyMarg < power->node.latencyMarg) {
			power->node.latencyMarg = power->children[i]->latencyMarg;
		}
	}
	if ((power->pwrDnLatency + power->pwrUpLatency) < power->node.latencyMarg) {
		power->node.latencyMarg -= power->pwrDnLatency + power->pwrUpLatency;
	} else {
		power->node.latencyMarg = 0U;
	}
}

/**
 * PmPowerDownCond() - Power the node down if conditions are satisfied
 * @power	Power node to conditionally power down
 *
 * @note	Conditions for powering down the node
 *		1) Use count is zero (power node is unused)
 *		2) Latency requirements of the children allow the power down
 */
static void PmPowerDownCond(PmPower* const power)
{
	if (0U == power->useCount) {
		PmPowerUpdateLatencyMargin(power);
		if (power->node.latencyMarg > 0U) {
			if (XST_SUCCESS != PmPowerDown(power)) {
				PmWarn("Error in power down for %s\r\n", power->node.name);
			}
		}
	}
}

/**
 * PmPowerUpdateLatencyReq() - Child updates its power parent about latency req
 * @node	Child node whose latency requirement have changed
 *
 * @return	If the change of the latency requirement caused the power up of
 *		the power parent, the status of performing power up operation
 *		is returned. Otherwise, XST_SUCCESS.
 */
s32 PmPowerUpdateLatencyReq(const PmNode* const node)
{
	s32 status = XST_SUCCESS;
	PmPower* power = node->parent;

	if (PM_PWR_STATE_ON == power->node.currState) {
		/* Try to power down the node if all conditions are ok */
		PmPowerDownCond(power);
		if (PM_PWR_STATE_OFF == power->node.currState) {
			if (NULL != power->node.parent) {
				PmPowerReleaseParent(&power->node);
			}
		}
		goto done;
	}

	/* Power is down, check if latency requirements trigger the power up */
	if (node->latencyMarg < (power->pwrDnLatency + power->pwrUpLatency)) {
		power->node.latencyMarg = 0U;
		if (NULL != power->node.parent) {
			status = PmPowerRequestParent(&power->node);
			if (XST_SUCCESS != status) {
				goto done;
			}
		}
		status = PmPowerUp(power);
	}

done:
	return status;
}

/**
 * PmPowerRequestInt() - Used internally to request a power node
 * @power	Requested power node
 *
 * @return	XST_SUCCESS if power is already powered up, otherwise status
 *		of powering up.
 */
static s32 PmPowerRequestInt(PmPower* const power)
{
	s32 status = XST_SUCCESS;

	if (PM_PWR_STATE_OFF == power->node.currState) {
		status = PmPowerUp(power);
	}

	if (XST_SUCCESS == status) {
		power->useCount++;
	}

	return status;
}

/**
 * PmPowerRequest() - Request for power to be powered up
 * @power	Requested power
 *
 * @return	XST_SUCCESS if power is already powered up, otherwise status
 *		of powering up.
 */
static s32 PmPowerRequest(PmPower* const power)
{
	s32 status = XST_SUCCESS;

	if (NULL != power->node.parent) {
		if (0U == (power->node.flags & NODE_LOCKED_POWER_FLAG)) {
			status = PmPowerRequestInt(power->node.parent);
			if (XST_SUCCESS != status) {
				goto done;
			}
			power->node.flags |= NODE_LOCKED_POWER_FLAG;
		}
	}
	status = PmPowerRequestInt(power);

done:
	return status;
}

/**
 * PmPowerReleaseInt() - Used internally to release the power node
 * @power	Power node
 */
static void PmPowerReleaseInt(PmPower* const power)
{
	if (power->useCount > 0U) {
		power->useCount--;
		PmPowerDownCond(power);
	}
}

/**
 * PmPowerRelease() - Release the power
 * @power	Released power
 */
static void PmPowerRelease(PmPower* const power)
{
	PmPowerReleaseInt(power);
	if (NULL != power->node.parent) {
		if (0U != (power->node.flags & NODE_LOCKED_POWER_FLAG)) {
			PmPowerReleaseInt(power->node.parent);
			power->node.flags &= ~(u8)NODE_LOCKED_POWER_FLAG;
		}
	}
}

/**
 * PmPowerRequestParent() - Request power parent to be powered up
 * @node	Node which requests its power parent
 *
 * @return	XST_SUCCESS if power parent is already up, status of powering up
 *		otherwise.
 */
s32 PmPowerRequestParent(PmNode* const node)
{
	s32 status = XST_SUCCESS;

	if (0U == (NODE_LOCKED_POWER_FLAG & node->flags)) {
		status = PmPowerRequest(node->parent);
		if (XST_SUCCESS == status) {
			node->flags |= NODE_LOCKED_POWER_FLAG;
		}
	}

	return status;
}

/**
 * PmPowerReleaseParent() - Release power parent
 * @node	Node which releases its power parent
 */
void PmPowerReleaseParent(PmNode* const node)
{
	if (0U != (NODE_LOCKED_POWER_FLAG & node->flags)) {
		node->flags &= ~(u8)NODE_LOCKED_POWER_FLAG;
		PmPowerRelease(node->parent);
	}
}

/**
 * PmPowerReleaseRpu() - Release RPU (TCM doesn't depend on RPU anymore)
 * @tcm		TCM which releases RPU
 */
void PmPowerReleaseRpu(PmSlaveTcm* const tcm)
{
	pmPowerIslandRpu_g.deps &= ~tcm->id;

	/* If no other TCM depends on RPU release it */
	if (0U == pmPowerIslandRpu_g.deps) {
		PmPowerRelease(&pmPowerIslandRpu_g.power);
	}
}

/**
 * PmPowerRequestRpu() - Request RPU (TCM now depends on RPU)
 * @tcm		TCM which requests RPU
 *
 * @return	XST_SUCCESS or error code if powering up of RPU failed
 */
s32 PmPowerRequestRpu(PmSlaveTcm* const tcm)
{
	s32 status = XST_SUCCESS;
	u32 resetMask = CRL_APB_RST_LPD_TOP_RPU_PGE_RESET_MASK |
			CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK;
	u32 reset;

	if (0U != pmPowerIslandRpu_g.deps) {
		goto done;
	}

	/* Ensure that the RPU island is ON */
	status = PmPowerRequest(&pmPowerIslandRpu_g.power);
	if (XST_SUCCESS != status) {
		goto ret;
	}

	/*
	 * Ensure the comparators are in clean state when rpu comes up
	 */
	XPfw_RMW32(RPU_RPU_ERR_INJ,
			RPU_RPU_ERR_INJ_DCCMINP2_MASK | RPU_RPU_ERR_INJ_DCCMINP_MASK,
			0x0);

	reset = XPfw_Read32(CRL_APB_RST_LPD_TOP);
	/* If PGE and AMBA resets are asserted, deassert them now */
	if (0U != (reset & resetMask)) {
		XPfw_Write32(CRL_APB_RST_LPD_TOP, reset & ~resetMask);
	}
	/* If RPU0 reset is asserted, halt the core and deassert its reset */
	if (0U != (reset & CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK)) {
		XPfw_RMW32(RPU_RPU_0_CFG,
			   RPU_RPU_0_CFG_NCPUHALT_MASK,
			  ~RPU_RPU_0_CFG_NCPUHALT_MASK);
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK,
			  ~CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
	}
	/* If RPU1 reset is asserted, halt the core and deassert its reset */
	if (0U != (reset & CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK)) {
		XPfw_RMW32(RPU_RPU_1_CFG,
			   RPU_RPU_1_CFG_NCPUHALT_MASK,
			  ~RPU_RPU_1_CFG_NCPUHALT_MASK);
		XPfw_RMW32(CRL_APB_RST_LPD_TOP,
			   CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK,
			  ~CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
	}
done:
	pmPowerIslandRpu_g.deps |= tcm->id;
ret:
	return status;
}

/**
 * PmPowerClearConfig() - Clear configuration of the power node
 * @powerNode	Pointer to the power node
 */
static void PmPowerClearConfig(PmNode* const powerNode)
{
	PmPower* const power = (PmPower*)powerNode->derived;

	power->forcePerms = 0U;
	power->useCount = 0U;
}

/**
 * PmPowerConstruct() - Constructor method for the power node
 * @powerNode	Power node to construct
 */
static void PmPowerConstruct(PmNode* const powerNode)
{
	PmPower* const power = (PmPower*)powerNode->derived;

	if ((NULL != power->class) && (NULL != power->class->construct)) {
		power->class->construct(power);
	}
}

/**
 * PmPowerGetWakeUpLatency() - Get wake-up latency of the power node
 * @node	Power node whose wake-up latency should be get
 * @lat		Pointer to the location where the latency value should be stored
 *
 * @return	XST_SUCCESS always
 */
static s32 PmPowerGetWakeUpLatency(const PmNode* const node, u32* const lat)
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
static s32 PmPowerGetPowerData(const PmNode* const powerNode, u32* const data)
{
	PmNode* node;
	u32 val = 0U;
	s32 status = XST_NO_FEATURE;

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
static s32 PmPowerForceDown(PmNode* const powerNode)
{
	PmNode* node;
	PmPower* const power = (PmPower*)powerNode->derived;
	s32 status = XST_FAILURE;

	PmPowerDfsBegin(power);
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
	if ((NULL != power->class) && (NULL != power->class->forceDown)) {
		power->class->forceDown(power);
	}

done:
	return status;
}

/**
 * PmPowerInit() - Initialize power node
 * @powerNode	Power node to initialize
 *
 * @return	Status of initializing the node
 */
static s32 PmPowerInit(PmNode* const powerNode)
{
	s32 status = XST_SUCCESS;

	if (PM_PWR_STATE_OFF == powerNode->currState) {
		goto done;
	}
	if (NULL != powerNode->parent) {
		status = PmPowerRequestParent(powerNode);
		if (XST_SUCCESS != status) {
			goto done;
		}
	}
	if (NULL != powerNode->clocks) {
		status = PmClockRequest(powerNode);
	}

done:
	return status;
}

/**
 * PmPowerIsUsable() - Check if power node could be used by current config
 * @powerNode	Power node to check
 *
 * @return	True if power node is usable, false otherwise
 */
static bool PmPowerIsUsable(PmNode* const powerNode)
{
	PmNode* node;
	bool usable = false;

	PmPowerDfsBegin((PmPower*)powerNode->derived);
	node = PmPowerDfsGetNext();
	while (NULL != node) {
		if (!NODE_IS_POWER(node)) {
			if (NULL != node->class->isUsable) {
				usable = node->class->isUsable(node);
				if (true == usable) {
					goto done;
				}
			}
		}
		node = PmPowerDfsGetNext();
	}

done:
	return usable;
}

/**
 * PmPowerGetPerms() - Get permissions to control node's clocks
 * @powerNode	Power node
 *
 * @return	ORed IPI masks of permissible masters
 *
 * @note	Permissions are determined based on permissions to do the same
 *		for non-power children nodes of the target power node. If at
 *		least one child's clocks no one has permission to control, this
 *		stands for the power parent (this node) as well.
 */
static u32 PmPowerGetPerms(const PmNode* const powerNode)
{
	const PmNode* node;
	u32 perms = 0U;
	u32 node_perms;

	PmPowerDfsBegin((PmPower*)powerNode->derived);
	node = PmPowerDfsGetNext();
	while (NULL != node) {
		if (!NODE_IS_POWER(node)) {
			if (NULL == node->class->getPerms) {
				perms = 0U;
				goto done;
			}
			node_perms = node->class->getPerms(node);
			if (0U == node_perms) {
				perms = 0U;
				goto done;
			}
			perms |= node_perms;
		}
		node = PmPowerDfsGetNext();
	}

done:
	return perms;
}

/* Collection of power nodes */
static PmNode* pmNodePowerBucket[] = {
	&pmPowerIslandApu_g.node,
	&pmPowerIslandRpu_g.power.node,
	&pmPowerDomainFpd_g.power.node,
	&pmPowerDomainPld_g.power.node,
	&pmPowerDomainLpd_g.power.node,
};

PmNodeClass pmNodeClassPower_g = {
	DEFINE_NODE_BUCKET(pmNodePowerBucket),
	.id = NODE_CLASS_POWER,
	.clearConfig = PmPowerClearConfig,
	.construct = PmPowerConstruct,
	.getWakeUpLatency = PmPowerGetWakeUpLatency,
	.getPowerData = PmPowerGetPowerData,
	.forceDown = PmPowerForceDown,
	.init = PmPowerInit,
	.isUsable = PmPowerIsUsable,
	.getPerms = PmPowerGetPerms,
};

#endif
