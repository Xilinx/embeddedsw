/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_PIN_H_
#define XPM_RUNTIME_PIN_H_

#include "xpm_node.h"
#include "xpm_pinfunc.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_pin.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct PmPinGroup XPm_PinGroup;
typedef struct XPmRuntime_PinOps XPmRuntime_PinOps;

struct PmPinGroup {
	u16 GroupCount;
	u16 *GroupList;
};
/* Size of bit fields for XPm_PinNode structure */
#define PIN_NODE_BANK_BIT_FIELD_SIZE 		2
#define PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE 	1
#define PIN_NODE_PULLCTRL_BIT_FIELD_SIZE 	1
#define PIN_NODE_TRISTATE_BIT_FIELD_SIZE 	1
/**
 * The Pin class.
 */
struct XPmRuntime_PinOps {
	u8 NumGroups; /**< Number of function groups allocated to this pin */
	u16 SubsysIdx;  /**< Subsystem Idx of the owner who is using this pin */
	u8 Bank:PIN_NODE_BANK_BIT_FIELD_SIZE; /**< Specifies the bank number */
	u8 BiasStatus:PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE; /**< BiasStatus: 0 – Disable; 1 – Enable */
	u8 PullCtrl:PIN_NODE_PULLCTRL_BIT_FIELD_SIZE; /**< PullCtrl: 0 – Pull Down; 1 – Pull Up */
	u8 TriState:PIN_NODE_TRISTATE_BIT_FIELD_SIZE; /**< TriState: 0 – Disable; 1 – Enable */
	u8 FuncId; /**< Function unique ID of the pin */
	u16 *Groups; /**< Array of group identifier supported by this pin */
};
/************************** Function Prototypes ******************************/
XPm_PinNode *XPmPin_GetById(u32 PinId);

XStatus XPmPin_SetPinFunction(u32 PinId, u32 FuncId);
XStatus XPmPin_GetPinFunction(u32 PinId, u32 *FuncId);
XStatus XPmPin_SetPinConfig(u32 PinId, u32 Param, u32 ParamValue);
XStatus XPmPin_GetPinConfig(u32 PinId, u32 Param, u32 *Value);
XStatus XPmPin_GetNumPins(u32 *NumPins);
XStatus XPmPin_GetPinGroups(u32 PinId, u32 Index, u16 *Groups);
XStatus XPmPin_CheckPerms(const u32 SubsystemId, const u32 PinId);
XStatus XPmPin_Release(const u32 SubsystemId, const u32 PinId);
XStatus XPmPin_Request(const u32 SubsystemId, const u32 PinId);
XStatus XPmPin_QueryAttributes(const u32 PinIndex, u32 *Resp);
XPm_PinNode *XPmPin_GetByIndex(const u32 PinIndex);
XPm_PinGroup *XPmPin_GetGroupByIdx(const u32 PinIndex);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RUNTIME_PIN_H_ */
