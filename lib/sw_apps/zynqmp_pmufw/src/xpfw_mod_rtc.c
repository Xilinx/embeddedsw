/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

	/* Enable SLVERR for RTC */
	XPfw_RMW32(RTC_CONTROL, RTC_CONTROL_SLVERR_ENABLE_MASK,
			RTC_CONTROL_SLVERR_ENABLE_MASK);

	/* Enable Seconds Alarm */
	Xil_Out32(RTC_RTC_INT_EN, 1U);
	Xil_Out32(RTC_RTC_INT_STATUS, 1U);
	XPfw_Printf(DEBUG_DETAILED,"RTC (MOD-%d): Initialized.\r\n",ModPtr->ModId);
}
void ModRtcInit(void)
{
	const XPfw_Module_t *RtcModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(RtcModPtr, RtcCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"RTC: Set Cfg handler failed\r\n");
	} else if (XST_SUCCESS !=
			XPfw_CoreSetEventHandler(RtcModPtr, RtcEventHandler)) {
		XPfw_Printf(DEBUG_DETAILED,"RTC: Set Event handler failed\r\n");
	}
}
#else /* ENABLE_RTC_TEST */
void ModRtcInit(void) { }
#endif /* ENABLE_RTC_TEST */
