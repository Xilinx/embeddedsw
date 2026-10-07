/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xtime_l.h
*
* @addtogroup a53_32_time_apis Cortex A53 32bit Mode Time Functions
* xtime_l.h provides access to the 64-bit physical timer counter.
*
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------
* 5.2	pkp	   28/05/15 First release
* 6.6   srm    10/23/17 Updated the macros to support user configurable sleep
*			implementation
* 7.5   mus    04/30/21  Moved pragma message from xtime_l.h to xtime_l.c, to avoid
*                        displaying same warnings multiple times. It fixes CR#1090562.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#ifndef XTIME_H
#define XTIME_H

/**
 *@cond nocomments
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xparameters.h"

/***************** Macros (Inline Functions) Definitions *********************/
static inline u64 arch_counter_get_cntvct(void)
 {
          u64 cval;
          __asm__ __volatile__("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
          return cval;
  }
/**************************** Type Definitions *******************************/

typedef u64 XTime;

/************************** Constant Definitions *****************************/
#if defined (SLEEP_TIMER_BASEADDR)
#define COUNTS_PER_SECOND     SLEEP_TIMER_FREQUENCY
#else
#define COUNTS_PER_SECOND     XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ
#endif

#define XIOU_SCNTRS_BASEADDR               0xFF260000U
#define XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET   0x00000000U
#define XIOU_SCNTRS_FREQ_REG_OFFSET    	   0x00000020U
#define XIOU_SCNTRS_FREQ		   XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN       0x00000001U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK  0x00000001U

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

void XTime_StartTimer(void);
/**
 *@endcond
 */

/**
 * @brief Compatibility no-op; the continuously running generic timer cannot be set.
 * @param[in] Xtime_Global Unused; the timer counter is not modified.
 */
void XTime_SetTime(XTime Xtime_Global);

/**
 * @brief Get the current global timer count.
 * @param[out] Xtime_Global Pointer to store the current timer counter value.
 */
void XTime_GetTime(XTime *Xtime_Global);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XTIME_H */
/**
* @} End of "addtogroup a53_32_time_apis".
*/
