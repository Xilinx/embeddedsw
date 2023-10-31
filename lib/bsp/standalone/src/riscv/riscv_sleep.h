/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file riscv_sleep.h
*
* @addtogroup riscv_sleep_routines Sleep Routines for RISC-V
*
* The riscv_sleep.h file contains RISC-V sleep APIs. These APIs
* provides delay for requested duration.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.6   mus  10/24/23 Initial version
*
* </pre>
*
* @note
* The riscv_sleep.h file may contain architecture-dependent items.
*
******************************************************************************/

/**
 *@cond nocomments
 */
#ifndef RISCV_SLEEP_H		/* prevent circular inclusions */
#define RISCV_SLEEP_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "riscv_interface.h"
#include "xparameters.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

#if defined (XSLEEP_TIMER_IS_AXI_TIMER)

/** @name Register Offset Definitions
 * Register offsets within the Axi timer counter, there are multiple
 * timer counters within a single device
 * @{
 */
#define XSLEEP_TIMER_AXITIMER_TCSR0_OFFSET	0U	/**< Control/Status register */
#define XSLEEP_TIMER_AXITIMER_TLR_OFFSET	4U	/**< Load register */
#define XSLEEP_TIMER_AXITIMER_TCR_OFFSET	8U	/**< Timer counter register */
/* @} */

/** @name Control Status Register Bit Definitions
 * Control Status Register bit masks
 * Used to configure the Axi timer counter device.
 * @{
 */
#define XSLEEP_TIMER_AXITIMER_CSR_ENABLE_TMR_MASK	0x00000080U
				 /**< Enables only the specific timer */
#define XSLEEP_TIMER_AXITIMER_CSR_AUTO_RELOAD_MASK	0x00000010U
			         /**< In compare mode, configures the timer
 *                                    counter to  reload from the Load Register.
 *                                    The default mode causes the timer counter
 *                                    to hold when the compare value is hit. In
 *                                    capture mode, configures the timer counter
 *                                    to not hold the previous capture value if
 *                                    a new event occurs. The default mode cause
 *                                   the timer counter to hold the capture value
 *                                    until recognized.*/
#define XSLEEP_TIMER_AXITIMER_CSR_LOAD_MASK		0x00000020U
				 /**< Loads the timer using the  load value
 *                                    provided earlier in the Load Register,
 *                                    XSLEEP_TIMER_AXITIMER_TLR_OFFSET. */
#define XSLEEP_TIMER_AXITIMER_CSR_INT_OCCURED_MASK	0x00000100U
				 /**< If bit is set, an interrupt has occurred.
 *                                    If set and '1' is written to this bit
 *                                    position, bit is cleared.*/
#define XSLEEP_TIMER_AXITIMER_CSR_ENABLE_TMR_MASK   0x00000080U
				 /**< Enables only the specific timer */
/* @} */

/**************************** Type Definitions *******************************/
/************************** Function Prototypes ******************************/

static void Xil_SleepAxiTimer(u32 delay, u64 frequency);
static void XTime_StartAxiTimer(void);
#else
u32 Xil_SetRISCVFrequency(u32 Val);
u32 Xil_GetRISCVFrequency(void);
#endif
/**
 *@endcond
 */


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup microblaze_sleep_routines".
*/
