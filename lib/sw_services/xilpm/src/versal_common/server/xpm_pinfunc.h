/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PINFUNC_H_
#define XPM_PINFUNC_H_

#include "xpm_device.h"
#include "xpm_pinfunc_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FUNC_NAME_SIZE		(16U)		/* Function name string size */
#define MAX_GROUPS_PER_RES	(6U)		/* Max No. of groups per query response */
#define INVALID_FUNC_ID		(0xFFFFU)
#define END_OF_GRP		(0xFFU)		/* -1 */
#define RESERVED_GRP		(0xFFFEU)	/* -2 */

typedef struct XPm_PinFunc XPm_PinFunc;

/**
 * The Pin Function class.
 */
struct XPm_PinFunc {
	char Name[FUNC_NAME_SIZE]; /**< Function name */
	u8 Id; /**< Function ID */
	u8 NumPins; /**< Number of pins needed for this function */
	u8 NumGroups; /**< Number of groups for this function */
	u16 DevIdx; /**< Device index for this function */
	u16 LmioRegMask; /**< Register mask value to select this function */
	u16 PmioRegMask; /**< Register mask value to select this function */
	u16 *Groups; /**< Array of group identifier */
};

/************************** Function Prototypes ******************************/
XPm_PinFunc *XPmPinFunc_GetById(u32 FuncId);
XStatus XPmPinFunc_GetNumFuncs(u32 *NumFuncs);
XStatus XPmPinFunc_GetFuncName(u32 FuncId, char *FuncName);
XStatus XPmPinFunc_GetNumFuncGroups(u32 FuncId, u32 *NumGroups);
XStatus XPmPinFunc_GetFuncGroups(u32 FuncId, u32 Index, u16 *Groups);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PINFUNC_H_ */
