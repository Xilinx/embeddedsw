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
#include <xreg_cortexr5.h>
#include <xpseudo_asm.h>
#include "xreg_cortexr5.h"

#define PM_CLIENT_RPU_ERR_INJ            0xFF9A0020U
#define PM_CLIENT_RPU_FAULT_LOG_EN_MASK  0x00000101U

/* Mask to get affinity level 0 */
#define PM_CLIENT_AFL0_MASK              0xFF

static struct XPm_Master pm_rpu_0_master = {
	.node_id = NODE_RPU_0,
	.pwrctl = RPU_RPU_0_PWRDWN,
	.pwrdn_mask = RPU_RPU_0_PWRDWN_EN_MASK,
	.ipi = NULL,
};

static struct XPm_Master pm_rpu_1_master = {
	.node_id = NODE_RPU_1,
	.pwrctl = RPU_RPU_1_PWRDWN,
	.pwrdn_mask = RPU_RPU_1_PWRDWN_EN_MASK,
	.ipi = NULL,
};

/* Order in pm_master_all array must match cpu ids */
static struct XPm_Master *const pm_masters_all[] = {
	&pm_rpu_0_master,
	&pm_rpu_1_master,
};

/**
 * pm_get_master() - returns pointer to the master structure
 * @cpuid:	id of the cpu whose master struct pointer should be returned
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct XPm_Master *pm_get_master(const u32 cpuid)
{
	if (PM_ARRAY_SIZE(pm_masters_all)) {
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

const enum XPmNodeId subsystem_node = NODE_RPU;
/* By default, lock-step mode is assumed */
struct XPm_Master *primary_master = &pm_rpu_0_master;

void XPm_ClientSuspend(const struct XPm_Master *const master)
{
	u32 pwrdn_req;

	/* Disable interrupts at processor level */
	pm_disable_int();
	/* Set powerdown request */
	pwrdn_req = pm_read(master->pwrctl);
	pwrdn_req |= master->pwrdn_mask;
	pm_write(master->pwrctl, pm_read(master->pwrctl) | master->pwrdn_mask);
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

	/*
	 * Unconditionally disable fault log.
	 * BSP enables it once the processor resumes.
	 */
	pm_dbg("Disabling RPU Lock-Step Fault Log...\n");
	pm_write(PM_CLIENT_RPU_ERR_INJ,
			pm_read(PM_CLIENT_RPU_ERR_INJ) & ~PM_CLIENT_RPU_FAULT_LOG_EN_MASK);

	/* Flush data cache if the cache is enabled */
	ctrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if (XREG_CP15_CONTROL_C_BIT & ctrlReg)
		Xil_DCacheFlush();

	pm_dbg("Going to WFI...\n");
	__asm__("wfi");
	pm_dbg("WFI exit...\n");
}

/**
 * XPm_GetMasterName() - Get name of the master
 *
 * This function determines name of the master based on current configuration.
 *
 * @return     Name of the master
 */
char* XPm_GetMasterName(void)
{
	bool lockstep = !(pm_read(RPU_RPU_GLBL_CNTL) &
		     RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);

	if (lockstep) {
		return "RPU";
	} else {
		switch (primary_master->node_id) {
		case NODE_RPU_0:
			return "RPU0";
		case NODE_RPU_1:
			return "RPU1";
		default:
			return "ERROR";
		};
	};
}

/**
 * XPm_ClientSetPrimaryMaster() -Set primary master
 *
 * This function determines the RPU configuration (split or lock-step mode)
 * and sets the primary master accordingly.
 *
 * If this function is not called, the default configuration is assumed
 * (i.e. lock-step)
 */
void XPm_ClientSetPrimaryMaster(void)
{
	u32 master_id;
	bool lockstep;

	master_id = mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & PM_CLIENT_AFL0_MASK;
	lockstep = !(pm_read(RPU_RPU_GLBL_CNTL) &
		     RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
	if (lockstep) {
		primary_master = &pm_rpu_0_master;
	} else {
		primary_master = pm_masters_all[master_id];
	}
	pm_print("Running in %s mode\n", lockstep ? "Lock-Step" : "Split");
}
