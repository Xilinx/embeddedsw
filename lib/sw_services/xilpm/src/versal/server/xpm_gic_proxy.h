/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

void XPmGicProxy_WakeEventSet(const XPm_Periph *Periph, u8 Enable);

#ifdef __cplusplus
}
#endif
#endif /*XPM_GIC_H_*/
