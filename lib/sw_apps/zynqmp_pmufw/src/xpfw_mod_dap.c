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
		XpbrServHndlrTbl[XPBR_SERV_EXT_DAPRPUWAKE]();
		XPfw_Printf(DEBUG_DETAILED,"XPFW: DAP RPU WAKE.. Done\r\n");
#ifdef ENABLE_PM
		XPfw_DapRpuWakeEvent();
#endif
	}
	if (XPFW_EV_DAP_FPD_WAKE == EventId) {
		/* Call ROM Handler for FPD Wake */
		XpbrServHndlrTbl[XPBR_SERV_EXT_DAPFPDWAKE]();
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

	(void) XPfw_CoreSetCfgHandler(DapModPtr, DapCfgInit);
	(void) XPfw_CoreSetEventHandler(DapModPtr, DapEventHandler);
}
