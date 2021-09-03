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
#include <xreg_cortexr5.h>
#include <xpseudo_asm.h>
#include "xreg_cortexr5.h"

/** @cond xilpm_internal */

#define PM_CLIENT_RPU_ERR_INJ            0xFF9A0020U
#define PM_CLIENT_RPU_FAULT_LOG_EN_MASK  0x00000101U

/* Mask to get affinity level 0 */
#define PM_CLIENT_AFL0_MASK              0xFFU

#if defined (__GNUC__)
#define WFI	__asm__("wfi");
#elif defined (__ICCARM__)
#define WFI	__asm("wfi");
#endif

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
 * @param  cpuid ID of the cpu whose master struct pointer should be returned
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct XPm_Master *pm_get_master(const u32 cpuid)
{
	struct XPm_Master *master = NULL;
	if (PM_ARRAY_SIZE(pm_masters_all) != 0U) {
		master = pm_masters_all[cpuid];
		goto done;
	}
done:
	return master;
}

/**
 * pm_get_master_by_node() - returns pointer to the master structure
 * @param  nid ndoe id of the cpu master
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

const enum XPmNodeId subsystem_node = NODE_RPU;
/* By default, lock-step mode is assumed */
struct XPm_Master *primary_master = &pm_rpu_0_master;

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
	u32 ctrlReg;

	/*
	 * Unconditionally disable fault log.
	 * BSP enables it once the processor resumes.
	 */
	pm_dbg("%s: Disabling RPU Lock-Step Fault Log...\n", __func__);
	pm_write(PM_CLIENT_RPU_ERR_INJ,
			pm_read(PM_CLIENT_RPU_ERR_INJ) & ~PM_CLIENT_RPU_FAULT_LOG_EN_MASK);
#if defined (__GNUC__)
	/* Flush data cache if the cache is enabled */
	ctrlReg = mfcp(XREG_CP15_SYS_CONTROL);
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_SYS_CONTROL, ctrlReg);
#endif
	if ((XREG_CP15_CONTROL_C_BIT & ctrlReg) != 0U) {
		Xil_DCacheFlush();
	}

	pm_dbg("%s: Going to WFI...\n", __func__);

	WFI;

	pm_dbg("%s: WFI exit...\n", __func__);
}

/**
 * XPm_GetMasterName() - Get name of the master
 *
 * This function determines name of the master based on current configuration.
 *
 * @return     Name of the master
 */
const char* XPm_GetMasterName(void)
{
	static const char* retptr;
	u32 mode = pm_read(RPU_RPU_GLBL_CNTL) &
		   (u32)RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;

	if (RPU_RPU_GLBL_CNTL_SLSPLIT_MASK != mode) {
		retptr = "RPU";
	} else {
		switch (primary_master->node_id) {
		case NODE_RPU_0:
			retptr = "RPU0";
			break;
		case NODE_RPU_1:
			retptr = "RPU1";
			break;
		default:
			retptr = "ERROR";
			break;
		};
	}

	return retptr;
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
	u32 mode;

#if defined (__GNUC__)
	master_id = mfcp(XREG_CP15_MULTI_PROC_AFFINITY) & PM_CLIENT_AFL0_MASK;
#elif defined (__ICCARM__)
	mfcp(XREG_CP15_MULTI_PROC_AFFINITY, master_id);
	master_id &= PM_CLIENT_AFL0_MASK;
#endif
	mode = pm_read(RPU_RPU_GLBL_CNTL) &
		(u32)RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
	if (RPU_RPU_GLBL_CNTL_SLSPLIT_MASK != mode) {
		primary_master = &pm_rpu_0_master;
		pm_print("Running in Lock-Step mode\n");
	} else {
		primary_master = pm_masters_all[master_id];
		pm_print("Running in Split mode\n");
	}
}
/** @endcond */
