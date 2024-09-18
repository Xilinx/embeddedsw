/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PIN_PLAT_H_
#define XPM_PIN_PLAT_H_

#include "xpm_pinfunc.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Size of bit fields for XPm_PinNode structure */
#define PIN_NODE_BANK_BIT_FIELD_SIZE		2
#define PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE	1
#define PIN_NODE_PULLCTRL_BIT_FIELD_SIZE	1
#define PIN_NODE_TRISTATE_BIT_FIELD_SIZE	1
/**
 * The Pin class.
 */
struct XPm_PinNode {
	XPm_Node Node; /**< Node: Base class */
	u8 FuncId; /**< Function unique ID of the pin */
	u16 *Groups; /**< Array of group identifier supported by this pin */
	u16 SubsysIdx;	/**< Subsystem Idx of the owner who is using this pin */
	u8 NumGroups; /**< Number of function groups allocated to this pin */
	u8 Bank:PIN_NODE_BANK_BIT_FIELD_SIZE; /**< Specifies the bank number */
	u8 BiasStatus:PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE; /**< BiasStatus: 0 – Disable; 1 – Enable */
	u8 PullCtrl:PIN_NODE_PULLCTRL_BIT_FIELD_SIZE; /**< PullCtrl: 0 – Pull Down; 1 – Pull Up */
	u8 TriState:PIN_NODE_TRISTATE_BIT_FIELD_SIZE; /**< TriState: 0 – Disable; 1 – Enable */
};
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PIN_PLAT_H_ */
