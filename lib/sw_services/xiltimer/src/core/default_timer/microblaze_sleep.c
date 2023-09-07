/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file microblaze_sleep.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * @details
 *
 * This file contains the definitions for sleep implementation using microblaze
 * processor instructions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0  adk	 24/11/21 Initial release.
 * 1.1 	adk      08/08/22 Added doxygen tags.
 * 1.3  adk      26/04/23 XGet_CpuFreq() API is defined in the system
 * 			  device-tree flow, update ifdef check for the same.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"

#ifdef XTIMER_IS_DEFAULT_TIMER
#ifdef SDT
#include "xmicroblaze_config.h"
#endif

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XMicroblaze_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType);
static u32 MBFreq;
/****************************************************************************/
/**
 * Initialize the microblaze sleep timer
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XMicroblaze_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;
#ifdef SDT
	MBFreq = XGet_CpuFreq();
#else
	MBFreq = XPAR_CPU_CORE_CLOCK_FREQ_HZ;
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function configures the sleep interval using microblaze processor
 * instructions
 *
 * @param  InstancePtr is Pointer to the XTimer instance
 * @param  Delay is the delay interval
 * @param  DelayType is the XTimer_DelayType
 *
 * @return	None
 *
 ****************************************************************************/
static void XMicroblaze_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType)
{
	(void)InstancePtr;
#ifndef SDT
        u32 CpuFreq = XPAR_CPU_CORE_CLOCK_FREQ_HZ;
#else
        u32 CpuFreq = Xil_GetMBFrequency();
#endif
        u32 iterpersec = CpuFreq / 4;
	u32 iters = iterpersec / DelayType;

	asm volatile (
			"1:               \n\t"
			"addik %1, %1, -1 \n\t"
			"add   r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"or  r0, r0, r0   \n\t"
			:
			: "r"(iters), "r"(delay)
			: "r0", "r7"
	);
}

#ifdef SDT
/*****************************************************************************/
/**
* @brief	Sets variable which stores Microblaze frequency value
* @param	Val - Frequency value to be set
* @return	XST_SUCCESS - If frequency updated successfully
* 			XST_INVALID_PARAM - If specified frequency value is not valid
*
* @note		It must be called after runtime change in Microblaze frequency,
* 			failing to do so would result in to incorrect behavior of sleep
* 			routines
*
******************************************************************************/
u32 Xil_SetMBFrequency(u32 Val)
{
	if ( Val != 0) {
		MBFreq = Val;
		return XST_SUCCESS;
	}
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* @brief	Returns current Microblaze frequency value
* @return	MBFreq - Current Microblaze frequency value
*
******************************************************************************/
u32 Xil_GetMBFrequency(void)
{
	return MBFreq;
}
#endif
#endif

#ifdef XTIMER_NO_TICK_TIMER
/****************************************************************************/
/**
 * Initialize the microblaze Tick Instance
 *
 * @param	InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = NULL;
	InstancePtr->XTimer_TickInterval = NULL;
	InstancePtr->XTickTimer_Stop = NULL;
	InstancePtr->XTickTimer_ClearInterrupt = NULL;
	return XST_SUCCESS;
}
#endif
