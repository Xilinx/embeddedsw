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
 * @statusAddr  Address of the PLL's status register
 * @perms	Permissions to directly control the PLL
 * @lockShift	Shift of the lock status bit in status register
 * @flags	PLL flags
 */
typedef struct PmPll {
	PmNode node;
	PmPllContext context;
	const u32 addr;
	const u32 statusAddr;
	u32 perms;
	const u8 lockShift;
	u8 flags;
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
void PmPllRequest(PmPll* const pll);
void PmPllRelease(PmPll* const pll);
void PmPllOpenAccess(PmPll* const pll, u32 ipiMask);

int PmPllSetModeInt(PmPll* const pll, const u32 mode);
int PmPllSetParameterInt(PmPll* const pll, const u32 paramId, const u32 val);
int PmPllGetParameterInt(PmPll* const pll, const u32 paramId, u32* const val);

u32 PmPllGetModeInt(PmPll* const pll);
static inline u32 PmPllGetPermissions(const PmPll* const pll)
{
	return pll->perms;
};

#endif
