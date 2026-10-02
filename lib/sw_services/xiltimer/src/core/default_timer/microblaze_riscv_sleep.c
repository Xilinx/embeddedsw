/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file microblaze_riscv_sleep.c
 * @addtogroup xiltimer_api XilTimer APIs
 * @{
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.4  mus	 24/11/23 Initial release.
 * 1.4  mus	 13/02/24 Update XilSleepTimer_Init properly initialize MB V
 *                        timer.
 *  2.5  ml    09/22/26 Fixed doxygen 1.15.0 and compliance checker warnings.
*</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xiltimer.h"

#ifdef XTIMER_IS_DEFAULT_TIMER
#ifdef SDT
#include "xmicroblaze_riscv_config.h"
#endif

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XMicroblaze_RISCV_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType);
static u32 RISCVFreq;

/****************************************************************************/
/**
 * Initialize the microblaze riscv sleep timer
 *
 * @param[out] InstancePtr is a pointer to the XTimer Instance
 *
 * @return	XST_SUCCESS always
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XMicroblaze_RISCV_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;
#ifdef SDT
	RISCVFreq = XGet_CpuFreq();
#else
	RISCVFreq = XPAR_CPU_CORE_CLOCK_FREQ_HZ;
#endif
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function configures the sleep interval using microblaze processor
 * instructions
 *
 * @param[in] InstancePtr is Pointer to the XTimer instance
 * @param[in] delay is the delay interval
 * @param[in] DelayType is the XTimer_DelayType
 *
 ****************************************************************************/
static void XMicroblaze_RISCV_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType)
{
	(void)InstancePtr;
	u64 tEnd = 0U;
        u64 tCur = 0U;
        u32 TimeHighVal = 0U;
        u32 TimeLowVal1 = 0U;
        u32 TimeLowVal2 = 0U;

	u32 CpuFreq = Xil_GetRISCVFrequency();

	CpuFreq = CpuFreq/DelayType;

        TimeLowVal1 = rdtime();
        tEnd = (u64)TimeLowVal1 + ((u64)(delay) * CpuFreq);
        do
        {
                TimeLowVal2 = rdtime();
                if (TimeLowVal2 < TimeLowVal1) {
                        TimeHighVal++;
                }
                TimeLowVal1 = TimeLowVal2;
                tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
        }while (tCur < tEnd);

}

#ifdef SDT
/*****************************************************************************/
/**
* @brief	Sets variable which stores Microblaze frequency value
* @param[in] Val - Frequency value to be set
* @return	XST_SUCCESS - If frequency updated successfully
* 			XST_INVALID_PARAM - If specified frequency value is not valid
*
* @note		It must be called after runtime change in Microblaze frequency,
* 			failing to do so would result in to incorrect behavior of sleep
* 			routines
*
******************************************************************************/
u32 Xil_SetRISCVFrequency(u32 Val)
{
	if ( Val != 0) {
		RISCVFreq = Val;
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
u32 Xil_GetRISCVFrequency(void)
{
	return RISCVFreq;
}
#endif
#endif

#ifdef XTIMER_NO_TICK_TIMER
/****************************************************************************/
/**
 * Initialize the microblaze Tick Instance
 *
 * @param[out] InstancePtr is a pointer to the XTimer Instance
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

/** @} */
