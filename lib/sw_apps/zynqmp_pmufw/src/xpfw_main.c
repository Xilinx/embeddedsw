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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xil_types.h"

#include "xpfw_version.h"
#include "xpfw_default.h"

#include "xpfw_core.h"
#include "xpfw_user_startup.h"
#include "xpfw_platform.h"
#include "pm_system.h"
#ifdef ENABLE_DDR_SR_WR
#include "pm_hooks.h"
#endif

XStatus XPfw_Main(void)
{
	XStatus Status;
	u32 xpbr_version;

	/* Start the Init Routine */
	XPfw_Printf(DEBUG_PRINT_ALWAYS,"PMU Firmware %s\t%s   %s\r\n",
			ZYNQMP_XPFW_VERSION, __DATE__, __TIME__);
	/* Print ROM version */
	xpbr_version = XPfw_Read32(PBR_VERSION_REG);
	XPfw_PrintPBRVersion(xpbr_version);

	/* Initialize the FW Core Object */
	Status = XPfw_CoreInit(0U);

	if (Status != XST_SUCCESS) {
		XPfw_Printf(DEBUG_ERROR,"%s: Error! Core Init failed\r\n", __func__);
		goto Done;
	}

	/* Call the User Start Up Code to add Mods, Handlers and Tasks */
	XPfw_UserStartUp();

	/* Configure the Modules. Calls CfgInit Handlers of all modules */
	Status = XPfw_CoreConfigure();

	if (Status != XST_SUCCESS) {
		XPfw_Printf(DEBUG_ERROR,"%s: Error! Core Cfg failed\r\n", __func__);
		goto Done;
	}

#ifdef ENABLE_DDR_SR_WR
	if (PM_SUSPEND_TYPE_POWER_OFF != PmSystemSuspendType())
		PmHookSystemStart();
#endif

	/* Restore system state in case of resume from Power Off Suspend */
#ifdef ENABLE_POS
	if (PM_SUSPEND_TYPE_POWER_OFF == PmSystemSuspendType()) {
		Status = PmSystemResumePowerOffSuspend();
		if (Status != XST_SUCCESS) {
			XPfw_Printf(DEBUG_ERROR,"%s: Error! Power Off Suspend resume failed\r\n", __func__);
			goto Done;
		}
	}
#endif

	/* Wait to Service the Requests */
	Status = XPfw_CoreLoop();

	if (Status != XST_SUCCESS) {
		XPfw_Printf(DEBUG_ERROR,"%s: Error! Unexpected exit from CoreLoop\r\n",
				__func__);
		goto Done;
	}
	Done:
	/* Control never comes here */
	return Status;
}
