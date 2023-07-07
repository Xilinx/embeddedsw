/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xtime_l.h
*
* @addtogroup r5_time_apis Cortex R5 Time Functions
* The xtime_l.h provides access to 32-bit TTC timer counter. These functions
* can be used by applications to track the time.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------
* 5.00 	pkp	   05/29/14 First release
* 5.04  pkp	   02/19/16 Added timer configuration register offset definitions
* 5.04	pkp	   03/11/16 Removed definitions for overflow interrupt register
*						and mask
* 6.6   srm    10/22/17 Added a warning message for the user configurable sleep
*                       implementation when default timer is selected by the user
* 7.2   mus    01/29/20 Updated #defines to support PMU cycle counter for sleep
*                       routines.
* 7.5   dp      01/05/21 Updated COUNTS_PER_SECOND/USECOND and ITERS_PER_SEC/USEC
*                        macros to round it off to nearest possible value so that
*                        delta error in time calculations can be minimized.
* 7.5   mus    04/30/21  Moved pragma message from xtime_l.h to xtime_l.c, to avoid
*                        displaying same warnings multiple times. It fixes CR#1090562.
* 8.0   mus    07/06/21 Added support for VERSAL NET.
* 8.0	sk	03/02/22 Update COUNTS_PER_USECOND macro to fix misra_c_2012_rule_
* 			 10_4 violation.
* 9.0   dp     03/29/23  Added support to use ttc as sleep timer for VersalNet
*                        Cortex-R52.
* 9.0   asa    07/07/23  Made changes to include bspconfig.h and update
*                        macros checks for r52 freertos bsp use case.
* </pre>
*
******************************************************************************/

/**
 *@cond nocomments
 */

#ifndef XTIME_H /* prevent circular inclusions */
#define XTIME_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xparameters.h"
#include "bspconfig.h"
/***************** Macros (Inline Functions) Definitions *********************/
#if defined (ARMR52)
/* TODO: Taken from ARMv8 32 bit BSP, check if we can keep it in some common location */
static inline u64 arch_counter_get_cntvct(void)
 {
          u64 cval;
          __asm__ __volatile__("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
          return cval;
  }
#endif
/************************** Constant Definitions *****************************/

#if defined (ARMR52)
#define COUNTS_PER_SECOND     XPAR_CPU_CORTEXR52_0_TIMESTAMP_CLK_FREQ
#elif defined (SLEEP_TIMER_BASEADDR)

#define COUNTS_PER_SECOND				SLEEP_TIMER_FREQUENCY

#else
#define ITERS_PER_SEC  ((XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ + 2)/ 4)
#define ITERS_PER_USEC  ((XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ + 2000000)/ 4000000)

/*
 * These constants are applicable for the CortexR5 PMU cycle counter.
 * As boot code is setting up "D" bit in PMCR, cycle counter increments
 * on every 64th bit of processor cycle
 */

#define COUNTS_PER_SECOND	((XPAR_CPU_CORTEXR5_0_CPU_CLK_FREQ_HZ + 32)/ 64)
#endif

#define COUNTS_PER_USECOND	((COUNTS_PER_SECOND + 500000U) / 1000000U)

#define IRQ_FIQ_MASK 	0xC0	/* Mask IRQ and FIQ interrupts in cpsr */

#if defined (ARMR52) && ((defined(FREERTOS_BSP)) || (defined(XSLEEP_TIMER_IS_DEFAULT_TIMER)))
#pragma message ("For the sleep routines, global timer is used")
#elif defined (SLEEP_TIMER_BASEADDR)
#pragma message ("For the sleep routines, TTC3/TTC2 is used")
#elif !defined (DONT_USE_PMU_FOR_SLEEP_ROUTINES)
#pragma message ("For the sleep routines, PMU cycle counter is used")
#else
#pragma message ("For the sleep routines, machine cycles are used")
#endif

#if defined (ARMR52)
#define XIOU_SCNTRS_BASEADDR 0xEB5B0000U
#define XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET 0x0U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK 0x1U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN 0x1U
#define XIOU_SCNTRS_FREQ_REG_OFFSET 0x20U
#define XIOU_SCNTRS_FREQ XPAR_CPU_CORTEXR52_0_TIMESTAMP_CLK_FREQ
#endif

/*
 * 1st bit of PROCESSOR_ACCESS_VALUE macro signifies trustzone
 * setting for IOU slcr address space
 */
#define IOU_SLCR_TZ_MASK	0x2U
/**************************** Type Definitions *******************************/
#if defined(ARMR52)
typedef u64 XTime;
#else
typedef u32 XTime;
#endif
/**
 *@endcond
 */

void XTime_SetTime(XTime Xtime_Global);
void XTime_GetTime(XTime *Xtime_Global);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XTIME_H */
/**
* @} End of "@addtogroup r5_time_apis".
*/
