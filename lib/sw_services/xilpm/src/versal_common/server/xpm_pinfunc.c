/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pinfunc.h"

#define FUNC_QUERY_NAME_LEN	(FUNC_NAME_SIZE)

/****************************************************************************/
/**
 * @brief  This function returns total number of functions available.
 *
 * @param NumFuncs	Number of functions.
 *
 * @return XST_SUCCESS.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetNumFuncs(u32 *NumFuncs)
{
	*NumFuncs = (u32)MAX_FUNCTION;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function returns function name based on function ID.
 *
 * @param FuncId	Function ID.
 * @param FuncName	Name of the function.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetFuncName(u32 FuncId, char *FuncName)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinFunc *PinFunc = NULL;
	const u32 CopySize = FUNC_QUERY_NAME_LEN;

	Status = Xil_SMemSet(FuncName, FUNC_QUERY_NAME_LEN, 0, FUNC_QUERY_NAME_LEN);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PinFunc = XPmPinFunc_GetById(FuncId);
	if (NULL == PinFunc) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = Xil_SMemCpy(FuncName, CopySize, &PinFunc->Name[0], CopySize, CopySize);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns number of groups in function based on
 *	   function ID.
 *
 * @param FuncId	Function ID.
 * @param NumGroups	Number of groups.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetNumFuncGroups(u32 FuncId, u32 *NumGroups)
{
	XStatus Status = XST_FAILURE;
	const XPm_PinFunc *PinFunc = NULL;

	PinFunc = XPmPinFunc_GetById(FuncId);
	if (NULL != PinFunc) {
		*NumGroups = PinFunc->NumGroups;
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns groups present in function based on
 *	   function ID. Index 0 returns the first 6 group IDs, index 6
 *	   returns the next 6 group IDs, and so forth.
 *
 * @param FuncId	Function ID.
 * @param Index		Index of next function groups
 * @param Groups	Function groups.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPinFunc_GetFuncGroups(u32 FuncId, u32 Index, u16 *Groups)
{
	XStatus Status = XST_FAILURE;
	u32 i;
	u32 num_read;
	const XPm_PinFunc *PinFunc = NULL;
	u32 Size = MAX_GROUPS_PER_RES * sizeof(u16);

	Status = Xil_SMemSet(Groups, Size, (s32)END_OF_GRP, Size);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PinFunc = XPmPinFunc_GetById(FuncId);
	if ((NULL == PinFunc) || (Index > PinFunc->NumGroups)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Read up to 6 group IDs from Index */
	if ((PinFunc->NumGroups - Index) > MAX_GROUPS_PER_RES) {
		num_read = MAX_GROUPS_PER_RES;
	} else {
		num_read = PinFunc->NumGroups - Index;
	}

	for (i = 0; i < num_read; i++) {
		Groups[i] = PinFunc->Groups[i + Index];
	}

	Status = XST_SUCCESS;

done:
	return Status;
}
