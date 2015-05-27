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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * Definitions of commonly used macros and enums in PMU Power
 * Management (PM).
 *********************************************************************/

#ifndef PM_COMMON_H_
#define PM_COMMON_H_

#include "pmu_local.h"
#include "xpfw_default.h"
#include "xil_types.h"

/*********************************************************************
 * Typedefs (common use in PMU Power Management)
 ********************************************************************/
/*
 * stdint.h is not available, and pmu-fw framework defined similar
 * macros but as 0 and 1 (signed). PM, as in general, uses unsigned.
 */
#ifndef bool
	typedef u8 bool;
	#define true	1U
	#define false	0U
#endif

typedef u32 (*const PmTranHandler)(void);

/*********************************************************************
 * Macros
 ********************************************************************/
/*
 * Undefine DEBUG_PM macro to disable power management prints
 */
#ifdef DEBUG_MODE
	#define DEBUG_PM
#endif
/*
 * Conditional debugging prints used for PM. PM prints should never
 * appear if pmu-fw has disabled debug prints, however even when pmu-fw
 * has enabled debug prints, power management prints should be configurable.
 */
#ifdef DEBUG_PM
	#define PmDbg(MSG, ...)	fw_printf("PMUFW: %s: " MSG, __func__, ##__VA_ARGS__)
#else
	#define PmDbg(MSG, ...) {}
#endif

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#define BIT0(x) (x & 1U)

/* All WFI bitfields in GPI2 */
#define PMU_LOCAL_GPI2_ENABLE_ALL_PWRDN_REQ_MASK \
		(PMU_LOCAL_GPI2_ENABLE_ACPU0_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU1_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU2_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU3_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_R5_0_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_R5_1_PWRDWN_REQ_MASK)

/* All GIC wakes in GPI1 */
#define PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_1_WAKE_MASK)

/*
 * Macro for all wake events in GPI1 that PM handles.
 * Used for initialization to avoid looping through all slaves array.
 */
#define PMU_IOMODULE_GPI1_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK | \
		PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK | \
		PMU_LOCAL_GPI1_ENABLE_MIO_WAKE_MASK | \
		PMU_LOCAL_GPI1_ENABLE_USB1_WAKE_MASK | \
		PMU_LOCAL_GPI1_ENABLE_USB0_WAKE_MASK)

/* Enable/disable macros for wake events in GPI1 register */
#define ENABLE_WAKE(mask)   XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, mask, mask);
#define DISABLE_WAKE(mask)  XPfw_RMW32(PMU_LOCAL_GPI1_ENABLE, mask, ~(mask));

/*********************************************************************
 * Function declarations
 ********************************************************************/
#ifdef DEBUG_PM
const char* PmStrNode(const u32 node);
const char* PmStrMaster(const u32 master);
const char* PmStrAck(const u32 ack);
const char* PmStrReason(const u32 reason);
#endif

#endif /* PM_COMMON_H_ */
