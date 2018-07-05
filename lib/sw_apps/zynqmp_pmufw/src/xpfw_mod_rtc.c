/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"

#include "xpfw_mod_rtc.h"

#ifdef ENABLE_RTC_TEST
static void RtcEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	XPfw_Printf(DEBUG_DETAILED,"MOD%d:EVENTID: %lu\r\n",
			ModPtr->ModId, EventId);

	if (XPFW_EV_RTC_SECONDS == EventId) {
			/* Ack the Int in RTC Module */
			Xil_Out32(RTC_RTC_INT_STATUS, 1U);
			XPfw_Printf(DEBUG_DETAILED,"RTC: %lu \r\n",
					Xil_In32(RTC_CURRENT_TIME));
	}
}

static void RtcCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_RTC_SECONDS) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: RtcCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_RTC_SECONDS)
	}

	/* Enable Seconds Alarm */
	Xil_Out32(RTC_RTC_INT_EN, 1U);
	Xil_Out32(RTC_RTC_INT_STATUS, 1U);
	XPfw_Printf(DEBUG_DETAILED,"RTC (MOD-%d): Initialized.\r\n",ModPtr->ModId);
}
void ModRtcInit(void)
{
	const XPfw_Module_t *RtcModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(RtcModPtr, RtcCfgInit);
	(void)XPfw_CoreSetEventHandler(RtcModPtr, RtcEventHandler);
}
#else /* ENABLE_RTC_TEST */
void ModRtcInit(void) { }
#endif /* ENABLE_RTC_TEST */
