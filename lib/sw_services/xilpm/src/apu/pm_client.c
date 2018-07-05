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
*
******************************************************************************/

/*
 * CONTENT
 * Each PU client in the system have such file with definitions of
 * masters in the subsystem and functions for getting informations
 * about the master.
 */

#include "pm_client.h"
#include "xparameters.h"
#include <xil_cache.h>
#include <xreg_cortexa53.h>
#include <xpseudo_asm.h>

/* Mask to get affinity level 0 */
#define PM_AFL0_MASK   0xFF

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
	if (cpuid < PM_ARRAY_SIZE(pm_masters_all)) {
		return pm_masters_all[cpuid];
	}

	return NULL;
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

	for (i = 0; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (nid == pm_masters_all[i]->node_id) {
			return pm_masters_all[i];
		}
	}

	return NULL;
}

static u32 pm_get_cpuid(const enum XPmNodeId node)
{
	u32 i;

	for (i = 0; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (pm_masters_all[i]->node_id == node) {
			return i;
		}
	}

	return UNDEFINED_CPUID;
}

const enum XPmNodeId subsystem_node = NODE_APU;
struct XPm_Master *primary_master = &pm_apu_0_master;

void XPm_ClientSuspend(const struct XPm_Master *const master)
{
	u32 pwrdn_req;

	/* Disable interrupts at processor level */
	pm_disable_int();
	/* Set powerdown request */
	pwrdn_req = pm_read(master->pwrctl);
	pwrdn_req |= master->pwrdn_mask;
	pm_write(master->pwrctl, pwrdn_req);
}

void XPm_ClientAbortSuspend(void)
{
	u32 pwrdn_req = pm_read(primary_master->pwrctl);

	/* Clear powerdown request */
	pwrdn_req &= ~primary_master->pwrdn_mask;
	pm_write(primary_master->pwrctl, pwrdn_req);
	/* Enable interrupts at processor level */
	pm_enable_int();
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
	u32 ctrlReg;

	/* Flush the data cache only if it is enabled */
#ifdef __aarch64__
	ctrlReg = mfcp(SCTLR_EL3);
	if (XREG_CONTROL_DCACHE_BIT & ctrlReg)
		Xil_DCacheFlush();
#else
	ctrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if (XREG_CP15_CONTROL_C_BIT & ctrlReg)
		Xil_DCacheFlush();
#endif

	pm_dbg("Going to WFI...\n");
	__asm__("wfi");
	pm_dbg("WFI exit...\n");
}

/**
 *  XPm_ClientSetPrimaryMaster() - Set primary master based on master ID
 */
void XPm_ClientSetPrimaryMaster(void)
{
	u32 master_id;
#ifdef __aarch64__
	master_id = mfcp(MPIDR_EL1);
#else
	master_id = mfcp(XREG_CP15_MULTI_PROC_AFFINITY);
#endif

	master_id &= PM_AFL0_MASK;
	primary_master = pm_masters_all[master_id];
}
