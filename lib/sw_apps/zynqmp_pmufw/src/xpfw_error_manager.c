/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/
#include "xpfw_error_manager.h"

/* Data Structure to hold Error Info */
struct XPfw_Error_t {
	const u32 RegMask;
	XPfw_ErrorHandler_t Handler;
	const u8 Type;
	u8 Action;
};


static struct XPfw_Error_t ErrorTable[EM_ERR_ID_MAX] = {
	[EM_ERR_ID_INVALID] = {.Type = (u8)0U, .RegMask = MASK32_ALL_LOW, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_CSU_ROM] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_CSU_ROM_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PMU_PB] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_PB_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PMU_SERVICE] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_SERVICE_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PMU_FW] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_FW_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PMU_UC] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_UC_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_CSU] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_CSU_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PLL_LOCK] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PLL_LOCK_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PL] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_PL_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_TO] = { .Type = EM_ERR_TYPE_2, .RegMask = PMU_GLOBAL_ERROR_STATUS_2_TO_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_AUX3] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX3_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_AUX2] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX2_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_AUX1] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX1_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_AUX0] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX0_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_DFT] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_DFT_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_CLK_MON] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_CLK_MON_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_XMPU] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_XMPU_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_PWR_SUPPLY] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_PWR_SUPPLY_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_FPD_SWDT] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_FPD_SWDT_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_LPD_SWDT] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_LPD_SWDT_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_RPU_CCF] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU_CCF_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_RPU_LS] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU_LS_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_FPD_TEMP] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_FPD_TEMP_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_LPD_TEMP] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_LPD_TEMP_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_RPU1] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU1_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_RPU0] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU0_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_OCM_ECC] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_OCM_ECC_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
	[EM_ERR_ID_DDR_ECC] = { .Type = EM_ERR_TYPE_1, .RegMask = PMU_GLOBAL_ERROR_STATUS_1_DDR_ECC_MASK, .Action = EM_ACTION_NONE, .Handler = NULL},
};

static s32 XPfw_EmDisable(u8 ErrorId)
{
	s32 Status;
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		/* Invalid Error ID */
		Status = XST_FAILURE;
		goto Done;
	}

	/* Set the Handler to NULL */
	ErrorTable[ErrorId].Handler = NULL;

	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		/* Disable POR, SRST and Interrupt */
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_DIS_1, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_DIS_1, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		/* Disable POR, SRST and Interrupt */
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_DIS_2, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_DIS_2, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_2, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Error Type */
		Status = XST_FAILURE;
		break;
	}

Done:
	return Status;
}

static s32 XPfw_EmEnablePOR(u8 ErrorId)
{
	s32 Status;
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		Status = XST_FAILURE;
		goto Done;
	}
	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_EN_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_EN_2, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

Done:
	return Status;
}

static s32 XPfw_EmEnableSRST(u8 ErrorId)
{
	s32 Status;
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		Status = XST_FAILURE;
		goto Done;
	}
	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_EN_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_EN_2, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

Done:
	return Status;
}

static s32 XPfw_EmEnableInt(u8 ErrorId)
{
	s32 Status;
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		/* Invalid Error Id */
		Status = XST_FAILURE;
		goto Done;
	}
	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_2, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Err Type */
		Status = XST_FAILURE;
		break;
	}

Done:
	return Status;
}

void XPfw_EmInit(void)
{
	u8 ErrorId;
	/* Disable all the Error Actions */
	for (ErrorId = 1U; ErrorId < EM_ERR_ID_MAX; ErrorId++) {
		XPfw_EmDisable(ErrorId);
	}

	/* Clear the error status registers */
	XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_1, MASK32_ALL_HIGH);
	XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_2, MASK32_ALL_HIGH);

	/* Enable all error signals in HW */
	XPfw_Write32(PMU_GLOBAL_ERROR_EN_1, MASK32_ALL_HIGH);
	XPfw_Write32(PMU_GLOBAL_ERROR_EN_2, MASK32_ALL_HIGH);

}

s32 XPfw_EmSetAction(u8 ErrorId, u8 ActionId,
		XPfw_ErrorHandler_t ErrorHandler)
{
	s32 Status;
	/* Check for Valid Error ID */
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		/* Invalid Error Id */
		Status = XST_FAILURE;
		goto Done;
	}

	switch (ActionId) {

	case EM_ACTION_POR:
		XPfw_EmDisable(ErrorId);
		ErrorTable[ErrorId].Action = ActionId;
		Status = XPfw_EmEnablePOR(ErrorId);
		break;

	case EM_ACTION_SRST:
		XPfw_EmDisable(ErrorId);
		ErrorTable[ErrorId].Action = ActionId;
		Status = XPfw_EmEnableSRST(ErrorId);
		break;

	case EM_ACTION_CUSTOM:
		if (ErrorHandler != NULL) {
			XPfw_EmDisable(ErrorId);
			ErrorTable[ErrorId].Action = ActionId;
			ErrorTable[ErrorId].Handler = ErrorHandler;
			Status = XPfw_EmEnableInt(ErrorId);
		} else {
			/* Null handler */
			Status = XST_FAILURE;
		}
		break;

	default:
		/* Invalid Action Id */
		Status = XST_FAILURE;
		break;

	}

Done:
	return Status;
}

s32 XPfw_EmProcessError(u8 ErrorType)
{
	s32 Status;
	u32 ErrRegVal;
	u8 Index;
	u32 RegAddress;

	switch (ErrorType) {
	case EM_ERR_TYPE_1:
		RegAddress = PMU_GLOBAL_ERROR_STATUS_1;
		ErrRegVal = XPfw_Read32(RegAddress);
		Status = XST_SUCCESS;
		break;

	case EM_ERR_TYPE_2:
		RegAddress = PMU_GLOBAL_ERROR_STATUS_2;
		ErrRegVal = XPfw_Read32(RegAddress);
		Status = XST_SUCCESS;
		break;

	default:
		Status = XST_FAILURE;
	}

	/* Proceed only if ErrorType is Valid */
	if (Status != XST_SUCCESS) {
		goto Done;
	}
	for (Index = 1U; Index < EM_ERR_ID_MAX; Index++) {
		if (ErrorTable[Index].Type == ErrorType) {
			/* check if this error is triggered */
			if ((ErrRegVal & ErrorTable[Index].RegMask) != 0U) {
				if ((ErrorTable[Index].Handler != NULL)
						&& (ErrorTable[Index].Action == EM_ACTION_CUSTOM)) {
					/* Call the Error Handler */
					ErrorTable[Index].Handler(Index);
				}
			}

		}
	}
	/* Ack the Processed Error */
	XPfw_Write32(RegAddress, ErrRegVal);

Done:
	return Status;
}

s32 XPfw_EmEnablePSError(u8 ErrorId)
{
	s32 Status;
	/* If Error ID is not in range, fail*/
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		Status = XST_FAILURE;
		goto Done;
	}

	/* Enable the specified Error to propagate to PSERR pin	*/
	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		XPfw_Write32(PMU_GLOBAL_ERROR_SIG_EN_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		XPfw_Write32(PMU_GLOBAL_ERROR_SIG_EN_2, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

Done:
	return Status;
}
