/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_NODE_H_
#define XPM_NODE_H_

#include <xil_types.h>
#include <xstatus.h>
#include "xillibpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_IDLE_DONE			(0x4U)

typedef struct XPm_Node XPm_Node;

/**
 * The node class.  This is the base class for all the power, clock, pin and
 * reset node classes.
 */
struct XPm_Node {
	u32 Id;	/**< Node ID: For LibPM clock and pin APIs */
	u32 State; /**< Node state: Specific to node type */
	u32 BaseAddress; /**< Base address: Specify to node type */
	u32 LatencyMarg; /**< lowest latency requirement - powerup latency */
	u8  Flags;
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event);
		/**< HandleEvent: Pointer to event handler */
};

/************************** Function Prototypes ******************************/
XStatus XPmNode_Init(XPm_Node *Node,
		u32 Id, u32 State, u32 BaseAddress);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NODE_H_ */
