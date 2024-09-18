/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PIN_H_
#define XPM_PIN_H_

#include "xpm_node.h"
#include "xpm_pinfunc.h"
#include "xpm_device.h"
#include "xpm_api.h"
#include "xpm_pin_plat.h"
#ifdef __cplusplus
extern "C" {
#endif

/* IOU_SLCR register related macros */
#define BITS_IN_REG			(32U)
#define PINS_PER_BANK			(26U)
#define BNK_OFFSET			(0x200U)
#define PINNUM(Id)			((NODEINDEX(Id) - (u32)XPM_NODEIDX_STMIC_LMIO_0) % PINS_PER_BANK)
#define ABS_PINNUM(Id, Bank)		(PINNUM(Id) + ((Bank) * PINS_PER_BANK))
#define SEL_SLEW			(0x00000120U)
#define EN_WK_PD			(0x00000110U)
#define EN_WK_PU			(0x00000114U)
#define EN_RX_SCHMITT_HYST		(0x0000010CU)
#define SEL_DRV0			(0x00000118U)
#define SEL_DRV1			(0x0000011CU)
#define SEL_DRV_WIDTH			(2U)
#define SEL_DRV0_MASK(PinIdx)		((u32)0x3U << (PINNUM(PinIdx)))
#define SEL_DRV1_MASK(PinIdx)		((u32)0x3U << (PINNUM(PinIdx) - (BITS_IN_REG / SEL_DRV_WIDTH)))
#define VMODE				(0x0000015CU)
#define VMODE_MASK			(0x1U)
#define TRI_STATE			(0x200U)

/* Pin states */
typedef enum {
	XPM_PINSTATE_UNUSED,
	XPM_PINSTATE_ASSIGNED,
} XPm_PinState;

typedef struct XPm_PinNode XPm_PinNode;
typedef struct PmPinGroup XPm_PinGroup;

struct PmPinGroup {
	u16 GroupCount;
	u16 *GroupList;
};

/************************** Function Prototypes ******************************/
XStatus XPmPin_Init(XPm_PinNode *Pin, u32 PinId, u32 BaseAddress);
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
#endif /* XPM_PIN_H_ */
