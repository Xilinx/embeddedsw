/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
		XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUP_REQS]();
		XPfw_Printf(DEBUG_DETAILED,"Done\r\n");
	}

	if (XPFW_EV_REQ_PWRDN == EventId) {
		/* Call ROM Handler for PwrDn */
		XPfw_Printf(DEBUG_DETAILED,"XPFW: Calling ROM PWRDN Handler..");
		XpbrServHndlrTbl[XPBR_SERV_EXT_PWRDN_REQS]();
		XPfw_Printf(DEBUG_DETAILED,"Done\r\n");
	}

	if (XPFW_EV_REQ_ISOLATION == EventId) {
		/* Call ROM Handler for Isolation */
		XPfw_Printf(DEBUG_DETAILED,"XPFW: Calling ROM Isolation Handler..");
		XpbrServHndlrTbl[XPBR_SERV_EXT_ISO_REQS]();
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

	(void) XPfw_CoreSetCfgHandler(LegacyModPtr, LegacyCfgInit);
	(void) XPfw_CoreSetEventHandler(LegacyModPtr, LegacyEventHandler);
}
