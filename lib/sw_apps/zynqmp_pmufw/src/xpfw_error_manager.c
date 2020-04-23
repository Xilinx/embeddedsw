/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_error_manager.h"
#include "xpfw_mod_em.h"
#include "xpfw_xpu.h"
#include "xpfw_ipi_manager.h"

u32 ErrorLog[EM_ERROR_LOG_MAX] = {0U};

/*
 * Structure to define error action type and handler if action type
 * is EM_ACTION_CUSTOM for each error.
 */
struct XPfw_Error_t ErrorTable[EM_ERR_ID_MAX] = {
	[EM_ERR_ID_INVALID] =
	{
			.RegMask = MASK32_ALL_LOW,
			.Handler = NULL,
			.Type = (u8)0U,
			.Action = EM_ACTION_NONE,
			.ChngPerm = (u16)EM_ACTION_CHANGE_PERM_NONE
	},
	[EM_ERR_ID_CSU_ROM] = {
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_CSU_ROM_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PMU_PB] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_PB_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PMU_SERVICE] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_SERVICE_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PMU_FW] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_FW_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PMU_UC] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PMU_UC_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_CSU] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_CSU_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PLL_LOCK] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PLL_LOCK_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_NONE,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PL] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_PL_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_TO] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_2_TO_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_2,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_AUX3] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX3_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_AUX2] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX2_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_AUX1] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX1_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_AUX0] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_AUX0_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_CSU_SWDT] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_CSU_SWDT_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_SRST,
			.ChngPerm = (u16)EM_ACTION_CHANGE_PERM_NONE
	},
	[EM_ERR_ID_CLK_MON] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_CLK_MON_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_XMPU] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_XMPU_MASK,
			.Handler = XPfw_XpuIntrHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_CUSTOM,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_PWR_SUPPLY] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_PWR_SUPPLY_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_FPD_SWDT] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_FPD_SWDT_MASK,
			.Handler = SwdtHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_SRST,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_LPD_SWDT] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_LPD_SWDT_MASK,
			.Handler = SwdtHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_SRST,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_RPU_CCF] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU_CCF_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_RPU_LS] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU_LS_MASK,
			.Handler = RpuLsHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_CUSTOM,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_FPD_TEMP] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_FPD_TEMP_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_LPD_TEMP] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_LPD_TEMP_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_RPU1] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU1_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_RPU0] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_RPU0_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_OCM_ECC] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_OCM_ECC_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
	[EM_ERR_ID_DDR_ECC] =
	{
			.RegMask = PMU_GLOBAL_ERROR_STATUS_1_DDR_ECC_MASK,
			.Handler = NullHandler,
			.Type = EM_ERR_TYPE_1,
			.Action = EM_ACTION_PSERR,
			.ChngPerm = (u16)(IPI_PMU_0_IER_APU_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_0_MASK) |
						(u16)(IPI_PMU_0_IER_RPU_1_MASK)
	},
};

s32 XPfw_EmDisable(u8 ErrorId)
{
	s32 Status;
	if (!((ErrorId > EM_ERR_ID_INVALID) && (ErrorId < EM_ERR_ID_MAX))) {
		/* Invalid Error ID */
		Status = XST_FAILURE;
		goto Done;
	}

	switch (ErrorTable[ErrorId].Type) {
	case EM_ERR_TYPE_1:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_DIS_1, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_DIS_1, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_1, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SIG_DIS_1, ErrorTable[ErrorId].RegMask);
		Status = XST_SUCCESS;
		break;
	case EM_ERR_TYPE_2:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPfw_Write32(PMU_GLOBAL_ERROR_POR_DIS_2, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SRST_DIS_2, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_2, ErrorTable[ErrorId].RegMask);
		XPfw_Write32(PMU_GLOBAL_ERROR_SIG_DIS_2, ErrorTable[ErrorId].RegMask);
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
		if(XPfw_EmDisable(ErrorId) != XST_SUCCESS) {
			XPfw_Printf(DEBUG_DETAILED,"Warning: XPfw_EmInit: Failed to "
					"disable Error ID: %d\r\n",ErrorId)
		}
	}

	/* Capture the error registers before clearing them */
	ErrorLog[EM_ERR_TYPE_1] = XPfw_Read32(PMU_GLOBAL_ERROR_STATUS_1);
	ErrorLog[EM_ERR_TYPE_2] = XPfw_Read32(PMU_GLOBAL_ERROR_STATUS_2);

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
	if((EM_ACTION_CUSTOM == ActionId) && (NULL == ErrorHandler)) {
		/* Null handler */
		Status = XST_FAILURE;
		goto Done;
	}

	if((ActionId > EM_ACTION_NONE) && (ActionId < EM_ACTION_MAX)) {
		/* Disable the error actions for Error ID for configuring
		 * the requested error action */
		if (XST_SUCCESS != XPfw_EmDisable(ErrorId)) {
			/* Error action disabling failure */
			Status = XST_FAILURE;
			goto Done;
		}
	}

	switch (ActionId) {

	case EM_ACTION_POR:
		/* Set the error action and enable it */
		ErrorTable[ErrorId].Action = ActionId;
		Status = XPfw_EmEnablePOR(ErrorId);
		break;

	case EM_ACTION_SRST:
		/* Set error action POR for the errorId */
		ErrorTable[ErrorId].Action = ActionId;
		Status = XPfw_EmEnableSRST(ErrorId);
		break;

	case EM_ACTION_CUSTOM:
		/* Set custom handler as error action for the errorId */
		ErrorTable[ErrorId].Action = ActionId;
		ErrorTable[ErrorId].Handler = ErrorHandler;
		Status = XPfw_EmEnableInt(ErrorId);
		break;

	case EM_ACTION_PSERR:
		ErrorTable[ErrorId].Action = ActionId;
		/* Set error action PSERR signal for the errorId */
		Status = XPfw_EmEnablePSError(ErrorId);
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
	u32 ErrRegVal = 0U;
	u8 Index;
	u32 RegAddress = 0U;

	/* Read the error status register based on error type */
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
		break;
	}

	/* Proceed only if ErrorType is Valid */
	if (Status != XST_SUCCESS) {
		goto Done;
	}
	for (Index = 1U; Index < EM_ERR_ID_MAX; Index++) {
		if (ErrorTable[Index].Type == ErrorType) {
			/* check if this error is triggered */
			if ((ErrRegVal & ErrorTable[Index].RegMask) != 0U) {
				/* Logging the error */
				ErrorLog[ErrorType] = ErrorLog[ErrorType] |
						(ErrRegVal & ErrorTable[Index].RegMask);

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

void XPfw_PulseErrorInt(void)
{
	u32 IntMaskReg1;
	u32 IntMaskReg2;

	IntMaskReg1 = ~(XPfw_Read32(PMU_GLOBAL_ERROR_INT_MASK_1));
	IntMaskReg2 = ~(XPfw_Read32(PMU_GLOBAL_ERROR_INT_MASK_2));

	/* Disable PMU interrupts in PMU Global register */
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_1, IntMaskReg1);
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_DIS_2, IntMaskReg2);

	/* Enable PMU interrupts in PMU Global register */
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_1, IntMaskReg1);
	XPfw_Write32(PMU_GLOBAL_ERROR_INT_EN_2, IntMaskReg2);
}
