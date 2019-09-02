/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef XPM_PLDOMAIN_H_
#define XPM_PLDOMAIN_H_

#include "xpm_powerdomain.h"
#include "xcframe.h"
#include "xcfupmc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PlDomain XPm_PlDomain;

/**
 * The PL power domain node class.
 */
typedef struct XPm_PlDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 CfuApbBaseAddr; /**< CFU APB base address */
	u32 Cframe0RegBaseAddr; /**< CFRAME0 Register base address */
} XPm_PlDomain;

/* TRIM Types */
#define XPM_PL_TRIM_VGG          (0x1U)
#define XPM_PL_TRIM_CRAM         (0x2U)
#define XPM_PL_TRIM_BRAM         (0x3U)
#define XPM_PL_TRIM_URAM         (0x4U)

/************************** Function Prototypes ******************************/
XStatus XPmPlDomain_Init(XPm_PlDomain *Pld, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent,  u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_PLDOMAIN_H_ */
