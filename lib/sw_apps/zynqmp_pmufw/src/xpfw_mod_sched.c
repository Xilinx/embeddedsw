/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
