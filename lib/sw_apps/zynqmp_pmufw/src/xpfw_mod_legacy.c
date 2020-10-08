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
#include "xpfw_mod_legacy.h"

/* CfgInit Handler */
static void LegacyCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData,
		u32 Len)
{
	/* Used for Power Up/Dn request handling */
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_REQ_PWRUP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,"LegacyCfgInit: Failed to register "
				"event ID: %d\r\n",XPFW_EV_REQ_PWRUP)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_REQ_PWRDN) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,"LegacyCfgInit: Failed to register "
				"event ID: %d\r\n",XPFW_EV_REQ_PWRDN)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_REQ_ISOLATION) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,"LegacyCfgInit: Failed to register "
				"event ID: %d\r\n",XPFW_EV_REQ_ISOLATION)
	}

	XPfw_Printf(DEBUG_DETAILED,"LEGACY PWR UP/DN/ISO (MOD-%d): "
			"Initialized.\r\n", ModPtr->ModId);
}

/* Event Handler */
static void LegacyEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	if (XPFW_EV_REQ_PWRUP == EventId) {
		/* Call ROM Handler for PwrUp */
		XPfw_Printf(DEBUG_DETAILED,"XPFW: Calling ROM PWRUP Handler..");
		(void)XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUP_REQS]();
		XPfw_Printf(DEBUG_DETAILED,"Done\r\n");
	}

	if (XPFW_EV_REQ_PWRDN == EventId) {
		/* Call ROM Handler for PwrDn */
		XPfw_Printf(DEBUG_DETAILED,"XPFW: Calling ROM PWRDN Handler..");
		(void)XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDN_REQS]();
		XPfw_Printf(DEBUG_DETAILED,"Done\r\n");
	}

	if (XPFW_EV_REQ_ISOLATION == EventId) {
		/* Call ROM Handler for Isolation */
		XPfw_Printf(DEBUG_DETAILED,"XPFW: Calling ROM Isolation Handler..");
		(void)XpbrServHndlrTbl[XPBR_SERV_EXT_ISO_REQS]();
		XPfw_Printf(DEBUG_DETAILED,"Done\r\n");
	}

}

/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModLegacyInit(void)
{
	const XPfw_Module_t *LegacyModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(LegacyModPtr, LegacyCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"Legacy: Set Cfg handler failed\r\n");
	} else if (XST_SUCCESS !=
			XPfw_CoreSetEventHandler(LegacyModPtr, LegacyEventHandler)) {
		XPfw_Printf(DEBUG_DETAILED,"Legacy: Set Event handler failed\r\n");
	} else {
		/* For MISRA-C compliance */
	}
}
