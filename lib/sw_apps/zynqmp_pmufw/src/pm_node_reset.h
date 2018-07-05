/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Implementation of individual node reset mechanism within
 * power management.
 *********************************************************************/

#ifndef SRC_PM_NODE_RESET_H_
#define SRC_PM_NODE_RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Idle request macro
 */
#define NODE_NO_IDLE_REQ	0U
#define NODE_IDLE_REQ		1U

/*
 * Idle and reset the node
 */
void PmNodeReset(const PmMaster *const Master, const u32 NodeId, const u32 IdleReq);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PM_NODE_RESET_H_ */
