/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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

#ifndef PM_CLOCK_H_
#define PM_CLOCK_H_

#include "pm_pll.h"

typedef struct PmClock PmClock;
typedef struct PmNode PmNode;
typedef struct PmClockDeps PmClockDeps;

/*********************************************************************
 * Macros
 ********************************************************************/
#define PM_CLOCK_MUX_SELECT_MASK	0x3U

/*********************************************************************
 * Structure definitions
 ********************************************************************/

/**
 * PmClockSel2Pll - Pair of multiplexer select value and selected PLL
 * @select	Select value of the clock multiplexer
 * @pll		Pointer to the PLL that is selected with the 'select' value
 */
typedef struct {
	PmPll* const pll;
	const u8 select;
} PmClockSel2Pll;

/**
 * PmClockMux - Structure to encapsulate MUX select values to pll mapping
 * @inputs	Mux select to pll mapping at the input of the multiplexer
 * @size	Size of the array to which the sel2Pll points
 */
typedef struct {
	const PmClockSel2Pll* const inputs;
	const u8 size;
} PmClockMux;

/**
 * PmClock - Clock configuration data
 * @mux		Mux select to pll mapping associated with a slave's clocks
 * @pll		Pointer to the PLL that currently drives the clock
 * @users	Pointer to the list of nodes that use this clock
 * @ctrlAddr	Address of the control register of the clock
 */
typedef struct PmClock {
	const PmClockMux* const mux;
	PmPll* pll;
	PmClockHandle* users;
	const u32 ctrlAddr;
	u32 ctrlVal;
} PmClock;

/**
 * PmClockHandle - Models a clock/node pair (node using the clock)
 * @clock	Pointer to the clock used by the node
 * @node	Pointer to the node that uses a clock
 * @nextClock	Pointer to the next structure for another clock used by the same
 *		node
 * @nextNode	Pointer to the next structure for the same clock used by some
 *		other node
 */
typedef struct PmClockHandle {
	PmClock* clock;
	PmNode* node;
	PmClockHandle* nextClock;
	PmClockHandle* nextNode;
	s32 (*IsActiveClk)(PmClockHandle* const ch);
} PmClockHandle;

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmClockRequest(PmNode* const node);

void PmClockRelease(PmNode* const node);
void PmClockSnoop(const u32 addr, const u32 mask, const u32 val);
void PmClockConstructList(void);
void PmClockRestore(PmNode* const node);
void PmClockSave(PmNode* const node);
s32 PmClockIsActive(PmNode* const node);
void PmClockRestoreDdr();

#endif
