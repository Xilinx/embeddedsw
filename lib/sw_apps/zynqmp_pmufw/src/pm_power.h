/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Power nodes (power islands and power domains) related structures
 * and functions
 *********************************************************************/

#ifndef PM_POWER_H_
#define PM_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"
#include "pm_node.h"
#include "pm_master.h"
#include "xpfw_rom_interface.h"

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
struct PmPower {
	PmNode node;
	PmPowerClass* const class;
	PmNode** const children;
	s32 (*const powerUp)(void);
	s32 (*const powerDown)(void);
	const u32 pwrDnLatency;
	const u32 pwrUpLatency;
	u32 forcePerms;
	const u8 childCnt;
	u8 useCount;
};

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
struct PmPowerClass {
	void (*const construct)(PmPower* const power);
	void (*const forceDown)(PmPower* const power);
};

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

s32 PmPowerRequestRpu(PmSlaveTcm* const tcm);
s32 PmPowerRequestParent(PmNode* const node);
s32 PmPowerUpdateLatencyReq(const PmNode* const node);
void PmFpdSaveContext(void);
void PmFpdRestoreContext(void);
s32 PmPowerDown(PmPower* const power);

#ifdef __cplusplus
}
#endif

#endif /* PM_POWER_H_ */
