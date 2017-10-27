/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef XPFW_CONFIG_H_
#define XPFW_CONFIG_H_

/************* User Configurable Options ***************/

/* PMUFW print levels */
#define XPFW_PRINT_VAL (1U)
#define XPFW_DEBUG_ERROR_VAL (0U)
#define XPFW_DEBUG_DETAILED_VAL (0U)

/**
 * PMUFW Debug options
 */

#if XPFW_PRINT_VAL
#define XPFW_PRINT
#endif

#if XPFW_DEBUG_ERROR_VAL
#define XPFW_DEBUG_ERROR
#endif

#if XPFW_DEBUG_DETAILED_VAL
#define XPFW_DEBUG_DETAILED
#endif

/* PMU clock frequency in Hz */
#ifndef XPFW_CFG_PMU_CLK_FREQ
#define XPFW_CFG_PMU_CLK_FREQ XPAR_CPU_CORE_CLOCK_FREQ_HZ
#endif

/* Let the MB sleep when it is Idle in Main Loop */
#define SLEEP_WHEN_IDLE

/*
 * Define the below parameters to enable corresponding modules
 * ENABLE_PM        - Enable Power Management Module
 * ENABLE_EM        - Enable Error Management Module
 * ENABLE_RTC_TEST  - Enable RTC Event Handler Test Module
 * ENABLE_SCHEDULER - Enable Scheduler Test Module
 */

/* Enable Power Management Module */
#define ENABLE_PM

/* FPD WDT recovery action */
#ifdef ENABLE_RECOVERY
#define FPD_WDT_EM_ACTION EM_ACTION_CUSTOM
#else
#define FPD_WDT_EM_ACTION EM_ACTION_SRST
#endif

#endif /* XPFW_CONFIG_H_ */
