/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 */

#ifndef PM_GIC_H_
#define PM_GIC_H_

#include "pm_common.h"
#include "pm_slave.h"

/**
 * PmWakeEventGicProxy - GIC Proxy wake event, derived from PmWakeEvent
 * @wake	Basic PmWakeEvent structure
 * @mask	Interrupt mask associated with the slave's wake event in the
 *		GIC Proxy group
 * @group	Index of the group containing the interrupt in the GIC Proxy
 */
typedef struct PmWakeEventGicProxy {
	PmWakeEvent wake;
	const u32 mask;
	const u8 group;
} PmWakeEventGicProxy;

/**
 * GicProxyGroup - Properties of a GIC Proxy group
 * @setMask	When GIC Proxy is enabled, enable the interrupts whose masks
 *		are set in this variable
 */
typedef struct {
	u32 setMask;
} PmGicProxyGroup;

/**
 * PmGicProxy - Structure containing GIC Proxy properties
 * @groups	Pointer to the array of GIC Proxy groups
 * @groupsCnt	Number of elements in the array of GIC Proxy groups
 * @clear	Clear all set wake-up sources (flags for all groups)
 * @enable	Function that enables GIC Proxy and all interrupts that are set
 *		as wake sources
 * @flags	GIC Proxy flags (is enabled or not)
 */
typedef struct {
	PmGicProxyGroup* const groups;
	void (*const clear)(void);
	void (*const enable)(void);
	const u8 groupsCnt;
	u8 flags;
} PmGicProxy;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmGicProxy pmGicProxy;

extern PmWakeEventClass pmWakeEventClassGicProxy_g;

#endif
