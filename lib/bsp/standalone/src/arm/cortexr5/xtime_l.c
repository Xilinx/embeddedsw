/******************************************************************************
* Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xtime_l.c
*
* This file contains low level functions to get/set time from the Global Timer
* register in the ARM Cortex R5 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------
* 5.00 	pkp    08/29/14 First release
* 5.04  pkp	   02/19/16 XTime_StartTimer API is added to configure TTC3 timer
*						when present. XTime_GetTime is modified to give 64bit
*						output using timer overflow when TTC3 present.
*						XTime_SetTime is modified to configure TTC3 counter
*						value when present.
* 5.04	pkp	   03/11/16 XTime_StartTimer is modified to avoid enabling the
*						overflow interrupt and XTime_GetTime & XTime_SetTime
*						are modified to read and write TTC counter value
*						respectively
* 5.04	pkp
* 6.0   mus    08/11/16  Removed implementation of XTime_SetTime API, since
*                        TTC counter value register is read only.
* 6.6   srm    10/18/17 Removed XTime_StartTimer API and made XTime_GetTime,
*                       XTime_SetTime applicable for all the instances of TTC
* 7.2   mus    01/29/20 Updated XTime_GetTime to use CortexR5 PMU cycle
*                       counter, in case sleep timer (ttc3/ttc2) is not
*                       present in the HW design. It fixes CR#1051591.
* 7.5   mus    04/30/21  Moved pragma message from xtime_l.h to xtime_l.c, to avoid
*                        displaying same warnings multiple times. It fixes CR#1090562.
* 8.0   mus    07/06/21  Added support for VERSAL NET
* 9.0   dp     03/29/23  Added support to use ttc as sleep timer for VersalNet
*                        Cortex-R52.
* 9.0   asa    07/07/23  Made changes to include XTime_StartTimer for
*                        r52 freertos bsp.
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xtime_l.h"
#include "xpseudo_asm.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xdebug.h"
#include "xpm_counter.h"
#include "xil_sleeptimer.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
#if defined (ARMR52) && ((defined(FREERTOS_BSP)) || (defined(XSLEEP_TIMER_IS_DEFAULT_TIMER)))

#define LPD_RST_TIMESTAMP  0xEB5E035CU
/**
* @brief	Start the 64-bit physical timer counter.
*
* @return	None.
*
* @note		The timer is initialized only if it is disabled. If the timer is
*		already running this function does not perform any operation.
*
****************************************************************************/
void XTime_StartTimer(void)
{
		/* Take LPD_TIMESTAMP out of reset, TODO: remove this once FW flow is up */
		Xil_Out32(LPD_RST_TIMESTAMP, 0x0);

		/*write frequency to System Time Stamp Generator Register*/
		Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_FREQ_REG_OFFSET),
				XIOU_SCNTRS_FREQ);
		/*Enable the timer/counter*/
		Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET)
						,XIOU_SCNTRS_CNT_CNTRL_REG_EN);
}
#endif

#if defined(XSLEEP_TIMER_IS_DEFAULT_TIMER) && defined(ARMR52)
#pragma message ("For the sleep routines, global timer is used")
#elif defined (SLEEP_TIMER_BASEADDR)
#pragma message ("For the sleep routines, TTC3/TTC2 is used")
#elif !defined (DONT_USE_PMU_FOR_SLEEP_ROUTINES)
#pragma message ("For the sleep routines, PMU cycle counter is used")
#else
#pragma message ("For the sleep routines, machine cycles are used")
#endif

/****************************************************************************/
/**
* @brief    TTC Timer runs continuously and the time can not be set as
*			desired. This API doesn't contain anything. It is defined to have
*			uniformity across platforms.
*
* @param	Xtime_Global: 32 bit value to be written to the timer counter
*           register.
*
* @return	None.
*
* @note		In multiprocessor environment reference time will reset/lost for
*		    all processors, when this function called by any one processor.
*
****************************************************************************/
void XTime_SetTime(XTime Xtime_Global)
{
	(void) Xtime_Global;
/*Timer cannot be set to desired value, so the API is left unimplemented*/
    xdbg_printf(XDBG_DEBUG_GENERAL,
                "XTime_SetTime:Timer cannot be set to desired value,so API is not implemented\n");
}

/****************************************************************************/
/**
* @brief    Get the time from the timer counter register.
*
* @param	Xtime_Global: Pointer to the 32 bit location to be updated with
*           the time current value of timer counter register.
*
* @return	None.
*
****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global)
{
#if defined(XSLEEP_TIMER_IS_DEFAULT_TIMER) && defined(ARMR52)
	*Xtime_Global = arch_counter_get_cntvct();
#elif defined (SLEEP_TIMER_BASEADDR)
	*Xtime_Global = Xil_In32(SLEEP_TIMER_BASEADDR +
				      XSLEEP_TIMER_TTC_COUNT_VALUE_OFFSET);
#elif !defined (DONT_USE_PMU_FOR_SLEEP_ROUTINES)
	#if defined (__GNUC__)
	*Xtime_Global = Xpm_ReadCycleCounterVal();
	#elif defined (__ICCARM__)
	Xpm_ReadCycleCounterVal(*Xtime_Global);
	#endif
#endif
}
