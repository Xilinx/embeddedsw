/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * All functions, data and definitions needed for
 * managing PM slaves' states.
 *********************************************************************/

#ifndef PM_SLAVE_H_
#define PM_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_defs.h"
#include "pm_common.h"
#include "pm_node.h"

/*********************************************************************
 * Macros
 ********************************************************************/

/* Mask definitions for slave's flags */
#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U

#define DEFINE_SLAVE_STATES(s)	.states = (s), \
				.statesCnt = ARRAY_SIZE(s)

#define DEFINE_SLAVE_TRANS(t)	.trans = (t), \
				.transCnt = ARRAY_SIZE(t)

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmWakeEventClass - Class of the wake event
 * @set		Set event as the wake source (must be defined by each class)
 * @config	Configure the propagation of wake event (master requested)
 */
typedef struct PmWakeEventClass {
	void (*const set)(PmWakeEvent* const wake, const u32 ipi, const u32 en);
	void (*const config)(PmWakeEvent* const wake, const u32 ipi, const u32 en);
} PmWakeEventClass;

/**
 * PmWakeEvent - Structure to model wake event that can be triggered by slave
 * @derived	Pointer to the derived structure
 * @class	Pointer to the class specific to the derived structure
 */
struct PmWakeEvent {
	void* const derived;
	PmWakeEventClass* const class;
};

/**
 * PmStateTran - Transition for a state in finite state machine
 * @latency     Transition latency in microseconds
 * @fromState   From which state the transition is taken
 * @toState     To which state the transition is taken
 */
typedef struct {
	const u16 latency;
	PmStateId fromState;
	PmStateId toState;
} PmStateTran;

/**
 * PmSlaveFsm - Finite state machine data for slaves
 * @states      Pointer to states array. Index in array is a state id.
 * @enterState  Pointer to a function that executes FSM actions to enter a state
 * @trans       Pointer to array of transitions of the FSM
 * @transCnt    Number of elements in transition array
 * @statesCnt   Number of states in state array
 */
typedef struct {
	const u8* const states;
	s32 (*const enterState)(PmSlave* const slave, const PmStateId nextState);
	const PmStateTran* const trans;
	const u8 statesCnt;
	const u8 transCnt;
} PmSlaveFsm;

/**
 * PmSlave - Slave structure used for managing slave's states
 * @node        Pointer to the node structure of this slave
 * @class       Slave class (NULL if derived slave has no specific methods to be
 *              called in addition to methods of PmNodeClass)
 * @reqs        Pointer to the list of masters' requirements for the slave
 * @wake        Wake event this slave can generate
 * @slvFsm      Slave finite state machine
 * @flags       Slave's flags (bit 0: whether the slave is shareable (1) or
 *              exclusive (0) resource)
 */
struct PmSlave {
	PmNode node;
	PmSlaveClass* const class;
	PmRequirement* reqs;
	PmWakeEvent* const wake;
	const PmSlaveFsm* slvFsm;
	u8 flags;
};

/**
 * PmSlaveClass - Slave class to model properties of PmSlave derived objects
 * @init	Initialize the slave
 * @forceDown	Force down specific to the slave
 */
struct PmSlaveClass {
	s32 (*const init)(PmSlave* const slave);
	s32 (*const forceDown)(PmSlave* const slave);
};

/*********************************************************************
 * Global data declarations
 ********************************************************************/

extern PmNodeClass pmNodeClassSlave_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmUpdateSlave(PmSlave* const slave);
s32 PmCheckCapabilities(const PmSlave* const slave, const u32 capabilities);
s32 PmSlaveHasWakeUpCap(const PmSlave* const slv);
s32 PmSlaveSetConfig(PmSlave* const slave, const u32 policy, const u32 perms);

s32 PmSlaveVerifyRequest(const PmSlave* const slave);

u32 PmSlaveGetUsersMask(const PmSlave* const slave);

u32 PmSlaveGetUsageStatus(const PmSlave* const slave,
			  const PmMaster* const master);
u32 PmSlaveGetRequirements(const PmSlave* const slave,
			   const PmMaster* const master);
void PmResetSlaveStates(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_SLAVE_H_ */
