/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_module.h"

#include "xpfw_mod_wdt.h"

#ifdef ENABLE_WDT
#include "xwdtps.h"

/* Check if PMU has access to CSU WDT (psu_csu_wdt) */
#ifdef XPMU_PMUWDT
	#include "xwdtps.h"
#else /* XPMU_PMUWDT */
	#error "ENABLE_WDT is defined but psu_csu_wdt is not defined in the design"
#endif

/* Instance of WDT Driver */
static XWdtPs WdtInst;
static XWdtPs *WdtInstPtr = &WdtInst;

/*
 * WDT expire time in milliseconds.
 */
#define XPFW_WDT_EXPIRE_TIME XPFW_CFG_PMU_DEFAULT_WDT_TIMEOUT

/*
 * WDT restart time in milliseconds.
 */
#define XPFW_WDT_RESTART_TIME 50U
#define XPFW_WDT_CRV_SHIFT 12U
#define XPFW_WDT_PRESCALER 8U

#define XPFW_WDT_CLK_PER_MSEC ((XPMU_PMUWDT_WDT_CLK) / (XPFW_WDT_PRESCALER * 1000U))
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
 * @brief  This scheduler sets the watchdog timer.
 *
 * @param  TimeOutVal - Watchdog timeout in ms.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
void XPfw_WdtSetVal(u32 TimeOutVal)
{
	u32 CounterValue;

	if (TimeOutVal > 0U) {
		/* Stop the Watchdog timer */
		XWdtPs_Stop(WdtInstPtr);

		/* Watchdog counter reset value for Expire time of 100Sec,
		 * i.e., XPFW_WDT_EXPIRE_TIME
		 */
		CounterValue = ((TimeOutVal) * (XPFW_WDT_CLK_PER_MSEC)) >> XPFW_WDT_CRV_SHIFT;

		/* Set the Watchdog counter reset value */
		XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_COUNTER_RESET,
				CounterValue);

		/* Enable reset output */
		XWdtPs_EnableOutput(WdtInstPtr, XWDTPS_RESET_SIGNAL);

		/* Start the Watchdog timer */
		XWdtPs_Start(WdtInstPtr);

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
void InitCsuPmuWdt(void)
{
	s32 Status;
	XWdtPs_Config *WdtConfigPtr;
	u32 CounterValue;

	XPfw_Printf(DEBUG_DETAILED, "In InitCsuPmuWdt\r\n");

	/* Load Config for WDT */
	WdtConfigPtr = XWdtPs_LookupConfig(XPMU_PMUWDT);

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
 * @brief  WDT module init
 *
 * @param  ModPtr Module pointer
 *         CfgData Module config data
 *         Len Length of config data
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void WdtCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
}

void ModWdtInit(void)
{
	WdtModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(WdtModPtr, WdtCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"WDT: Set Cfg handler failed\r\n");
	}
}
#else /* ENABLE_WDT */
void ModWdtInit(void) { }
#endif /* ENABLE_WDT */
