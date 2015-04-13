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
 * Contains all functions, datas and definitions needed for
 * managing processor's states.
 *********************************************************************/

#ifndef PM_PROC_H_
#define PM_PROC_H_

#include "pm_common.h"
#include "pm_node.h"

typedef u8 PmProcEvent;

/*********************************************************************
 * Macros
 ********************************************************************/
/* Used for designated initialization */
#define PM_PROC_APU_0   0U
#define PM_PROC_APU_1   1U
#define PM_PROC_APU_2   2U
#define PM_PROC_APU_3   3U
#define PM_PROC_APU_MAX 4U

#define PM_PROC_RPU_0   0U
#define PM_PROC_RPU_1   1U
#define PM_PROC_RPU_MAX 2U

/* Enable/disable macros for processor's wfi event in GPI2 register */
#define ENABLE_WFI(mask)    XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, mask, mask);
#define DISABLE_WFI(mask)   XPfw_RMW32(PMU_LOCAL_GPI2_ENABLE, mask, ~(mask));

/*
 * Processor is powered down as requested by a master which is priviledged
 * to request so. Processor has not saved its context.
 */
#define PM_PROC_STATE_FORCEDOFF     0U

/*
 * Processor sleep state (specific for the processor implementation,
 * if processor has its own power domain it is powered down)
 */
#define PM_PROC_STATE_SLEEP         2U

/*
 * Processor active state. If it executes WFI without previously requesting
 * suspend through PM API it is considered active.
 */
#define PM_PROC_STATE_ACTIVE        1U

/*
 * Processor suspending state. It has called pm_self_suspend but WFI
 * interrupt from this processor is not yet received.
 */
#define PM_PROC_STATE_SUSPENDING    3U

/* Triggered when pm_self_suspend call is received for a processor */
#define PM_PROC_EVENT_SELF_SUSPEND  1U

/*
 * Triggered by pm_abort_suspend call made by a processor to cancel its
 * own suspend.
 */
#define PM_PROC_EVENT_ABORT_SUSPEND 2U

/* Triggered when processor has executed wfi instruction */
#define PM_PROC_EVENT_SLEEP         3U

/* Triggered when a master requested force powerdown for this processor */
#define PM_PROC_EVENT_FORCE_PWRDN   4U

/* Triggered when PMU receives wake interrupt targeted to the processor */
#define PM_PROC_EVENT_WAKE          5U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
typedef struct PmMaster PmMaster;
/**
 * PmProc - Processor node's structure
 * @node            Processor's node structure
 * @master          Master channel used by this processor
 * @isPrimary       True if this is a primary core (owner of Master channel)
 * @wfiStatusMask   Mask in PM_IOMODULE_GPI2 register (WFI interrupt)
 * @wakeStatusMask  Mask in PM_IOMODULE_GPI1 register (GIC wake interrupt)
 * @wfiEnableMask   Mask in PM_LOCAL_GPI2_ENABLE register (WFI interrupt)
 * @wakeEnableMask  mask in PM_LOCAL_GPI1_ENABLE register (GIC wake interrupt)
 */
typedef struct {
	PmNode node;
	PmMaster* const master;
	bool isPrimary;
	const u32 wfiStatusMask;
	const u32 wakeStatusMask;
	const u32 wfiEnableMask;
	const u32 wakeEnableMask;
} PmProc;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmProc pmApuProcs_g[PM_PROC_APU_MAX];
extern PmProc pmRpuProcs_g[PM_PROC_RPU_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
u32 PmProcFsm(PmProc* const proc, const PmProcEvent event);

#endif
