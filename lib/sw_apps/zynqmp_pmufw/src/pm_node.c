/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Global array of all nodes, and GetbyId function
 *********************************************************************/

#include "pm_node.h"
#include "pm_power.h"
#include "pm_proc.h"
#include "pm_slave.h"
#include "pm_notifier.h"
#include "pm_clock.h"

static PmNodeClass* pmNodeClasses[] = {
	&pmNodeClassProc_g,
	&pmNodeClassPower_g,
	&pmNodeClassSlave_g,
	&pmNodeClassPll_g,
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

	/*
	 * Clear NODE_IDLE_DONE flag, if current state of node is OFF and
	 * new state is other than OFF.
	 */
	if (NODE_IS_OFF(node)) {
		node->flags &= ~(u8)NODE_IDLE_DONE;
	}

	node->currState = newState;

	PmNotifierEvent(node, EVENT_STATE_CHANGE);

done:
	return;
}

/**
 * PmNodeGetPowerInfo() - Get power consumption information for a node
 * @node	Pointer to the node
 * @data	Pointer to the location where the power info should be stored
 *
 * @returns	Status about returning power info
 *		XST_SUCCESS if power info is stored in *data
 *		XST_NO_FEATURE if no power info can be provided for the node
 */
s32 PmNodeGetPowerInfo(const PmNode* const node, u32* const data)
{
	s32 status = XST_SUCCESS;

	if (NULL == node->powerInfo) {
		status = XST_NO_FEATURE;
		goto done;
	}

	*data = node->powerInfo[node->currState];

done:
	return status;
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
 * PmNodeGetDerived() - Get pointer to the derived structure of the node
 * @nodeClass	Node class
 * @nodeId      ID of the node
 *
 * @return      Pointer to the derived node object if found, NULL otherwise
 */
void* PmNodeGetDerived(const u8 nodeClass, const u32 nodeId)
{
	void* ptr = NULL;
	PmNode* node = NULL;
	PmNodeClass* class = PmNodeGetClassById(nodeClass);

	if (NULL != class) {
		node = PmNodeGetFromClass(class, (u8)nodeId);
	}

	if (NULL != node) {
		ptr = node->derived;
	}

	return ptr;
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
			node->flags = 0U;

			if (NULL != pmNodeClasses[i]->clearConfig) {
				pmNodeClasses[i]->clearConfig(node);
			}
		}
	}
}

/**
 * PmNodeConstruct() - Call constructors for all nodes
 */
void PmNodeConstruct(void)
{
	u32 i, n;

	PmClockConstructList();

	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		for (n = 0U; n < pmNodeClasses[i]->bucketSize; n++) {
			PmNode* node = pmNodeClasses[i]->bucket[n];

			if (NULL != pmNodeClasses[i]->construct) {
				pmNodeClasses[i]->construct(node);
			}
		}
	}
}

/**
 * PmNodeInit() - Initialize all nodes
 * @return	XST_SUCCESS or error code if failed to initialize a node
 */
s32 PmNodeInit(void)
{
	u32 i, n;
	s32 status;
	s32 ret = XST_SUCCESS;

	PmClockInit();
	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		for (n = 0U; n < pmNodeClasses[i]->bucketSize; n++) {
			PmNode* node = pmNodeClasses[i]->bucket[n];

			if (NULL == pmNodeClasses[i]->init) {
				continue;
			}
			status = pmNodeClasses[i]->init(node);
			if (XST_SUCCESS != status) {
				ret = XST_FAILURE;
				PmErr("init %s failed\r\n", node->name);
			}

		}
	}

	return ret;
}

/**
 * PmNodeForceDown() - Force down the node
 * @node	Node to force down
 *
 * @return	XST_FAILURE if no force operation to execute for the node,
 *		otherwise status of performing force down operation
 */
s32 PmNodeForceDown(PmNode* const node)
{
	s32 status = XST_FAILURE;

	if (NULL != node->class->forceDown) {
		status = node->class->forceDown(node);
	}

	return status;
}

/**
 * PmNodeForceDownUnusable() - Force down nodes that are unusable
 *
 * @note	Function puts in the lowest power states the nodes which have no
 *		provision to be used by currently set configuration.
 */
void PmNodeForceDownUnusable(void)
{
	u32 i, n;

	for (i = 0U; i < ARRAY_SIZE(pmNodeClasses); i++) {
		for (n = 0U; n < pmNodeClasses[i]->bucketSize; n++) {
			PmNode* node = pmNodeClasses[i]->bucket[n];
			bool usable = true;

			if (NULL != pmNodeClasses[i]->isUsable) {
				usable = pmNodeClasses[i]->isUsable(node);
			}
			if (true == usable) {
				continue;
			}
			if (NULL != pmNodeClasses[i]->forceDown) {
				(void)pmNodeClasses[i]->forceDown(node);
			}
		}
	}
}

/**
 * PmNodeLogUnknownState() - Log an unknown state error for a given node
 * @node	Node pointer
 */
void PmNodeLogUnknownState(const PmNode* const node, const PmStateId state)
{
	PmErr("Unknown %s state #%u\r\n", node->name, state);
}

/**
 * PmNodeGetPermissions() - Get permissions of masters to control node's clocks
 * @node	Target node
 *
 * @return	Zero if no master has permission or node class does not
 *		implement get permissions method. ORed ipi masks of masters
 *		that have permissions otherwise.
 */
u32 PmNodeGetPermissions(PmNode* const node)
{
	u32 perms = 0U;

	if (NULL != node->class->getPerms) {
		perms = node->class->getPerms(node);
	}

	return perms;
}

#endif
