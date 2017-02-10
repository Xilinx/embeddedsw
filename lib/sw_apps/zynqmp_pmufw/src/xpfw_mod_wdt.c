/******************************************************************************
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_module.h"

#include "xpfw_mod_wdt.h"

#ifdef ENABLE_WDT
#include "xwdtps.h"

/* Instance of WDT Driver */
static XWdtPs WdtInst;
static XWdtPs *WdtInstPtr = &WdtInst;

#define XPFW_WDT_EXPIRE_TIME 100U
/*
 * Restart time is multiplied with 1000 to get time in milliseconds,
 * which is needed for scheduler during scheduling a task.
 */
#define XPFW_WDT_RESTART_TIME (((XPFW_WDT_EXPIRE_TIME)-10U)*1000U)
#define XPFW_WDT_CRV_SHIFT 12U
#define XPFW_WDT_PRESCALER 4096U

#define XPFW_WDT_CLK_PER_SEC ((XPAR_XWDTPS_0_WDT_CLK_FREQ_HZ) / (XPFW_WDT_PRESCALER))
#define XPFW_WDT_COUNTER_VAL ((XPFW_WDT_EXPIRE_TIME) * (XPFW_WDT_CLK_PER_SEC))

static void XPfw_WdtRestart(void)
{
	XWdtPs_RestartWdt(WdtInstPtr);
}

/****************************************************************************/
/**
 * @brief  This function initializes the LPD IOU Watchdog timer.
 *
 * @param  None.
 *
 * @return Returns the status as XST_SUCCESS or XST_FAILURE.
 *
 * @note   None.
 *
 ****************************************************************************/
static void WdtCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;
	XWdtPs_Config *WdtConfigPtr;
	u32 CounterValue;

	/* Load Config for WDT */
	WdtConfigPtr = XWdtPs_LookupConfig(XPAR_XWDTPS_0_DEVICE_ID);

	if (NULL == WdtConfigPtr) {
		Status = XST_FAILURE;
		fw_printf("WDT (MOD-%d): WDT LookupConfig failed.\r\n", ModPtr->ModId);
		goto Done;
	}

	/* Initialize the WDT driver */
	Status = XWdtPs_CfgInitialize(WdtInstPtr, WdtConfigPtr,
			WdtConfigPtr->BaseAddress);

	if (XST_FAILURE == Status) {
		fw_printf("WDT (MOD-%d): Initialization failed.\r\n", ModPtr->ModId);
		goto Done;
	}

	/* Setting the divider value */
	XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_CLK_PRESCALE,
			XWDTPS_CCR_PSCALE_4096);

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

	Status = XPfw_CoreScheduleTask(ModPtr, XPFW_WDT_RESTART_TIME, XPfw_WdtRestart);
	if (XST_FAILURE == Status) {
		fw_printf("WDT (MOD-%d):Scheduling WDT restart failed.",ModPtr->ModId);
	}

	fw_printf("WDT (MOD-%d): Initialized.\r\n", ModPtr->ModId);
Done:
	return;
}

void ModWdtInit(void)
{
	const XPfw_Module_t *WdtModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(WdtModPtr, WdtCfgInit);
}
#else /* ENABLE_WDT */
void ModWdtInit(void) { }
#endif /* ENABLE_WDT */
