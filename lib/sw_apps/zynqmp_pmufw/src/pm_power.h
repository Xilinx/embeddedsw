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
 * Power nodes (power islands and power domains) related structures
 * and functions
 *********************************************************************/

#ifndef PM_POWER_H_
#define PM_POWER_H_

#include "pm_common.h"
#include "pm_node.h"

/*********************************************************************
 * Macros
 ********************************************************************/
/* States of power island/domain */
#define PM_PWR_STATE_OFF    0U
#define PM_PWR_STATE_ON     1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmPower - Structure for power related nodes
 *           Basically an abstraction of power islands and power domains.
 *           Not all power entities in the system have this struct. If a node has
 *           its own power, which does not depend to other nodes, its power is
 *           controlled within its transition actions. Otherwise, this power
 *           structure must exist.
 * @node     Node structure of this power entity
 * @children Pointer to the array of children
 * @childCnt Number of childs in children array
 */
typedef struct PmPower {
	PmNode node;
	PmNode** const children;
	const PmNodeId childCnt;
} PmPower;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmPower pmPowerIslandRpu_g;
extern PmPower pmPowerIslandApu_g;
extern PmPower pmPowerDomainFpd_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
void PmOpportunisticSuspend(PmPower* const power);
u32 PmTriggerPowerUp(PmPower* const power);
u32 PmForceDownTree(PmPower* const root);

#endif
