/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Pm Node related structures and definitions
 *********************************************************************/

#ifndef PM_NODE_H_
#define PM_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_defs.h"
#include "pm_common.h"
#include "xil_types.h"
#include "xstatus.h"

typedef u8 PmNodeId;
typedef u8 PmStateId;

/* Function pointer for wake/sleep transition functions */
typedef int (*const PmNodeTranHandler)(PmNode* const nodePtr);

/*********************************************************************
 * Macros
 ********************************************************************/

#define NODE_CLASS_PROC		1U
#define NODE_CLASS_POWER	2U
#define NODE_CLASS_SLAVE	3U
#define NODE_CLASS_PLL		4U

#define NODE_IS_PROC(nodePtr)	(NODE_CLASS_PROC == (nodePtr)->class->id)
#define NODE_IS_POWER(nodePtr)	(NODE_CLASS_POWER == (nodePtr)->class->id)
#define NODE_IS_SLAVE(nodePtr)	(NODE_CLASS_SLAVE == (nodePtr)->class->id)
#define NODE_IS_PLL(nodePtr)	(NODE_CLASS_PLL == (nodePtr)->class->id)

#define NODE_IS_OFF(nodePtr)     (0U == ((nodePtr)->currState & 1U))

#define NODE_LOCKED_POWER_FLAG	0x1U
#define NODE_LOCKED_CLOCK_FLAG	0x2U
#define NODE_IDLE_DONE			0x4U

#define DEFINE_NODE_BUCKET(b)	.bucket = (b), \
				.bucketSize = ARRAY_SIZE(b)

#define DEFINE_PM_POWER_INFO(i)	.powerInfo = (i), \
				.powerInfoCnt = ARRAY_SIZE(i)

#if defined(PM_LOG_LEVEL) && (PM_LOG_LEVEL > 0)
#define DEFINE_NODE_NAME(n)	.name = n
#else
#define DEFINE_NODE_NAME(n)	.name = ""
#endif
/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmNode - Structure common for all entities that have node id
 * @derived     Pointer to a derived node structure
 * @class	Pointer to the node class of the node
 * @parent      Pointer to power parent node
 * @clocks      Pointer to the list of clocks that the node uses
 * @latencyMarg Latency margin: lowest latency requirement - powerup latency
 * @nodeId      Node id defined in pm_defs.h
 * @typeId      Type id, used to distinguish the nodes
 * @currState   Id of the node's current state. Interpretation depends on type
 *              of the node, bit 0 value is reserved for off states
 * @powerInfo   Pointer to the array of power consumptions arranged by
 *              stateId
 * @powerInfoCnt  Number of power consumptions in powerInfo array based on
 *                number of states
 * @flags       Node flags
 * @name	Node name
 */
struct PmNode {
	void* const derived;
	PmNodeClass* const class;
	PmPower* const parent;
	PmClockHandle* clocks;
	u32 latencyMarg;
	const char* const name;
	const u8 *const powerInfo;
	const u8 powerInfoCnt;
	const PmNodeId nodeId;
	PmStateId currState;
	u8 flags;
};

/**
 * PmNodeClass - Node class models common behavior for a collection of nodes
 * @clearConfig		Clear current configuration of the node
 * @construct		Constructor for the node, call only once on startup
 * @getWakeUpLatency	Get wake-up latency of the node
 * @getPowerData	Get power consumption of the node
 * @forceDown		Put node in the lowest power state
 * @init		Initialize the node
 * @isUsable		Check if the node is usable by current configuration
 * @getPerms		Get permissions (ORed masks of masters allowed to
 *			control node's clocks)
 * @bucket		Pointer to the array of nodes from the class
 * @bucketSize		Number of nodes in the bucket
 * @id			Nodes' class/type ID
 */
struct PmNodeClass {
	void (*const clearConfig)(PmNode* const node);
	void (*const construct)(PmNode* const node);
	s32 (*const getWakeUpLatency)(const PmNode* const node, u32* const lat);
	s32 (*const getPowerData)(const PmNode* const node, u32* const data);
	s32 (*const forceDown)(PmNode* const node);
	s32 (*const init)(PmNode* const node);
	bool (*const isUsable)(PmNode* const node);
	u32 (*const getPerms)(const PmNode* const node);
	PmNode** const bucket;
	const u32 bucketSize;
	const u8 id;
};

/*********************************************************************
 * Function declarations
 ********************************************************************/
PmNode* PmGetNodeById(const u32 nodeId);
void* PmNodeGetDerived(const u8 nodeClass, const u32 nodeId);

static inline void* PmNodeGetSlave(const u32 nodeId) {
	return PmNodeGetDerived(NODE_CLASS_SLAVE, nodeId);
}
static inline void* PmNodeGetPower(const u32 nodeId) {
	return PmNodeGetDerived(NODE_CLASS_POWER, nodeId);
}
static inline void* PmNodeGetProc(const u32 nodeId) {
	return PmNodeGetDerived(NODE_CLASS_PROC, nodeId);
}
static inline void* PmNodeGetPll(const u32 nodeId) {
	return PmNodeGetDerived(NODE_CLASS_PLL, nodeId);
}

void PmNodeUpdateCurrState(PmNode* const node, const PmStateId newState);
void PmNodeClearConfig(void);
void PmNodeConstruct(void);
void PmNodeForceDownUnusable(void);
void PmNodeLogUnknownState(const PmNode* const node, const PmStateId state);

s32 PmNodeGetPowerInfo(const PmNode* const node, u32* const data);
s32 PmNodeForceDown(PmNode* const node);
s32 PmNodeInit(void);

u32 PmNodeGetPermissions(PmNode* const node);

#ifdef __cplusplus
}
#endif

#endif /* PM_NODE_H_ */
