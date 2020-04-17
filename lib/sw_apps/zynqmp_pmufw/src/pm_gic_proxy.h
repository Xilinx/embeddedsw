/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_GIC_H_
#define PM_GIC_H_

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* PM_GIC_H_ */
