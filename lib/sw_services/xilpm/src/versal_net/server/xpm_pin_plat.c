/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pin.h"

static XPm_PinGroup PmPinGroups[XPM_NODEIDX_STMIC_MAX] = {
	[XPM_NODEIDX_STMIC_PMIO_0] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_1] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_2] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_3] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_4] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_5] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_6] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_7] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_8] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_9] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_10] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0_SS,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_11] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0_SS,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_12] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0_RST_N,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_13] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0_ECC_FAIL_0,
		}),
	},
	[XPM_NODEIDX_STMIC_PMIO_26] = {
		.GroupCount = 1,
		.GroupList = ((u16 []) {
			PIN_GRP_OSPI0_0_ECC_FAIL_1,
		}),
	},
};

/****************************************************************************/
/**
 * @brief  Get requested pin group by node index
 *
 * @param PinIndex     Pin Index.
 *
 * @return Pointer to requested XPm_PinGroup, NULL otherwise
 *
 * @note Requires only node index
 *
 ****************************************************************************/
XPm_PinGroup *XPmPin_GetGroupByIdx(const u32 PinIndex)
{
	return &PmPinGroups[PinIndex];
}
