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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * This file contains PM master related data structures and
 * functions for accessing them. Also, each PM masters have defined
 * data structures used for tracking it's requests for each slave's
 * capabilities/states.
 *********************************************************************/

#ifndef PM_MASTER_H_
#define PM_MASTER_H_

#include "pm_slave.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_node.h"
#include "pm_power.h"
#include "xil_types.h"

typedef u8 PmMasterId;
typedef u8 PmRequirementId;
typedef u8 PmMasterEvent;

typedef struct PmMaster PmMaster;

/*********************************************************************
 * Macros
 ********************************************************************/
/* Supported masters (macros are used as indexes in array of all masters) */
#define PM_MASTER_APU   0U
#define PM_MASTER_RPU_0 1U
#define PM_MASTER_RPU_1 2U
#define PM_MASTER_MAX   3U

/* Master states */
/* All processors within the master are sleeping */
#define PM_MASTER_STATE_SLEEP   0U

/* At least one processor within master is not sleeping */
#define PM_MASTER_STATE_ACTIVE  1U

/* Master FSM events */
/* Triggered after a processor within the master goes to sleep */
#define PM_MASTER_EVENT_SLEEP       1U

/* Triggered before a processor within the master wakes-up */
#define PM_MASTER_EVENT_WAKE        2U

/* Triggered by abort suspend */
#define PM_MASTER_EVENT_ABORT       3U

/* Apu slaves */
#define PM_MASTER_APU_SLAVE_OCM0    0U
#define PM_MASTER_APU_SLAVE_OCM1    1U
#define PM_MASTER_APU_SLAVE_OCM2    2U
#define PM_MASTER_APU_SLAVE_OCM3    3U
#define PM_MASTER_APU_SLAVE_L2      4U
#define PM_MASTER_APU_SLAVE_USB0    5U
#define PM_MASTER_APU_SLAVE_USB1    6U
#define PM_MASTER_APU_SLAVE_TTC0    7U
#define PM_MASTER_APU_SLAVE_SATA    8U
#define PM_MASTER_APU_SLAVE_MAX     9U

/* Rpu0 slaves */
#define PM_MASTER_RPU_0_SLAVE_TCM0A 0U
#define PM_MASTER_RPU_0_SLAVE_TCM0B 1U
#define PM_MASTER_RPU_0_SLAVE_TCM1A 2U
#define PM_MASTER_RPU_0_SLAVE_TCM1B 3U
#define PM_MASTER_RPU_0_SLAVE_OCM0  4U
#define PM_MASTER_RPU_0_SLAVE_OCM1  5U
#define PM_MASTER_RPU_0_SLAVE_OCM2  6U
#define PM_MASTER_RPU_0_SLAVE_OCM3  7U
#define PM_MASTER_RPU_0_SLAVE_USB0  8U
#define PM_MASTER_RPU_0_SLAVE_USB1  9U
#define PM_MASTER_RPU_0_SLAVE_TTC0  10U
#define PM_MASTER_RPU_0_SLAVE_SATA  11U
#define PM_MASTER_RPU_0_SLAVE_MAX   12U

/* Pm Master request info masks */
#define PM_MASTER_WAKEUP_REQ_MASK   0x1U
#define PM_MASTER_USING_SLAVE_MASK  0x2U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRequirement - structure for tracking requirements of a master for the slave
 *              setting. One structure should be statically assigned for each
 *              possible combination of master/slave, because dynamic memory
 *              allocation cannot be used.
 * @info        Contains information about master's request - a bit for
 *              encoding has master requested or released node, other bits
 *              are used to encode master index in array of all masters.
 *              PM_MASTER_USING_SLAVE_MASK - extracts a bitfield for usage flag
 * @slave       Pointer to the slave structure
 * @master      Pointer to the master structure. Can be removed if need to
 *              optimize for space instead performance
 * @currReq     Currently holding requirements of a master for this slave
 * @nextReq     Requirements of a master to be configured when it changes the
 *              state (after it goes to sleep or before it gets awake)
 */
typedef struct PmRequirement {
	PmSlave* const slave;
	PmMaster* const requestor;
	u8 info;
	u32 currReq;
	u32 nextReq;
} PmRequirement;

/**
 * PmMaster - contains PM master related informations
 * @procs       Pointer to the array of processors within the master
 * @procsCnt    Number of processors within the master
 * @ipiMask     Mask dedicated to the master in IPI registers
 * @ipiTrigMask Trigger mask dedicated to the master in IPI registers
 * @pmuBuffer   IPI buffer address into which PMU can write (PMU's buffer)
 * @buffer      IPI buffer address into which this master can write
 *              (master's buffer)
 * @reqs        Pointer to the masters array of requirements for slave
 *              capabilities
 * @reqsCnt     Number of requirement elements (= worst case for number of
 *              used slaves)
 */
typedef struct PmMaster {
	PmProc* const procs;
	const u8 procsCnt;
	const u32 ipiMask;
	const u32 ipiTrigMask;
	const u32 pmuBuffer;
	const u32 buffer;
	PmRequirement* const reqs;
	const PmRequirementId reqsCnt;
} PmMaster;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmMaster pmMasterApu_g;
extern PmMaster pmMasterRpu0_g;
extern PmMaster pmMasterRpu1_g;

extern PmRequirement pmApuReq_g[PM_MASTER_APU_SLAVE_MAX];
extern PmRequirement pmRpu0Req_g[PM_MASTER_RPU_0_SLAVE_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
/* Get functions */
const PmMaster* PmGetMasterByIpiMask(const u32 mask);
PmProc* PmGetProcByWfiStatus(const u32 mask);
PmProc* PmGetProcByWakeStatus(const u32 mask);
PmProc* PmGetProcByNodeId(const PmNodeId nodeId);
PmProc* PmGetProcOfThisMaster(const PmMaster* const master,
			      const PmNodeId nodeId);
PmProc* PmGetProcOfOtherMaster(const PmMaster* const master,
			       const PmNodeId nodeId);
PmRequirement* PmGetRequirementForSlave(const PmMaster* const master,
					const PmNodeId nodeId);
u32 PmMasterGetAwakeProcCnt(const PmMaster* const master);

/* Requirements related functions */
u32 PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps);
u32 PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps);
void PmRequirementUpdateScheduled(const PmMaster* const master,
				  const bool swap);
void PmRequirementCancelScheduled(const PmMaster* const master);

void PmEnableAllMasterIpis(void);
void PmMasterNotify(PmMaster* const master, const PmProcEvent event);
void PmEnableProxyWake(PmMaster* const master);

#endif
