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
 * Power nodes (power islands and power domains) related structures
 * and functions
 *********************************************************************/

#ifndef PM_POWER_H_
#define PM_POWER_H_

#include "pm_common.h"
#include "pm_node.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

typedef struct PmPowerClass PmPowerClass;
typedef struct PmSlaveTcm PmSlaveTcm;

/*********************************************************************
 * Macros
 ********************************************************************/

/* States of power island/domain */
#define PM_PWR_STATE_OFF    0U
#define PM_PWR_STATE_ON     1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmPower - Structure for power related nodes
 *           Basically an abstraction of power islands and power domains.
 *           Not all power entities in the system have this struct. If a node
 *           has its own power, which does not depend to other nodes, its power
 *           is controlled within its transition actions. Otherwise, this power
 *           structure must exist.
 * @node     Node structure of this power entity
 * @class    If power node has derived structure this is the pointer the class
 * @children Pointer to the array of children
 * @powerUp  Handler for powering up the node
 * @powerDown Handler for powering down the node
 * @pwrDnLatency Latency (in us) for transition to OFF state
 * @pwrUpLatency Latency (in us) for transition to ON state
 * @childCnt Number of childs in children array
 * @forcePerms  ORed masks of masters which are allowed to force power down this
 *              power node
 * @useCount    How many nodes currently use this power node
 */
typedef struct PmPower {
	PmNode node;
	PmPowerClass* const class;
	PmNode** const children;
	int (*const powerUp)(void);
	int (*const powerDown)(void);
	const u32 pwrDnLatency;
	const u32 pwrUpLatency;
	u32 forcePerms;
	const u8 childCnt;
	u8 useCount;
} PmPower;

/**
 * PmPowerDomain - Structure for power domains (do not have power parent)
 * @power		Basic power structure
 * @supplyCheckHook	PMU-ROM hook to check power supply on power up
 * @supplyCheckHookId	PMU-ROM service ID for the supply check
 */
typedef struct PmPowerDomain {
	PmPower power;
	u32 (*const supplyCheckHook)(const XpbrServHndlr_t RomHandler);
	enum xpbr_serv_ext_id supplyCheckHookId;
} PmPowerDomain;

/**
 * PmPowerIslandRpu - Structure for RPU power island
 * @power	Basic power structure
 * @deps	ORed IDs of TCMs which currently depend on the island's state
 */
typedef struct PmPowerIslandRpu {
	PmPower power;
	u8 deps;
} PmPowerIslandRpu;

/**
 * PmPowerClass - Power class to model properties of PmPower derived objects
 * @construct	Constructor for the power node, call only once on startup
 * @forceDown	Puts power node in the lowest power state
 */
typedef struct PmPowerClass {
	void (*const construct)(PmPower* const power);
	void (*const forceDown)(PmPower* const power);
} PmPowerClass;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmPower pmPowerIslandApu_g;
extern PmPowerIslandRpu pmPowerIslandRpu_g;
extern PmPowerDomain pmPowerDomainFpd_g;
extern PmPowerDomain pmPowerDomainLpd_g;
extern PmPowerDomain pmPowerDomainPld_g;

extern PmNodeClass pmNodeClassPower_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
void PmPowerReleaseParent(PmNode* const node);
void PmPowerReleaseRpu(PmSlaveTcm* const tcm);

int PmPowerRequestRpu(PmSlaveTcm* const tcm);
int PmPowerRequestParent(PmNode* const node);
int PmPowerUpdateLatencyReq(const PmNode* const node);
void PmFpdSaveContext(void);
void PmFpdRestoreContext(void);
int PmPowerDown(PmPower* const power);

#endif
