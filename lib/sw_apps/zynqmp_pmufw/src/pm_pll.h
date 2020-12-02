/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

#ifdef __cplusplus
extern "C" {
#endif

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
 * @errmask	PMU GLOBAL error mask to disable and enable error interrupt
 * @errValue	PMU GLOBAL error value
 */
typedef struct PmPll {
	PmNode node;
	PmPllContext context;
	const u32 addr;
	const u32 statusAddr;
	u32 perms;
	const u8 lockShift;
	u8 flags;
	u32 childCount;
#ifdef ENABLE_EM
	u32 errShift;
	u32 errValue;
#endif
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

s32 PmPllSetModeInt(PmPll* const pll, const u32 mode);
s32 PmPllSetParameterInt(PmPll* const pll, const u32 paramId, const u32 val);
s32 PmPllGetParameterInt(PmPll* const pll, const u32 paramId, u32* const val);

u32 PmPllGetModeInt(PmPll* const pll);
static inline u32 PmPllGetPermissions(const PmPll* const pll)
{
	return pll->perms;
};

#ifdef __cplusplus
}
#endif

#endif /* PM_PLL_H_ */
