/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
	u32 SetMask; /**< Interrupt mask enabled when GIC Proxy is enabled. */
} XPm_GicProxyGroup;

/**
 * XPm_GicProxy - Structure containing GIC Proxy properties
 * @Groups	Pointer to the array of GIC Proxy Groups
 * @GroupsCnt	Number of elements in the array of GIC Proxy Groups
 * @Clear	Clear all set wake-up sources (Flags for all Groups)
 * @Enable	Function that Enables GIC Proxy and all interrupts that are set
 *		as wake sources. Returns XST_SUCCESS or an error code.
 * @Flags	GIC Proxy Flags (is Enabled or not)
 */
typedef struct {
	XPm_GicProxyGroup* const Groups; /**< Pointer to GIC Proxy group data. */
	void (*const Clear)(void); /**< Clear all cached wake-up sources. */
	XStatus (*const Enable)(void); /**< Enable GIC Proxy wake interrupts. */
	const u8 GroupsCnt; /**< Number of GIC Proxy groups. */
	u8 Flags; /**< GIC Proxy state flags. */
} XPm_GicProxy_t;

/*********************************************************************
 * Global data declarations
 ********************************************************************/

extern XPm_GicProxy_t XPm_GicProxy;

XStatus XPmGicProxy_WakeEventSet(const XPm_Periph *Periph, u8 Enable);

#ifdef __cplusplus
}
#endif
#endif /*XPM_GIC_H_*/
