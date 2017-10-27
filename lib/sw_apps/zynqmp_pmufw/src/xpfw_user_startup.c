/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
	u32 val = 0U;

	val = XPfw_Read32(LPD_SLCR_LPD_APU);
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
}
