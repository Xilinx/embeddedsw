/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_config.h"

#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"

#include "xpfw_user_startup.h"

#include "xpfw_mod_dap.h"
#include "xpfw_mod_legacy.h"
#include "xpfw_mod_em.h"
#include "xpfw_mod_pm.h"
#include "xpfw_mod_rtc.h"
#include "xpfw_mod_sched.h"
#include "xpfw_mod_stl.h"
#include "xpfw_mod_wdt.h"
#include "xpfw_mod_common.h"

#include "xpfw_mod_ultra96.h"
#include "xpfw_mod_rpu.h"
#include "xpfw_mod_extwdt.h"
#include "xpfw_mod_overtemp.h"

#if defined (XPAR_LPD_IS_CACHE_COHERENT) || defined (XPAR_FPD_IS_CACHE_COHERENT) || defined (XPAR_PL_IS_CACHE_COHERENT)
/*****************************************************************************
*
* Enable the broadcasting of Inner Shareable transactions for APU.
*
* @param	None.
*
* @return	None.
*
******************************************************************************/
static void XPfw_Enable_Inner_Shareable_Broadcast(void)
{
	u32 val = XPfw_Read32(LPD_SLCR_LPD_APU);

	val |= (1U << LPD_SLCR_LPD_APU_BRDC_INNER_SHIFT);
	XPfw_Write32(LPD_SLCR_LPD_APU , val);
}
#endif

void XPfw_UserStartUp(void)
{
#if defined (XPAR_LPD_IS_CACHE_COHERENT) || defined (XPAR_FPD_IS_CACHE_COHERENT) || defined (XPAR_PL_IS_CACHE_COHERENT)
    /*
	 * LPD/FPD peripheral is configured to use CCI,
     * enable the broadcasting of inner shareable transactions
	 */
        XPfw_Enable_Inner_Shareable_Broadcast();
#endif
	ModStlInit();
	ModRtcInit();
	ModEmInit();
	ModPmInit();
	(void)ModSchInit();
	ModDapInit();
	ModLegacyInit();
	ModWdtInit();
	ModRpuInit();
#ifdef ENABLE_CUSTOM_MOD
	/*
	 * This ModCustomInit function is a placeholder to the user
	 * for creating his own module. Define the symbol ENABLE_CUSTOM_MOD,
	 * add the custom source code and recompile the firmware.
	 * Refer "Creating a custom module" section in
	 * Chapter:10 platform management unit firmware of UG1137.
	 */
	ModCustomInit();
#endif

	ModUltra96Init();
	ModCommonInit();
	ModExtWdtInit();
#ifndef ENABLE_RUNTIME_OVERTEMP
	ModOverTempInit();
#endif
}
