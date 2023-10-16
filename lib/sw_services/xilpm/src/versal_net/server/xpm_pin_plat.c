/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pin.h"

static XPm_PinNode *PmMioPins[XPM_NODEIDX_STMIC_MAX];
static u16 PmNumPins;

struct PmPinGroup {
	u16 GroupCount;
	u16 *GroupList;
};

static struct PmPinGroup PmPinGroups[XPM_NODEIDX_STMIC_MAX] = {
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
 * @brief  This function initializes the XPm_PinNode data staructure.
 *
 * @param Pin		XPm_PinNode data staructure.
 * @param PinId		PinNode ID.
 * @param BaseAddress	BaseAddress of the pin.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_Init(XPm_PinNode *Pin, u32 PinId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 PinIdx;

	PinIdx = NODEINDEX(PinId);
	if ((PinIdx >= (u32)XPM_NODEIDX_STMIC_MAX) ||
	    (PinIdx == (u32)XPM_NODEIDX_STMIC_MIN)) {
		goto done;
	}

	XPmNode_Init(&Pin->Node, PinId, (u8)XPM_PINSTATE_UNUSED, BaseAddress);

	Pin->Groups = PmPinGroups[PinIdx].GroupList;
	Pin->NumGroups = (u8)(PmPinGroups[PinIdx].GroupCount);
	Pin->PinFunc = NULL;
	Pin->SubsysIdx = (u16)NODEINDEX(INVALID_SUBSYSID);

	if (PinIdx <= PINS_PER_BANK) {
		Pin->Bank = 0;
	} else {
		Pin->Bank =
			(u8)(((PinIdx - PINS_PER_BANK - 1U) / PINS_PER_BANK) &
			BITMASK(PIN_NODE_BANK_BIT_FIELD_SIZE));
	}

	Pin->BiasStatus = (u8)PINCTRL_BIAS_ENABLE;
	Pin->PullCtrl = (u8)PINCTRL_BIAS_PULL_UP;
	Pin->TriState = (u8)PINCTRL_TRI_STATE_ENABLE;

	PmMioPins[PinIdx] = Pin;
	PmNumPins++;
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns instance to XPm_PinNode based on PinId.
 *
 * @param PinId		PinNode ID.
 *
 * @return Instance of XPm_PinNode if successful else NULL.
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetById(u32 PinId)
{
	XPm_PinNode *PinNode = NULL;
	u32 PinIndex = NODEINDEX(PinId);

	if ((u32)XPM_NODECLASS_STMIC != NODECLASS(PinId)) {
		goto done;
	} else if ((u32)XPM_NODESUBCL_PIN != NODESUBCLASS(PinId)) {
		goto done;
	} else if (((u32)XPM_NODETYPE_LPD_MIO != NODETYPE(PinId)) &&
		   ((u32)XPM_NODETYPE_PMC_MIO != NODETYPE(PinId))) {
		goto done;
	} else if (PinIndex >= (u32)XPM_NODEIDX_STMIC_MAX) {
		goto done;
	} else {
		/* Required by MISRA */
	}

	PinNode =  PmMioPins[PinIndex];

done:
	return PinNode;
}

/****************************************************************************/
/**
 * @brief  This function returns total number of pins added.
 *
 * @param NumPins	Number of pins.
 *
 * @return XST_SUCCESS.
 *
 ****************************************************************************/
XStatus XPmPin_GetNumPins(u32 *NumPins)
{
	*NumPins = PmNumPins;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Get requested pin node by node index
 *
 * @param PinIndex     Pin Index.
 *
 * @return Pointer to requested XPm_PinNode, NULL otherwise
 *
 * @note Requires only node index
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetByIndex(const u32 PinIndex)
{
	XPm_PinNode *Pin = NULL;

	Pin = PmMioPins[PinIndex];

	return Pin;
}
