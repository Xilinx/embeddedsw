/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*********************************************************************
 * CONTENT
 * Timer interface, contains function to initialize timer and
 * global variable tick_count which stores total number of interrupts
 * generated by the timer.
 *********************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDT
 #define TTC0_0_DEVICE_ID	XPAR_XTTCPS_0_BASEADDR
 #define COUNT_PER_SEC		(XPAR_XTTCPS_0_CLOCK_FREQ / 65535)
#else
 #define TTC0_0_DEVICE_ID	XPAR_XTTCPS_0_DEVICE_ID
 #define COUNT_PER_SEC		(XPAR_XTTCPS_0_CLOCK_HZ / 65535)
#endif

#define TTC_INT_ID0		XPAR_XTTCPS_0_INTR

#define TIMER_PERIOD		3

s32 TimerInit(u32 timeout);

s32 TimerConfigure(u32 timer_period);

extern volatile u32 TickCount;

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
