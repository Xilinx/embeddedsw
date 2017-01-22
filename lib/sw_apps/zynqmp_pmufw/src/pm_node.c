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
 * Global array of all nodes, and GetbyId function
 *********************************************************************/

#include "pm_node.h"
#include "pm_power.h"
#include "pm_proc.h"
#include "pm_slave.h"
#include "pm_notifier.h"

static PmNodeClass* pmNodeClasses[] = {
	&pmNodeClassProc_g,
	&pmNodeClassPower_g,
	&pmNodeClassSlave_g,
};

/**
 * PmGetNodeById() - Find node that matches a given node ID
 * @nodeId      ID of the node to find
 *
 * @returns     Pointer to PmNode structure (or NULL if not found)
 */
PmNode* PmGetNodeById(const u32 nodeId)
{
	u32 i, n;
	PmNode* node = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		for (n = 0U; n < pmNodeClasses[i]->bucketSize; n++) {
			if (nodeId == pmNodeClasses[i]->bucket[n]->nodeId) {
				node = pmNodeClasses[i]->bucket[n];
				goto done;
			}
		}
	}

done:
	return node;
}

/**
 * PmNodeUpdateCurrState() - Call to update currState variable of the node
 * @node        Pointer to the node whose state has to be updated
 * @newState    New state value to be written in currState variable of the node
 */
void PmNodeUpdateCurrState(PmNode* const node, const PmStateId newState)
{
	if (newState == node->currState) {
		goto done;
	}
	node->currState = newState;

	PmNotifierEvent(node, EVENT_STATE_CHANGE);

done:
	return;
}

/**
 * PmNodeLookupConsumption() - Lookup the power consumption of a node in a
 *			       specific state
 * @node       Pointer to the node of the slave
 * @state      State of the slave
 *
 * @returns    Power consumption for the slave in the specified state
 */
static u32 PmNodeLookupConsumption(PmNode* const nodePtr, PmStateId state)
{
	u32 power = POWER_MISSING;

	if ((NULL == nodePtr) || (NULL == nodePtr->powerInfo) ||
	    (nodePtr->powerInfoCnt <= state)) {
		goto done;
	}

	power = nodePtr->powerInfo[state];

done:
	return power;
}

/**
 * PmNodeGetChildPos() - Find child's position in its parent array of children
 *                 matching a given node ID
 * @node        ID of the node
 *
 * @returns     Child's position in its parent's array of children
 */
static u32 PmNodeGetChildPos(PmNodeId node)
{
	u8 i;
	u32 childPos = ~0;
	PmNode* nodePtr = PmGetNodeById(node);

	if ((NULL == nodePtr) || (NULL == nodePtr->parent)) {
		goto done;
	}

	for (i = 0U; i < nodePtr->parent->childCnt; i++) {
		if (node == nodePtr->parent->children[i]->nodeId) {
			childPos = i;
			break;
		}
	}

done:
	return childPos;
}

/**
 * PmNodeGetPowerConsumption() - Get power consumption of a node in a specific
 *                               state
 * @node       Pointer to the node
 * @state      State of the node
 *
 * @returns    Power consumption for the node in a specific state
 *
 * If the requested component represents power island or domain, return the
 * power consumption as a sum of consumptions of the children.
 */
u32 PmNodeGetPowerConsumption(PmNode* const nodePtr, const PmStateId state)
{
	u32 childPwr;
	u32 childPos;
	PmPower* root;
	PmPower* currParent;
	PmNode* childNode;
	u32 power = 0U;

	if ((NULL == nodePtr) || (NULL == nodePtr->powerInfo) ||
	    (nodePtr->powerInfoCnt <= state)) {
		goto done;
	}

	power = PmNodeLookupConsumption(nodePtr, state);

	if ((false == NODE_IS_POWER(nodePtr)) ||
	    (true == NODE_IS_OFF(nodePtr))) {
		goto done;
	}

	/*
	 * If requested node is power island or domain, account
	 * power consumptions of components in hierarchy below
	 */
	root = (PmPower*)nodePtr->derived;
	currParent = root;
	childPos = 0U;

	/*
	 * Since MISRA-C doesn't allow recursion, we need an iterative
	 * algorithm to determine the power consumption for the complete tree.
	 * Using a depth-first approach:
	 */
	do {
		while (childPos < currParent->childCnt) {
			childNode = currParent->children[childPos];
			/* Every node may have a power consumption: */
			childPwr = PmNodeLookupConsumption(childNode,
							   childNode->currState);

			if (childPwr == POWER_MISSING) {
				power = POWER_MISSING;
				goto done;
			} else {
				power += childPwr;
			}

			if (true == NODE_IS_POWER(childNode)) {
				/* Work our way down to the lowest child */
				currParent = (PmPower*)childNode->derived;
				childPos = 0U;
			} else {
				childPos++;
			}
		}

		if (currParent == root) {
			break;
		}

		/* Now go back up one level, and continue where we left off */
		childPos = PmNodeGetChildPos(currParent->node.nodeId);
		if (currParent->childCnt <= childPos) {
			power = POWER_MISSING;
			goto done;
		}
		childPos++;
		currParent = currParent->node.parent;
	} while (1);

done:
	return power;
}

/**
 * PmNodeGetWakeLatency() - Get wake-up latency of a node
 * @node       Pointer to the node
 *
 * @returns    Wake-up latency for the node
 */
u32 PmNodeGetWakeLatency(PmNode* const nodePtr)
{
	u32 result = 0U;
	PmPower* parent = nodePtr->parent;
	const PmPower* power;
	const PmSlave* slave;
	const PmProc* proc;

	if (true == NODE_IS_SLAVE(nodePtr)) {
		slave = (PmSlave*)nodePtr->derived;
		result = PmGetLatencyFromState(slave, nodePtr->currState);
	} else if ((true == NODE_IS_PROC(nodePtr)) &&
		   (true == NODE_IS_OFF(nodePtr))) {
		proc = (PmProc*)nodePtr->derived;
		result = proc->pwrUpLatency;
	} else if ((true == NODE_IS_POWER(nodePtr)) &&
		   (true == NODE_IS_OFF(nodePtr))) {
		power = (PmPower*)nodePtr->derived;
		result = power->pwrUpLatency;
	}

	/*
	 * In case when parent power islands/domains are turned off,
	 * hierarchically account its powerUp latencies
	 */
	while ((NULL != parent) && (true == NODE_IS_OFF(&parent->node))) {
		result += parent->pwrUpLatency;
		parent = parent->node.parent;
	}

	return result;
}

/**
 * PmNodeDependsOnClock() - Check whether node currently depends on its clock
 */
bool PmNodeDependsOnClock(const PmNode* const node)
{
	bool deps = false;

	if (true == NODE_IS_SLAVE(node)) {
		const PmSlave* const slv = (PmSlave*)node->derived;

		if (0U != (PM_CAP_CLOCK & slv->slvFsm->states[node->currState])) {
			deps = true;
		}
	} else if (false == NODE_IS_OFF(node)) {
		deps = true;
	} else {
		/* Empty else required by MISRA */
	}

	return deps;
}

/**
 * PmNodeGetClassById() - Get node class by class ID
 * @id		ID of the class to get
 *
 * @return	Pointer to class if found, NULL otherwise.
 */
static PmNodeClass* PmNodeGetClassById(const u8 id)
{
	u32 i;
	PmNodeClass* class = NULL;

	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		if (id == pmNodeClasses[i]->id) {
			class = pmNodeClasses[i];
			break;
		}
	}

	return class;
}

/**
 * PmNodeGetFromClass() - Get node with given ID from class
 * @class	Node class to search through
 * @nid		ID of the node to get
 *
 * @return	Pointer to node if found, NULL otherwise.
 */
static PmNode* PmNodeGetFromClass(const PmNodeClass* const class, const u8 nid)
{
	u32 i;
	PmNode* node = NULL;

	for (i = 0U; i < class->bucketSize; i++) {
		if (nid == class->bucket[i]->nodeId) {
			node = class->bucket[i];
			break;
		}
	}

	return node;
}

/**
 * PmNodeGetSlave() - Get pointer to the slave by node ID
 * @nodeId      ID of the slave node
 *
 * @return      Pointer to the slave if found, NULL otherwise
 */
PmSlave* PmNodeGetSlave(const u32 nodeId)
{
	PmSlave* slave = NULL;
	PmNodeClass* class = PmNodeGetClassById(NODE_CLASS_SLAVE);
	PmNode* node = PmNodeGetFromClass(class, nodeId);

	if (NULL != node) {
		slave = (PmSlave*)node->derived;
	}

	return slave;
}

/**
 * PmNodeGetPower() - Get pointer to the power by node ID
 * @nodeId      ID of the power node
 *
 * @return      Pointer to the power if found, NULL otherwise
 */
PmPower* PmNodeGetPower(const u32 nodeId)
{
	PmPower* power = NULL;
	PmNodeClass* class = PmNodeGetClassById(NODE_CLASS_POWER);
	PmNode* node = PmNodeGetFromClass(class, nodeId);

	if (NULL != node) {
		power = (PmPower*)node->derived;
	}

	return power;
}

/**
 * PmNodeGetProc() - Get pointer to the processor structure by node ID
 * @nodeId      ID of the processor node
 *
 * @return      Pointer to the processor if found, NULL otherwise
 */
PmProc* PmNodeGetProc(const u32 nodeId)
{
	PmProc* proc = NULL;
	PmNodeClass* class = PmNodeGetClassById(NODE_CLASS_PROC);
	PmNode* node = PmNodeGetFromClass(class, nodeId);

	if (NULL != node) {
		proc = (PmProc*)node->derived;
	}

	return proc;
}

/**
 * PmNodeClearConfig() - Clear configuration for all nodes
 */
void PmNodeClearConfig(void)
{
	u32 i, n;

	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		for (n = 0U; n < pmNodeClasses[i]->bucketSize; n++) {
			PmNode* node = pmNodeClasses[i]->bucket[n];

			node->latencyMarg = MAX_LATENCY;

			if (NULL != pmNodeClasses[i]->clearConfig) {
				pmNodeClasses[i]->clearConfig(node);
			}
		}
	}
}
