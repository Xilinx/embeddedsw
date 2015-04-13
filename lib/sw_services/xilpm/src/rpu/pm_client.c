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
 * CONTENT
 * Each PU client in the system have such file with definitions of
 * masters in the subsystem and functions for getting informations
 * about the master.
 *********************************************************************/

#include "pm_client.h"

static const struct XPm_Ipi rpu_0_ipi = {
	.mask = IPI_RPU_MASK,
	.base = IPI_BASEADDR,
	.buffer_base = IPI_BUFFER_RPU_0_BASE,
};

static const struct XPm_Master pm_rpu_0_master = {
	.node_id = NODE_RPU_0,
	.pwrdn_mask = RPU_RPU_0_PWRDWN_EN_MASK,
	.ipi = &rpu_0_ipi,
};

#if 0
static const struct XPm_Master pm_rpu_1_master = {
	.node_id = NODE_APU_1,
	.pwrdn_mask = RPU_RPU_0_PWRDWN_EN_MASK,
	.ipi = &rpu_0_ipi,
};
#endif

/*
 * Order in pm_master_all array must match cpu ids
 */
static const struct XPm_Master *const pm_masters_all[] = {
	&pm_rpu_0_master,
#if 0
	&pm_rpu_1_master,
#endif
};

/**
 * pm_get_master() - returns pointer to the master structure
 * @cpuid:	id of the cpu whose master struct pointer should be returned
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
const struct XPm_Master *pm_get_master(const uint32_t cpuid)
{
	if (cpuid >=0 && PM_ARRAY_SIZE(pm_masters_all)) {
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
const struct XPm_Master *pm_get_master_by_node(const enum XPmNodeId nid)
{
	uint8_t i;
	for (i = 0; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (nid == pm_masters_all[i]->node_id) {
			return pm_masters_all[i];
		}
	}
	return NULL;
}

static uint32_t pm_get_cpuid(const enum XPmNodeId node)
{
	uint32_t i;
	for (i = 0; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (pm_masters_all[i]->node_id == node) {
			return i;
		}
	}
	return UNDEFINED_CPUID;
}

const enum XPmNodeId subsystem_node = NODE_RPU;
const struct XPm_Master *primary_master = &pm_rpu_0_master;

void XPm_ClientSuspend(const struct XPm_Master *const master)
{
	/* Disable interrupts at processor level */
	pm_disable_int();
	/* Set powerdown request */
	pm_write(MASTER_PWRCTL, pm_read(MASTER_PWRCTL) | master->pwrdn_mask);
}

void XPm_ClientAbortSuspend()
{
	/* Enable interrupts at processor level */
	pm_enable_int();
	/* Clear powerdown request */
	pm_write(MASTER_PWRCTL, pm_read(MASTER_PWRCTL) & ~primary_master->pwrdn_mask);
}

void XPm_ClientWakeup(const struct XPm_Master *const master)
{
	uint32_t cpuid = pm_get_cpuid(master->node_id);

	if (UNDEFINED_CPUID != cpuid) {
		uint32_t val = pm_read(MASTER_PWRCTL);
		val &= ~(master->pwrdn_mask);
		pm_write(MASTER_PWRCTL, val);
	}
}
