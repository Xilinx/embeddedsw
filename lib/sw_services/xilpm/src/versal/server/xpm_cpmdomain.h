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

#ifndef XPM_CPMDOMAIN_H_
#define XPM_CPMDOMAIN_H_

#include "xpm_powerdomain.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CPM domain node class.
 */
typedef struct XPm_CpmDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 CpmSlcrBaseAddr; /**< CPM SLCR Base Address */
	u32 CpmSlcrSecureBaseAddr; /**< CPM SLCR Secure base address */
	u32 CpmPcsrBaseAddr; /**< CPM PCSR Base address */
	u32 CpmCrCpmBaseAddr; /**< CPM CRCPM Base address */
} XPm_CpmDomain;

/************************** Function Prototypes ******************************/
XStatus XPmCpmDomain_Init(XPm_CpmDomain *CpmDomain, u32 Id, u32 BaseAddress,
			  XPm_Power *Parent,  u32 *OtherBaseAddresses,
			  u32 OtherBaseAddressCnt);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_CPMDOMAIN_H_ */
