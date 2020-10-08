/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_rom_interface.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"
#include "pm_binding.h"
#include "xpfw_mod_dap.h"

/* CfgInit Handler */
static void DapCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	/* Used for DAP Wakes */
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_DAP_RPU_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: DapCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_DAP_RPU_WAKE)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_DAP_FPD_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: DapCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_DAP_FPD_WAKE)
	}

	XPfw_Printf(DEBUG_DETAILED,"DAP_WAKE (MOD-%d): Initialized.\r\n",
			ModPtr->ModId);
}

/* Event Handler */
static void DapEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	if (XPFW_EV_DAP_RPU_WAKE == EventId) {
		/* Call ROM Handler for RPU Wake */
		(void)XpbrServHndlrTbl[XPBR_SERV_EXT_DAPRPUWAKE]();
		XPfw_Printf(DEBUG_DETAILED,"XPFW: DAP RPU WAKE.. Done\r\n");
#ifdef ENABLE_PM
		XPfw_DapRpuWakeEvent();
#endif
	}
	if (XPFW_EV_DAP_FPD_WAKE == EventId) {
		/* Call ROM Handler for FPD Wake */
		(void)XpbrServHndlrTbl[XPBR_SERV_EXT_DAPFPDWAKE]();
		XPfw_Printf(DEBUG_DETAILED,"XPFW: DAP FPD WAKE.. Done\r\n");
#ifdef ENABLE_PM
		XPfw_DapFpdWakeEvent();
#endif
	}
}

/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModDapInit(void)
{
	const XPfw_Module_t *DapModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(DapModPtr, DapCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"DAP: Set Cfg handler failed\r\n");
	} else if (XST_SUCCESS !=
			XPfw_CoreSetEventHandler(DapModPtr, DapEventHandler)) {
		XPfw_Printf(DEBUG_DETAILED,"DAP: Set Event handler failed\r\n");
	} else {
		/* For MISRA-C compliance */
	}
}
