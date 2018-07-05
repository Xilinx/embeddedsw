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

#include "xpfw_mod_sched.h"

#ifdef ENABLE_SCHEDULER
static void SchCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
}

void ModSchInit(void)
{
	const XPfw_Module_t *SchModPtr = XPfw_CoreCreateMod();

	if (XPfw_CoreSetCfgHandler(SchModPtr, SchCfgInit) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: ModSchInit: Failed to set CfgHandler \r\n")
	}

}
#else /* ENABLE_SCHEDULER */
void ModSchInit(void) { }
#endif /* ENABLE_SCHEDULER */
