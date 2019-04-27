/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef XPM_DOMAIN_ISO_H
#define XPM_DOMAIN_ISO_H

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum XPmDomainIso {
	XPM_DOMAIN_ISO_FPD_PL_TEST,
	XPM_DOMAIN_ISO_FPD_PL,
	XPM_DOMAIN_ISO_FPD_SOC,
	XPM_DOMAIN_ISO_LPD_CPM_DFX,
	XPM_DOMAIN_ISO_LPD_CPM,
	XPM_DOMAIN_ISO_LPD_PL_TEST,
	XPM_DOMAIN_ISO_LPD_PL,
	XPM_DOMAIN_ISO_LPD_SOC,
	XPM_DOMAIN_ISO_PMC_LPD_DFX,
	XPM_DOMAIN_ISO_PMC_LPD,
	XPM_DOMAIN_ISO_PMC_PL_CFRAME,
	XPM_DOMAIN_ISO_PMC_PL_TEST,
	XPM_DOMAIN_ISO_PMC_PL,
	XPM_DOMAIN_ISO_PMC_SOC_NPI,
	XPM_DOMAIN_ISO_PMC_SOC,
	XPM_DOMAIN_ISO_PL_SOC,
	XPM_DOMAIN_ISO_VCCAUX_SOC,
	XPM_DOMAIN_ISO_VCCRAM_SOC,
	XPM_DOMAIN_ISO_VCCAUX_VCCRAM,
	XPM_DOMAIN_ISO_MAX
};

XStatus XPmDomainIso_Control(enum XPmDomainIso IsoId, u32 Enable);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DOMAIN_ISO_H */
