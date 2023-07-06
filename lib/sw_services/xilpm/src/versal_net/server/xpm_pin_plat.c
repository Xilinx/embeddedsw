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

static XStatus SavePinNode(XPm_PinNode* ThisData) {
	XStatus Status = XST_FAILURE;
	u32* SavedData =  NULL;

	BEGIN_SAVE_STRUCT(SavedData, XPmNode_SaveNode, ((XPm_Node*)ThisData));
	SaveStruct(Status, done, ThisData->SubsysIdx);
	SaveStruct(Status, done, (ThisData->Bank << 24) | (ThisData->BiasStatus << 16) | (ThisData->PullCtrl << 8) | (ThisData->TriState) );
	if (ThisData->PinFunc != NULL){
		SaveStruct(Status, done, ThisData->PinFunc->Id);
	}
	END_SAVE_STRUCT(SavedData);

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus RestorePinNode(u32* SavedData, XPm_PinNode *ThisData){
	u32* DataAddr = NULL;
	XStatus Status = XPmNode_RestoreNode(SavedData, &(ThisData->Node), &DataAddr);
	if (XST_SUCCESS != Status)
	{
		goto done;
	}
	RestoreStruct(DataAddr,  ThisData->SubsysIdx);
	u32 tmp = 0;
	RestoreStruct(DataAddr, tmp);
	ThisData->Bank = ((tmp >> 24) & BITMASK(PIN_NODE_BANK_BIT_FIELD_SIZE));
	ThisData->BiasStatus = ((tmp >> 16) & BITMASK(PIN_NODE_BIASSTATUS_BIT_FIELD_SIZE));
	ThisData->PullCtrl = ((tmp >> 8) & BITMASK(PIN_NODE_PULLCTRL_BIT_FIELD_SIZE));
	ThisData->TriState = ((tmp) & BITMASK(PIN_NODE_TRISTATE_BIT_FIELD_SIZE));
	if (SAVED_DATA_GET_SIZE(SavedData) > 2){
			u8 FuncId = 0;
			RestoreStruct(DataAddr, FuncId);
			XPm_PinFunc* PinFunc = XPmPinFunc_GetById(FuncId);
			if (NULL != PinFunc)
			{
				Status = XPmPin_SetPinFunction(ThisData->Node.Id, FuncId);
				if (XST_SUCCESS != Status)
				{
					if (XST_INVALID_PARAM != Status){
						goto done;
					} else {
						/* TODO: Set pin function still error during this stage of PLM update
						 * Ignore the error for now.
						 */
					}
				}
			} else {
				/* Ignore when PinFunc is NULL */
				/* noop */
			}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

static XStatus DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op){
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE  == Op){
		Status = SavePinNode((XPm_PinNode*)ThisData);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = RestorePinNode(SavedData, (XPm_PinNode*)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}

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

	XPmNode_Init(&Pin->Node, PinId, (u8)XPM_PINSTATE_UNUSED, BaseAddress, DoSaveRestore);

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
