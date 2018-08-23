/******************************************************************************
* Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_module.h"

#include "xpfw_mod_wdt.h"

#ifdef ENABLE_WDT

/* Check if PMU has access to CSU WDT (psu_csu_wdt) */
#ifdef XPAR_PSU_CSU_WDT_DEVICE_ID
	#include "xwdtps.h"
#else /* XPAR_PSU_CSU_WDT_DEVICE_ID */
	#error "ENABLE_WDT is defined but psu_csu_wdt is not defined in the design"
#endif

/* Instance of WDT Driver */
static XWdtPs WdtInst;
static XWdtPs *WdtInstPtr = &WdtInst;

/*
 * WDT expire time in milliseconds.
 */
#define XPFW_WDT_EXPIRE_TIME 90U

/*
 * WDT restart time in milliseconds.
 */
#define XPFW_WDT_RESTART_TIME 50U
#define XPFW_WDT_CRV_SHIFT 12U
#define XPFW_WDT_PRESCALER 8U

#define XPFW_WDT_CLK_PER_MSEC ((XPAR_PSU_CSU_WDT_WDT_CLK_FREQ_HZ) / (XPFW_WDT_PRESCALER * 1000))
#define XPFW_WDT_COUNTER_VAL ((XPFW_WDT_EXPIRE_TIME) * (XPFW_WDT_CLK_PER_MSEC))

const XPfw_Module_t *WdtModPtr;

/****************************************************************************/
/**
 * @brief  This scheduler task restarts CSU PMU WDT.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void XPfw_WdtRestart(void)
{
	if (WdtInstPtr != NULL) {
		XWdtPs_RestartWdt(WdtInstPtr);
	}
}

/****************************************************************************/
/**
 * @brief  This function initializes the CSU PMU Watchdog timer.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void InitCsuPmuWdt(void)
{
	s32 Status;
	XWdtPs_Config *WdtConfigPtr;
	u32 CounterValue;

	XPfw_Printf(DEBUG_DETAILED, "In InitCsuPmuWdt\r\n");

	/* Load Config for WDT */
	WdtConfigPtr = XWdtPs_LookupConfig(XPAR_PSU_CSU_WDT_DEVICE_ID);

	if (NULL == WdtConfigPtr) {
		Status = XST_FAILURE;
		XPfw_Printf(DEBUG_ERROR,"WDT (MOD-%d): WDT LookupConfig failed.\r\n",
				WdtModPtr->ModId);
		goto Done;
	}

	/* Initialize the WDT driver */
	Status = XWdtPs_CfgInitialize(WdtInstPtr, WdtConfigPtr,
			WdtConfigPtr->BaseAddress);

	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"WDT (MOD-%d): Initialization failed.\r\n",
				WdtModPtr->ModId);
		goto Done;
	}

	/* Setting the divider value */
	XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_CLK_PRESCALE,
			XWDTPS_CCR_PSCALE_0008);

	/* Watchdog counter reset value for Expire time of 100Sec,
	 * i.e., XPFW_WDT_EXPIRE_TIME
	 */
	CounterValue = XPFW_WDT_COUNTER_VAL >> XPFW_WDT_CRV_SHIFT;

	/* Set the Watchdog counter reset value */
	XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_COUNTER_RESET,
			CounterValue);

	/* Enable reset output */
	XWdtPs_EnableOutput(WdtInstPtr, XWDTPS_RESET_SIGNAL);

	/* Start the Watchdog timer */
	XWdtPs_Start(WdtInstPtr);

	XWdtPs_RestartWdt(WdtInstPtr);

	Status = XPfw_CoreScheduleTask(WdtModPtr, XPFW_WDT_RESTART_TIME, XPfw_WdtRestart);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"WDT (MOD-%d):Scheduling WDT restart failed.",
				WdtModPtr->ModId);
	}

	XPfw_Printf(DEBUG_DETAILED,"WDT (MOD-%d): Initialized.\r\n", WdtModPtr->ModId);
Done:
	return;
}

/****************************************************************************/
/**
 * @brief  This scheduler task checks for psu init completion and initializes
 * 			CSU PMU WDT.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void CheckPsuInitConfig(void)
{
	s32 Status;
	u32 FsblCompletionStatus = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	if ((FsblCompletionStatus & FSBL_COMPLETION) == FSBL_COMPLETION) {
		InitCsuPmuWdt();
		Status = XPfw_CoreRemoveTask(WdtModPtr, CHECK_FSBL_COMPLETION,
				CheckPsuInitConfig);
		if(XST_FAILURE == Status) {
			XPfw_Printf(DEBUG_ERROR,"WDT (MOD-%d):Removing WDT Cfg task failed.",
							WdtModPtr->ModId);
		}
	}
}

/****************************************************************************/
/**
 * @brief  This function schedules PSU init completion checking task.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void WdtCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	Status = XPfw_CoreScheduleTask(ModPtr, CHECK_FSBL_COMPLETION, CheckPsuInitConfig);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"WDT (MOD-%d):Scheduling WDT Cfg task failed.",
				ModPtr->ModId);
	}
}


void ModWdtInit(void)
{
	WdtModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(WdtModPtr, WdtCfgInit);
}
#else /* ENABLE_WDT */
void ModWdtInit(void) { }
#endif /* ENABLE_WDT */
