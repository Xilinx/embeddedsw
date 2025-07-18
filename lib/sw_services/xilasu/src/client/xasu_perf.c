/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_perf.c
 *
 * This file contains the implementation of the performance measurement APIs for client.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   03/05/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_perf_client_apis PERF Client APIs
 * @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_perf.h"
#include "xil_io.h"
#include <stdio.h>

/* If performance measurement is enabled */
#ifdef XASU_PERF_MEASUREMENT_ENABLE

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/************************************ Macros (Inline Functions) Definitions **********************/
#define XASU_32BIT_SHIFT	(32U) /**< Shift value for 32-bit upper portion. */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
/** Global variable to store the start timestamp. */
static u64 Start_Time = 0U;

/*************************************************************************************************/
/**
 * @brief	This function reads the 64-bit timestamp counter atomically.
 *
 * @return
 * 		- 64-bit timestamp value.
 *
 *************************************************************************************************/
static u64 XAsu_ReadTimestamp(void)
{
	u32 Lower32bit;
	u32 Upper32bit_Prev;
	u32 Upper32bit_Final;

	do {
		/**
		 * Reading upper 32bits twice to avoid race conditions, lower 32bits should not be
		 * overflowed between the reads, leading to incorrect values.
		 */
		Upper32bit_Prev = Xil_In32(XASU_LPD_SYSTMR_CTRL_BASE_ADDRESS +
			XASU_LPD_SYSTMR_CTRL_CNTCVU_OFFSET);
		Lower32bit = Xil_In32(XASU_LPD_SYSTMR_CTRL_BASE_ADDRESS +
			XASU_LPD_SYSTMR_CTRL_CNTCVL_OFFSET);
		Upper32bit_Final = Xil_In32(XASU_LPD_SYSTMR_CTRL_BASE_ADDRESS +
			XASU_LPD_SYSTMR_CTRL_CNTCVU_OFFSET);
		/**
		 * If the upper 32 bits changed between the two reads, it means the
		 * lower 32 bits rolled over in between. In this case, we retry to
		 * ensure a consistent 64-bit timestamp.
		 */
	} while (Upper32bit_Prev != Upper32bit_Final);

	return (((u64)Upper32bit_Final << XASU_32BIT_SHIFT) | Lower32bit);
}

/*************************************************************************************************/
/**
 * @brief	This function initializes performance measurement, by enabling the counter control.
 *
 *************************************************************************************************/
void XAsu_PerfInit(void)
{
	/** Reset the start time. */
	Start_Time = 0U;

	/** Enable the counter. */
	Xil_Out32((XASU_LPD_SYSTMR_CTRL_BASE_ADDRESS +
		XASU_LPD_SYSTMR_CTRL_CNTCR_OFFSET), XASU_LPD_SYSTMR_CTRL_CNTCR_ENABLE);
}

/*************************************************************************************************/
/**
 * @brief	This function starts execution timinga and stores the timestamp internally.
 *
 *************************************************************************************************/
void XAsu_StartTiming(void)
{
	Start_Time = XAsu_ReadTimestamp();
}

/*************************************************************************************************/
/**
 * @brief	This function ends execution timing, calculates elapsed time, and prints it.
 *
 * @param	Function_name	Name of the function being measured.
 *
 *************************************************************************************************/
void XAsu_EndTiming(const char* Function_name)
{
	float ElapsedTime_ms = 0.0f;
	u64 End_Time = XAsu_ReadTimestamp();

	if (Start_Time != 0U) {
		/** Calculate execution time in milliseconds. */
		ElapsedTime_ms = ((float)(End_Time - Start_Time)) /
			(XPAR_CPU_TIMESTAMP_CLK_FREQ / 1000.0f);
	}

	/** Print execution time. */
	printf("Execution time for %s: %.3f ms\n", Function_name, ElapsedTime_ms);

	/** Reset start time to avoid accidental reuse. */
	Start_Time = 0U;
}

#endif  /* XASU_PERF_MEASUREMENT_ENABLE */
/** @} */