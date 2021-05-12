/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * PM requirements are data structures allocated for each valid
 * master/slave pair, used for tracking master's requests for slave's
 * capabilities/states.
 *********************************************************************/

#ifndef PM_REQUIREMENT_H_
#define PM_REQUIREMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"
#include "xil_types.h"
#include "pm_node.h"

typedef enum {
	RELEASE_ONE,
	RELEASE_ALL,
	RELEASE_UNREQUESTED,
} PmReleaseScope;

/*********************************************************************
 * Macros
 ********************************************************************/
/*
 * Max number of master/slave pairs (max number of combinations that can
 * exist at the runtime). The value is used to statically initialize
 * size of the pmReqData array, which is used as the heap.
 */
#define PM_REQUIREMENT_MAX	200U

/* Requirement flags */
#define PM_MASTER_WAKEUP_REQ_MASK	0x1U
#define PM_MASTER_REQUESTED_SLAVE_MASK	0x2U
#define PM_MASTER_SET_LATENCY_REQ	0x4U
#define PM_SYSTEM_USING_SLAVE_MASK	0x8U

#define MASTER_REQUESTED_SLAVE(reqPtr)	\
	(0U != (PM_MASTER_REQUESTED_SLAVE_MASK & (reqPtr)->info))

#define SYSTEM_USING_SLAVE(reqPtr)	\
	(0U != (PM_SYSTEM_USING_SLAVE_MASK & (reqPtr)->info))

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRequirement - structure for tracking requirements of a master for the slave
 *              setting. One structure should be statically assigned for each
 *              possible combination of master/slave, because dynamic memory
 *              allocation cannot be used.
 * @slave       Pointer to the slave structure
 * @master      Pointer to the master structure
 * @nextSlave   Pointer to the master's requirement for a next slave in the list
 * @nextMaster  Pointer to the requirement of a next master that uses the slave
 * @preReq      Requirements of a master that it cannot request for itself (when
 *              a master starts the cold boot there are some resources it will
 *              use before it is capable of requesting them, like memories)
 * @defaultReq  Default requirements of a master - requirements for slave
 *              capabilities without which the master cannot run
 * @currReq     Currently holding requirements of a master for this slave
 * @nextReq     Requirements of a master to be configured when it changes the
 *              state (after it goes to sleep or before it gets awake)
 * @latencyReq  Latency requirements of master for the slave's transition time
 *              from any to its maximum (highest id) state
 * @info        Contains information about master's request - a bit for
 *              encoding has master requested or released node, and a bit to
 *              encode has master requested a wake-up of this slave.
 */
struct PmRequirement {
	PmSlave* slave;
	PmMaster* master;
	PmRequirement* nextSlave;
	PmRequirement* nextMaster;
	u8 preReq;
	u8 defaultReq;
	u8 currReq;
	u8 nextReq;
	u32 latencyReq;
	u8 info;
};

/*********************************************************************
 * Function declarations
 ********************************************************************/

void PmRequirementCancelScheduled(const PmMaster* const master);
void PmRequirementPreRequest(const PmMaster* const master);
void PmRequirementClockRestore(const PmMaster* const master);
void PmRequirementFreeAll(void);
void PmRequirementClear(PmRequirement* const req);

s32 PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps);
s32 PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps);
s32 PmRequirementUpdateScheduled(const PmMaster* const master, const bool swap);
s32 PmRequirementRequest(PmRequirement* const req, const u32 caps);
s32 PmRequirementRelease(PmRequirement* const first, const PmReleaseScope scope);

PmRequirement* PmRequirementAdd(PmMaster* const master, PmSlave* const slave);
PmRequirement* PmRequirementGet(const PmMaster* const master,
				const PmSlave* const slave);
PmRequirement* PmRequirementGetNoMaster(const PmSlave* const slave);

s32 PmRequirementSetConfig(PmRequirement* const req, const u32 flags,
			   const u32 currReq, const u32 defaultReq);

#ifdef __cplusplus
}
#endif

#endif /* PM_REQUIREMENT_H_ */
