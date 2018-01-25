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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Contains:
 * - PLL slave implementation
 * - Functions for saving and restoring PLLs' context
 *
 * Note: PMU does not control states of PLLs. When none of FPD PLLs
 * is used and FPD is going to be powered down, PMU saves context of
 * PLLs in FPD and asserts their reset. After powering up FPD, PMU
 * restores the state of PLL based on saved context.
 *********************************************************************/

#ifndef PM_PLL_H_
#define PM_PLL_H_

#include "pm_node.h"

/* PLL states: */
#define PM_PLL_STATE_RESET	0U
#define PM_PLL_STATE_LOCKED	1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmPllContext - Structure for saving context of PLL registers.
 *              Contains variable to store default content of:
 * @ctrl        Control register
 * @cfg         Configuration register
 * @frac        Fractional control register
 * @toCtrl      Control for a cross domain (a divisor)
 * @saved       Flag stating are variables of this structure containing values
 *              to be restored or not
 *
 * Note: context of the PLL is saved when PM framework suspends a PLL (when
 * no node requires PLL to be locked). It is assumed that all used PLLs get
 * initially configured/locked by the FSBL and no user code would unlock a PLL
 * afterwards.
 */
typedef struct PmPllContext {
	u32 ctrl;
	u32 cfg;
	u32 frac;
	u32 toCtrl;
	bool saved;
} PmPllContext;

/**
 * PmPll - Structure used to model PLL
 * @node        Node structure
 * @context     Data to store context of the PLL - if after boot PLL has no
 *              context, it should not be initially locked by PMU, but by a
 *              master. To inform PMU that initially PLL has no context, this
 *              field should be initialized with the PM_PLL_CTRL_RESET_MASK
 *              set, statically or through PCW.
 * @addr        Base address of the PLL's control registers
 * @toCtrlAddr  Absolute address of cross-domain control register
 * @statusAddr  Address of the PLL's status register
 * @lockMask    Mask of the lock in status register
 * @useCount    The number of clocks currently driven by this PLL
 */
typedef struct PmPll {
	PmNode node;
	PmPllContext context;
	const u32 addr;
	const u32 toCtrlAddr;
	const u32 statusAddr;
	const u32 lockMask;
	u32 useCount;
} PmPll;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmPll pmApll_g;
extern PmPll pmDpll_g;
extern PmPll pmVpll_g;
extern PmPll pmRpll_g;
extern PmPll pmIOpll_g;

extern PmNodeClass pmNodeClassPll_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmPllRequest(PmPll* const pll);
void PmPllRelease(PmPll* const pll);

#endif
