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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_GIC_H_
#define XPM_GIC_H_

#include "xpm_periph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GicProxyGroup - Properties of a GIC Proxy group
 * @SetMask	When GIC Proxy is Enabled, Enable the interrupts whose masks
 *		are set in this variable
 */
typedef struct {
	u32 SetMask;
} XPm_GicProxyGroup;

/**
 * XPm_GicProxy - Structure containing GIC Proxy properties
 * @Groups	Pointer to the array of GIC Proxy Groups
 * @GroupsCnt	Number of elements in the array of GIC Proxy Groups
 * @Clear	Clear all set wake-up sources (Flags for all Groups)
 * @Enable	Function that Enables GIC Proxy and all interrupts that are set
 *		as wake sources
 * @Flags	GIC Proxy Flags (is Enabled or not)
 */
typedef struct {
	XPm_GicProxyGroup* const Groups;
	void (*const Clear)(void);
	void (*const Enable)(void);
	const u8 GroupsCnt;
	u8 Flags;
} XPm_GicProxy_t;

/*********************************************************************
 * Global data declarations
 ********************************************************************/

extern XPm_GicProxy_t XPm_GicProxy;

XStatus XPmGicProxy_WakeEventSet(XPm_Periph *Periph, u8 Enable);

#ifdef __cplusplus
}
#endif
#endif /*XPM_GIC_H_*/
