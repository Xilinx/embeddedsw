/*
 * Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
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
