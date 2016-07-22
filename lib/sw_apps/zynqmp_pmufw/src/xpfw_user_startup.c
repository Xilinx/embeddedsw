/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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

#include "xpfw_config.h"

#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"

#include "xpfw_user_startup.h"

#include "pm_binding.h"
#include "pm_api.h"
#include "ipi_buffer.h"
#include "pm_defs.h"

#include "xpfw_mod_dap.h"
#include "xpfw_mod_legacy.h"
#include "xpfw_mod_em.h"
#include "xpfw_mod_pm.h"

#ifdef ENABLE_RTC_TEST
static void RtcEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	fw_printf("MOD%d:EVENTID: %d\r\n", ModPtr->ModId, EventId);
	//XPfw_CorePrintStats();
	if (XPFW_EV_RTC_SECONDS == EventId) {
			/* Ack the Int in RTC Module */
			Xil_Out32(RTC_RTC_INT_STATUS, 1U);
			fw_printf("RTC: %d \r\n", Xil_In32(RTC_CURRENT_TIME));
	}
}

static void RtcCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_RTC_SECONDS);
	/* Enable Seconds Alarm */
	Xil_Out32(RTC_RTC_INT_EN, 1U);
	Xil_Out32(RTC_RTC_INT_STATUS, 1U);
	fw_printf("RTC (MOD-%d): Initialized.\r\n", ModPtr->ModId);
}
static void ModRtcInit(void)
{
	const XPfw_Module_t *RtcModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(RtcModPtr, RtcCfgInit);
	(void)XPfw_CoreSetEventHandler(RtcModPtr, RtcEventHandler);
}
#else /* ENABLE_RTC_TEST */
static void ModRtcInit(void) { }
#endif /* ENABLE_RTC_TEST */

#ifdef ENABLE_SCHEDULER
static void PrintMsg1(void)
{
	fw_printf("Task#1\r\n");
}
static void PrintMsg2(void)
{
	fw_printf("Task#2\r\n");
}

static void SchCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	/* Task every 5 seconds - For our convenience in manual testing */
	fw_printf("Adding Task1 - Status: %ld\n", XPfw_CoreScheduleTask(ModPtr, 5000U, PrintMsg1));
	/* Every 10 seconds */
	fw_printf("Adding Task2 - Status:%ld\n", XPfw_CoreScheduleTask(ModPtr, 10000U, PrintMsg2));
}

static XStatus ModSchInit(void)
{
	const XPfw_Module_t *SchModPtr = XPfw_CoreCreateMod();

	return XPfw_CoreSetCfgHandler(SchModPtr, SchCfgInit);
}
#else /* ENABLE_SCHEDULER */
static void ModSchInit(void) { }
#endif /* ENABLE_SCHEDULER */

void XPfw_UserStartUp(void)
{
	ModRtcInit();
	ModEmInit();
	ModPmInit();
	(void)ModSchInit();
	ModDapInit();
	ModLegacyInit();
}
