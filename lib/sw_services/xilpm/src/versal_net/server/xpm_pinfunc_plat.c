/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pinfunc.h"

/* TODO: Each function can not be mapped with their corresponding
 *       device. Keeping those devIdx as 0.
 */
static XPm_PinFunc PmPinFuncs[MAX_FUNCTION] = {
	[PIN_FUNC_OSPI0] = {
		.Id = (u8)PIN_FUNC_OSPI0,
		.Name = "ospi0",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 10,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[PIN_FUNC_OSPI0_SS] = {
		.Id = (u8)PIN_FUNC_OSPI0_SS,
		.Name = "ospi0_ss",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 2,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0_SS,
		}),
	},
	[PIN_FUNC_OSPI0_RST_N] = {
		.Id = (u8)PIN_FUNC_OSPI0,
		.Name = "ospi0_rst_n",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 1,
		.NumGroups = 1,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0_RST_N,
		}),
	},
	[PIN_FUNC_OSPI0_ECC_FAIL] = {
		.Id = (u8)PIN_FUNC_OSPI0,
		.Name = "ospi0_ecc_fail",
		.DevIdx = (u16)XPM_NODEIDX_DEV_OSPI,
		.LmioRegMask = 0x3FF,
		.PmioRegMask = 0x4,
		.NumPins = 1,
		.NumGroups = 2,
		.Groups = ((u16 []) {
			PIN_GRP_OSPI0_0_ECC_FAIL_0,
			PIN_GRP_OSPI0_0_ECC_FAIL_1,
		}),
	},
};

/****************************************************************************/
/**
 * @brief  This function returns handle to requested XPm_PinFunc struct
 *
 * @param FuncId	Function ID.
 *
 * @return Pointer to XPm_PinFunc if successful, NULL otherwise
 *
 ****************************************************************************/
XPm_PinFunc *XPmPinFunc_GetById(u32 FuncId)
{
	XPm_PinFunc *PinFunc = NULL;

	if ((u32)MAX_FUNCTION > FuncId) {
		PinFunc = &PmPinFuncs[FuncId];
	}

	return PinFunc;
}
