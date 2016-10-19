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
 * Pm Node related structures and definitions
 *********************************************************************/

#ifndef PM_NODE_H_
#define PM_NODE_H_

#include "pm_defs.h"
#include "pm_common.h"
#include "xil_types.h"
#include "xstatus.h"

typedef u8 PmNodeId;
typedef u8 PmStateId;
typedef u8 PmNodeTypeId;

/* Forward declaration */
typedef struct PmPower PmPower;
typedef struct PmClockHandle PmClockHandle;
typedef struct PmNode PmNode;

/* Function pointer for wake/sleep transition functions */
typedef int (*const PmNodeTranHandler)(PmNode* const nodePtr);

/*********************************************************************
 * Macros
 ********************************************************************/
/* Node types: Slave types must be at the bottom */
#define PM_TYPE_PROC            1U
#define PM_TYPE_PWR_ISLAND      2U
#define PM_TYPE_PWR_DOMAIN      3U
#define PM_TYPE_SLAVE           4U
/* All types from here onward are slave types */
#define PM_TYPE_SRAM            (PM_TYPE_SLAVE + 0U)
#define PM_TYPE_USB             (PM_TYPE_SLAVE + 1U)
#define PM_TYPE_GPU_PP          (PM_TYPE_SLAVE + 2U)
#define PM_TYPE_TTC             (PM_TYPE_SLAVE + 3U)
#define PM_TYPE_SATA            (PM_TYPE_SLAVE + 4U)
#define PM_TYPE_PLL             (PM_TYPE_SLAVE + 5U)
#define PM_TYPE_DDR             (PM_TYPE_SLAVE + 6U)

#define NODE_IS_PROC(type)       (PM_TYPE_PROC == (type))

#define NODE_IS_POWER(type)      ((PM_TYPE_PWR_ISLAND == (type)) || \
				(PM_TYPE_PWR_DOMAIN == (type)))

#define NODE_IS_SLAVE(type)      ((type) >= PM_TYPE_SLAVE)

#define NODE_IS_OFF(nodePtr)     (0U == ((nodePtr)->currState & 1U))

#define NODE_HAS_SLEEP(opsPtr)   ((NULL != (opsPtr)) && (NULL != (opsPtr)->sleep))


/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmNodeOps - Node operations
 * @sleep       Put node into sleep (what the sleep is depends on node's type)
 * @wake        Wake up this node (procedure depends on node type)
 */
typedef struct PmNodeOps {
	PmNodeTranHandler sleep;
	PmNodeTranHandler wake;
} PmNodeOps;

/**
 * PmNode - Structure common for all entities that have node id
 * @derived     Pointer to a derived node type structure
 * @parent      Pointer to power parent node
 * @clocks      Pointer to the list of clocks that the node uses
 * @ops         Pointer to the operations structure
 * @latencyMarg Latency margin: lowest latency requirement - powerup latency
 * @nodeId      Node id defined in pm_defs.h
 * @typeId      Type id, used to distinguish the nodes
 * @currState   Id of the node's current state. Interpretation depends on type
 *              of the node, bit 0 value is reserved for off states
 * @powerInfo   Pointer to the array of power consumptions arranged by
 *              stateId
 * @powerInfoCnt  Number of power consumptions in powerInfo array based on
 *                number of states
 */
typedef struct PmNode {
	void* const derived;
	PmPower* const parent;
	PmClockHandle* clocks;
	const PmNodeOps* const ops;
	const u32 *const powerInfo;
	const u32 powerInfoCnt;
	u32 latencyMarg;
	const PmNodeId nodeId;
	const PmNodeTypeId typeId;
	PmStateId currState;
} PmNode;

/*********************************************************************
 * Function declarations
 ********************************************************************/
PmNode* PmGetNodeById(const u32 nodeId);

void PmNodeUpdateCurrState(PmNode* const node, const PmStateId newState);
u32 PmNodeGetPowerConsumption(PmNode* const node, const PmStateId state);
u32 PmNodeGetWakeLatency(PmNode* const node);

bool PmNodeDependsOnClock(const PmNode* const node);

#endif /* PM_NODE_H_ */
