/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*
 * CONTENT
 * Each PU client in the system have such file with definitions of
 * masters in the subsystem and functions for getting information
 * about the master.
 */

#include "pm_client.h"
#include "xparameters.h"
#include "xil_cache.h"
#include <xreg_cortexa53.h>
#include <xpseudo_asm.h>

/** @cond xilpm_internal */

/* Mask to get affinity level 0 */
#define PM_AFL0_MASK   0xFFU

#define WFI		__asm__ ("wfi")

static struct XPm_Master pm_apu_0_master = {
	.node_id = NODE_APU_0,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct XPm_Master pm_apu_1_master = {
	.node_id = NODE_APU_1,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct XPm_Master pm_apu_2_master = {
	.node_id = NODE_APU_2,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_2_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct XPm_Master pm_apu_3_master = {
	.node_id = NODE_APU_3,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_3_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

/* Order in pm_master_all array must match cpu ids */
static struct XPm_Master *const pm_masters_all[] = {
	&pm_apu_0_master,
	&pm_apu_1_master,
	&pm_apu_2_master,
	&pm_apu_3_master,
};

/**
 * pm_get_master() - returns pointer to the master structure
 * @cpuid:	id of the cpu whose master struct pointer should be returned
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct XPm_Master *pm_get_master(const u32 cpuid)
{
	struct XPm_Master *master = NULL;
	if (cpuid < PM_ARRAY_SIZE(pm_masters_all)) {
		master = pm_masters_all[cpuid];
		goto done;
	}
done:
	return master;
}

/**
 * pm_get_master_by_node() - returns pointer to the master structure
 * @nid:	ndoe id of the cpu master
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid)
{
	u8 i;
	struct XPm_Master *master = NULL;

	for (i = 0U; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (nid == pm_masters_all[i]->node_id) {
			master = pm_masters_all[i];
			goto done;
		}
	}

done:
	return master;
}

static u32 pm_get_cpuid(const enum XPmNodeId node)
{
	u32 i;
	u32 ret;

	for (i = 0U; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (pm_masters_all[i]->node_id == node) {
			ret = i;
			goto done;
		}
	}

	ret = UNDEFINED_CPUID;

done:
	return ret;
}

const enum XPmNodeId subsystem_node = NODE_APU;
struct XPm_Master *primary_master = &pm_apu_0_master;

void XPm_ClientSuspend(const struct XPm_Master *const master)
{
	u32 pwrdn_req;

	/* Disable interrupts at processor level */
	pm_disable_int();
	/* Set powerdown request */
	if (NULL != master) {
		pwrdn_req = pm_read(master->pwrctl);
		pwrdn_req |= master->pwrdn_mask;
		pm_write(master->pwrctl, pwrdn_req);
	}
}

void XPm_ClientAbortSuspend(void)
{
	u32 pwrdn_req;
	if (NULL != primary_master) {
		pwrdn_req = pm_read(primary_master->pwrctl);

		/* Clear powerdown request */
		pwrdn_req &= ~primary_master->pwrdn_mask;
		pm_write(primary_master->pwrctl, pwrdn_req);
		/* Enable interrupts at processor level */
		pm_enable_int();
	}
}

void XPm_ClientWakeup(const struct XPm_Master *const master)
{
	u32 cpuid = pm_get_cpuid(master->node_id);

	if (UNDEFINED_CPUID != cpuid) {
		u32 val = pm_read(master->pwrctl);
		val &= ~(master->pwrdn_mask);
		pm_write(master->pwrctl, val);
	}
}

/**
 * XPm_ClientSuspendFinalize() - Finalize suspend procedure by executing
 * 				 wfi instruction
 */
void XPm_ClientSuspendFinalize(void)
{
	/* Flush the data cache only if it is enabled */
#ifdef __aarch64__
	u64 ctrlReg;
	ctrlReg = mfcp(SCTLR_EL3);
	if ((XREG_CONTROL_DCACHE_BIT & ctrlReg) != 0U) {
		Xil_DCacheFlush();
	}
#else
	u32 ctrlReg;
	ctrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if ((XREG_CP15_CONTROL_C_BIT & ctrlReg) != 0U) {
		Xil_DCacheFlush();
	}
#endif

	pm_dbg("Going to WFI...\n");
	WFI;
	pm_dbg("WFI exit...\n");
}

/**
 *  XPm_ClientSetPrimaryMaster() - Set primary master based on master ID
 */
void XPm_ClientSetPrimaryMaster(void)
{
#ifdef __aarch64__
	u64 master_id;
	master_id = mfcp(MPIDR_EL1);
#else
	u32 master_id;
	master_id = mfcp(XREG_CP15_MULTI_PROC_AFFINITY);
#endif

	master_id &= PM_AFL0_MASK;
	primary_master = pm_masters_all[master_id];
}
/** @endcond */
